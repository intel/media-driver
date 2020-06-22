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
//! \file     mhw_mi_hwcmd_g11_X.h
//! \brief    Auto-generated constructors for MHW and states.
//! \details  This file may not be included outside of g11_X as other components
//!           should use MHW interface to interact with MHW commands and states.
//!
#ifndef __MHW_MI_HWCMD_G11_X_H__
#define __MHW_MI_HWCMD_G11_X_H__

#pragma once
#pragma pack(1)

#include <cstdint>
#include <cstddef>

class mhw_mi_g11_X
{
public:
    // Internal Macros
    #define __CODEGEN_MAX(_a, _b) (((_a) > (_b)) ? (_a) : (_b))
    #define __CODEGEN_BITFIELD(l, h) (h) - (l) + 1
    #define __CODEGEN_OP_LENGTH_BIAS 2
    #define __CODEGEN_OP_LENGTH(x) (uint32_t)((__CODEGEN_MAX(x, __CODEGEN_OP_LENGTH_BIAS)) - __CODEGEN_OP_LENGTH_BIAS)

    static uint32_t GetOpLength(uint32_t uiLength) { return __CODEGEN_OP_LENGTH(uiLength); }

    //!
    //! \brief MI_BATCH_BUFFER_END
    //! \details
    //!     The MI_BATCH_BUFFER_END command is used to terminate the execution of
    //!     commands stored in a batch buffer initiated using a
    //!     MI_BATCH_BUFFER_START command.
    //!     
    struct MI_BATCH_BUFFER_END_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 EndContext                                       : __CODEGEN_BITFIELD( 0,  0)    ; //!< End Context
                uint32_t                 Reserved1                                        : __CODEGEN_BITFIELD( 1, 22)    ; //!< Reserved
                uint32_t                 MiCommandOpcode                                  : __CODEGEN_BITFIELD(23, 28)    ; //!< MI_COMMAND_OPCODE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;

        //! \name Local enumerations

        enum MI_COMMAND_OPCODE
        {
            MI_COMMAND_OPCODE_MIBATCHBUFFEREND                               = 10, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_MICOMMAND                                           = 0, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MI_BATCH_BUFFER_END_CMD();

        static const size_t dwSize = 1;
        static const size_t byteSize = 4;
    };

    //!
    //! \brief MI_NOOP
    //! \details
    //!     The MI_NOOP command basically performs a "no operation" in the command
    //!     stream and is typically used to pad the command stream (e.g., in order
    //!     to pad out a batch buffer to a QWord boundary). However, there is one
    //!     minor (optional) function this command can perform - a 22-bit value can
    //!     be loaded into the MI NOPID register. This provides a general-purpose
    //!     command stream tagging ("breadcrumb") mechanism (e.g., to provide
    //!     sequencing information for a subsequent breakpoint interrupt).
    //!     
    struct MI_NOOP_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 IdentificationNumber                             : __CODEGEN_BITFIELD( 0, 21)    ; //!< Identification Number
                uint32_t                 IdentificationNumberRegisterWriteEnable          : __CODEGEN_BITFIELD(22, 22)    ; //!< IDENTIFICATION_NUMBER_REGISTER_WRITE_ENABLE
                uint32_t                 MiCommandOpcode                                  : __CODEGEN_BITFIELD(23, 28)    ; //!< MI_COMMAND_OPCODE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;

        //! \name Local enumerations

        //! \brief IDENTIFICATION_NUMBER_REGISTER_WRITE_ENABLE
        //! \details
        //!     This field enables the value in the Identification Number field to be
        //!     written into the MI NOPID register. If disabled, that register is
        //!     unmodified - making this command an effective "no operation" function.
        enum IDENTIFICATION_NUMBER_REGISTER_WRITE_ENABLE
        {
            IDENTIFICATION_NUMBER_REGISTER_WRITE_ENABLE_WRITETHENOPIDREGISTER = 1, //!< No additional details
        };

        enum MI_COMMAND_OPCODE
        {
            MI_COMMAND_OPCODE_MINOOP                                         = 0, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_MICOMMAND                                           = 0, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MI_NOOP_CMD();

        static const size_t dwSize = 1;
        static const size_t byteSize = 4;
    };

    //!
    //! \brief MI_ARB_CHECK
    //! \details
    //!     The command allows software to add preemption points in the ring buffer.
    //!      The command streamer will preempt in the case arbitration is enabled,
    //!     there is a pending execution list and this command is currently being
    //!     parsed.
    //!     
    //!     This instruction cannot be placed in a batch buffer.
    //!     
    struct MI_ARB_CHECK_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 Reserved0                                        : __CODEGEN_BITFIELD( 0, 22)    ; //!< Reserved
                uint32_t                 MiInstructionOpcode                              : __CODEGEN_BITFIELD(23, 28)    ; //!< MI_INSTRUCTION_OPCODE
                uint32_t                 MiInstructionType                                : __CODEGEN_BITFIELD(29, 31)    ; //!< MI_INSTRUCTION_TYPE
            };
            uint32_t                     Value;
        } DW0;

        //! \name Local enumerations

        enum MI_INSTRUCTION_OPCODE
        {
            MI_INSTRUCTION_OPCODE_MIARBCHECK                                 = 5, //!< No additional details
        };

        enum MI_INSTRUCTION_TYPE
        {
            MI_INSTRUCTION_TYPE_MIINSTRUCTION                                = 0, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MI_ARB_CHECK_CMD();

        static const size_t dwSize = 1;
        static const size_t byteSize = 4;
    };

    //!
    //! \brief MI_LOAD_REGISTER_IMM
    //! \details
    //!     The MI_LOAD_REGISTER_IMM command requests a write of up to a DWord
    //!     constant supplied in the command to the specified Register Offset (i.e.,
    //!     offset into Memory-Mapped Register Range).
    //!     Any offset that is to a destination outside of the GT core will allow
    //!     the parser to continue once the cycle is at the GT boundry and not
    //!     destination.  Any other address will ensure the destination is updated
    //!     prior to parsing the next command
    //!     
    //!     When executed from a non-privileged batch buffer, MMIO writes are only
    //!     allowed to the registers listed in User Mode Non-Privileged Registers
    //!     for the corresponding engine, any writes targeting the register not
    //!     listed in the User Mode Non-Privileged Register will convert this
    //!     command to a NOOP.
    //!     
    //!     The following addresses should NOT be used for LRIs:
    //!     
    //!     0x8800 - 0x88FF
    //!     
    //!     >= 0xC0000
    //!     
    //!     
    //!     
    //!     Limited LRI cycles to the Display Engine (0x40000-0xBFFFF) are allowed,
    //!     but must be spaced to allow only one pending at a time. This can be done
    //!     by issuing an SRM to the same address immediately after each LRI.
    //!     
    struct MI_LOAD_REGISTER_IMM_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< DWORD_LENGTH
                uint32_t                 ByteWriteDisables                                : __CODEGEN_BITFIELD( 8, 11)    ; //!< Byte Write Disables
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 18)    ; //!< Reserved
                uint32_t                 AddCsMmioStartOffset                             : __CODEGEN_BITFIELD(19, 19)    ; //!< ADD_CS_MMIO_START_OFFSET
                uint32_t                 Reserved20                                       : __CODEGEN_BITFIELD(20, 22)    ; //!< Reserved
                uint32_t                 MiCommandOpcode                                  : __CODEGEN_BITFIELD(23, 28)    ; //!< MI_COMMAND_OPCODE
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
                uint32_t                 RegisterOffset                                   : __CODEGEN_BITFIELD( 2, 22)    ; //!< Register Offset
                uint32_t                 Reserved55                                       : __CODEGEN_BITFIELD(23, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 DataDword                                                                        ; //!< Data DWord
            };
            uint32_t                     Value;
        } DW2;

        //! \name Local enumerations

        //! \brief ADD_CS_MMIO_START_OFFSET
        //! \details
        //!     <p>This bit controls the functionality of the "Register Address" field
        //!     in the command.</p>
        enum ADD_CS_MMIO_START_OFFSET
        {
            ADD_CS_MMIO_START_OFFSET_UNNAMED0                                = 0, //!< "Register Address" field in the command is absolute and not an offset from the executing command streamer MMIO start offset..
            ADD_CS_MMIO_START_OFFSET_UNNAMED1                                = 1, //!< "Register Address" field in the command is treated as an offset from the executing Command Streamer’s MMIO start offset. Bits [22:2] of the "Register Address" are considered as dword offset to be added to the MMIO start offset of the corresponding command streamer. However, during context restore bits [11:2] of the "Register Address" are considered as dword offset to be added to the MMIO start offset of the corresponding command streamer. Command executed from Ring Buffer or Batch BufferExample: MI_LOAD_REGISTER_IMMEDIATE, ADD_CS_MMIO_START_OFFSET: true, Data:0xABCD, Register Address: 0x00_2000The above command when executed on RenderCS will result in a write to MMIO offset 0x00_4000 (0x00_2000 + 0x00_2000) instead to 0x00_2000. Note that RenderCS MMIO start offset is 0x2000. Table below shows the result of this command executed by various command streamers.<div>
        };

        enum MI_COMMAND_OPCODE
        {
            MI_COMMAND_OPCODE_MILOADREGISTERIMM                              = 34, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_MICOMMAND                                           = 0, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MI_LOAD_REGISTER_IMM_CMD();

        static const size_t dwSize = 3;
        static const size_t byteSize = 12;
    };

    //!
    //! \brief MI_LOAD_REGISTER_MEM
    //! \details
    //!     The MI_LOAD_REGISTER_MEM command requests from a memory location and
    //!     stores that DWord to a register.
    //!     
    //!     The command temporarily halts commands that will cause cycles down the
    //!     3D pipeline.
    //!     
    //!     The following addresses should NOT be used for MMIO writes:
    //!     
    //!     0x8800 - 0x88FF
    //!     
    //!     >= 0xC0000
    //!     
    //!     
    //!     
    //!     Limited MMIO writes cycles to the Display Engine 0x40000-0xBFFFF) are
    //!     allowed, but must be spaced to allow only one pending at a time. This
    //!     can be done by issuing an SRM to the same address immediately after each
    //!     MMIO write.
    //!     
    //!     This command should not be used within a non-privilege batch buffer to
    //!     access global virtual space, doing so will be treated as privilege
    //!     access violation. Refer "User Mode Privilege Command" in
    //!     MI_BATCH_BUFFER_START command section to know HW behavior on
    //!     encountering privilege access violation.
    //!     
    //!     This command is not allowed to update the privilege register range when
    //!     executed from a non-privilege batch buffer.
    //!     
    struct MI_LOAD_REGISTER_MEM_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved8                                        : __CODEGEN_BITFIELD( 8, 18)    ; //!< Reserved
                uint32_t                 AddCsMmioStartOffset                             : __CODEGEN_BITFIELD(19, 19)    ; //!< ADD_CS_MMIO_START_OFFSET
                uint32_t                 Reserved20                                       : __CODEGEN_BITFIELD(20, 20)    ; //!< Reserved
                uint32_t                 AsyncModeEnable                                  : __CODEGEN_BITFIELD(21, 21)    ; //!< Async Mode Enable
                uint32_t                 UseGlobalGtt                                     : __CODEGEN_BITFIELD(22, 22)    ; //!< Use Global GTT
                uint32_t                 MiCommandOpcode                                  : __CODEGEN_BITFIELD(23, 28)    ; //!< MI_COMMAND_OPCODE
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
                uint32_t                 RegisterAddress                                  : __CODEGEN_BITFIELD( 2, 22)    ; //!< Register Address
                uint32_t                 Reserved55                                       : __CODEGEN_BITFIELD(23, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2..3
            struct
            {
                uint64_t                 Reserved64                                       : __CODEGEN_BITFIELD( 0,  1)    ; //!< Reserved
                uint64_t                 MemoryAddress                                    : __CODEGEN_BITFIELD( 2, 63)    ; //!< Memory Address
            };
            uint32_t                     Value[2];
        } DW2_3;

        //! \name Local enumerations

        //! \brief ADD_CS_MMIO_START_OFFSET
        //! \details
        //!     This bit controls the functionality of the "Register Address" field in
        //!     the command.
        enum ADD_CS_MMIO_START_OFFSET
        {
            ADD_CS_MMIO_START_OFFSET_UNNAMED0                                = 0, //!< "Register Address" field in the command is absolute and not an offset from the executing command streamer MMIO start offset.
            ADD_CS_MMIO_START_OFFSET_UNNAMED1                                = 1, //!< "Register Address" field in the command is treated as an offset from the executing Command Streamer’s MMIO start offset.Bits [22:2] of the "Register Address" are considered as dword offset to be added to the MMIO start offset of the corresponding command streamer.Example: MI_LOAD_REGISTER_MEM, ADD_CS_MMIO_START_OFFSET: true, Memory Address:0xABCD, Register Address: 0x1C_0030The above command when executed on RenderCS will result in a write to MMIO offset 0x1C_2030 (0x00_2000 + 0x1C_0030) instead to 0x1C_0030. Note that RenderCS MMIO start offset is 0x2000.
        };

        enum MI_COMMAND_OPCODE
        {
            MI_COMMAND_OPCODE_MILOADREGISTERMEM                              = 41, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_MICOMMAND                                           = 0, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MI_LOAD_REGISTER_MEM_CMD();

        static const size_t dwSize = 4;
        static const size_t byteSize = 16;
    };

    //!
    //! \brief MI_LOAD_REGISTER_REG
    //! \details
    //!     The MI_LOAD_REGISTER_REG command reads from a source register location
    //!     and writes that value to a destination register location.
    //!     Any offset that is to a destination outside of the GT core will allow
    //!     the parser to continue once the cycle is at the GT boundry and not
    //!     destination.  Any other address will ensure the destination is updated
    //!     prior to parsing the next command
    //!     
    //!     The command temporarily halts commands that will cause cycles down the
    //!     3D pipeline.
    //!     
    //!     Destination register with mask implemented (Ex: some
    //!     registers have bits [31:16] as mask bits and bits[15:0] as data) will
    //!     not get updated unless the  value read from source register has the bits
    //!     corresponding to the mask bits set. Note that any mask implemented
    //!     register when read returns "0" for the bits corresponding to mask
    //!     location. When the source and destination are mask implemented
    //!     registers, destination register will not get updated with the source
    //!     register contents.
    //!     
    //!     This command is not allowed to update the privilege register range when
    //!     executed from a non-privilege batch buffer.
    //!     
    struct MI_LOAD_REGISTER_REG_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved8                                        : __CODEGEN_BITFIELD( 8, 17)    ; //!< Reserved
                uint32_t                 AddCsMmioStartOffsetSource                       : __CODEGEN_BITFIELD(18, 18)    ; //!< ADD_CS_MMIO_START_OFFSET_SOURCE
                uint32_t                 AddCsMmioStartOffsetDestination                  : __CODEGEN_BITFIELD(19, 19)    ; //!< ADD_CS_MMIO_START_OFFSET_DESTINATION
                uint32_t                 Reserved20                                       : __CODEGEN_BITFIELD(20, 22)    ; //!< Reserved
                uint32_t                 MiCommandOpcode                                  : __CODEGEN_BITFIELD(23, 28)    ; //!< MI_COMMAND_OPCODE
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
                uint32_t                 SourceRegisterAddress                            : __CODEGEN_BITFIELD( 2, 22)    ; //!< Source Register Address
                uint32_t                 Reserved55                                       : __CODEGEN_BITFIELD(23, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 Reserved64                                       : __CODEGEN_BITFIELD( 0,  1)    ; //!< Reserved
                uint32_t                 DestinationRegisterAddress                       : __CODEGEN_BITFIELD( 2, 22)    ; //!< Destination Register Address
                uint32_t                 Reserved87                                       : __CODEGEN_BITFIELD(23, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;

        //! \name Local enumerations

        //! \brief ADD_CS_MMIO_START_OFFSET_SOURCE
        //! \details
        //!     <p>This bit controls the functionality of the "Register Address Source"
        //!     field in the command.</p>
        enum ADD_CS_MMIO_START_OFFSET_SOURCE
        {
            ADD_CS_MMIO_START_OFFSET_SOURCE_UNNAMED0                         = 0, //!< "Register Address" field in the command is absolute and not an offset from the executing command streamer MMIO start offset.
            ADD_CS_MMIO_START_OFFSET_SOURCE_UNNAMED1                         = 1, //!< "Source Register Address" field in the command is treated as an offset from the executing Command Streamer’s MMIO start offset. Bits [22:2] of the "Source Register Address" are considered as dword offset to be added to the MMIO start offset of the corresponding command streamer.Example: MI_LOAD_REGISTER_REGISTER_REG, DEST_ADD_CS_MMIO_START_OFFSET: false, SRC_ADD_CS_MMIO_START_OFFSET:true, Source Register Address:0x1C_0130, Destination Register Address: 0x1C_0030The above command when executed on RenderCS will result in a MMIO read from 0x1C _2130 instead of read from 0x1C_0130 and write to MMIO offset 0x1C_0030. Note that RenderCS MMIO start offset is 0x2000.
        };

        //! \brief ADD_CS_MMIO_START_OFFSET_DESTINATION
        //! \details
        //!     <p>This bit controls the functionality of the "Register Address
        //!     Destination" field in the command.</p>
        enum ADD_CS_MMIO_START_OFFSET_DESTINATION
        {
            ADD_CS_MMIO_START_OFFSET_DESTINATION_UNNAMED0                    = 0, //!< "Destination Register Address" field in the command is absolute and not an offset from the executing command streamer MMIO start offset.
            ADD_CS_MMIO_START_OFFSET_DESTINATION_UNNAMED1                    = 1, //!< "Destination Register Address" field in the command is treated as an offset from the executing Command Streamer’s MMIO start offset. Bits [22:2] of the "Destination Register Address" are considered as dword offset to be added to the MMIO start offset of the corresponding command streamer.Example: MI_LOAD_REGISTER_REGISTER_REG, DEST_ADD_CS_MMIO_START_OFFSET: true, SRC_ADD_CS_MMIO_START_OFFSET:true, Source Register Address:0x1C_0130, Destination Register Address: 0x1C_0030The above command when executed on RenderCS will result in a MMIO read from 0x1C_2130 (0x00_2000 + 0x1C_0130) and write to MMIO offset 0x1C_2030 (0x00_2000 + 0x1C_0030) instead of read from 0x1C_0130 and write to 0x1C_0030. Note that RenderCS MMIO start offset is 0x2000.
        };

        enum MI_COMMAND_OPCODE
        {
            MI_COMMAND_OPCODE_MILOADREGISTERREG                              = 42, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_MICOMMAND                                           = 0, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MI_LOAD_REGISTER_REG_CMD();

        static const size_t dwSize = 3;
        static const size_t byteSize = 12;
    };

    //!
    //! \brief MI_STORE_REGISTER_MEM
    //! \details
    //!     The MI_STORE_REGISTER_MEM command requests a register read from a
    //!     specified memory mapped register location in the device and store of
    //!     that DWord to memory. The register address is specified along with the
    //!     command to perform the read.
    //!     
    //!     
    //!     The command temporarily halts command execution.
    //!     
    //!     The memory address for the write is snooped on the host bus.
    //!     
    //!     This command should not be used from within a "non-privilege" batch
    //!     buffer to access global virtual space. doing so will be treated as
    //!     privilege access violation. Refer "User Mode Privilege Command" in
    //!     MI_BATCH_BUFFER_START command section to know HW behavior on
    //!     encountering privilege access violation. This command can be used within
    //!     ring buffers and/or "privilege" batch buffers to access global virtual
    //!     space.
    //!     
    //!     This command will cause undefined data to be written to memory if given
    //!     register addresses for the PGTBL_CTL_0 or FENCE registers.
    //!     
    //!     
    //!     
    struct MI_STORE_REGISTER_MEM_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved8                                        : __CODEGEN_BITFIELD( 8, 18)    ; //!< Reserved
                uint32_t                 AddCsMmioStartOffset                             : __CODEGEN_BITFIELD(19, 19)    ; //!< ADD_CS_MMIO_START_OFFSET
                uint32_t                 Reserved20                                       : __CODEGEN_BITFIELD(20, 21)    ; //!< Reserved
                uint32_t                 UseGlobalGtt                                     : __CODEGEN_BITFIELD(22, 22)    ; //!< Use Global GTT
                uint32_t                 MiCommandOpcode                                  : __CODEGEN_BITFIELD(23, 28)    ; //!< MI_COMMAND_OPCODE
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
                uint32_t                 RegisterAddress                                  : __CODEGEN_BITFIELD( 2, 22)    ; //!< Register Address
                uint32_t                 Reserved55                                       : __CODEGEN_BITFIELD(23, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2..3
            struct
            {
                uint64_t                 Reserved64                                       : __CODEGEN_BITFIELD( 0,  1)    ; //!< Reserved
                uint64_t                 MemoryAddress                                    : __CODEGEN_BITFIELD( 2, 63)    ; //!< Memory Address
            };
            uint32_t                     Value[2];
        } DW2_3;

        //! \name Local enumerations

        //! \brief ADD_CS_MMIO_START_OFFSET
        //! \details
        //!     <p>This bit controls the functionality of the "Register Address" field
        //!     in the command.</p>
        enum ADD_CS_MMIO_START_OFFSET
        {
            ADD_CS_MMIO_START_OFFSET_UNNAMED0                                = 0, //!< "Register Address" field in the command is absolute and not an offset from the executing command streamer MMIO start offset.
            ADD_CS_MMIO_START_OFFSET_UNNAMED1                                = 1, //!< "Register Address" field in the command is treated as an offset from the executing Command Streamer’s MMIO start offset. Bits [22:2] of the "Register Address" are considered as dword offset to be added to the MMIO start offset of the corresponding command streamer.Example: MI_STORE_REGISTER_MEM, ADD_CS_MMIO_START_OFFSET: true, Memory Address:0xABCD, Register Address: 0x1C_0030The above command when executed on RenderCS will result in updating the memory address with the content of the MMIO offset 0x1C_2030 (0x00_2000 + 0x1C_0030) instead to 0x1C_0030. Note that RenderCS MMIO start offset is 0x2000.
        };

        enum MI_COMMAND_OPCODE
        {
            MI_COMMAND_OPCODE_MISTOREREGISTERMEM                             = 36, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_MICOMMAND                                           = 0, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MI_STORE_REGISTER_MEM_CMD();

        static const size_t dwSize = 4;
        static const size_t byteSize = 16;
    };

    //!
    //! \brief MI_BATCH_BUFFER_START
    //! \details
    //!     The MI_BATCH_BUFFER_START command is used to initiate the execution of
    //!     commands stored in a batch buffer. For restrictions on the location of
    //!     batch buffers, see Batch Buffers in the Device Programming Interface
    //!     chapter of MI Functions. The batch buffer can be specified as privileged
    //!     or non-privileged, determining the operations considered valid when
    //!     initiated from within the buffer and any attached (chained) batch
    //!     buffers. See Batch Buffer Protection in the Device Programming Interface
    //!     chapter of MI Functions. 
    //!     
    //!      A batch buffer initiated with this command must end either with a
    //!     MI_BATCH_BUFFER_END command or by chaining to another batch buffer with
    //!     an MI_BATCH_BUFFER_START command.
    //!      It is essential that the address location beyond the current page be
    //!     populated inside the GTT. HW performs over-fetch of the command
    //!     addresses and any over-fetch requires a valid TLB entry. A single extra
    //!     page beyond the batch buffer is sufficient.
    //!     
    //!     
    //!     
    struct MI_BATCH_BUFFER_START_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< DWORD_LENGTH
                uint32_t                 AddressSpaceIndicator                            : __CODEGEN_BITFIELD( 8,  8)    ; //!< ADDRESS_SPACE_INDICATOR
                uint32_t                 Reserved9                                        : __CODEGEN_BITFIELD( 9, 21)    ; //!< Reserved
                uint32_t                 SecondLevelBatchBuffer                           : __CODEGEN_BITFIELD(22, 22)    ; //!< SECOND_LEVEL_BATCH_BUFFER
                uint32_t                 MiCommandOpcode                                  : __CODEGEN_BITFIELD(23, 28)    ; //!< MI_COMMAND_OPCODE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1..2
            struct
            {
                uint64_t                 Reserved32                                       : __CODEGEN_BITFIELD( 0,  1)    ; //!< Reserved
                uint64_t                 BatchBufferStartAddress                          : __CODEGEN_BITFIELD( 2, 63)    ; //!< Batch Buffer Start Address
            };
            uint32_t                     Value[2];
        } DW1_2;

        //! \name Local enumerations

        //! \brief ADDRESS_SPACE_INDICATOR
        //! \details
        //!     <p>Batch buffers accessed via PPGTT are considered as non-privileged.
        //!     Certain operations (e.g., MI_STORE_DATA_IMM commands to GGTT memory) are
        //!     prohibited within non-privileged buffers. More details mentioned in User
        //!     Mode Privileged command section. When MI_BATCH_BUFFER_START command is
        //!     executed from within a batch buffer (i.e., is a "chained" or "second
        //!     level" batch buffer command), the current active batch buffer's "Address
        //!     Space Indicator" and this field determine the "Address Space Indicator"
        //!     of the next buffer in the chain.</p>
        //!     <ul>
        //!     <li>Chained or Second level batch buffer can be in GGTT or PPGTT if the
        //!     parent batch buffer is in GGTT.</li>
        //!     <li>Chained or Second level batch buffer can only be in PPGTT if the
        //!     parent batch buffer is in PPGTT. This is enforced by Hardware.</li>
        //!     </ul>
        enum ADDRESS_SPACE_INDICATOR
        {
            ADDRESS_SPACE_INDICATOR_GGTT                                     = 0, //!< This batch buffer is located in GGTT memory and is privileged.
            ADDRESS_SPACE_INDICATOR_PPGTT                                    = 1, //!< This batch buffer is located in PPGTT memory and is Non-Privileged.
        };

        //! \brief SECOND_LEVEL_BATCH_BUFFER
        //! \details
        //!     <p>The command streamer contains three storage elements; one for the
        //!     ring head address, one for the batch head address, and one for the
        //!     second level batch head address. When performing batch buffer chaining,
        //!     hardware simply updates the head pointer of the first level batch
        //!     address storage. There is no stack in hardware. When this bit is set,
        //!     hardware uses the 2nd level batch head address storage element. Upon
        //!     MI_BATCH_BUFFER_END, it will automatically return to the first level
        //!     batch buffer address. This allows hardware to mimic a simple 3-level
        //!     stack.</p>
        enum SECOND_LEVEL_BATCH_BUFFER
        {
            SECOND_LEVEL_BATCH_BUFFER_FIRSTLEVELBATCH                        = 0, //!< Place the batch buffer address in the 1st (traditional) level batch address storage element.
            SECOND_LEVEL_BATCH_BUFFER_SECONDLEVELBATCH                       = 1, //!< Place the batch buffer address in the second-level batch address storage element.
        };

        enum MI_COMMAND_OPCODE
        {
            MI_COMMAND_OPCODE_MIBATCHBUFFERSTART                             = 49, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_MICOMMAND                                           = 0, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MI_BATCH_BUFFER_START_CMD();

        static const size_t dwSize = 3;
        static const size_t byteSize = 12;
    };

    //!
    //! \brief MI_SET_PREDICATE
    //! \details
    //!     This command sets the Predication Check for the subsequent commands in
    //!     the command buffer except for MI_SET_PREDICATE itself.   Render Command
    //!     Streamer NOOPs the following commands based on the PREDICATE_ENABLE from
    //!     MI_SET_PREDICATE, MI_SET_PREDICATE_RESULT and MI_SET_PREDICATE_RESULT_2
    //!     status.  Resource Streamer doesn't take any action of parsing
    //!     MI_SET_PREDICATE, this command is similar to any other command which is
    //!     not meant for resource streamer.
    //!     
    //!     Executing MI_SET_PREDICATE command sets PREDICATE_ENABLE bits in MI_MODE
    //!     register, MI_MODE register gets render context save restored.
    //!     
    //!      MI_SET_PREDICATE predication scope must be confined within a Batch
    //!     Buffer to set of commands.
    //!       MI_SET_PREDICATE with Predicate Enable Must always have a
    //!     corresponding MI_SET_PREDICATE with Predicate Disable within the same
    //!     Batch Buffer.
    //!      MI_ARB_CHK command must be programmed outside the Predication Scope of
    //!     MI_SET_PREDICATE.
    //!      MI_SET_PREDICATE Predication Scope must not involve any RC6 triggering
    //!     events.
    //!      
    //!     
    //!     Only the following command(s) can be programmed between the
    //!     MI_SET_PREDICATE command enabled for predication: 3DSTATE_URB_VS
    //!     3DSTATE_URB_HS 3DSTATE_URB_DS 3DSTATE_URB_GS
    //!     3DSTATE_PUSH_CONSTANT_ALLOC_VS 3DSTATE_PUSH_CONSTANT_ALLOC_HS
    //!     3DSTATE_PUSH_CONSTANT_ALLOC_DS 3DSTATE_PUSH_CONSTANT_ALLOC_GS
    //!     3DSTATE_PUSH_CONSTANT_ALLOC_PS MI_LOAD_REGISTER_IMM MEDIA_VFE_STATE
    //!     MEDIA_OBJECT MEDIA_OBJJECT_WALKER MEDIA_INTERFACE_DESCRIPTOR_LOAD
    //!     3DSTATE_WM_HZ_OP MI_STORE_DATA_IMM
    //!     
    struct MI_SET_PREDICATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 PredicateEnable                                  : __CODEGEN_BITFIELD( 0,  3)    ; //!< PREDICATE_ENABLE
                uint32_t                 Reserved4                                        : __CODEGEN_BITFIELD( 4, 22)    ; //!< Reserved
                uint32_t                 MiCommandOpcode                                  : __CODEGEN_BITFIELD(23, 28)    ; //!< MI_COMMAND_OPCODE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;

        //! \name Local enumerations

        //! \brief PREDICATE_ENABLE
        //! \details
        //!     This field sets the predication logic in render command streamer when
        //!     parsed. Predicate Disable is the default mode of operation.
        enum PREDICATE_ENABLE
        {
            PREDICATE_ENABLE_NOOPNEVER                                       = 0, //!< Predication is Disabled and RCS will process commands as usual.
            PREDICATE_ENABLE_NOOPONRESULT2CLEAR                              = 1, //!< Following Commands will be NOOPED by RCS only if the MI_PREDICATE_RESULT_2 is clear.
            PREDICATE_ENABLE_NOOPONRESULT2SET                                = 2, //!< Following Commands will be NOOPED by RCS only if the MI_PREDICATE_RESULT_2 is set.
            PREDICATE_ENABLE_NOOPONRESULTCLEAR                               = 3, //!< Following Commands will be NOOPED by RCS only if the MI_PREDICATE_RESULT is clear.
            PREDICATE_ENABLE_NOOPONRESULTSET                                 = 4, //!< Following Commands will be NOOPED by RCS only if the MI_PREDICATE_RESULT is set.
            PREDICATE_ENABLE_NOOPINRENDERCS                                  = 11, //!< When RenderCS parses MI_SET_PREDICATE command with "Predicate Enable" set to "NOOP in RenderCS", RenderCS NOOP’s all the subsequent commands parsed unconditionally until the predication is disabled/modified using MI_SET_PREDICATE command. Other command streamers (non RenderCS) on parsing MI_SET_PREDICATE command with "Predicate Enable" set to "NOOP in RenderCS" don’t take any action and is equivalent to parsing MI_NOOP command.
            PREDICATE_ENABLE_NOOPINPOSITIONCS                                = 12, //!< When PositionCS parses MI_SET_PREDICATE command with "Predicate Enable" set to "NOOP in PositionCS", PositionCS NOOP’s all the subsequent commands parsed unconditionally until the predication is disabled/modified using MI_SET_PREDICATE command. Other command streamers (non PositionCS) on parsing MI_SET_PREDICATE command with "Predicate Enable" set to "NOOP in PositionCS" don’t take any action and is equivalent to parsing MI_NOOP command.
            PREDICATE_ENABLE_NOOPALWAYS                                      = 15, //!< Following Commands will be NOOPED by RCS unconditionally.
        };

        enum MI_COMMAND_OPCODE
        {
            MI_COMMAND_OPCODE_MISETPREDICATE                                 = 1, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_MICOMMAND                                           = 0, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MI_SET_PREDICATE_CMD();

        static const size_t dwSize = 1;
        static const size_t byteSize = 4;
    };

    //!
    //! \brief MI_COPY_MEM_MEM
    //! \details
    //!     The MI_COPY_MEM_MEM command reads a DWord from memory and stores the
    //!     value of that DWord to back to memory.   The source and destination
    //!     addresses are specified in the command. The command temporarily halts
    //!     command execution.
    //!     
    //!     This command should not be used within a "non-privileged" batch buffer
    //!     to access global virtual space; doing so will be treated as privilege
    //!     access violation. Refer to the "User Mode Privilege Command" in
    //!     MI_BATCH_BUFFER_START command section to learn more about HW behavior on
    //!     encountering a privilege access violation.
    //!     
    //!     This command can be used within ring buffers and/or privilege batch
    //!     buffers to access global virtual space.
    //!     
    struct MI_COPY_MEM_MEM_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved8                                        : __CODEGEN_BITFIELD( 8, 20)    ; //!< Reserved
                uint32_t                 UseGlobalGttDestination                          : __CODEGEN_BITFIELD(21, 21)    ; //!< USE_GLOBAL_GTT_DESTINATION
                uint32_t                 UseGlobalGttSource                               : __CODEGEN_BITFIELD(22, 22)    ; //!< USE_GLOBAL_GTT_SOURCE
                uint32_t                 MiCommandOpcode                                  : __CODEGEN_BITFIELD(23, 28)    ; //!< MI_COMMAND_OPCODE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1..2
            struct
            {
                uint64_t                 Reserved32                                       : __CODEGEN_BITFIELD( 0,  1)    ; //!< Reserved
                uint64_t                 DestinationMemoryAddress                         : __CODEGEN_BITFIELD( 2, 63)    ; //!< Destination Memory Address
            };
            uint32_t                     Value[2];
        } DW1_2;
        union
        {
            //!< DWORD 3..4
            struct
            {
                uint64_t                 Reserved96                                       : __CODEGEN_BITFIELD( 0,  1)    ; //!< Reserved
                uint64_t                 SourceMemoryAddress                              : __CODEGEN_BITFIELD( 2, 63)    ; //!< Source Memory Address
            };
            uint32_t                     Value[2];
        } DW3_4;

        //! \name Local enumerations

        //! \brief USE_GLOBAL_GTT_DESTINATION
        //! \details
        //!     It is allowed for this bit to be set when executing this command from a
        //!     privileged (secure) batch buffer or ring buffer. This bit must be clear
        //!     when programmed from within a non-privileged batch buffer. This bit must
        //!     be 1 if the Per Process GTT Enable bit is clear.
        enum USE_GLOBAL_GTT_DESTINATION
        {
            USE_GLOBAL_GTT_DESTINATION_PERPROCESSGRAPHICSADDRESS             = 0, //!< No additional details
            USE_GLOBAL_GTT_DESTINATION_GLOBALGRAPHICSADDRESS                 = 1, //!< This command will use the global GTT to translate the Address and this command must be executing from a privileged (secure) batch buffer.
        };

        //! \brief USE_GLOBAL_GTT_SOURCE
        //! \details
        //!     It is allowed for this bit to be set when executing this command from a
        //!     privileged (secure) batch buffer or ring buffer. This bit must be clear
        //!     when programmed from within a non-privileged batch buffer. This bit must
        //!     be 1 if the Per Process GTT Enable bit is clear.
        enum USE_GLOBAL_GTT_SOURCE
        {
            USE_GLOBAL_GTT_SOURCE_PERPROCESSGRAPHICSADDRESS                  = 0, //!< No additional details
            USE_GLOBAL_GTT_SOURCE_GLOBALGRAPHICSADDRESS                      = 1, //!< This command will use the global GTT to translate the Address and this command must be executing from a privileged (secure) batch buffer.
        };

        enum MI_COMMAND_OPCODE
        {
            MI_COMMAND_OPCODE_MIMEMTOMEM                                     = 46, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_MICOMMAND                                           = 0, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MI_COPY_MEM_MEM_CMD();

        static const size_t dwSize = 5;
        static const size_t byteSize = 20;
    };

    //!
    //! \brief MI_STORE_DATA_IMM
    //! \details
    //!     The MI_STORE_DATA_IMM command requests a write of the QWord constant
    //!     supplied in the packet to the specified Memory Address. As the write
    //!     targets a System Memory Address, the write operation is coherent with
    //!     the CPU cache (i.e., the processor cache is snooped).
    //!     
    //!     This command supports writing to multiple consecutive dwords or qwords
    //!     memory locations from the starting address.
    //!     
    //!      This command should not be used within a "non-privilege" batch buffer
    //!     to access global virtual space, doing so will be treated as privilege
    //!     access violation. Refer "User Mode Privilege Command" in
    //!     MI_BATCH_BUFFER_START command section to know HW behavior on
    //!     encountering privilege access violation. This command can be used within
    //!     ring buffers and/or privilege batch buffers to access global virtual
    //!     space.
    //!      This command can be used for general software synchronization through
    //!     variables in cacheable memory (i.e., where software does not need to
    //!     poll un-cached memory or device registers).
    //!      This command simply initiates the write operation with command
    //!     execution proceeding normally. Although the write operation is
    //!     guaranteed to complete eventually, there is no mechanism to synchronize
    //!     command execution with the completion (or even initiation) of these
    //!     operations.
    //!     
    //!     
    //!     
    struct MI_STORE_DATA_IMM_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0,  9)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved10                                       : __CODEGEN_BITFIELD(10, 20)    ; //!< Reserved
                uint32_t                 StoreQword                                       : __CODEGEN_BITFIELD(21, 21)    ; //!< Store Qword
                uint32_t                 UseGlobalGtt                                     : __CODEGEN_BITFIELD(22, 22)    ; //!< Use Global GTT
                uint32_t                 MiCommandOpcode                                  : __CODEGEN_BITFIELD(23, 28)    ; //!< MI_COMMAND_OPCODE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1..2
            struct
            {
                uint64_t                 CoreModeEnable                                   : __CODEGEN_BITFIELD( 0,  0)    ; //!< Core Mode Enable
                uint64_t                 Reserved33                                       : __CODEGEN_BITFIELD( 1,  1)    ; //!< Reserved
                uint64_t                 Address                                          : __CODEGEN_BITFIELD( 2, 63)    ; //!< Address
            };
            uint32_t                     Value[2];
        } DW1_2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 DataDword0                                                                       ; //!< Data DWord 0
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 DataDword1                                                                       ; //!< Data DWord 1
            };
            uint32_t                     Value;
        } DW4;

        //! \name Local enumerations

        enum MI_COMMAND_OPCODE
        {
            MI_COMMAND_OPCODE_MISTOREDATAIMM                                 = 32, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_MICOMMAND                                           = 0, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MI_STORE_DATA_IMM_CMD();

        static const size_t dwSize = 5;
        static const size_t byteSize = 20;
    };

    //!
    //! \brief MI_SEMAPHORE_SIGNAL
    //! \details
    //!     This command is used to signal the target engine stating the memory
    //!     semaphore update occurrence to one of its contexts with Target Context
    //!     ID. MI_SEMPHORE_SIGNAL and MI_SEMAPHORE_WAIT together replace the
    //!     MI_SEMAPHORE_MBOX command on BDW. MI_ATOMIC (non-posted) command will be
    //!     programmed prior to this command to update the semaphore data in memory.
    //!     
    //!     Bellow programming notes must be followed while programming
    //!     MI_SEMAPHORE_SIGNAL for POCS. MI_SEMAPHOR_SIGNAL command in POCS command
    //!     sequence may be used to signal RCS but not any other command streamer
    //!     (VCS, VECS, BCS). Semaphore signal received by POCS will never be
    //!     forwarded to GUC. Only RCS can semaphore signal POCS and no other
    //!     command streamer must signal POCS. Semaphore signal received by RCS from
    //!     POCS will never get forwarded to GUC.
    //!
    //!     Post-Sync operation bit must not be set when Target Engine Select is
    //!     set to RCS.
    //!     
    struct MI_SEMAPHORE_SIGNAL_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved8                                        : __CODEGEN_BITFIELD( 8, 14)    ; //!< Reserved
                uint32_t                 TargetEngineSelect                               : __CODEGEN_BITFIELD(15, 18)    ; //!< TARGET_ENGINE_SELECT
                uint32_t                 Reserved19                                       : __CODEGEN_BITFIELD(19, 22)    ; //!< Reserved
                uint32_t                 MiCommandOpcode                                  : __CODEGEN_BITFIELD(23, 28)    ; //!< MI_COMMAND_OPCODE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 TargetContextId                                                                  ; //!< Target Context ID
            };
            uint32_t                     Value;
        } DW1;

        //! \name Local enumerations

        //! \brief TARGET_ENGINE_SELECT
        //! \details
        //!     This field selects the target engine to which SIGNAL will be send to.
        enum TARGET_ENGINE_SELECT
        {
            TARGET_ENGINE_SELECT_RCS                                         = 0, //!< No additional details
            TARGET_ENGINE_SELECT_VCS0                                        = 1, //!< No additional details
            TARGET_ENGINE_SELECT_BCS                                         = 2, //!< No additional details
            TARGET_ENGINE_SELECT_VECS                                        = 3, //!< No additional details
            TARGET_ENGINE_SELECT_VCS1                                        = 4, //!< No additional details
            TARGET_ENGINE_SELECT_VCS2                                        = 6, //!< No additional details
            TARGET_ENGINE_SELECT_VCS3                                        = 7, //!< No additional details
            TARGET_ENGINE_SELECT_VCS4                                        = 8, //!< No additional details
            TARGET_ENGINE_SELECT_VCS5                                        = 9, //!< No additional details
            TARGET_ENGINE_SELECT_VCS6                                        = 10, //!< No additional details
            TARGET_ENGINE_SELECT_VCS7                                        = 11, //!< No additional details
            TARGET_ENGINE_SELECT_VECS1                                       = 12, //!< No additional details
            TARGET_ENGINE_SELECT_VECS2                                       = 13, //!< No additional details
            TARGET_ENGINE_SELECT_VECS3                                       = 14, //!< No additional details
        };

        enum MI_COMMAND_OPCODE
        {
            MI_COMMAND_OPCODE_MISEMAPHORESIGNAL                              = 27, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_MICOMMAND                                           = 0, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MI_SEMAPHORE_SIGNAL_CMD();

        static const size_t dwSize = 2;
        static const size_t byteSize = 8;
    };

    //!
    //! \brief MI_SEMAPHORE_WAIT
    //! \details
    //!     
    //!     This command supports memory based Semaphore WAIT. Memory based
    //!     semaphores will be used for synchronization between the Producer and the
    //!     Consumer contexts. Producer and Consumer Contexts could be running on
    //!     different engines or on the same engine inside GT. Producer Context
    //!     implements a Signal and Consumer context implements a Wait.
    //!     Command Streamer on parsing this command fetches data from the Semaphore
    //!     Address mentioned in this command and compares it with the inline
    //!     Semaphore Data Dword.
    //!     
    //!     If comparison passes, the command streamer moves to the next command.
    //!     
    //!     If comparison fails Command streamer switches out the context. Context
    //!     switch can be inhibited by setting "Inhibit Synchronous Context Switch"
    //!     in CTXT_SR_CTL register.
    //!     
    //!     If "Inhibit Synchronous context Switch" is enabled and comparison fails,
    //!     Command Streamer evaluates the Compare Operation based on the Wait Mode
    //!     until the compare operation is true or Wait is canceled by SW.
    //!     
    //!     CS generates semaphore wait interrupt to the scheduler when
    //!     MI_SEMAPHORE_WAIT command is un-successful and when "Inhibit Synchronous
    //!     Context Switch" is set. Scheduler can use this interrupt to preempt the
    //!     context waiting on semaphore wait.
    //!     
    //!     
    //!     
    //!     
    //!     MI_SEMAPHORE_WAIT command also supports register based Semaphore WAIT.
    //!     Command Streamer on parsing this command fetches data from the MMIO
    //!     offset mentioned in this command and compares it with the inline
    //!     Semaphore Data Dword. This functionality is supported when "Register
    //!     Poll" bit is set in the command header. In register poll mode of
    //!     operation "Wait Mode" supported is always Poll mode and no Signal mode
    //!     is supported.
    //!     
    //!     If comparison passes, the command streamer moves to the next command.
    //!     
    //!     Unlike in Memory based semaphore, there is no context switch on an
    //!     un-successful semaphore wait in "Register Poll" mode, however preemption
    //!     is supported on unsuccessful semaphore wait in "Register Poll" mode.
    //!     Semaphore wait interrupt is not generated by default on wait
    //!     un-successful in "Register Poll" mode. However interrupt generation can
    //!     be enabled by setting debug mode bit "Semaphore Interrupt Enable in
    //!     Register Poll Mode" in specified register.
    //!     
    //!     Also unlike in Memory based semaphore, generation of an interrupt for a
    //!     semaphore wait in "Register Poll" mode is not dependent on the value of
    //!     bit "Inhibit Synchronous Context Switch" in register "CTXT_SR_CTL"
    //!     
    //!     Register Poll mode of Semaphore Wait command operation is non-privileged
    //!     and will be supported from PPGTT batch buffers.
    //!     
    //!     HW will trigger Render DOP CG on semaphore wait unsuccessful by default
    //!     and can be disabled if not desired by programming "Register
    //!     Poll Mode Semaphore Wait Event IDLE message Disable" bit in "INSTPM"
    //!     register. Note that Render DOP CG will not be triggered on register
    //!     semaphore wait un-successfull from INDIRECT_CTX pointer or
    //!     BB_PER_CTX_PTR buffers.
    //!
    //!     MI_SEMAPHORE_WAIT command must be always programmed with "Wait Mode" set
    //!     to "Polling Mode" Or MI_SEMAPHORE_WAIT command with "Wait Mode" set to
    //!     "Polling Mode" can be programmed when "Semaphore Wait Event IDLE message
    //!     Disable" bit in "RC_PSMI_CTRL" register is set to disable Idle messaging
    //!     on unsuccessful MI_SEMPAHORE_WAIT.
    //!     
    struct MI_SEMAPHORE_WAIT_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved8                                        : __CODEGEN_BITFIELD( 8, 11)    ; //!< Reserved
                uint32_t                 CompareOperation                                 : __CODEGEN_BITFIELD(12, 14)    ; //!< COMPARE_OPERATION
                uint32_t                 WaitMode                                         : __CODEGEN_BITFIELD(15, 15)    ; //!< WAIT_MODE
                uint32_t                 RegisterPollMode                                 : __CODEGEN_BITFIELD(16, 16)    ; //!< REGISTER_POLL_MODE
                uint32_t                 Reserved17                                       : __CODEGEN_BITFIELD(17, 21)    ; //!< Reserved
                uint32_t                 MemoryType                                       : __CODEGEN_BITFIELD(22, 22)    ; //!< MEMORY_TYPE
                uint32_t                 MiCommandOpcode                                  : __CODEGEN_BITFIELD(23, 28)    ; //!< MI_COMMAND_OPCODE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 SemaphoreDataDword                                                               ; //!< Semaphore Data Dword
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2..3
            struct
            {
                uint64_t                 Reserved64                                       : __CODEGEN_BITFIELD( 0,  1)    ; //!< Reserved
                uint64_t                 SemaphoreAddress                                 : __CODEGEN_BITFIELD( 2, 63)    ; //!< Semaphore Address
            };
            uint32_t                     Value[2];
        } DW2_3;

        //! \name Local enumerations

        //! \brief COMPARE_OPERATION
        //! \details
        //!     This field specifies the operation that will be executed to create the
        //!     result that will either allow the context to continue or wait.
        enum COMPARE_OPERATION
        {
            COMPARE_OPERATION_SADGREATERTHANSDD                              = 0, //!< If Indirect fetched data is greater than inline data then continue.
            COMPARE_OPERATION_SADGREATERTHANOREQUALSDD                       = 1, //!< If Indirect fetched data is greater than or equal to inline data then continue.
            COMPARE_OPERATION_SADLESSTHANSDD                                 = 2, //!< If Indirect fetched data is less than inline data then continue.
            COMPARE_OPERATION_SADLESSTHANOREQUALSDD                          = 3, //!< If Indirect fetched data is less than or equal to inline data then continue.
            COMPARE_OPERATION_SADEQUALSDD                                    = 4, //!< If Indirect fetched data is equalto inline data then continue.
            COMPARE_OPERATION_SADNOTEQUALSDD                                 = 5, //!< If Indirect fetched data is not equal to inline data then continue.
        };

        //! \brief WAIT_MODE
        //! \details
        //!     This bit specifies the WAIT behavior when the semaphore comparison fails
        //!     and before the context is switched out.
        enum WAIT_MODE
        {
            WAIT_MODE_SIGNALMODE                                             = 0, //!< In this mode HW will reacquire the semaphore data from memory on receiving SIGNAL with the same Context ID.
            WAIT_MODE_POLLINGMODE                                            = 1, //!< In this mode HW periodically reads the semaphore data from memory for comparison until it is context switched out. Periodicity will be mentioned in a SEMA_WAIT_POLL register.
        };

        //! \brief REGISTER_POLL_MODE
        //! \details
        //!     <p>This field control the seamphore wait behavior of polling from memory
        //!     vs MMIO register.</p>
        enum REGISTER_POLL_MODE
        {
            REGISTER_POLL_MODE_MEMORYPOLL                                    = 0, //!< In this mode HW will functional as in regular mode and checks for semaphore data in memory.
            REGISTER_POLL_MODE_REGISTERPOLL                                  = 1, //!< In this mode HW periodically reads the semaphore data from MMIO register instead of memory for comparison until the condition is satisfied. Periodicity will be mentioned in a SEMA_WAIT_POLL register.When operating in register poll mode, DW2 "Semaphore Address" (bits 22:2) carries the register MMIO offset to be polled.In register poll mode "Memory Type" field of this command are ignored by HW. 
        };

        //! \brief MEMORY_TYPE
        //! \details
        //!     This bit will be ignored and treated as if clear when executing from a
        //!     non-privileged batch buffer. It is allowed for this bit to be clear when
        //!     executing this command from a privileged (secure) batch buffer. This bit
        //!     <i>must</i> be 1 if the <b>Per Process GTT Enable</b> bit is clear.
        enum MEMORY_TYPE
        {
            MEMORY_TYPE_PERPROCESSGRAPHICSADDRESS                            = 0, //!< No additional details
            MEMORY_TYPE_GLOBALGRAPHICSADDRESS                                = 1, //!< This command will use the global GTT to translate the Address and this command must beexecuting from a privileged (secure) batch buffer.
        };

        enum MI_COMMAND_OPCODE
        {
            MI_COMMAND_OPCODE_MISEMAPHOREWAIT                                = 28, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_MICOMMAND                                           = 0, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MI_SEMAPHORE_WAIT_CMD();

        static const size_t dwSize = 4;
        static const size_t byteSize = 16;
    };

    //!
    //! \brief MI_CONDITIONAL_BATCH_BUFFER_END
    //! \details
    //!     The MI_CONDITIONAL_BATCH_BUFFER_END command is used to conditionally
    //!     terminate the execution of commands stored in a batch buffer initiated
    //!     using a MI_BATCH_BUFFER_START command. Termination of second level batch
    //!     buffer due to this command will also terminate the parent/first level
    //!     batch buffer.
    //!     
    //!     
    struct MI_CONDITIONAL_BATCH_BUFFER_END_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved8                                        : __CODEGEN_BITFIELD( 8, 18)    ; //!< Reserved
                uint32_t                 CompareMaskMode                                  : __CODEGEN_BITFIELD(19, 19)    ; //!< COMPARE_MASK_MODE
                uint32_t                 Reserved20                                       : __CODEGEN_BITFIELD(20, 20)    ; //!< Reserved
                uint32_t                 CompareSemaphore                                 : __CODEGEN_BITFIELD(21, 21)    ; //!< COMPARE_SEMAPHORE
                uint32_t                 UseGlobalGtt                                     : __CODEGEN_BITFIELD(22, 22)    ; //!< USE_GLOBAL_GTT
                uint32_t                 MiCommandOpcode                                  : __CODEGEN_BITFIELD(23, 28)    ; //!< MI_COMMAND_OPCODE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 CompareDataDword                                                                 ; //!< Compare Data Dword
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2..3
            struct
            {
                uint64_t                 Reserved64                                       : __CODEGEN_BITFIELD( 0,  2)    ; //!< Reserved
                uint64_t                 CompareAddress                                   : __CODEGEN_BITFIELD( 3, 63)    ; //!< Compare Address
            };
            uint32_t                     Value[2];
        } DW2_3;

        //! \name Local enumerations

        //! \brief COMPARE_MASK_MODE
        //! \details
        //!     When "Compare Mask Mode" is enabled, "Compare Address" must be qword
        //!     aligned.
        enum COMPARE_MASK_MODE
        {
            COMPARE_MASK_MODE_COMPAREMASKMODEDISABLED                        = 0, //!< Compare address points to Dword in memory consisting of Data Dword(DW0). HW will compare Data Dword(DW0) against Semaphore Data Dword.
            COMPARE_MASK_MODE_COMPAREMASKMODEENABLED                         = 1, //!< Compare address points to Qword in memory consisting of compare Mask (DW0) and Data Dword(DW1). HW will do AND operation on Mask(DW0) with Data Dword(DW1) and then compare the result against Semaphore Data Dword.
        };

        //! \brief COMPARE_SEMAPHORE
        //! \details
        //!     <p>If set, the value from the Compare Data Dword is compared to the
        //!     value from the Compare Address in memory. If the value at Compare
        //!     Address is greater than the Compare Data Dword, execution of current
        //!     command buffer should continue. If clear, the parser will continue to
        //!     the next command and not exit the batch buffer.</p>
        enum COMPARE_SEMAPHORE
        {
            COMPARE_SEMAPHORE_UNNAMED0                                       = 0, //!< No additional details
        };

        //! \brief USE_GLOBAL_GTT
        //! \details
        //!     If set, this command will use the global GTT to translate the <b>Compare
        //!     Address</b> and this command must be executing from a privileged
        //!     (secure) batch buffer. If clear, the PPGTT will be used to translate the
        //!     <b>Compare Address</b>.
        enum USE_GLOBAL_GTT
        {
            USE_GLOBAL_GTT_UNNAMED0                                          = 0, //!< No additional details
        };

        enum MI_COMMAND_OPCODE
        {
            MI_COMMAND_OPCODE_MICONDITIONALBATCHBUFFEREND                    = 54, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_MICOMMAND                                           = 0, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MI_CONDITIONAL_BATCH_BUFFER_END_CMD();

        static const size_t dwSize = 4;
        static const size_t byteSize = 16;
    };

    //!
    //! \brief MI_ATOMIC
    //! \details
    //!     MI_ATOMIC is used to carry atomic operation on data in graphics memory.
    //!     Atomic operations are supported on data granularity of 4B, 8B and 16B.
    //!     The atomic operation leads to a read-modify-write operation on the data
    //!     in graphics memory with the option of returning value. The data in
    //!     graphics memory is modified by doing arithmetic and logical operation
    //!     with the inline/indirect data provided with the MI_ATOMIC command.
    //!     Inline/Indirect provided in the command can be one or two operands based
    //!     on the atomic operation. Ex: Atomic-Compare operation needs two operands
    //!     while Atomic-Add operation needs single operand and Atomic-increment
    //!     requires no operand. Refer "Atomics" sub-section of "L3 Cache and URB"
    //!     section for detailed atomic operations supported. Atomic
    //!     operations can be enabled to return value by setting "Return Data
    //!     Control" field in the command, return data is stored to CS_GPR
    //!     registers.
    //!     CS_GPR4/5 registers are updated with memory Return Data based on the
    //!     "Data Size". Each GPR register is qword in size and occupies two MMIO
    //!     registers.
    //!     Note: Any references to CS_GPR registers in the command should be
    //!     understood as the CS_GPR registers belonging to the corresponding
    //!     engines *CS_GPR registers.
    //!     
    //!     
    //!     Indirect Source Operands:
    //!     
    //!     Operand1 is sourced from [CS_GPR1, CS_GPR0]
    //!     
    //!     Operand2 is sourced from [CS_GPR3, CS_GPR2]
    //!     
    //!     Read return Data is stored in [CS_GPR_5, CS_GPR4]
    //!     
    //!     When "Data Size" is DWORD lower dword of CS_GPR4 (Qword) is updated with
    //!     the dword data returned from memory. When "Data Size" is QWORD only
    //!     CS_GPR4 (Qword) is updated with the qword data returned from memory.
    //!     When the data size is OCTWORD CS_GPR4/5 are updated with the OCTWORD
    //!     data returned from memory. CS_GPR4 is loaded with lower qword returned
    //!     from memory and CS_GPR5 is loaded with upper qword returned from memory.
    //!     
    //!      When Inline Data mode is not set, Dwords 3..10 must not be included as
    //!     part of the command. Dword Length field in the header must be programmed
    //!     accordingly.
    //!      When Inline Data Mode is set, Dwords3..10 must be included based on the
    //!     Data Size field of the header. Both Operand-1 and Operand-2 dwords must
    //!     be programmed based on the Data Size field. Operand-2 must be programmed
    //!     to 0x0 if the atomic operation doesn't require it. Dword Length field in
    //!     the header must be programmed accordingly.
    //!     
    //!     
    //!     
    struct MI_ATOMIC_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< DWORD_LENGTH
                uint32_t                 AtomicOpcode                                     : __CODEGEN_BITFIELD( 8, 15)    ; //!< ATOMIC OPCODE
                uint32_t                 ReturnDataControl                                : __CODEGEN_BITFIELD(16, 16)    ; //!< Return Data Control
                uint32_t                 CsStall                                          : __CODEGEN_BITFIELD(17, 17)    ; //!< CS STALL
                uint32_t                 InlineData                                       : __CODEGEN_BITFIELD(18, 18)    ; //!< Inline Data
                uint32_t                 DataSize                                         : __CODEGEN_BITFIELD(19, 20)    ; //!< DATA_SIZE
                uint32_t                 Reserved21                                       : __CODEGEN_BITFIELD(21, 21)    ; //!< Reserved
                uint32_t                 MemoryType                                       : __CODEGEN_BITFIELD(22, 22)    ; //!< MEMORY_TYPE
                uint32_t                 MiCommandOpcode                                  : __CODEGEN_BITFIELD(23, 28)    ; //!< MI_COMMAND_OPCODE
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
                uint32_t                 MemoryAddress                                    : __CODEGEN_BITFIELD( 2, 31)    ; //!< Memory Address
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 MemoryAddressHigh                                : __CODEGEN_BITFIELD( 0, 15)    ; //!< Memory Address High
                uint32_t                 Reserved80                                       : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 Operand1DataDword0                                                               ; //!< Operand1 Data Dword 0
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 Operand2DataDword0                                                               ; //!< Operand2 Data Dword 0
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 Operand1DataDword1                                                               ; //!< Operand1 Data Dword 1
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 Operand2DataDword1                                                               ; //!< Operand2 Data Dword 1
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 Operand1DataDword2                                                               ; //!< Operand1 Data Dword 2
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            //!< DWORD 8
            struct
            {
                uint32_t                 Operand2DataDword2                                                               ; //!< Operand2 Data Dword 2
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            //!< DWORD 9
            struct
            {
                uint32_t                 Operand1DataDword3                                                               ; //!< Operand1 Data Dword 3
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            //!< DWORD 10
            struct
            {
                uint32_t                 Operand2DataDword3                                                               ; //!< Operand2 Data Dword 3
            };
            uint32_t                     Value;
        } DW10;

        //! \name Local enumerations

        //! \brief DATA_SIZE
        //! \details
        //!     This field indicates the size of the operand in dword/qword/octword on
        //!     which atomic operation will be performed. Data size must match with the
        //!     Atomic Opcode. Operation Data size could be 4B, 8B or 16B
        enum DATA_SIZE
        {
            DATA_SIZE_DWORD                                                  = 0, //!< Operand size used by Atomic Operation is DWORD.
            DATA_SIZE_QWORD                                                  = 1, //!< Operand Size used by Atomic Operation is QWORD.
            DATA_SIZE_OCTWORD                                                = 2, //!< Operand Size used by Atomic Operation is OCTWORD.
        };

        //! \brief MEMORY_TYPE
        //! \details
        //!     This bit will be ignored and treated as if clear when executing from a
        //!     non-privileged batch buffer. It is allowed for this bit to be clear when
        //!     executing this command from a privileged (secure) batch buffer. This bit
        //!     must be 1 if the <b>Per Process GTT Enable</b> bit is clear.
        enum MEMORY_TYPE
        {
            MEMORY_TYPE_PERPROCESSGRAPHICSADDRESS                            = 0, //!< No additional details
            MEMORY_TYPE_GLOBALGRAPHICSADDRESS                                = 1, //!< This command will use the global GTT to translate the Address and this command must be executing from a privileged (secure) batch buffer.
        };

        enum MI_COMMAND_OPCODE
        {
            MI_COMMAND_OPCODE_MIATOMIC                                       = 47, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_MICOMMAND                                           = 0, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MI_ATOMIC_CMD();

        static const size_t dwSize = 11;
        static const size_t byteSize = 44;
    };

    //!
    //! \brief MI_MATH
    //! \details
    //!     The MI_MATH command allows software to send instructions to the ALU in
    //!     the Command Streamer. This command is the means by which the ALU is
    //!     accessed. ALU instructions form the data payload of the MI_MATH command.
    //!     An ALU instruction takes one DWord in size. The MI_MATH DWord Length is
    //!     programmed based on the number of ALU instructions included, limited
    //!     only by the max DWord Length supported. When the command streamer parses
    //!     an MI_MATH command, it sends the included ALU instructions to the ALU.
    //!     The ALU processes any instruction in a single clock. See the ALU section
    //!     for more details.
    //!     
    struct MI_MATH_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved8                                        : __CODEGEN_BITFIELD( 8, 22)    ; //!< Reserved
                uint32_t                 MiCommandOpcode                                  : __CODEGEN_BITFIELD(23, 28)    ; //!< MI_COMMAND_OPCODE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;

        //! \name Local enumerations

        enum MI_COMMAND_OPCODE
        {
            MI_COMMAND_OPCODE_MIMATH                                         = 26, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_MICOMMAND                                           = 0, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MI_MATH_CMD();

        static const size_t dwSize = 1;
        static const size_t byteSize = 4;
    };

    //!
    //! \brief MI_FLUSH_DW
    //! \details
    //!     The MI_FLUSH_DW command is used to perform an internal "flush"
    //!     operation. The parser pauses on an internal flush until all drawing
    //!     engines have completed any pending operations. In addition, this command
    //!     can also be used to:Flush any dirty data to memory. Invalidate the TLB
    //!     cache inside the hardware Usage note: After this command is completed
    //!     with a Store DWord enabled, CPU access to graphics memory will be
    //!     coherent (assuming the Render Cache flush is not inhibited).
    //!     
    struct MI_FLUSH_DW_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0,  5)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved6                                        : __CODEGEN_BITFIELD( 6,  6)    ; //!< Reserved
                uint32_t                 VideoPipelineCacheInvalidate                     : __CODEGEN_BITFIELD( 7,  7)    ; //!< Video Pipeline Cache invalidate
                uint32_t                 NotifyEnable                                     : __CODEGEN_BITFIELD( 8,  8)    ; //!< Notify Enable
                uint32_t                 FlushLlc                                         : __CODEGEN_BITFIELD( 9,  9)    ; //!< Flush LLC
                uint32_t                 Reserved10                                       : __CODEGEN_BITFIELD(10, 13)    ; //!< Reserved
                uint32_t                 PostSyncOperation                                : __CODEGEN_BITFIELD(14, 15)    ; //!< POST_SYNC_OPERATION
                uint32_t                 Reserved16                                       : __CODEGEN_BITFIELD(16, 17)    ; //!< Reserved
                uint32_t                 TlbInvalidate                                    : __CODEGEN_BITFIELD(18, 18)    ; //!< TLB Invalidate
                uint32_t                 Reserved19                                       : __CODEGEN_BITFIELD(19, 20)    ; //!< Reserved
                uint32_t                 StoreDataIndex                                   : __CODEGEN_BITFIELD(21, 21)    ; //!< Store Data Index
                uint32_t                 Reserved22                                       : __CODEGEN_BITFIELD(22, 22)    ; //!< Reserved
                uint32_t                 MiCommandOpcode                                  : __CODEGEN_BITFIELD(23, 28)    ; //!< MI_COMMAND_OPCODE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1..2
            struct
            {
                uint64_t                 Reserved32                                       : __CODEGEN_BITFIELD( 0,  1)    ; //!< Reserved
                uint64_t                 DestinationAddressType                           : __CODEGEN_BITFIELD( 2,  2)    ; //!< DESTINATION_ADDRESS_TYPE
                uint64_t                 Address                                          : __CODEGEN_BITFIELD( 3, 47)    ; //!< Address
                uint64_t                 Reserved80                                       : __CODEGEN_BITFIELD(48, 63)    ; //!< Reserved
            };
            uint32_t                     Value[2];
        } DW1_2;
        union
        {
            //!< DWORD 3..4
            struct
            {
                uint64_t                 ImmediateData                                                                    ; //!< Immediate Data
            };
            uint32_t                     Value[2];
        } DW3_4;

        //! \name Local enumerations

        //! \brief POST_SYNC_OPERATION
        //! \details
        //!     BitFieldDesc
        enum POST_SYNC_OPERATION
        {
            POST_SYNC_OPERATION_NOWRITE                                      = 0, //!< No write occurs as a result of this instruction. This can be used to implement a "trap" operation, etc.
            POST_SYNC_OPERATION_WRITEIMMEDIATEDATA                           = 1, //!< HW implicitly detects the Data size to be Qword or Dword to be written to memory based on the command dword length programmed  . When Dword Length indicates Qword, Writes the QWord containing Immediate Data Low, High DWs to the Destination Address . When Dword Length indicates Dword, Writes the DWord containing Immediate Data Low to the Destination Address
            POST_SYNC_OPERATION_UNNAMED3                                     = 3, //!< Write the TIMESTAMP register to the Destination Address. The upper 28 bits of the TIMESTAMP register are tied to '0'.
        };

        enum MI_COMMAND_OPCODE
        {
            MI_COMMAND_OPCODE_MIFLUSHDW                                      = 38, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_MICOMMAND                                           = 0, //!< No additional details
        };

        //! \brief DESTINATION_ADDRESS_TYPE
        //! \details
        //!     Defines address space of Destination Address
        enum DESTINATION_ADDRESS_TYPE
        {
            DESTINATION_ADDRESS_TYPE_PPGTT                                   = 0, //!< Use PPGTT address space for DW write
            DESTINATION_ADDRESS_TYPE_GGTT                                    = 1, //!< Use GGTT address space for DW write
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MI_FLUSH_DW_CMD();

        static const size_t dwSize = 5;
        static const size_t byteSize = 20;
    };

    //!
    //! \brief PIPE_CONTROL
    //! \details
    //!     The PIPE_CONTROL command is used to effect the synchronization described
    //!     above.
    //!     
    //!     SW must follow below programming restrictions when
    //!     programming PIPECONTROL command ifor POCS:
    //!     
    //!     
    //!     Write cache flush bits must not be set (Render Target Cache Flush
    //!     Enable, DC Flush Enable, Depth Cache Flush Enable )
    //!     
    //!     Post Sync Operations must not be set to "Write PS Depth Count"
    //!     
    //!     "Stall at Pixel Scoreboard" must not be set
    //!     
    //!     "Notify Enable" must not be set.
    //!     
    //!     "Depth Stall Enable" must not be set.
    //!     
    //!     "Generic Media State Clear" must not be set.
    //!     
    //!     "PSD Sync Enable" must not be set.
    //!     
    struct PIPE_CONTROL_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved8                                        : __CODEGEN_BITFIELD( 8, 15)    ; //!< Reserved
                uint32_t                 Command3DSubOpcode                               : __CODEGEN_BITFIELD(16, 23)    ; //!< _3D_COMMAND_SUB_OPCODE
                uint32_t                 Command3DOpcode                                  : __CODEGEN_BITFIELD(24, 26)    ; //!< _3D_COMMAND_OPCODE
                uint32_t                 CommandSubtype                                   : __CODEGEN_BITFIELD(27, 28)    ; //!< COMMAND_SUBTYPE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 DepthCacheFlushEnable                            : __CODEGEN_BITFIELD( 0,  0)    ; //!< DEPTH_CACHE_FLUSH_ENABLE
                uint32_t                 StallAtPixelScoreboard                           : __CODEGEN_BITFIELD( 1,  1)    ; //!< STALL_AT_PIXEL_SCOREBOARD
                uint32_t                 StateCacheInvalidationEnable                     : __CODEGEN_BITFIELD( 2,  2)    ; //!< State Cache Invalidation Enable
                uint32_t                 ConstantCacheInvalidationEnable                  : __CODEGEN_BITFIELD( 3,  3)    ; //!< Constant Cache Invalidation Enable
                uint32_t                 VfCacheInvalidationEnable                        : __CODEGEN_BITFIELD( 4,  4)    ; //!< VF Cache Invalidation Enable
                uint32_t                 DcFlushEnable                                    : __CODEGEN_BITFIELD( 5,  5)    ; //!< DC  Flush Enable
                uint32_t                 Reserved38                                       : __CODEGEN_BITFIELD( 6,  6)    ; //!< Reserved
                uint32_t                 PipeControlFlushEnable                           : __CODEGEN_BITFIELD( 7,  7)    ; //!< Pipe Control Flush Enable
                uint32_t                 NotifyEnable                                     : __CODEGEN_BITFIELD( 8,  8)    ; //!< Notify Enable
                uint32_t                 IndirectStatePointersDisable                     : __CODEGEN_BITFIELD( 9,  9)    ; //!< Indirect State Pointers Disable
                uint32_t                 TextureCacheInvalidationEnable                   : __CODEGEN_BITFIELD(10, 10)    ; //!< Texture Cache Invalidation Enable
                uint32_t                 InstructionCacheInvalidateEnable                 : __CODEGEN_BITFIELD(11, 11)    ; //!< Instruction Cache Invalidate Enable
                uint32_t                 RenderTargetCacheFlushEnable                     : __CODEGEN_BITFIELD(12, 12)    ; //!< RENDER_TARGET_CACHE_FLUSH_ENABLE
                uint32_t                 DepthStallEnable                                 : __CODEGEN_BITFIELD(13, 13)    ; //!< DEPTH_STALL_ENABLE
                uint32_t                 PostSyncOperation                                : __CODEGEN_BITFIELD(14, 15)    ; //!< POST_SYNC_OPERATION
                uint32_t                 GenericMediaStateClear                           : __CODEGEN_BITFIELD(16, 16)    ; //!< Generic Media State Clear
                uint32_t                 PsdSyncEnable                                    : __CODEGEN_BITFIELD(17, 17)    ; //!< PSD Sync Enable
                uint32_t                 TlbInvalidate                                    : __CODEGEN_BITFIELD(18, 18)    ; //!< TLB Invalidate
                uint32_t                 GlobalSnapshotCountReset                         : __CODEGEN_BITFIELD(19, 19)    ; //!< GLOBAL_SNAPSHOT_COUNT_RESET
                uint32_t                 CommandStreamerStallEnable                       : __CODEGEN_BITFIELD(20, 20)    ; //!< Command Streamer Stall Enable
                uint32_t                 StoreDataIndex                                   : __CODEGEN_BITFIELD(21, 21)    ; //!< Store Data Index
                uint32_t                 Reserved54                                       : __CODEGEN_BITFIELD(22, 22)    ; //!< Reserved
                uint32_t                 LriPostSyncOperation                             : __CODEGEN_BITFIELD(23, 23)    ; //!< LRI_POST_SYNC_OPERATION
                uint32_t                 DestinationAddressType                           : __CODEGEN_BITFIELD(24, 24)    ; //!< DESTINATION_ADDRESS_TYPE
                uint32_t                 Reserved57                                       : __CODEGEN_BITFIELD(25, 25)    ; //!< Reserved
                uint32_t                 FlushLlc                                         : __CODEGEN_BITFIELD(26, 26)    ; //!< Flush LLC
                uint32_t                 Reserved59                                       : __CODEGEN_BITFIELD(27, 27)    ; //!< Reserved
                uint32_t                 TileCacheFlushEnable                             : __CODEGEN_BITFIELD(28, 28)    ; //!< TILE_CACHE_FLUSH_ENABLE
                uint32_t                 Reserved61                                       : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 Reserved64                                       : __CODEGEN_BITFIELD( 0,  1)    ; //!< Reserved
                uint32_t                 Address                                          : __CODEGEN_BITFIELD( 2, 31)    ; //!< Address
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 AddressHigh                                                                      ; //!< Address High
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4..5
            struct
            {
                uint64_t                 ImmediateData                                                                    ; //!< Immediate Data
            };
            uint32_t                     Value[2];
        } DW4_5;

        //! \name Local enumerations

        enum _3D_COMMAND_SUB_OPCODE
        {
            _3D_COMMAND_SUB_OPCODE_PIPECONTROL                               = 0, //!< No additional details
        };

        enum _3D_COMMAND_OPCODE
        {
            _3D_COMMAND_OPCODE_PIPECONTROL                                   = 2, //!< No additional details
        };

        enum COMMAND_SUBTYPE
        {
            COMMAND_SUBTYPE_GFXPIPE3D                                        = 3, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_GFXPIPE                                             = 3, //!< No additional details
        };

        //! \brief DEPTH_CACHE_FLUSH_ENABLE
        //! \details
        //!     Setting this bit enables flushing (i.e. writing back the dirty lines to
        //!     memory and invalidating the tags) of depth related caches. This bit
        //!     applies to HiZ cache, Stencil cache and depth cache.
        enum DEPTH_CACHE_FLUSH_ENABLE
        {
            DEPTH_CACHE_FLUSH_ENABLE_FLUSHDISABLED                           = 0, //!< Depth relates caches (HiZ, Stencil and Depth) are NOT flushed.
            DEPTH_CACHE_FLUSH_ENABLE_FLUSHENABLED                            = 1, //!< Depth relates caches (HiZ, Stencil and Depth) are flushed.
        };

        //! \brief STALL_AT_PIXEL_SCOREBOARD
        //! \details
        //!     Defines the behavior of PIPE_CONTROL command at the pixel scoreboard.
        enum STALL_AT_PIXEL_SCOREBOARD
        {
            STALL_AT_PIXEL_SCOREBOARD_DISABLE                                = 0, //!< Stall at the pixel scoreboard is disabled.
            STALL_AT_PIXEL_SCOREBOARD_ENABLE                                 = 1, //!< Stall at the pixel scoreboard is enabled.
        };

        //! \brief RENDER_TARGET_CACHE_FLUSH_ENABLE
        //! \details
        //!     <p>Setting this bit will force Render Cache to be flushed to memory
        //!     prior to this synchronization point completing. This bit must be set for
        //!     all write fence sync operations to assure that results from operations
        //!     initiated prior to this command are visible in memory once software
        //!     observes this synchronization.</p>
        enum RENDER_TARGET_CACHE_FLUSH_ENABLE
        {
            RENDER_TARGET_CACHE_FLUSH_ENABLE_DISABLEFLUSH                    = 0, //!< Render Target Cache is NOT flushed.
            RENDER_TARGET_CACHE_FLUSH_ENABLE_ENABLEFLUSH                     = 1, //!< Render Target Cache is flushed.
        };

        //! \brief DEPTH_STALL_ENABLE
        //! \details
        //!     <p>This bit must be set when obtaining a "visible pixel" count to
        //!     preclude the possible inclusion in the PS_DEPTH_COUNT value written to
        //!     memory of some fraction of pixels from objects initiated after the
        //!     PIPE_CONTROL command.</p>
        enum DEPTH_STALL_ENABLE
        {
            DEPTH_STALL_ENABLE_DISABLE                                       = 0, //!< 3D pipeline will not stall subsequent primitives at the Depth Test stage.
            DEPTH_STALL_ENABLE_ENABLE                                        = 1, //!< 3D pipeline will stall any subsequent primitives at the Depth Test stage until the Sync and Post-Sync operations complete.
        };

        //! \brief POST_SYNC_OPERATION
        //! \details
        //!     This field specifies an optional action to be taken upon completion of
        //!     the synchronization operation.
        enum POST_SYNC_OPERATION
        {
            POST_SYNC_OPERATION_NOWRITE                                      = 0, //!< No write occurs as a result of this instruction. This can be used to implement a "trap" operation, etc.
            POST_SYNC_OPERATION_WRITEIMMEDIATEDATA                           = 1, //!< Write the QWord containing Immediate Data Low, High DWs to the Destination Address
            POST_SYNC_OPERATION_WRITEPSDEPTHCOUNT                            = 2, //!< Write the 64-bit PS_DEPTH_COUNT register to the Destination Address
            POST_SYNC_OPERATION_WRITETIMESTAMP                               = 3, //!< Write the 64-bit TIMESTAMP register(i.e. "Reported Timestamp Count" 0x2358 for render pipe) to the Destination Address.
        };

        //! \brief GLOBAL_SNAPSHOT_COUNT_RESET
        //! \details
        //!     <p>This debug mode bit must not be exercised on any product.</p>
        enum GLOBAL_SNAPSHOT_COUNT_RESET
        {
            GLOBAL_SNAPSHOT_COUNT_RESET_DONTRESET                            = 0, //!< Do not reset the snapshot counts or Statistics Counters.
            GLOBAL_SNAPSHOT_COUNT_RESET_RESET                                = 1, //!< Reset the snapshot count in Gen4 for all the units and reset the Statistics Counters except as noted above.
        };

        //! \brief LRI_POST_SYNC_OPERATION
        //! \details
        //!     This bit caues a post sync operation with an LRI (Load Register
        //!     Immediate) operation. If this bit is set then the Post-Sync Operation
        //!     field must be cleared.
        enum LRI_POST_SYNC_OPERATION
        {
            LRI_POST_SYNC_OPERATION_NOLRIOPERATION                           = 0, //!< No LRI operation occurs as a result of this instruction. The Post-Sync Operation field is valid and may be used to specify an operation.
            LRI_POST_SYNC_OPERATION_MMIOWRITEIMMEDIATEDATA                   = 1, //!< Write the DWord contained in Immediate Data Low (DW3) to the MMIO offset specifed in the Address field.
        };

        //! \brief DESTINATION_ADDRESS_TYPE
        //! \details
        //!     Defines address space of Destination Address
        enum DESTINATION_ADDRESS_TYPE
        {
            DESTINATION_ADDRESS_TYPE_PPGTT                                   = 0, //!< Use PPGTT address space for DW write
            DESTINATION_ADDRESS_TYPE_GGTT                                    = 1, //!< Use GGTT address space for DW write
        };

        //! \brief TILE_CACHE_FLUSH_ENABLE
        //! \details
        //!     <p>Setting this bit will force Tile Cache (contains both color and depth
        //!     data) to be flushed to memory prior to this synchronization point
        //!     completing. This bit must be set for all write fence sync operations to
        //!     assure that results from operations initiated prior to this command are
        //!     visible in memory once software observes this synchronization.</p>
        //!     <p></p>
        enum TILE_CACHE_FLUSH_ENABLE
        {
            TILE_CACHE_FLUSH_ENABLE_UNNAMED0                                 = 0, //!< Tile Cache is not flushed.
            TILE_CACHE_FLUSH_ENABLE_UNNAMED1                                 = 1, //!< Tile cache is flushed.
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        PIPE_CONTROL_CMD();

        static const size_t dwSize = 6;
        static const size_t byteSize = 24;
    };

    //!
    //! \brief MFX_WAIT
    //! \details
    //!     This command can be considered the same as an MI_NOOP except that the
    //!     command parser will not parse the next command until the following
    //!     happens  AVC or VC1 BSD mode: The command will stall the parser until
    //!     completion of the BSD object
    //!      IT, encoder, and MPEG2 BSD mode: The command will stall the parser
    //!     until the object package is sent down the pipelineThis command should be
    //!     used to ensure the preemption enable window occurs during the time the
    //!     object command is being executed down the pipeline.
    //!      
    //!     
    struct MFX_WAIT_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0,  5)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved6                                        : __CODEGEN_BITFIELD( 6,  7)    ; //!< Reserved
                uint32_t                 MfxSyncControlFlag                               : __CODEGEN_BITFIELD( 8,  8)    ; //!< MFX Sync Control Flag
                uint32_t                 Reserved9                                        : __CODEGEN_BITFIELD( 9, 15)    ; //!< Reserved
                uint32_t                 SubOpcode                                        : __CODEGEN_BITFIELD(16, 26)    ; //!< SUB_OPCODE
                uint32_t                 CommandSubtype                                   : __CODEGEN_BITFIELD(27, 28)    ; //!< COMMAND_SUBTYPE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;

        //! \name Local enumerations

        enum SUB_OPCODE
        {
            SUB_OPCODE_MFXWAIT                                               = 0, //!< No additional details
        };

        enum COMMAND_SUBTYPE
        {
            COMMAND_SUBTYPE_MFXSINGLEDW                                      = 1, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFX_WAIT_CMD();

        static const size_t dwSize = 1;
        static const size_t byteSize = 4;
    };

    //!
    //! \brief MEDIA_STATE_FLUSH
    //! \details
    //!     This command updates the Message Gateway state. In particular, it
    //!     updates the state for a selected Interface Descriptor.
    //!     
    //!     This command can be considered same as a MI_Flush except that only media
    //!     parser will get flushed instead of the entire 3D/media render pipeline.
    //!     The command should be programmed prior to new Media state, curbe and/or
    //!     interface descriptor commands when switching to a new context or
    //!     programming new state for the same context. With this command, pipelined
    //!     state change is allowed for the media pipe.
    //!     
    //!     Be cautious when using this command when child_present flag in the media
    //!     state is enabled. This is because that CURBE state as well as Interface
    //!     Descriptor state are shared between root threads and child threads.
    //!     Changing these states while child threads are generated on the fly may
    //!     cause unexpected behavior. Combining with MI_ARB_ON/OFF command, it is
    //!     possible to support interruptability with the following command sequence
    //!     where interrupt may be allowed only when MI_ARB_ON_OFF is ON:
    //!     
    //!     <pre>MEDIA_STATE_FLUSH 
    //!     VFE_STATE  VFE will hold CS if watermark isn't met 
    //!     MI_ARB_OFF  There must be at least one VFE command before this one
    //!     MEDIA_OBJECT ... MI_ARB_ON</pre>
    //!     
    struct MEDIA_STATE_FLUSH_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 15)    ; //!< DWORD_LENGTH
                uint32_t                 Subopcode                                        : __CODEGEN_BITFIELD(16, 23)    ; //!< SUBOPCODE
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
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
                uint32_t                 InterfaceDescriptorOffset                        : __CODEGEN_BITFIELD( 0,  5)    ; //!< Interface Descriptor Offset
                uint32_t                 Reserved38                                       : __CODEGEN_BITFIELD( 6,  6)    ; //!< Reserved
                uint32_t                 FlushToGo                                        : __CODEGEN_BITFIELD( 7,  7)    ; //!< Flush to GO
                uint32_t                 Reserved40                                       : __CODEGEN_BITFIELD( 8, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;

        //! \name Local enumerations

        enum SUBOPCODE
        {
            SUBOPCODE_MEDIASTATEFLUSHSUBOP                                   = 4, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_MEDIASTATEFLUSH                             = 0, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MEDIA                                                   = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_GFXPIPE                                             = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MEDIA_STATE_FLUSH_CMD();

        static const size_t dwSize = 2;
        static const size_t byteSize = 8;
    };

};

#pragma pack()

#endif  // __MHW_MI_HWCMD_G11_X_H__