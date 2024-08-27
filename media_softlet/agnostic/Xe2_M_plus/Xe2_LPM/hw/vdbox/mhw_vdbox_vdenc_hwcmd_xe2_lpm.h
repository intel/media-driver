/*===================== begin_copyright_notice ==================================

# Copyright (c) 2023, Intel Corporation

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
//! \file     mhw_vdbox_vdenc_hwcmd_xe2_lpm.h
//! \brief    Auto-generated constructors for MHW and states.
//! \details  This file may not be included outside of Xe2_LPM+ as other components
//!           should use MHW interface to interact with MHW commands and states.
//!

#ifndef __MHW_VDBOX_VDENC_HWCMD_XE2_LPM_H__
#define __MHW_VDBOX_VDENC_HWCMD_XE2_LPM_H__

#include "mhw_hwcmd.h"
#ifdef IGFX_VDENC_INTERFACE_EXT_SUPPORT
#include "mhw_vdbox_vdenc_hwcmd_ext.h"
#endif

#pragma once
#pragma pack(1)


namespace mhw
{
namespace vdbox
{
namespace vdenc
{
namespace xe2_lpm_base
{
namespace xe2_lpm
{
struct _VDENC_CMD1_CMD
{
    union
    {
        struct
        {
            uint32_t DwordLength : __CODEGEN_BITFIELD(0, 11);   //!< DWORD_LENGTH
            uint32_t Reserved12 : __CODEGEN_BITFIELD(12, 15);   //!< Reserved
            uint32_t Subopb : __CODEGEN_BITFIELD(16, 20);       //!< SUBOPB
            uint32_t Subopa : __CODEGEN_BITFIELD(21, 22);       //!< SUBOPA
            uint32_t Opcode : __CODEGEN_BITFIELD(23, 26);       //!< OPCODE
            uint32_t Pipeline : __CODEGEN_BITFIELD(27, 28);     //!< PIPELINE
            uint32_t CommandType : __CODEGEN_BITFIELD(29, 31);  //!< COMMAND_TYPE
        };
        uint32_t Value;
    } DW0;
    union
    {
        //!< DWORD 1
        struct
        {
            uint32_t VDENC_CMD1_DW1_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW1_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW1_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW1_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW1;
    union
    {
        //!< DWORD 2
        struct
        {
            uint32_t VDENC_CMD1_DW2_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW2_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW2_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW2_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW2;
    union
    {
        //!< DWORD 3
        struct
        {
            uint32_t VDENC_CMD1_DW3_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW3_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW3_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW3_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW3;
    union
    {
        //!< DWORD 4
        struct
        {
            uint32_t VDENC_CMD1_DW4_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW4_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW4_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW4_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW4;
    union
    {
        //!< DWORD 5
        struct
        {
            uint32_t VDENC_CMD1_DW5_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW5_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW5_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW5_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW5;
    union
    {
        //!< DWORD 6
        struct
        {
            uint32_t VDENC_CMD1_DW6_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW6_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW6_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW6_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW6;
    union
    {
        //!< DWORD 7
        struct
        {
            uint32_t VDENC_CMD1_DW7_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW7_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW7_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW7_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW7;
    union
    {
        //!< DWORD 8
        struct
        {
            uint32_t VDENC_CMD1_DW8_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW8_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW8_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW8_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW8;
    union
    {
        //!< DWORD 9
        struct
        {
            uint32_t VDENC_CMD1_DW9_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW9_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW9_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW9_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW9;
    union
    {
        //!< DWORD 10
        struct
        {
            uint32_t VDENC_CMD1_DW10_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW10_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW10_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW10_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW10;
    union
    {
        //!< DWORD 11
        struct
        {
            uint32_t VDENC_CMD1_DW11_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW11_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW11_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW11_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW11;
    union
    {
        //!< DWORD 12
        struct
        {
            uint32_t VDENC_CMD1_DW12_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW12_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW12_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW12_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW12;
    union
    {
        //!< DWORD 13
        struct
        {
            uint32_t VDENC_CMD1_DW13_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW13_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW13_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW13_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW13;
    union
    {
        //!< DWORD 14
        struct
        {
            uint32_t VDENC_CMD1_DW14_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW14_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW14_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW14_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW14;
    union
    {
        //!< DWORD 15
        struct
        {
            uint32_t VDENC_CMD1_DW15_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW15_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW15_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW15_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW15;
    union
    {
        //!< DWORD 16
        struct
        {
            uint32_t VDENC_CMD1_DW16_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW16_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW16_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW16_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW16;
    union
    {
        //!< DWORD 17
        struct
        {
            uint32_t VDENC_CMD1_DW17_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW17_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW17_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW17_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW17;
    union
    {
        //!< DWORD 18
        struct
        {
            uint32_t VDENC_CMD1_DW18_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW18_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW18_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW18_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW18;
    union
    {
        //!< DWORD 19
        struct
        {
            uint32_t VDENC_CMD1_DW19_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW19_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW19_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW19_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW19;
    union
    {
        //!< DWORD 20
        struct
        {
            uint32_t VDENC_CMD1_DW20_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW20_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW20_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW20_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW20;
    union
    {
        //!< DWORD 21
        struct
        {
            uint32_t VDENC_CMD1_DW21_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW21_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW21_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW21_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW21;
    union
    {
        //!< DWORD 22
        struct
        {
            uint32_t VDENC_CMD1_DW22_BIT0 : __CODEGEN_BITFIELD(0, 15);
            uint32_t VDENC_CMD1_DW22_BIT16 : __CODEGEN_BITFIELD(16, 24);
            uint32_t VDENC_CMD1_DW22_BIT25 : __CODEGEN_BITFIELD(25, 31);
        };
        uint32_t Value;
    } DW22;
    union
    {
        //!< DWORD 23
        struct
        {
            uint32_t VDENC_CMD1_DW23_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW23_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW23_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW23_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW23;
    union
    {
        //!< DWORD 24
        struct
        {
            uint32_t VDENC_CMD1_DW24_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW24_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW24_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW24_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW24;
    union
    {
        //!< DWORD 25
        struct
        {
            uint32_t VDENC_CMD1_DW25_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW25_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW25_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW25_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW25;
    union
    {
        //!< DWORD 26
        struct
        {
            uint32_t VDENC_CMD1_DW26_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW26_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW26_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW26_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW26;
    union
    {
        //!< DWORD 27
        struct
        {
            uint32_t VDENC_CMD1_DW27_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW27_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW27_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW27_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW27;
    union
    {
        //!< DWORD 28
        struct
        {
            uint32_t VDENC_CMD1_DW28_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW28_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW28_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW28_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW28;
    union
    {
        //!< DWORD 29
        struct
        {
            uint32_t VDENC_CMD1_DW29_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW29_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW29_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW29_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW29;
    union
    {
        //!< DWORD 30
        struct
        {
            uint32_t VDENC_CMD1_DW30_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW30_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW30_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW30_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW30;
    union
    {
        struct
        {
            uint32_t VDENC_CMD1_DW31_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW31_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW31_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW31_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW31;
    union
    {
        struct
        {
            uint32_t VDENC_CMD1_DW32_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW32_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW32_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW32_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW32;
    union
    {
        struct
        {
            uint32_t VDENC_CMD1_DW33_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW33_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW33_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW33_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW33;
    union
    {
        struct
        {
            uint32_t VDENC_CMD1_DW34_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW34_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW34_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW34_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW34;
    union
    {
        struct
        {
            uint32_t VDENC_CMD1_DW35_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW35_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW35_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW35_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW35;
    union
    {
        struct
        {
            uint32_t VDENC_CMD1_DW36_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW36_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW36_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW36_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW36;
    union
    {
        struct
        {
            uint32_t VDENC_CMD1_DW37_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW37_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW37_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW37_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW37;
    union
    {
        struct
        {
            uint32_t VDENC_CMD1_DW38_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD1_DW38_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD1_DW38_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD1_DW38_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW38;
    //! \name Local enumerations

    enum SUBOPB
    {
        SUBOPB_VDENCCMD1CMD = 10,  //!< No additional details
    };

    enum SUBOPA
    {
        SUBOPA_UNNAMED0 = 0,  //!< No additional details
    };

    enum OPCODE
    {
        OPCODE_VDENCPIPE = 1,  //!< No additional details
    };

    enum PIPELINE
    {
        PIPELINE_MFXCOMMON = 2,  //!< No additional details
    };

    enum COMMAND_TYPE
    {
        COMMAND_TYPE_PARALLELVIDEOPIPE = 3,  //!< No additional details
    };

    //! \name Initializations

    //! \brief Explicit member initialization function
    _VDENC_CMD1_CMD()
    {
        MOS_ZeroMemory(this, sizeof(*this));

        DW0.DwordLength = __CODEGEN_OP_LENGTH(dwSize);
        DW0.Subopb      = SUBOPB_VDENCCMD1CMD;
        DW0.Subopa      = SUBOPA_UNNAMED0;
        DW0.Opcode      = OPCODE_VDENCPIPE;
        DW0.Pipeline    = PIPELINE_MFXCOMMON;
        DW0.CommandType = COMMAND_TYPE_PARALLELVIDEOPIPE;
    }

    static const size_t dwSize   = 39;
    static const size_t byteSize = 156;
};

struct _VDENC_CMD3_CMD
{
    union
    {
        struct
        {
            uint32_t DwordLength : __CODEGEN_BITFIELD(0, 11);   //!< DWORD_LENGTH
            uint32_t Reserved12 : __CODEGEN_BITFIELD(12, 15);   //!< Reserved
            uint32_t Subopb : __CODEGEN_BITFIELD(16, 20);       //!< SUBOPB
            uint32_t Subopa : __CODEGEN_BITFIELD(21, 22);       //!< SUBOPA
            uint32_t Opcode : __CODEGEN_BITFIELD(23, 26);       //!< OPCODE
            uint32_t Pipeline : __CODEGEN_BITFIELD(27, 28);     //!< PIPELINE
            uint32_t CommandType : __CODEGEN_BITFIELD(29, 31);  //!< COMMAND_TYPE
        };
        uint32_t Value;
    } DW0;
    union
    {
        struct
        {
            uint32_t VDENC_CMD3_DW1_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD3_DW1_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD3_DW1_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD3_DW1_BIT24 : __CODEGEN_BITFIELD(24, 31);

            uint32_t VDENC_CMD3_DW2_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD3_DW2_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD3_DW2_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD3_DW2_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value[2];
    } DW1_2;
    uint8_t VDENC_CMD3_DW3_5[12];
    uint8_t VDENC_CMD3_DW6_8[12];
    union
    {
        struct
        {
            uint32_t VDENC_CMD3_DW9;
        };
        uint32_t Value;
    } DW9;
    union
    {
        struct
        {
            uint32_t VDENC_CMD3_DW10_BIT0 : __CODEGEN_BITFIELD(0, 15);
            uint32_t VDENC_CMD3_DW10_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD3_DW10_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW10;
    union
    {
        struct
        {
            uint32_t VDENC_CMD3_DW11;
        };
        uint32_t Value;
    } DW11;
    union
    {
        struct
        {
            uint32_t VDENC_CMD3_DW12_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD3_DW12_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD3_DW12_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD3_DW12_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW12;
    union
    {
        struct
        {
            uint32_t VDENC_CMD3_DW13_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD3_DW13_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD3_DW13_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD3_DW13_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW13;
    union
    {
        struct
        {
            uint32_t VDENC_CMD3_DW14_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD3_DW14_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD3_DW14_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD3_DW14_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW14;
    union
    {
        struct
        {
            uint32_t VDENC_CMD3_DW15_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD3_DW15_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD3_DW15_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD3_DW15_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW15;
    union
    {
        struct
        {
            uint32_t VDENC_CMD3_DW16_BIT0 : __CODEGEN_BITFIELD(0, 15);
            uint32_t VDENC_CMD3_DW16_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD3_DW16_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW16;
    union
    {
        struct
        {
            uint32_t VDENC_CMD3_DW17_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD3_DW17_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD3_DW17_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD3_DW17_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW17;
    union
    {
        struct
        {
            uint32_t VDENC_CMD3_DW18;
        };
        uint32_t Value;
    } DW18;
    union
    {
        struct
        {
            uint32_t VDENC_CMD3_DW19_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD3_DW19_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD3_DW19_BIT16 : __CODEGEN_BITFIELD(16, 23);
            uint32_t VDENC_CMD3_DW19_BIT24 : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW19;
    union
    {
        struct
        {
            uint32_t VDENC_CMD3_DW20_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD3_DW20_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD3_DW20_BIT16 : __CODEGEN_BITFIELD(16, 31);
        };
        uint32_t Value;
    } DW20;
    union
    {
        struct
        {
            uint32_t VDENC_CMD3_DW21_BIT0 : __CODEGEN_BITFIELD(0, 7);
            uint32_t VDENC_CMD3_DW21_BIT8 : __CODEGEN_BITFIELD(8, 15);
            uint32_t VDENC_CMD3_DW21_BIT16 : __CODEGEN_BITFIELD(16, 31);
        };
        uint32_t Value;
    } DW21;
    union
    {
        struct
        {
            uint32_t VDENC_CMD3_DW22_BIT0 : __CODEGEN_BITFIELD(0, 15);
            uint32_t VDENC_CMD3_DW22_BIT16 : __CODEGEN_BITFIELD(16, 24);
            uint32_t VDENC_CMD3_DW22_BIT25 : __CODEGEN_BITFIELD(25, 31);
        };
        uint32_t Value;
    } DW22;

    //! \name Initializations

    //! \brief Explicit member initialization function
    _VDENC_CMD3_CMD()
    {
        MOS_ZeroMemory(this, sizeof(*this));

        DW0.Value = 0x708a0015;
    }

    static const size_t dwSize   = 23;
    static const size_t byteSize = 92;
};

#ifdef IGFX_VDENC_INTERFACE_EXT_SUPPORT
#include "mhw_vdbox_vdenc_hwcmd_xe2_lpm_ext.h"
#else
struct _VDENC_CMD2_CMD
{
    union
    {
        //!< DWORD 0
        struct
        {
            uint32_t DwordLength : __CODEGEN_BITFIELD(0, 11);   //!< DWord Length
            uint32_t Reserved12 : __CODEGEN_BITFIELD(12, 15);   //!< Reserved
            uint32_t Subopb : __CODEGEN_BITFIELD(16, 20);       //!< SUBOPB
            uint32_t Subopa : __CODEGEN_BITFIELD(21, 22);       //!< SUBOPA
            uint32_t Opcode : __CODEGEN_BITFIELD(23, 26);       //!< OPCODE
            uint32_t Pipeline : __CODEGEN_BITFIELD(27, 28);     //!< PIPELINE
            uint32_t CommandType : __CODEGEN_BITFIELD(29, 31);  //!< COMMAND_TYPE
        };
        uint32_t Value;
    } DW0;
    union
    {
        //!< DWORD 1
        struct
        {
            uint32_t FrameWidthInPixelsMinusOne : __CODEGEN_BITFIELD(0, 15);    //!< FrameWidthInPixelsMinusOne
            uint32_t FrameHeightInPixelsMinusOne : __CODEGEN_BITFIELD(16, 31);  //!< FrameHeightInPixelsMinusOne
        };
        uint32_t Value;
    } DW1;
    union
    {
        //!< DWORD 2
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 19);
            uint32_t PictureType : __CODEGEN_BITFIELD(20, 21);               //!< Picture Type
            uint32_t TemporalMvpEnableFlag : __CODEGEN_BITFIELD(22, 22);     //!< TemporalMvpEnableFlag
            uint32_t Collocatedfroml0Flag : __CODEGEN_BITFIELD(23, 23);      //!< CollocatedFromL0Flag
            uint32_t LongTermReferenceFlagsL0 : __CODEGEN_BITFIELD(24, 26);  //!< LongTermReferenceFlags_L0
            uint32_t LongTermReferenceFlagsL1 : __CODEGEN_BITFIELD(27, 27);  //!< LongTermReferenceFlags_L1
            uint32_t : __CODEGEN_BITFIELD(28, 29);
            uint32_t TransformSkip : __CODEGEN_BITFIELD(30, 30);             //!< TransformSkip
            uint32_t ConstrainedIntraPredFlag : __CODEGEN_BITFIELD(31, 31);  //!< ConstrainedIntraPredFlag
        };
        uint32_t Value;
    } DW2;
    union
    {
        //!< DWORD 3
        struct
        {
            uint32_t FwdPocNumberForRefid0InL0 : __CODEGEN_BITFIELD(0, 7);   //!< FWD_POC_NUMBER_FOR_REFID_0_IN_L0
            uint32_t BwdPocNumberForRefid0InL1 : __CODEGEN_BITFIELD(8, 15);  //!< BWD_POC_NUMBER_FOR_REFID_0_IN_L1
            uint32_t PocNumberForRefid1InL0 : __CODEGEN_BITFIELD(16, 23);    //!< POC_NUMBER_FOR_REFID_1_IN_L0
            uint32_t PocNumberForRefid1InL1 : __CODEGEN_BITFIELD(24, 31);    //!< POC_NUMBER_FOR_REFID_1_IN_L1
        };
        uint32_t Value;
    } DW3;
    union
    {
        //!< DWORD 4
        struct
        {
            uint32_t PocNumberForRefid2InL0 : __CODEGEN_BITFIELD(0, 7);    //!< FWD_POC_NUMBER_FOR_REFID_2_IN_L0
            uint32_t PocNumberForRefid2InL1 : __CODEGEN_BITFIELD(8, 15);   //!< BWD_POC_NUMBER_FOR_REFID_2_IN_L1
            uint32_t PocNumberForRefid3InL0 : __CODEGEN_BITFIELD(16, 23);  //!< POC_NUMBER_FOR_REFID_3_IN_L0
            uint32_t PocNumberForRefid3InL1 : __CODEGEN_BITFIELD(24, 31);  //!< POC_NUMBER_FOR_REFID_3_IN_L1
        };
        uint32_t Value;
    } DW4;
    union
    {
        //!< DWORD 5
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 7);
            uint32_t StreaminRoiEnable : __CODEGEN_BITFIELD(8, 8);  //!< StreamIn ROI Enable
            uint32_t : __CODEGEN_BITFIELD(9, 9);
            uint32_t SubPelMode : __CODEGEN_BITFIELD(10, 11);  //!< SubPelMode
            uint32_t : __CODEGEN_BITFIELD(12, 23);
            uint32_t NumRefIdxL0Minus1 : __CODEGEN_BITFIELD(24, 27);  //!< NumRefIdxL0_minus1
            uint32_t NumRefIdxL1Minus1 : __CODEGEN_BITFIELD(28, 31);  //!< NumRefIdxL1_minus1
        };
        uint32_t Value;
    } DW5;
    union
    {
        //!< DWORD 6
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW6;
    union
    {
        //!< DWORD 7
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 3);
            uint32_t SegmentationEnable : __CODEGEN_BITFIELD(4, 4);                       //!< Segmentation Enable
            uint32_t SegmentationMapTemporalPredictionEnable : __CODEGEN_BITFIELD(5, 5);  //!< Segmentation map temporal prediction enable
            uint32_t : __CODEGEN_BITFIELD(6, 6);
            uint32_t TilingEnable : __CODEGEN_BITFIELD(7, 7);  //!< Tiling enable
            uint32_t : __CODEGEN_BITFIELD(8, 8);
            uint32_t VdencStreamInEnable : __CODEGEN_BITFIELD(9, 9);  //!< VDENC Stream IN
            uint32_t : __CODEGEN_BITFIELD(10, 15);
            uint32_t PakOnlyMultiPassEnable : __CODEGEN_BITFIELD(16, 16);  //!< PAK-Only Multi-Pass Enable
            uint32_t : __CODEGEN_BITFIELD(17, 31);
        };
        uint32_t Value;
    } DW7;
    union
    {
        //!< DWORD 8
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW8;
    union
    {
        //!< DWORD 9
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW9;
    union
    {
        //!< DWORD 10
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW10;
    union
    {
        //!< DWORD 11
        struct
        {
            uint32_t FwdRef0RefPic : __CODEGEN_BITFIELD(0, 2);
            uint32_t : __CODEGEN_BITFIELD(3, 7);
            uint32_t FwdRef1RefPic : __CODEGEN_BITFIELD(8, 10);
            uint32_t : __CODEGEN_BITFIELD(11, 15);
            uint32_t FwdRef2RefPic : __CODEGEN_BITFIELD(16, 18);
            uint32_t : __CODEGEN_BITFIELD(19, 23);
            uint32_t BwdRef0RefPic : __CODEGEN_BITFIELD(24, 26);
            uint32_t : __CODEGEN_BITFIELD(27, 31);
        };
        uint32_t Value;
    } DW11;
    union
    {
        //!< DWORD 12
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW12;
    union
    {
        //!< DWORD 13
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW13;
    union
    {
        //!< DWORD 14
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW14;
    union
    {
        //!< DWORD 15
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW15;
    union
    {
        //!< DWORD 16
        struct
        {
            uint32_t MinQp : __CODEGEN_BITFIELD(0, 7);   //!< MINQP
            uint32_t MaxQp : __CODEGEN_BITFIELD(8, 15);  //!< MAXQP
            uint32_t : __CODEGEN_BITFIELD(16, 31);
        };
        uint32_t Value;
    } DW16;
    union
    {
        //!< DWORD 17
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 19);
            uint32_t TemporalMVEnableForIntegerSearch : __CODEGEN_BITFIELD(20, 20);  //!< Setting this bit enables Temporal MV Enable for Integer search
            uint32_t : __CODEGEN_BITFIELD(21, 31);
        };
        uint32_t Value;
    } DW17;
    union
    {
        //!< DWORD 18
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW18;
    union
    {
        //!< DWORD 19
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW19;
    union
    {
        //!< DWORD 20
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW20;
    union
    {
        //!< DWORD 21
        struct
        {
            uint32_t IntraRefreshPos : __CODEGEN_BITFIELD(0, 8);               //!< IntraRefreshPos
            uint32_t : __CODEGEN_BITFIELD(9, 15);
            uint32_t IntraRefreshMBSizeMinusOne : __CODEGEN_BITFIELD(16, 23);  //!< IntraRefreshMBSizeMinusOne
            uint32_t IntraRefreshMode : __CODEGEN_BITFIELD(24, 24);            //!< IntraRefreshMode
            uint32_t IntraRefreshEnable : __CODEGEN_BITFIELD(25, 25);          //!< IntraRefreshEnable (Rolling I Enable)
            uint32_t : __CODEGEN_BITFIELD(26, 27);
            uint32_t QpAdjustmentForRollingI : __CODEGEN_BITFIELD(28, 31);     //!< QP_ADJUSTMENT_FOR_ROLLING_I
        };
        uint32_t Value;
    } DW21;
    union
    {
        //!< DWORD 22
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW22;
    union
    {
        //!< DWORD 23
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW23;
    union
    {
        //!< DWORD 24
        struct
        {
            uint32_t QpForSeg0 : __CODEGEN_BITFIELD(0, 7);    //!< QP for Seg0
            uint32_t QpForSeg1 : __CODEGEN_BITFIELD(8, 15);   //!< QP for Seg1
            uint32_t QpForSeg2 : __CODEGEN_BITFIELD(16, 23);  //!< QP for Seg2
            uint32_t QpForSeg3 : __CODEGEN_BITFIELD(24, 31);  //!< QP for Seg3
        };
        uint32_t Value;
    } DW24;
    union
    {
        //!< DWORD 25
        struct
        {
            uint32_t QpForSeg4 : __CODEGEN_BITFIELD(0, 7);    //!< QP for Seg4
            uint32_t QpForSeg5 : __CODEGEN_BITFIELD(8, 15);   //!< QP for Seg5
            uint32_t QpForSeg6 : __CODEGEN_BITFIELD(16, 23);  //!< QP for Seg6
            uint32_t QpForSeg7 : __CODEGEN_BITFIELD(24, 31);  //!< QP for Seg7
        };
        uint32_t Value;
    } DW25;
    union
    {
        //!< DWORD 26
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 24);
            uint32_t Vp9DynamicSliceEnable : __CODEGEN_BITFIELD(25, 25);  //!< VP9 Dynamic slice enable
            uint32_t : __CODEGEN_BITFIELD(26, 31);
        };
        uint32_t Value;
    } DW26;
    union
    {
        //!< DWORD 27
        struct
        {
            uint32_t QpPrimeYDc : __CODEGEN_BITFIELD(0, 7);   //!< QPPRIMEY_DC
            uint32_t QpPrimeYAc : __CODEGEN_BITFIELD(8, 15);  //!< QPPRIMEY_AC
            uint32_t : __CODEGEN_BITFIELD(16, 31);
        };
        uint32_t Value;
    } DW27;
    union
    {
        //!< DWORD 28
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW28;
    union
    {
        //!< DWORD 29
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW29;
    union
    {
        //!< DWORD 30
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW30;
    union
    {
        //!< DWORD 31
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW31;
    union
    {
        //!< DWORD 32
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW32;
    union
    {
        //!< DWORD 33
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW33;
    union
    {
        //!< DWORD 34
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW34;
    union
    {
        //!< DWORD 35
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW35;
    union
    {
        //!< DWORD 36
        struct
        {
            uint32_t IntraRefreshBoundaryRef0 : __CODEGEN_BITFIELD(0, 8);    //!< IntraRefreshBoundary Ref0
            uint32_t : __CODEGEN_BITFIELD(9, 9);
            uint32_t IntraRefreshBoundaryRef1 : __CODEGEN_BITFIELD(10, 18);  //!< IntraRefreshBoundary Ref1
            uint32_t : __CODEGEN_BITFIELD(19, 19);
            uint32_t IntraRefreshBoundaryRef2 : __CODEGEN_BITFIELD(20, 28);  //!< IntraRefreshBoundary Ref2
            uint32_t : __CODEGEN_BITFIELD(29, 31);
        };
        uint32_t Value;
    } DW36;
    union
    {
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW37;
    union
    {
        //!< DWORD 38
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW38;
    union
    {
        //!< DWORD 39
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW39;
    union
    {
        //!< DWORD 40
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW40;
    union
    {
        //!< DWORD 41
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW41;
    union
    {
        //!< DWORD 42
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW42;
    union
    {
        //!< DWORD 43
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW43;
    union
    {
        //!< DWORD 44
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW44;
    union
    {
        //!< DWORD 45
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW45;
    union
    {
        //!< DWORD 46
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW46;
    union
    {
        //!< DWORD 47
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW47;
    union
    {
        //!< DWORD 48
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW48;
    union
    {
        //!< DWORD 49
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW49;
    union
    {
        //!< DWORD 50
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW50;
    union
    {
        //!< DWORD 51
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW51;
    union
    {
        //!< DWORD 52
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW52;
    union
    {
        //!< DWORD 53
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW53;
    union
    {
        //!< DWORD 54
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW54;
    union
    {
        //!< DWORD 55
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW55;
    union
    {
        //!< DWORD 56
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW56;
    union
    {
        //!< DWORD 57
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW57;
    union
    {
        //!< DWORD 58
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW58;
    union
    {
        //!< DWORD 59
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW59;
    union
    {
        //!< DWORD 60
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW60;
    union
    {
        //!< DWORD 61
        struct
        {
            uint32_t Av1L0RefID0 : __CODEGEN_BITFIELD(0, 3);
            uint32_t Av1L1RefID0 : __CODEGEN_BITFIELD(4, 7);
            uint32_t Av1L0RefID1 : __CODEGEN_BITFIELD(8, 11);
            uint32_t Av1L1RefID1 : __CODEGEN_BITFIELD(12, 15);
            uint32_t Av1L0RefID2 : __CODEGEN_BITFIELD(16, 19);
            uint32_t Av1L1RefID2 : __CODEGEN_BITFIELD(20, 23);
            uint32_t Av1L0RefID3 : __CODEGEN_BITFIELD(24, 27);
            uint32_t Av1L1RefID3 : __CODEGEN_BITFIELD(28, 31);
        };
        uint32_t Value;
    } DW61;
    //! \name Local enumerations

    enum SUBOPB
    {
        SUBOPB_VDENCCMD2CMD = 9,  //!< No additional details
    };

    enum SUBOPA
    {
        SUBOPA_UNNAMED0 = 0,  //!< No additional details
    };

    enum OPCODE
    {
        OPCODE_VDENCPIPE = 1,  //!< No additional details
    };

    enum PIPELINE
    {
        PIPELINE_MFXCOMMON = 2,  //!< No additional details
    };

    enum COMMAND_TYPE
    {
        COMMAND_TYPE_PARALLELVIDEOPIPE = 3,  //!< No additional details
    };

    //! \name Initializations

    //! \brief Explicit member initialization function
    _VDENC_CMD2_CMD()
    {
        MOS_ZeroMemory(this, sizeof(*this));

        DW0.Value       = 0;
        DW0.DwordLength = __CODEGEN_OP_LENGTH(dwSize);
        DW0.Subopb      = SUBOPB_VDENCCMD2CMD;
        DW0.Subopa      = SUBOPA_UNNAMED0;
        DW0.Opcode      = OPCODE_VDENCPIPE;
        DW0.Pipeline    = PIPELINE_MFXCOMMON;
        DW0.CommandType = COMMAND_TYPE_PARALLELVIDEOPIPE;
    }

    static const size_t dwSize   = 62;
    static const size_t byteSize = 248;
};

struct _VDENC_AVC_IMG_STATE_CMD
{
    union
    {
        struct
        {
            uint32_t DwordLength : __CODEGEN_BITFIELD(0, 11);   //!< DWORD_LENGTH
            uint32_t Reserved12 : __CODEGEN_BITFIELD(12, 15);   //!< Reserved
            uint32_t Subopb : __CODEGEN_BITFIELD(16, 20);       //!< SUBOPB
            uint32_t Subopa : __CODEGEN_BITFIELD(21, 22);       //!< SUBOPA
            uint32_t Opcode : __CODEGEN_BITFIELD(23, 26);       //!< OPCODE
            uint32_t Pipeline : __CODEGEN_BITFIELD(27, 28);     //!< PIPELINE
            uint32_t CommandType : __CODEGEN_BITFIELD(29, 31);  //!< COMMAND_TYPE
        };
        uint32_t Value;
    } DW0;
    union
    {
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 1);
            uint32_t PictureType : __CODEGEN_BITFIELD(2, 3);
            uint32_t Transform8X8Flag : __CODEGEN_BITFIELD(4, 4);
            uint32_t colloc_mv_wr_en : __CODEGEN_BITFIELD(5, 5);
            uint32_t SubpelMode : __CODEGEN_BITFIELD(6, 7);
            uint32_t : __CODEGEN_BITFIELD(8, 31);
        };
        uint32_t Value;
    } DW1;
    union
    {
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 12);
            uint32_t colloc_mv_rd_en : __CODEGEN_BITFIELD(13, 13);
            uint32_t : __CODEGEN_BITFIELD(14, 17);
            uint32_t BidirectionalWeight : __CODEGEN_BITFIELD(18, 23);
            uint32_t : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW2;
    union
    {
        struct
        {
            uint32_t PictureHeightMinusOne : __CODEGEN_BITFIELD(0, 7);
            uint32_t : __CODEGEN_BITFIELD(8,  15);
            uint32_t PictureWidth : __CODEGEN_BITFIELD(16, 24);
            uint32_t : __CODEGEN_BITFIELD(25, 31);
        };
        uint32_t Value;
    } DW3;
    union
    {
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW4;
    union
    {
        struct
        {
            uint32_t FwdRefIdx0ReferencePicture : __CODEGEN_BITFIELD(0, 3);
            uint32_t BwdRefIdx0ReferencePicture : __CODEGEN_BITFIELD(4, 7);
            uint32_t FwdRefIdx1ReferencePicture : __CODEGEN_BITFIELD(8, 11);
            uint32_t : __CODEGEN_BITFIELD(12, 15);
            uint32_t FwdRefIdx2ReferencePicture : __CODEGEN_BITFIELD(16, 19);
            uint32_t NumberOfL0ReferencesMinusOne : __CODEGEN_BITFIELD(20, 23);
            uint32_t NumberOfL1ReferencesMinusOne : __CODEGEN_BITFIELD(24, 26);
            uint32_t : __CODEGEN_BITFIELD(27, 31);
        };
        uint32_t Value;
    } DW5;
    union
    {
        struct
        {
            uint32_t IntraRefreshMbPos : __CODEGEN_BITFIELD(0, 7);
            uint32_t IntraRefreshMbSizeMinusOne : __CODEGEN_BITFIELD(8, 15);
            uint32_t IntraRefreshEnableRollingIEnable : __CODEGEN_BITFIELD(16, 16);
            uint32_t IntraRefreshMode : __CODEGEN_BITFIELD(17, 17);
            uint32_t : __CODEGEN_BITFIELD(18, 23);
            uint32_t QpAdjustmentForRollingI : __CODEGEN_BITFIELD(24, 31);
        };
        uint32_t Value;
    } DW6;
    union
    {
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW7;
    union
    {
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW8;
    union
    {
        struct
        {
            uint32_t RoiQpAdjustmentForZone0 : __CODEGEN_BITFIELD(0, 3);
            uint32_t RoiQpAdjustmentForZone1 : __CODEGEN_BITFIELD(4, 7);
            uint32_t RoiQpAdjustmentForZone2 : __CODEGEN_BITFIELD(8, 11);
            uint32_t RoiQpAdjustmentForZone3 : __CODEGEN_BITFIELD(12, 15);
            uint32_t : __CODEGEN_BITFIELD(16, 31);
        };
        uint32_t Value;
    } DW9;
    union
    {
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW10;
    union
    {
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW11;
    union
    {
        struct
        {
            uint32_t MinQp : __CODEGEN_BITFIELD(0, 7);
            uint32_t MaxQp : __CODEGEN_BITFIELD(8, 15);
            uint32_t : __CODEGEN_BITFIELD(16, 28);
            uint32_t NumBframesBetweenFwdAndBwd : __CODEGEN_BITFIELD(29, 30);
            uint32_t : __CODEGEN_BITFIELD(31, 31);
        };
        uint32_t Value;
    } DW12;
    union
    {
        struct
        {
            uint32_t RoiEnable : __CODEGEN_BITFIELD(0, 0);
            uint32_t : __CODEGEN_BITFIELD(1, 2);
            uint32_t MbLevelQpEnable : __CODEGEN_BITFIELD(3, 3);
            uint32_t : __CODEGEN_BITFIELD(4, 4);
            uint32_t MbLevelDeltaQpEnable : __CODEGEN_BITFIELD(5, 5);
            uint32_t : __CODEGEN_BITFIELD(6, 9);
            uint32_t LongtermReferenceFrameBwdRef0Indicator : __CODEGEN_BITFIELD(10, 10);
            uint32_t : __CODEGEN_BITFIELD(11, 31);
        };
        uint32_t Value;
    } DW13;
    union
    {
        struct
        {
            uint32_t QpPrimeY : __CODEGEN_BITFIELD(0, 7);
            uint32_t : __CODEGEN_BITFIELD(8, 18);
            uint32_t TrellisQuantEn : __CODEGEN_BITFIELD(19, 19);
            uint32_t : __CODEGEN_BITFIELD(20, 31);
        };
        uint32_t Value;
    } DW14;
    union
    {
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 7);
            uint32_t PocNumberForCurrentPicture : __CODEGEN_BITFIELD(8, 15);
            uint32_t : __CODEGEN_BITFIELD(16, 31);
        };
        uint32_t Value;
    } DW15;
    union
    {
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 7);
            uint32_t PocNumberForFwdRef0 : __CODEGEN_BITFIELD(8, 15);
            uint32_t : __CODEGEN_BITFIELD(16, 31);
        };
        uint32_t Value;
    } DW16;
    union
    {
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 7);
            uint32_t PocNumberForFwdRef1 : __CODEGEN_BITFIELD(8, 15);
            uint32_t : __CODEGEN_BITFIELD(16, 31);
        };
        uint32_t Value;
    } DW17;
    union
    {
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 7);
            uint32_t PocNumberForFwdRef2 : __CODEGEN_BITFIELD(8, 15);
            uint32_t : __CODEGEN_BITFIELD(16, 31);
        };
        uint32_t Value;
    } DW18;
    union
    {
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 7);
            uint32_t PocNumberForBwdRef0 : __CODEGEN_BITFIELD(8, 15);
            uint32_t : __CODEGEN_BITFIELD(16, 31);
        };
        uint32_t Value;
    } DW19;
    union
    {
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW20;
    union
    {
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW21;
    union
    {
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW22;
    union
    {
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW23;
    union
    {
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW24;
    union
    {
        struct
        {
            uint32_t : __CODEGEN_BITFIELD(0, 31);
        };
        uint32_t Value;
    } DW25;

    //! \name Initializations

    //! \brief Explicit member initialization function
    _VDENC_AVC_IMG_STATE_CMD()
    {
        MOS_ZeroMemory(this, sizeof(*this));
        DW0.Value                                        = 0x70850018;

    }

    static const size_t dwSize   = 26;
    static const size_t byteSize = 104;
};

#endif

class Cmd
{
public:
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
        VDENC_64B_Aligned_Lower_Address_CMD()
        {
            DW0.Value = 0;
        } 

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
            struct
            {
                uint32_t                 AddressUpperDword                                                                ; //!< Address Upper DWord
            };
            uint32_t                     Value;
        } DW0;

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_64B_Aligned_Upper_Address_CMD()
        {
            DW0.Value = 0;
        }

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
            struct
            {
                uint32_t VDENC_Surface_Control_Bits_DW0_BIT0 : __CODEGEN_BITFIELD(0, 0);  
                uint32_t MemoryObjectControlState : __CODEGEN_BITFIELD(1, 6);             //!< Index to Memory Object Control State (MOCS) Tables:
                uint32_t ArbitrationPriorityControl : __CODEGEN_BITFIELD(7, 8);           //!< ARBITRATION_PRIORITY_CONTROL
                uint32_t MemoryCompressionEnable : __CODEGEN_BITFIELD(9, 9);
                uint32_t CompressionType : __CODEGEN_BITFIELD(10, 10);                    //!< Compression Type
                uint32_t Reserved11 : __CODEGEN_BITFIELD(11, 11);                         //!< Reserved
                uint32_t CacheSelect: __CODEGEN_BITFIELD(12, 12);                         //!< CACHE_SELECT
                uint32_t Reserved13 : __CODEGEN_BITFIELD(13, 15);                         //!< Reserved15_13
                uint32_t CompressionFormat : __CODEGEN_BITFIELD(16, 19);                  //!< COMPRESSION_FORMAT
                uint32_t Reserved20 : __CODEGEN_BITFIELD(20, 31);                         //!< Reserved
            };
            uint32_t Value;
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
            MEMORY_COMPRESSION_ENABLE_DISABLE = 0,  //!< No additional details
            MEMORY_COMPRESSION_ENABLE_ENABLE  = 1,  //!< No additional details
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

        //! \brief COMPRESSION_FORMAT
        //! \details
        //! 
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
        VDENC_Surface_Control_Bits_CMD()
        {
            DW0.Value                      = 0;
            DW0.ArbitrationPriorityControl = ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;
            DW0.MemoryCompressionEnable    = MEMORY_COMPRESSION_ENABLE_DISABLE;
            DW0.CacheSelect                = CACHE_SELECT_UNNAMED0;
        }

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
        VDENC_Sub_Mb_Pred_Mode_CMD()
        {
            DW0.Value = 0;
        }

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
        VDENC_Block_8x8_4_CMD()
        {
            DW0.Value = 0;
        }

        static const size_t dwSize = 0;
        static const size_t byteSize = 2;
    };

    //!
    //! \brief VDENC_Delta_MV_XY
    //! \details
    //!     
    //!     
    //!     Calculates the difference between the actual MV for the Sub Macroblock
    //!     and the predicted MV based onthe availability of the neighbors.
    //!     
    //!     This is calculated and populated for Inter frames only. In case of an
    //!     Intra MB in Inter frames, thisvalue should be 0.
    //!     
    struct VDENC_Delta_MV_XY_CMD
    {
        union
        {
            struct
            {
                uint32_t                 X0                                               : __CODEGEN_BITFIELD( 0, 15)    ; //!< X0
                uint32_t                 Y0                                               : __CODEGEN_BITFIELD(16, 31)    ; //!< Y0
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 X1                                               : __CODEGEN_BITFIELD( 0, 15)    ; //!< X1
                uint32_t                 Y1                                               : __CODEGEN_BITFIELD(16, 31)    ; //!< Y1
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 X2                                               : __CODEGEN_BITFIELD( 0, 15)    ; //!< X2
                uint32_t                 Y2                                               : __CODEGEN_BITFIELD(16, 31)    ; //!< Y2
            };
            uint32_t                     Value;
        } DW2;
        union
        {
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
        //!     <table>Calculates the x-axis MV for Picture List N for
        //!     the following:<tr>  <th>Mb_type</th>
        //!      <th>PartID</th></tr><tr>  <td
        //!     align="center">16x16</td>  <td align="center">0</td>
        //!        </tr><tr>  <td align="center">16x8</td>  
        //!        <td align="center">1</td></tr><tr>   
        //!       <td align="center">8x16</td>  <td
        //!     align="center">3</td></tr><tr>  <td
        //!     align="center">8x8</td>  <td align="center">5</td>  
        //!      </tr>  </table>
        enum X0
        {
            X0_UNNAMED0                                                      = 0, //!< No additional details
        };

        //! \brief Y0
        //! \details
        //!     <table>Calculates the y-axis MV for Picture List N for
        //!     the following:<tr>  <th>Mb_type</th>
        //!      <th>PartID</th></tr><tr>  <td
        //!     align="center">16x16</td>  <td align="center">0</td>
        //!        </tr><tr>  <td align="center">16x8</td>  
        //!        <td align="center">1</td></tr><tr>   
        //!       <td align="center">8x16</td>  <td
        //!     align="center">3</td></tr><tr>  <td
        //!     align="center">8x8</td>  <td align="center">5</td>  
        //!      </tr>  </table>
        enum Y0
        {
            Y0_UNNAMED0                                                      = 0, //!< No additional details
        };

        //! \brief X1
        //! \details
        //!     <table>Calculates the x-axis MV for Picture List N for
        //!     the following:<tr>  <th>Mb_type</th>
        //!      <th>PartID</th></tr><tr>  <td
        //!     align="center">16x8</td>  <td align="center">2</td> 
        //!       </tr><tr>  <td align="center">8x16</td>   
        //!       <td align="center">4</td></tr><tr>
        //!      <td align="center">8x8</td>  <td align="center">6</td> 
        //!       </tr>  </table>
        enum X1
        {
            X1_UNNAMED0                                                      = 0, //!< No additional details
        };

        //! \brief Y1
        //! \details
        //!     <table>Calculates the y-axis MV for Picture List N for
        //!     the following:<tr>  <th>Mb_type</th>
        //!      <th>PartID</th></tr><tr>  <td
        //!     align="center">16x8</td>  <td align="center">2</td> 
        //!       </tr><tr>  <td align="center">8x16</td>   
        //!       <td align="center">4</td></tr><tr>
        //!      <td align="center">8x8</td>  <td align="center">6</td> 
        //!       </tr>  </table>
        enum Y1
        {
            Y1_UNNAMED0                                                      = 0, //!< No additional details
        };

        //! \brief X2
        //! \details
        //!     <table>Calculates the x-axis MV for Picture List N for
        //!     the following:<tr>  <th>Mb_type</th>
        //!      <th>PartID</th></tr><tr>  <td
        //!     align="center">8x8</td>  <td align="center">7</td>  
        //!      </tr>  </table>
        enum X2
        {
            X2_UNNAMED0                                                      = 0, //!< No additional details
        };

        //! \brief Y2
        //! \details
        //!     <table>Calculates the y-axis MV for Picture List N for
        //!     the following:<tr>  <th>Mb_type</th>
        //!      <th>PartID</th></tr><tr>  <td
        //!     align="center">8x8</td>  <td align="center">7</td>  
        //!      </tr>  </table>
        enum Y2
        {
            Y2_UNNAMED0                                                      = 0, //!< No additional details
        };

        //! \brief X3
        //! \details
        //!     <table>Calculates the x-axis MV for Picture List N for
        //!     the following:<tr>  <th>Mb_type</th>
        //!      <th>PartID</th></tr><tr>  <td
        //!     align="center">8x8</td>  <td align="center">8</td>  
        //!      </tr>  </table>
        enum X3
        {
            X3_UNNAMED0                                                      = 0, //!< No additional details
        };

        //! \brief Y3
        //! \details
        //!     <table>Calculates the y-axis MV for Picture List N for
        //!     the following:<tr>  <th>Mb_type</th>
        //!      <th>PartID</th></tr><tr>  <td
        //!     align="center">8x8</td>  <td align="center">8</td>  
        //!      </tr>  </table>
        enum Y3
        {
            Y3_UNNAMED0                                                      = 0, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_Delta_MV_XY_CMD()
        {
            MOS_ZeroMemory(this, sizeof(*this));

            DW0.X0 = X0_UNNAMED0;
            DW0.Y0 = Y0_UNNAMED0;

            DW1.X1 = X1_UNNAMED0;
            DW1.Y1 = Y1_UNNAMED0;

            DW2.X2 = X2_UNNAMED0;
            DW2.Y2 = Y2_UNNAMED0;

            DW3.X3 = X3_UNNAMED0;
            DW3.Y3 = Y3_UNNAMED0;
        }

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
        VDENC_64B_Aligned_Lower_Address_CMD      LowerAddress;                                                            //!< DW0, Lower Address
        VDENC_64B_Aligned_Upper_Address_CMD      UpperAddress;                                                            //!< DW1, Upper Address
        VDENC_Surface_Control_Bits_CMD           PictureFields;                                                           //!< DW2, Picture Fields

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_Colocated_MV_Picture_CMD()
        {
            MOS_ZeroMemory(this, sizeof(*this));
        }

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
        VDENC_64B_Aligned_Lower_Address_CMD      LowerAddress;                                                            //!< DW0, Lower Address
        VDENC_64B_Aligned_Upper_Address_CMD      UpperAddress;                                                            //!< DW1, Upper Address
        VDENC_Surface_Control_Bits_CMD           PictureFields;                                                           //!< DW2, Picture Fields

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_Down_Scaled_Reference_Picture_CMD()
        {
            MOS_ZeroMemory(this, sizeof(*this));
        }

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
            struct
            {
                uint32_t                 AccumulatedDistortion                                                            ; //!< Accumulated distortion
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 IntraIso16X16MbCount                             : __CODEGEN_BITFIELD( 0, 15)    ; //!< Intra iso 16x16 MB count
                uint32_t                 IntraMbCount                                     : __CODEGEN_BITFIELD(16, 31)    ; //!< Intra MB count
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 IntraIso4X4MbCount                               : __CODEGEN_BITFIELD( 0, 15)    ; //!< Intra iso 4x4 MB count
                uint32_t                 IntraIso8X8MbCount                               : __CODEGEN_BITFIELD(16, 31)    ; //!< Intra iso 8x8 MB count
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 SegmentMapCount0                                 : __CODEGEN_BITFIELD( 0, 15)    ; //!< segment map count 0
                uint32_t                 SegmentMapCount1                                 : __CODEGEN_BITFIELD(16, 31)    ; //!< segment map count 1
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 SegmentMapCount2                                 : __CODEGEN_BITFIELD( 0, 15)    ; //!< segment map count 2
                uint32_t                 SegmentMapCount3                                 : __CODEGEN_BITFIELD(16, 31)    ; //!< segment map count 3
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            struct
            {
                uint32_t                 AccumulatedSad                                                                   ; //!< Accumulated_SAD
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            struct
            {
                uint32_t                 AccumulatedSse                                                                   ; //!< Accumulated SSE
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            struct
            {
                uint32_t                 Reserved224                                                                      ; //!< Reserved
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            struct
            {
                uint32_t                 Reserved256                                                                      ; //!< Reserved
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            struct
            {
                uint32_t                 BottomPopulationSadAccumulated                                                   ; //!< bottom_population_sad_accumulated
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            struct
            {
                uint32_t                 TopPopulationSadAccumulated                                                      ; //!< top_population_sad_accumulated
            };
            uint32_t                     Value;
        } DW10;
        union
        {
            struct
            {
                uint32_t                 SegmentMapCount0                                 : __CODEGEN_BITFIELD( 0, 15)    ; //!< segment map count 0
                uint32_t                 BottomPopulationCount                            : __CODEGEN_BITFIELD(16, 31)    ; //!< bottom_population_count
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
                uint32_t                 Reserved416                                                                      ; //!< Reserved
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
        union
        {
            struct
            {
                uint32_t                 Reserved512                                                                      ; //!< Reserved
            };
            uint32_t                     Value;
        } DW16;
        union
        {
            struct
            {
                uint32_t                 BottomPopulationDistortionAccumulated                                            ; //!< bottom_population_distortion_accumulated
            };
            uint32_t                     Value;
        } DW17;
        union
        {
            struct
            {
                uint32_t                 TopPopulationDistortionAccumulated                                               ; //!< top_population_distortion_accumulated
            };
            uint32_t                     Value;
        } DW18;
        union
        {
            struct
            {
                uint32_t                 TopPopulationCount                               : __CODEGEN_BITFIELD( 0, 15)    ; //!< top_population_count
                uint32_t                 BottomPopulationCount                            : __CODEGEN_BITFIELD(16, 31)    ; //!< bottom_population_count
            };
            uint32_t                     Value;
        } DW19;
        //uint32_t                                 Reserved640[12];                                                         //!< Reserved

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_FRAME_BASED_STATISTICS_STREAMOUT_CMD()
        {
            MOS_ZeroMemory(this, sizeof(*this));
        }

        //static const size_t dwSize = 21;
        //static const size_t byteSize = 84;
        static const size_t dwSize   = 20;
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
            struct
            {
                uint32_t                 Skiprawdistortion                                : __CODEGEN_BITFIELD( 0, 15)    ; //!< SkipRawDistortion
                uint32_t                 Interrawdistortion                               : __CODEGEN_BITFIELD(16, 31)    ; //!< InterRawDistortion
            };
            uint32_t                     Value;
        } DW1;
        union
        {
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
            struct
            {
                uint32_t                 Lumaintramode0                                   : __CODEGEN_BITFIELD( 0, 15)    ; //!< LumaIntraMode[0]
                uint32_t                 Lumaintramode1                                   : __CODEGEN_BITFIELD(16, 31)    ; //!< LumaIntraMode[1]
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 Lumaintramode2                                   : __CODEGEN_BITFIELD( 0, 15)    ; //!< LumaIntraMode[2]
                uint32_t                 Lumaintramode3                                   : __CODEGEN_BITFIELD(16, 31)    ; //!< LumaIntraMode[3]
            };
            uint32_t                     Value;
        } DW4;
        VDENC_Delta_MV_XY_CMD                    DeltaMv0;                                                                //!< DW5..8, Delta MV0
        VDENC_Delta_MV_XY_CMD                    DeltaMv1;                                                                //!< DW9..12, Delta MV1
        union
        {
            struct
            {
                uint32_t                 FwdRefids                                        : __CODEGEN_BITFIELD( 0, 15)    ; //!< FWD REFIDs
                uint32_t                 BwdRefids                                        : __CODEGEN_BITFIELD(16, 31)    ; //!< BWD REFIDs
            };
            uint32_t                     Value;
        } DW13;
        union
        {
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
        //!     This field indicates the InterMB Parition type for Inter
        //!     MB.<br>OR</br>This field indicates Chroma Prediction Mode for Intra MB.
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
        VDENC_Mode_StreamOut_Data_CMD()
        {
            MOS_ZeroMemory(this, sizeof(*this));

            DW2.IntermbmodeChromaPredictionMode = INTERMBMODECHROMA_PREDICTION_MODE_UNNAMED0;
            DW2.Intrambmode                     = INTRAMBMODE_UNNAMED0;
            DW2.Intrambflag                     = INTRAMBFLAG_INTER;
            DW2.Lastmbflag                      = LASTMBFLAG_NOTLAST;
        }

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
        VDENC_64B_Aligned_Lower_Address_CMD      LowerAddress;                                                            //!< DW0, Lower Address
        VDENC_64B_Aligned_Upper_Address_CMD      UpperAddress;                                                            //!< DW1, Upper Address
        VDENC_Surface_Control_Bits_CMD           PictureFields;                                                           //!< DW2, Picture Fields

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_Original_Uncompressed_Picture_CMD()
        {
            MOS_ZeroMemory(this, sizeof(*this));
        }

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
        VDENC_64B_Aligned_Lower_Address_CMD      LowerAddress;                                                            //!< DW0, Lower Address
        VDENC_64B_Aligned_Upper_Address_CMD      UpperAddress;                                                            //!< DW1, Upper Address
        VDENC_Surface_Control_Bits_CMD           PictureFields;                                                           //!< DW2, Picture Fields

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_Reference_Picture_CMD()
        {
            MOS_ZeroMemory(this, sizeof(*this));
        }

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
        VDENC_64B_Aligned_Lower_Address_CMD      LowerAddress;                                                            //!< DW0, Lower Address
        VDENC_64B_Aligned_Upper_Address_CMD      UpperAddress;                                                            //!< DW1, Upper Address
        VDENC_Surface_Control_Bits_CMD           BufferPictureFields;                                                     //!< DW2, Buffer Picture Fields

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_Row_Store_Scratch_Buffer_Picture_CMD()
        {
            MOS_ZeroMemory(this, sizeof(*this));
        }

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
        VDENC_64B_Aligned_Lower_Address_CMD      LowerAddress;                                                            //!< DW0, Lower Address
        VDENC_64B_Aligned_Upper_Address_CMD      UpperAddress;                                                            //!< DW1, Upper Address
        VDENC_Surface_Control_Bits_CMD           PictureFields;                                                           //!< DW2, Picture Fields

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_Statistics_Streamout_CMD()
        {
            MOS_ZeroMemory(this, sizeof(*this));
        }

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
        VDENC_64B_Aligned_Lower_Address_CMD      LowerAddress;                                                            //!< DW0, Lower Address
        VDENC_64B_Aligned_Upper_Address_CMD      UpperAddress;                                                            //!< DW1, Upper Address
        VDENC_Surface_Control_Bits_CMD           PictureFields;                                                           //!< DW2, Picture Fields

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_Streamin_Data_Picture_CMD()
        {
            MOS_ZeroMemory(this, sizeof(*this));
        }

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
            struct
            {
                uint32_t                 FwdPredictorX                                    : __CODEGEN_BITFIELD( 0, 15)    ; //!< Fwd Predictor.X
                uint32_t                 FwdPredictorY                                    : __CODEGEN_BITFIELD(16, 31)    ; //!< Fwd Predictor.Y
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 BwdPredictorX                                    : __CODEGEN_BITFIELD( 0, 15)    ; //!< Bwd Predictor.X
                uint32_t                 BwdPredictorY                                    : __CODEGEN_BITFIELD(16, 31)    ; //!< Bwd Predictor.Y
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 FwdRefid0                                        : __CODEGEN_BITFIELD( 0,  3)    ; //!< Fwd RefID0
                uint32_t                 BwdRefid0                                        : __CODEGEN_BITFIELD( 4,  7)    ; //!< Bwd RefID0
                uint32_t                 Reserved136                                      : __CODEGEN_BITFIELD( 8, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        uint32_t                                 Reserved160[11];                                                         //!< DWORD5..15

        //! \name Local enumerations

        //! \brief FORCEINTRA
        //! \details
        //!     This field specifies whether current macroblock should be coded as an
        //!     intra macroblock.   It is illegal to enable both ForceSkip
        //!     and ForceIntra for the same macroblock.   This should be
        //!     disabled if Rolling-I is enabled in the VDEnc Image State.
        enum FORCEINTRA
        {
            FORCEINTRA_DISABLE                                               = 0, //!< VDEnc determined macroblock type
            FORCEINTRA_ENABLE                                                = 1, //!< Force to be coded as an intra macroblock
        };

        //! \brief FORCESKIP
        //! \details
        //!     This field specifies whether current macroblock should be coded as a
        //!     skipped macroblock.   It is illegal to enable both ForceSkip
        //!     and ForceIntra for the same macroblock.   This should be
        //!     disabled if Rolling-I is enabled in the VDEnc Image State.  
        //!       It is illegal to enable ForceSkip for I-Frames.
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
        VDENC_STREAMIN_STATE_CMD()
        {
            MOS_ZeroMemory(this, sizeof(*this));

            DW0.Forceintra = FORCEINTRA_DISABLE;
            DW0.Forceskip  = FORCESKIP_DISABLE;

            DW1.Qpprimey = QPPRIMEY_UNNAMED0;
        }

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
            struct
            {
                uint32_t                 SumSadHaarForBestModeDecision                                                    ; //!< Sum sad\haar for best mode decision
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 IntraCuCountNormalized                           : __CODEGEN_BITFIELD( 0, 19)    ; //!< Intra CU count normalized
                uint32_t                 Reserved52                                       : __CODEGEN_BITFIELD(20, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 NonSkipInterCuCountNormalized                    : __CODEGEN_BITFIELD( 0, 19)    ; //!< Non-skip Inter CU count normalized
                uint32_t                 Reserved84                                       : __CODEGEN_BITFIELD(20, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 SegmentMapCount0                                 : __CODEGEN_BITFIELD( 0, 19)    ; //!< segment map count 0
                uint32_t                 Reserved116                                      : __CODEGEN_BITFIELD(20, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 SegmentMapCount1                                 : __CODEGEN_BITFIELD( 0, 19)    ; //!< segment map count 1
                uint32_t                 Reserved148                                      : __CODEGEN_BITFIELD(20, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            struct
            {
                uint32_t                 SegmentMapCount2                                 : __CODEGEN_BITFIELD( 0, 19)    ; //!< segment map count 2
                uint32_t                 Reserved180                                      : __CODEGEN_BITFIELD(20, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            struct
            {
                uint32_t                 SegmentMapCount3                                 : __CODEGEN_BITFIELD( 0, 19)    ; //!< segment map count 3
                uint32_t                 Reserved212                                      : __CODEGEN_BITFIELD(20, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            struct
            {
                uint32_t                 MvXGlobalMeSample025X25X                         : __CODEGEN_BITFIELD( 0, 15)    ; //!< MV.x Global ME sample 0 (.25x,.25x)
                uint32_t                 MvYGlobalMeSample025X25X                         : __CODEGEN_BITFIELD(16, 31)    ; //!< MV.y Global ME sample 0 (.25x,.25x)
            };
            uint32_t                     Value;
        } DW7;
        //uint16_t                                 MvXyGlobalMeSample15X25X[2];                                             //!< MV.xy Global ME sample 1 (.5x,.25x)
        //uint16_t                                 MvXyGlobalMeSample25X25X[2];                                             //!< MV.xy Global ME sample 2 (.5x,.25x)
        //uint16_t                                 MvXyGlobalMeSample35X25X[2];                                             //!< MV.xy Global ME sample 3 (.5x,.25x)
        //uint16_t                                 MvXyGlobalMeSample45X25X[2];                                             //!< MV.xy Global ME sample 4 (.5x,.25x)
        //uint16_t                                 MvXyGlobalMeSample55X25X[2];                                             //!< MV.xy Global ME sample 5 (.5x,.25x)
        //uint16_t                                 MvXyGlobalMeSample65X25X[2];                                             //!< MV.xy Global ME sample 6 (.5x,.25x)
        //uint16_t                                 MvXyGlobalMeSample75X25X[2];                                             //!< MV.xy Global ME sample 7 (.5x,.25x)
        //uint16_t                                 MvXyGlobalMeSample85X25X[2];                                             //!< MV.xy Global ME sample 8 (.5x,.25x)
        union
        {
            struct
            {
                uint32_t MvXGlobalMeSample125X25X : __CODEGEN_BITFIELD(0, 15);   //!< MV.x Global ME sample 1 (.25x,.25x)
                uint32_t MvYGlobalMeSample125X25X : __CODEGEN_BITFIELD(16, 31);  //!< MV.y Global ME sample 1 (.25x,.25x)
            };
            uint32_t Value;
        } DW8;
        union
        {
            struct
            {
                uint32_t MvXGlobalMeSample225X25X : __CODEGEN_BITFIELD(0, 15);   //!< MV.x Global ME sample 2 (.25x,.25x)
                uint32_t MvYGlobalMeSample225X25X : __CODEGEN_BITFIELD(16, 31);  //!< MV.y Global ME sample 2 (.25x,.25x)
            };
            uint32_t Value;
        } DW9;
        union
        {
            struct
            {
                uint32_t MvXGlobalMeSample325X25X : __CODEGEN_BITFIELD(0, 15);   //!< MV.x Global ME sample 3 (.25x,.25x)
                uint32_t MvYGlobalMeSample325X25X : __CODEGEN_BITFIELD(16, 31);  //!< MV.y Global ME sample 3 (.25x,.25x)
            };
            uint32_t Value;
        } DW10;
        union
        {
            struct
            {
                uint32_t MvXGlobalMeSample425X25X : __CODEGEN_BITFIELD(0, 15);   //!< MV.x Global ME sample 4 (.25x,.25x)
                uint32_t MvYGlobalMeSample425X25X : __CODEGEN_BITFIELD(16, 31);  //!< MV.y Global ME sample 4 (.25x,.25x)
            };
            uint32_t Value;
        } DW11;
        union
        {
            struct
            {
                uint32_t MvXGlobalMeSample525X25X : __CODEGEN_BITFIELD(0, 15);   //!< MV.x Global ME sample 5 (.25x,.25x)
                uint32_t MvYGlobalMeSample525X25X : __CODEGEN_BITFIELD(16, 31);  //!< MV.y Global ME sample 5 (.25x,.25x)
            };
            uint32_t Value;
        } DW12;
        union
        {
            struct
            {
                uint32_t MvXGlobalMeSample625X25X : __CODEGEN_BITFIELD(0, 15);   //!< MV.x Global ME sample 6 (.25x,.25x)
                uint32_t MvYGlobalMeSample625X25X : __CODEGEN_BITFIELD(16, 31);  //!< MV.y Global ME sample 6 (.25x,.25x)
            };
            uint32_t Value;
        } DW13;
        union
        {
            struct
            {
                uint32_t MvXGlobalMeSample725X25X : __CODEGEN_BITFIELD(0, 15);   //!< MV.x Global ME sample 7 (.25x,.25x)
                uint32_t MvYGlobalMeSample725X25X : __CODEGEN_BITFIELD(16, 31);  //!< MV.y Global ME sample 7 (.25x,.25x)
            };
            uint32_t Value;
        } DW14;
        union
        {
            struct
            {
                uint32_t MvXGlobalMeSample825X25X : __CODEGEN_BITFIELD(0, 15);   //!< MV.x Global ME sample 8 (.25x,.25x)
                uint32_t MvYGlobalMeSample825X25X : __CODEGEN_BITFIELD(16, 31);  //!< MV.y Global ME sample 8 (.25x,.25x)
            };
            uint32_t Value;
        } DW15;
        union
        {
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
            struct
            {
                uint32_t                 PaletteCuCountNormalized                         : __CODEGEN_BITFIELD( 0, 19)    ; //!< Palette CU Count Normalized
                uint32_t                 Reserved564                                      : __CODEGEN_BITFIELD(20, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW17;
        union
        {
            struct
            {
                uint32_t                 IbcCuCountNormalized                             : __CODEGEN_BITFIELD( 0, 19)    ; //!< IBC CU Count Normalized
                uint32_t                 Reserved596                                      : __CODEGEN_BITFIELD(20, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW18;
        union
        {
            struct
            {
                uint32_t                 Reserved                                                                         ; //!< Reserved
            };
            uint32_t                     Value;
        } DW19;
        union
        {
            struct
            {
                uint32_t                 Reserved                                                                         ; //!< Reserved
            };
            uint32_t                     Value;
        } DW20;
        union
        {
            struct
            {
                uint32_t                 Reserved672                                                                      ; //!< Reserved
            };
            uint32_t                     Value;
        } DW21;
        union
        {
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
            struct
            {
                uint32_t                 LocationOfPanic                                  : __CODEGEN_BITFIELD( 0, 15)    ; //!< Location of panic
                uint32_t                 PanicDetected                                    : __CODEGEN_BITFIELD(16, 16)    ; //!< Panic detected
                uint32_t                 Reserved753                                      : __CODEGEN_BITFIELD(17, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW23;
        uint32_t                                 Reserved768[5];                                                          //!< Reserved
        union
        {
            struct
            {
                uint32_t                 SumSadHaarForBestModeDecisionBottomHalfPopulation                                 ; //!< Sum sad\haar for best mode decision bottom half population
            };
            uint32_t                     Value;
        } DW29;
        union
        {
            struct
            {
                uint32_t                 SumSadHaarForBestModeDecisionTopHalfPopulation                                   ; //!< Sum sad\haar for best mode decision top half population
            };
            uint32_t                     Value;
        } DW30;
        union
        {
            struct
            {
                uint32_t                 SumTopHalfPopulationOccurrences                  : __CODEGEN_BITFIELD( 0, 15)    ; //!< Sum top half population occurrences
                uint32_t                 SumBottomHalfPopulationOccurrences               : __CODEGEN_BITFIELD(16, 31)    ; //!< Sum bottom half population occurrences
            };
            uint32_t                     Value;
        } DW31;
        union
        {
            struct
            {
                uint32_t                 ReadRequestBank0                                 : __CODEGEN_BITFIELD( 0, 23)    ; //!< Read Request Bank 0
                uint32_t                 Reserved1048                                     : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW32;
        union
        {
            struct
            {
                uint32_t                 CacheMissCountBank0                              : __CODEGEN_BITFIELD( 0, 23)    ; //!< Cache Miss count Bank 0
                uint32_t                 Reserved1080                                     : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW33;
        union
        {
            struct
            {
                uint32_t                 ReadRequestBank1                                 : __CODEGEN_BITFIELD( 0, 23)    ; //!< Read Request Bank 1
                uint32_t                 Reserved1112                                     : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW34;
        union
        {
            struct
            {
                uint32_t                 CacheMissCountBank1                              : __CODEGEN_BITFIELD( 0, 23)    ; //!< Cache Miss count Bank 1
                uint32_t                 Reserved1144                                     : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW35;
        union
        {
            struct
            {
                uint32_t                 ReadRequestBank2                                 : __CODEGEN_BITFIELD( 0, 23)    ; //!< Read Request Bank 2
                uint32_t                 Reserved1176                                     : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW36;
        union
        {
            struct
            {
                uint32_t                 CacheMissCountBank2                              : __CODEGEN_BITFIELD( 0, 23)    ; //!< Cache Miss count Bank 2
                uint32_t                 Reserved1208                                     : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW37;
        union
        {
            struct
            {
                uint32_t                 ReadRequestBank3                                 : __CODEGEN_BITFIELD( 0, 23)    ; //!< Read Request Bank 3
                uint32_t                 Reserved1240                                     : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW38;
        union
        {
            struct
            {
                uint32_t                 CacheMissCountBank3                              : __CODEGEN_BITFIELD( 0, 23)    ; //!< Cache Miss count Bank 3
                uint32_t                 Reserved1272                                     : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW39;
        union
        {
            struct
            {
                uint32_t                 ReadRequestBank4                                 : __CODEGEN_BITFIELD( 0, 23)    ; //!< Read Request Bank 4
                uint32_t                 Reserved1304                                     : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW40;
        union
        {
            struct
            {
                uint32_t                 CacheMissCountBank4                              : __CODEGEN_BITFIELD( 0, 23)    ; //!< Cache Miss count Bank 4
                uint32_t                 Reserved1336                                     : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW41;
        union
        {
            struct
            {
                uint32_t                 HimeRequestCount                                 : __CODEGEN_BITFIELD( 0, 23)    ; //!< HIME request count
                uint32_t                 Reserved1368                                     : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW42;
        union
        {
            struct
            {
                uint32_t                 HimeRequestArbitraionLostCycleCount              : __CODEGEN_BITFIELD( 0, 23)    ; //!< HIME request arbitraion lost cycle count
                uint32_t                 Reserved1400                                     : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW43;
        union
        {
            struct
            {
                uint32_t                 HimeRequestStallCount                            : __CODEGEN_BITFIELD( 0, 23)    ; //!< HIME Request Stall count
                uint32_t                 Reserved1432                                     : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW44;
        union
        {
            struct
            {
                uint64_t                 Reserved1440                                                                     ; //!< Reserved
            };
            uint32_t                     Value[2];
        } DW45_46;
        union
        {
            struct
            {
                uint32_t                 TotalReferenceReadCount                                                          ; //!< Total Reference Read count
            };
            uint32_t                     Value;
        } DW47;
        uint32_t                                 NumberofpixelsLumaCbCrValues150Each[256];                                //!< NumberOfPixels Luma/Cb/Cr values [15:0] each

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_HEVC_VP9_FRAME_BASED_STATISTICS_STREAMOUT_CMD()
        {
            MOS_ZeroMemory(this, sizeof(*this));
        }

        static const size_t dwSize = 304;
        static const size_t byteSize = 1216;
    };

    //!
    //! \brief VDENC_HEVC_VP9_STREAMIN_STATE
    //! \details
    //!       Restrictions for I-Frame:Only valid I-frame parameters are ForceQPEn,
    //!     ROI and SegID Enable.
    //!     DW8-12 (Stream Predictors and Refids) are not valid.
    //!     Force RefID Enable, NumMergeCand*, NumIMEPredictors, PUType-forceMV,
    //!     PUType-forceIntra are all ignored.
    //!     MaxTUSize and MaxCUSize are also ignored.
    //!     VP9 Resctrictions:Only one streamin predictor is supported.
    //!     Force RefID, PUType-ForceMV and PUType-ForceIntra are not supported.
    //!     SegID is used instead for QP in the streamin for VP9.
    //!     
    //!     
    //!     
    //!     
    //!     
    struct VDENC_HEVC_VP9_STREAMIN_STATE_CMD
    {
        union
        {
            struct
            {
                uint32_t                 Roi32X32016X1603                                 : __CODEGEN_BITFIELD( 0,  7)    ; //!< ROI 32x32_0 16x16_03
                uint32_t                 Maxtusize                                        : __CODEGEN_BITFIELD( 8,  9)    ; //!< MaxTUSize
                uint32_t                 Maxcusize                                        : __CODEGEN_BITFIELD(10, 11)    ; //!< MaxCUSize
                uint32_t                 Numimepredictors                                 : __CODEGEN_BITFIELD(12, 15)    ; //!< NUMIMEPREDICTORS
                uint32_t                 Reserved16                                       : __CODEGEN_BITFIELD(16, 21)    ; //!< Reserved
                uint32_t                 PaletteDisable                                   : __CODEGEN_BITFIELD(22, 22)    ; //!< Palette Disable
                uint32_t                 Reserved23                                       : __CODEGEN_BITFIELD(23, 23)    ; //!< Reserved
                uint32_t                 PuType32X32016X1603                              : __CODEGEN_BITFIELD(24, 31)    ; //!< PU Type 32x32_0 16x16_03
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 ForceMvX32X32016X160                             : __CODEGEN_BITFIELD( 0, 15)    ; //!< force_mv.x 32x32_0 16x16_0
                uint32_t                 ForceMvY32X32016X160                             : __CODEGEN_BITFIELD(16, 31)    ; //!< force_mv.y 32x32_0 16x16_0
            };
            uint32_t                     Value;
        } DW1;
        //uint16_t                                 ForceMvXY32X32016X1613[6];                                               //!< force_mv.x/y 32x32_0 16x16_1-3
        union
        {
            struct
            {
                uint32_t ForceMvX32X32016X161 : __CODEGEN_BITFIELD(0, 15);   //!< force_mv.x 32x32_0 16x16_1
                uint32_t ForceMvY32X32016X161 : __CODEGEN_BITFIELD(16, 31);  //!< force_mv.y 32x32_0 16x16_1
            };
            uint32_t Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t ForceMvX32X32016X162 : __CODEGEN_BITFIELD(0, 15);   //!< force_mv.x 32x32_0 16x16_2
                uint32_t ForceMvY32X32016X162 : __CODEGEN_BITFIELD(16, 31);  //!< force_mv.y 32x32_0 16x16_2
            };
            uint32_t Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t ForceMvX32X32016X163 : __CODEGEN_BITFIELD(0, 15);   //!< force_mv.x 32x32_0 16x16_3
                uint32_t ForceMvY32X32016X163 : __CODEGEN_BITFIELD(16, 31);  //!< force_mv.y 32x32_0 16x16_3
            };
            uint32_t Value;
        } DW4;
        union
        {
            struct
            {
                uint32_t                 Reserved160                                                                      ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;
        union
        {
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
            struct
            {
                uint32_t                 ImePredictor0X32X320                             : __CODEGEN_BITFIELD( 0, 15)    ; //!< ime_predictor0.x 32x32_0
                uint32_t                 ImePredictor0Y32X320                             : __CODEGEN_BITFIELD(16, 31)    ; //!< ime_predictor0.y 32x32_0
            };
            uint32_t                     Value;
        } DW8;
        //uint16_t                                 ImePredictor13XY32X320[6];                                               //!< ime_predictor1-3.x/y 32x32_0
        union
        {
            //!< DWORD 9
            struct
            {
                uint32_t ImePredictor0X32X321 : __CODEGEN_BITFIELD(0, 15);   //!< ime_predictor0.x 32x32_1
                uint32_t ImePredictor0Y32X321 : __CODEGEN_BITFIELD(16, 31);  //!< ime_predictor0.y 32x32_1
            };
            uint32_t Value;
        } DW9;
        union
        {
            //!< DWORD 10
            struct
            {
                uint32_t ImePredictor0X32X322 : __CODEGEN_BITFIELD(0, 15);   //!< ime_predictor0.x 32x32_2
                uint32_t ImePredictor0Y32X322 : __CODEGEN_BITFIELD(16, 31);  //!< ime_predictor0.y 32x32_2
            };
            uint32_t Value;
        } DW10;
        union
        {
            //!< DWORD 11
            struct
            {
                uint32_t ImePredictor0X32X323 : __CODEGEN_BITFIELD(0, 15);   //!< ime_predictor0.x 32x32_3
                uint32_t ImePredictor0Y32X323 : __CODEGEN_BITFIELD(16, 31);  //!< ime_predictor0.y 32x32_3
            };
            uint32_t Value;
        } DW11;
        union
        {
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
            struct
            {
                uint32_t                 Panicmodelcuthreshold                            : __CODEGEN_BITFIELD( 0, 15)    ; //!< PanicModeLCUThreshold
                uint32_t                 Reserved432                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW13;
        union
        {
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
            struct
            {
                uint32_t                 Reserved480                                                                      ; //!< Reserved
            };
            uint32_t                     Value;
        } DW15;

        //! \name Local enumerations

        //! \brief NUMIMEPREDICTORS
        //! \details
        //!     This parameter specifes the number of IME predictors to be processed
        //!     in stage3 IME.  />
        enum NUMIMEPREDICTORS
        {
            NUMIMEPREDICTORS_UNNAMED0                                        = 0, //!< No additional details
            NUMIMEPREDICTORS_UNNAMED4                                        = 4, //!< No additional details
            NUMIMEPREDICTORS_UNNAMED8                                        = 8, //!< No additional details
            NUMIMEPREDICTORS_UNNAMED12                                       = 12, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_HEVC_VP9_STREAMIN_STATE_CMD()
        {
            MOS_ZeroMemory(this, sizeof(*this));

            DW0.Numimepredictors = NUMIMEPREDICTORS_UNNAMED0;
        }

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
            struct
            {
                uint32_t                 TileMode                                         : __CODEGEN_BITFIELD( 0,  1)    ; //!< TILEMODE
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
            struct
            {
                uint32_t                 YOffsetForVCr                                    : __CODEGEN_BITFIELD( 0, 15)    ; //!< Y Offset for V(Cr)
                uint32_t                 XOffsetForVCr                                    : __CODEGEN_BITFIELD(16, 28)    ; //!< X Offset for V(Cr)
                uint32_t                 Reserved125                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;

        //! \name Local enumerations

        //enum TILEMODE
        //{
            //TILEMODE_LINEAR                                                  = 0, //!< No additional details
            //TILEMODE_TILES_64K                                               = 1, //!< No additional details
            //TILEMODE_TILEX                                                   = 2, //!< No additional details
            //TILEMODE_TILEF                                                   = 3, //!< No additional details
        //};
        //! \brief TILE_MODE
        enum TILE_MODE
        {
            TILE_LINEAR                                                      = 0, //!< Linear
            TILE_S                                                           = 1, //!< TileS(64K)
            TILE_X                                                           = 2, //!< Tile X
            TILE_F                                                           = 3, //!< Tile F
        };

        //! \brief HALF_PITCH_FOR_CHROMA
        //! \details
        //!     (This field must be set to Disable.) This field indicates that the
        //!     chroma plane(s) will use a pitch equalto half the value specified in the
        //!     Surface Pitch field. This field is only used for PLANAR surface
        //!     formats.This field is igored by VDEnc (unless we support YV12).
        enum HALF_PITCH_FOR_CHROMA
        {
            HALF_PITCH_FOR_CHROMA_DISABLE                                    = 0, //!< No additional details
            HALF_PITCH_FOR_CHROMA_ENABLE                                     = 1, //!< No additional details
        };

        //! \brief INTERLEAVE_CHROMA_
        //! \details
        //!     This field indicates that the chroma fields are interleaved in a single
        //!     plane rather than stored astwo separate planes. This field is only used
        //!     for PLANAR surface formats.
        enum INTERLEAVE_CHROMA_
        {
            INTERLEAVE_CHROMA_DISABLE                                        = 0, //!< No additional details
            INTERLEAVE_CHROMA_ENABLE                                         = 1, //!< No additional details
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
            SURFACE_FORMAT_YCRCB_SWAPY_422                                   = 5,   //!< UYVY (8:8:8:8 MSB Y1 V0 Y0 U0)
            SURFACE_FORMAT_YCRCB_SWAPUV_422                                  = 6,   //!< YVYU (8:8:8:8 MSB U0 Y1 V0 Y0)
            SURFACE_FORMAT_YCRCB_SWAPUVY_422                                 = 7,   //!< VYUY (8:8:8:8 MSB Y1 U0 Y0 V0)
            SURFACE_FORMAT_P010                                              = 8, //!< 10-bit planar 420 (Tile-Y/Linear/Tile-X)
            SURFACE_FORMAT_RGBA10_BITPACKED                                  = 9, //!< Need to convert to YUV. 2 bits Alpha, 10 bits R 10 bits G 10 bits B
            SURFACE_FORMAT_Y410                                              = 10, //!< 10 bit 4:4:4 packed
            SURFACE_FORMAT_NV21                                              = 11, //!< 8-bit, same as NV12 but UV interleave is reversed.
            SURFACE_FORMAT_P010_VARIANT                                      = 12, //!< >8 bit planar 420 with MSB together and LSB at an offset in x direction.
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_Surface_State_Fields_CMD()
        {
            MOS_ZeroMemory(this, sizeof(*this));

            DW1.TileMode           = TILE_F;
            DW1.HalfPitchForChroma = HALF_PITCH_FOR_CHROMA_DISABLE;
        }

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
                uint32_t DwordCountN : __CODEGEN_BITFIELD(0, 11);          //!< DWORD_COUNT_N
                uint32_t Reserved12 : __CODEGEN_BITFIELD(12, 15);          //!< Reserved
                uint32_t Subopcodeb : __CODEGEN_BITFIELD(16, 20);          //!< SUBOPCODEB
                uint32_t Subopcodea : __CODEGEN_BITFIELD(21, 22);          //!< SUBOPCODEA
                uint32_t MediaCommandOpcode : __CODEGEN_BITFIELD(23, 26);  //!< MEDIA_COMMAND_OPCODE
                uint32_t Pipeline : __CODEGEN_BITFIELD(27, 28);            //!< PIPELINE
                uint32_t CommandType : __CODEGEN_BITFIELD(29, 31);         //!< COMMAND_TYPE
            };
            uint32_t Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t HevcPipelineDone : __CODEGEN_BITFIELD(0, 0);             //!< HEVC pipeline Done
                uint32_t VdencPipelineDone : __CODEGEN_BITFIELD(1, 1);            //!< VD-ENC pipeline Done
                uint32_t MflPipelineDone : __CODEGEN_BITFIELD(2, 2);              //!< MFL pipeline Done
                uint32_t MfxPipelineDone : __CODEGEN_BITFIELD(3, 3);              //!< MFX pipeline Done
                uint32_t VdCommandMessageParserDone : __CODEGEN_BITFIELD(4, 4);   //!< VD command/message parser Done
                uint32_t HucPipelineDone : __CODEGEN_BITFIELD(5, 5);              //!< HuC Pipeline Done
                uint32_t AvpPipelineDone : __CODEGEN_BITFIELD(6, 6);              //!< AVP Pipeline Done
                uint32_t VdaqmPipelineDone : __CODEGEN_BITFIELD(7, 7);
                uint32_t VvcpPipelineDone : __CODEGEN_BITFIELD(8, 8);             //!< VVCP Pipeline Done
                uint32_t Reserved40 : __CODEGEN_BITFIELD(9, 15);                  //!< Reserved
                uint32_t HevcPipelineCommandFlush : __CODEGEN_BITFIELD(16, 16);   //!< HEVC pipeline command flush
                uint32_t VdencPipelineCommandFlush : __CODEGEN_BITFIELD(17, 17);  //!< VD-ENC pipeline command flush
                uint32_t MflPipelineCommandFlush : __CODEGEN_BITFIELD(18, 18);    //!< MFL pipeline command flush
                uint32_t MfxPipelineCommandFlush : __CODEGEN_BITFIELD(19, 19);    //!< MFX pipeline command flush
                uint32_t HucPipelineCommandFlush : __CODEGEN_BITFIELD(20, 20);    //!< HuC pipeline command flush
                uint32_t AvpPipelineCommandFlush : __CODEGEN_BITFIELD(21, 21);    //!< AVP Pipeline command Flush
                uint32_t VdaqmPipelineCommandFlush : __CODEGEN_BITFIELD(22, 22);
                uint32_t VvcpPipelineCommandFlush : __CODEGEN_BITFIELD(23, 23);   //!< VVCP Pipeline command Flush
                uint32_t Reserved54 : __CODEGEN_BITFIELD(24, 31);                 //!< Reserved
            };
            uint32_t Value;
        } DW1;

        //! \name Local enumerations

        //! \brief DWORD_COUNT_N
        //! \details
        //!     Total Length - 2
        enum DWORD_COUNT_N
        {
            DWORD_COUNT_N_EXCLUDESDWORD_0 = 0,  //!< No additional details
        };

        enum SUBOPCODEB
        {
            SUBOPCODEB_UNNAMED0 = 0,  //!< No additional details
        };

        enum SUBOPCODEA
        {
            SUBOPCODEA_UNNAMED0 = 0,  //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_EXTENDEDCOMMAND = 15,  //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MEDIA = 2,  //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE = 3,  //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VD_PIPELINE_FLUSH_CMD()
        {
            MOS_ZeroMemory(this, sizeof(*this));

            DW0.DwordCountN        = __CODEGEN_OP_LENGTH(dwSize);
            DW0.Subopcodeb         = SUBOPCODEB_UNNAMED0;
            DW0.Subopcodea         = SUBOPCODEA_UNNAMED0;
            DW0.MediaCommandOpcode = MEDIA_COMMAND_OPCODE_EXTENDEDCOMMAND;
            DW0.Pipeline           = PIPELINE_MEDIA;
            DW0.CommandType        = COMMAND_TYPE_PARALLELVIDEOPIPE;
        }

        static const size_t dwSize   = 2;
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
            struct
            {
                uint32_t                 WeightsForwardReference2                         : __CODEGEN_BITFIELD( 0,  7)    ; //!< Weights Forward Reference2
                uint32_t                 OffsetForwardReference2                          : __CODEGEN_BITFIELD( 8, 15)    ; //!< Offset Forward Reference2
                uint32_t                 WeightsBackwardReference0                        : __CODEGEN_BITFIELD(16, 23)    ; //!< Weights Backward Reference0
                uint32_t                 OffsetBackwardReference0                         : __CODEGEN_BITFIELD(24, 31)    ; //!< Offset Backward Reference0
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 CbWeightsForwardReference0                       : __CODEGEN_BITFIELD( 0,  7)    ; //!< CB Chroma Weights Forward Reference0
                uint32_t                 CbOffsetForwardReference0                        : __CODEGEN_BITFIELD( 8, 15)    ; //!< CB Chroma Offset Forward Reference0
                uint32_t                 CbWeightsForwardReference1                       : __CODEGEN_BITFIELD(16, 23)    ; //!< CBChromaWeights Forward Reference1
                uint32_t                 CbOffsetForwardReference1                        : __CODEGEN_BITFIELD(24, 31)    ; //!< CB Chroma Offset Forward Reference1
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 CbWeightsForwardReference2                       : __CODEGEN_BITFIELD( 0,  7)    ; //!< CB Chroma Weights Forward Reference2
                uint32_t                 CbOffsetForwardReference2                        : __CODEGEN_BITFIELD( 8, 15)    ; //!< CB Chroma Offset Forward Reference2
                uint32_t                 CbWeightsBackwardReference0                      : __CODEGEN_BITFIELD(16, 23)    ; //!< Chroma Weights Backward Reference0
                uint32_t                 CbOffsetBackwardReference0                       : __CODEGEN_BITFIELD(24, 31)    ; //!< CB Chroma Offset Backward Reference0
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            struct
            {
                uint32_t                 CrWeightsForwardReference0                       : __CODEGEN_BITFIELD( 0,  7)    ; //!< CR Chroma Weights Forward Reference0
                uint32_t                 CrOffsetForwardReference0                        : __CODEGEN_BITFIELD( 8, 15)    ; //!< CR Chroma Offset Forward Reference0
                uint32_t                 CrWeightsForwardReference1                       : __CODEGEN_BITFIELD(16, 23)    ; //!< CR ChromaWeights Forward Reference1
                uint32_t                 CrOffsetForwardReference1                        : __CODEGEN_BITFIELD(24, 31)    ; //!< CR Chroma Offset Forward Reference1
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            struct
            {
                uint32_t                 CrWeightsForwardReference2                 : __CODEGEN_BITFIELD( 0,  7)    ; //!< CR Chroma Weights Forward Reference2
                uint32_t                 CrOffsetForwardReference2                  : __CODEGEN_BITFIELD( 8, 15)    ; //!< CR Chroma Offset Forward Reference2
                uint32_t                 CrWeightsBackwardReference0                : __CODEGEN_BITFIELD(16, 23)    ; //!< CR Chroma Weights Backward Reference0
                uint32_t                 CrOffsetBackwardReference0                 : __CODEGEN_BITFIELD(24, 31)    ; //!< CR Chroma Offset Backward Reference0
            };
            uint32_t                     Value;
        } DW6;

        //! \name Local enumerations

        //! \brief DW_LENGTH
        //! \details
        //!     Total Length - 2
        enum DW_LENGTH
        {
            DW_LENGTH_THREEFWDANDONEBWDLUMAONLY                              = 1, //!< No additional details
            DW_LENGTH_THREEFWDANDONEBWDLUMACHROMAONLY                        = 5, //!< No additional details
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
        VDENC_WEIGHTSOFFSETS_STATE_CMD()
        {
            MOS_ZeroMemory(this, sizeof(*this));

            DW0.DwLength    = __CODEGEN_OP_LENGTH(dwSize);
            DW0.Subopb      = SUBOPB_VDENCAVCWEIGHTSOFFSETSTATE;
            DW0.Subopa      = SUBOPA_UNNAMED0;
            DW0.Opcode      = OPCODE_VDENCPIPE;
            DW0.Pipeline    = PIPELINE_MFXCOMMON;
            DW0.CommandType = COMMAND_TYPE_PARALLELVIDEOPIPE;

            DW1.WeightsForwardReference0 = 1;
            DW1.OffsetForwardReference0  = 0;
            DW1.WeightsForwardReference1 = 1;
            DW1.OffsetForwardReference1  = 0;

            DW2.WeightsForwardReference2  = 1;
            DW2.OffsetForwardReference2   = 0;
            DW2.WeightsBackwardReference0 = 1;
            DW2.OffsetBackwardReference0  = 0;
        }

        static const size_t dwSize = 7;
        static const size_t byteSize = 28;
    };

    //!
    //! \brief VDENC_DS_REF_SURFACE_STATE
    //! \details
    //!     This command specifies the surface state parameters for the downscaled
    //!     reference surfaces.
    //!     
    //!     For HEVC/VP9 VDEnc, additional HME Down-scaled stage is added. The AVC
    //!     HME 4X DS surface maps to 8X DS surface for HEVC/VP9. A new 4X DS
    //!     surface is added for HEVC/VP VDEnc.
    //!     
    struct VDENC_DS_REF_SURFACE_STATE_CMD
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
                uint32_t                 Reserved32                                                                       ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        VDENC_Surface_State_Fields_CMD           Dwords25;                                                                //!< DW2..5, Dwords 2..5
        VDENC_Surface_State_Fields_CMD           Dwords69;                                                                //!< DW6..9, Dwords 6..9

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
        VDENC_DS_REF_SURFACE_STATE_CMD()
        {
            MOS_ZeroMemory(this, sizeof(*this));

            DW0.DwordLength = __CODEGEN_OP_LENGTH(dwSize);
            DW0.Subopb      = SUBOPB_VDENCDSREFSURFACESTATE;
            DW0.Subopa      = SUBOPA_UNNAMED0;
            DW0.Opcode      = OPCODE_VDENCPIPE;
            DW0.Pipeline    = PIPELINE_MFXCOMMON;
            DW0.CommandType = COMMAND_TYPE_PARALLELVIDEOPIPE;
        }

        static const size_t dwSize = 10;
        static const size_t byteSize = 40;
    };

    //!
    //! \brief VDENC_PIPE_BUF_ADDR_STATE
    //! \details
    //!     This state command provides the memory base addresses for all row
    //!     stores, Streamin/StreamOut, DMV bufferalong with the uncompressed
    //!     source, reference pictures and downscaled reference pictures required by
    //!     theVDENC pipeline. All reference pixel surfaces in the Encoder are
    //!     programmed with the same surface state(NV12 and TileY format), except
    //!     each has its own frame buffer base address. Same holds true for
    //!     thedown-scaled reference pictures too. In the tile format, there is no
    //!     need to provide buffer offset foreach slice; since from each MB address,
    //!     the hardware can calculated the corresponding memory locationwithin the
    //!     frame buffer directly.   VDEnc supports 3 Downscaled reference frames (
    //!     2 fwd, 1 bwd) and 4 normal reference frames ( 3 fwd, 1 bwd).The driver
    //!     will sort out the base address from the DPB table and populate the base
    //!     addresses that map tothe corresponding reference index for both DS
    //!     references and normal reference frames.  Each of the individual DS ref/
    //!     Normal ref frames have their own MOCS DW that corresponds to the
    //!     respectivebase address. The only thing that is different in the MOCS DW
    //!     amongst the DS reference frames is the MMCDcontrols (specified in bits
    //!     [10:9] of the MOCS DW). Driver needs to ensure that the other bits need
    //!     to bethe same across the different DS ref frames. The same is applicable
    //!     for the normal reference frames.
    //!     
    struct VDENC_PIPE_BUF_ADDR_STATE_CMD
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
        VDENC_Down_Scaled_Reference_Picture_CMD  DsFwdRef0;                                                               //!< DW1..3, DS FWD REF0
        VDENC_Down_Scaled_Reference_Picture_CMD  DsFwdRef1;                                                               //!< DW4..6, DS FWD REF1
        VDENC_Down_Scaled_Reference_Picture_CMD  DsBwdRef0;                                                               //!< DW7..9, DS BWD REF 0
        VDENC_Original_Uncompressed_Picture_CMD  OriginalUncompressedPicture;                                             //!< DW10..12, Original Uncompressed Picture
        VDENC_Streamin_Data_Picture_CMD          StreaminDataPicture;                                                     //!< DW13..15, Streamin Data Picture
        VDENC_Row_Store_Scratch_Buffer_Picture_CMD RowStoreScratchBuffer;                                                 //!< DW16..18, Row Store Scratch Buffer
        VDENC_Colocated_MV_Picture_CMD           ColocatedMv;                                                             //!< DW19..21, Colocated MV Read Buffer
        VDENC_Reference_Picture_CMD              FwdRef0;                                                                 //!< DW22..24, FWD REF0
        VDENC_Reference_Picture_CMD              FwdRef1;                                                                 //!< DW25..27, FWD REF1
        VDENC_Reference_Picture_CMD              FwdRef2;                                                                 //!< DW28..30, FWD REF2
        VDENC_Reference_Picture_CMD              BwdRef0;                                                                 //!< DW31..33, BWD REF0
        VDENC_Statistics_Streamout_CMD           VdencStatisticsStreamout;                                                //!< DW34..36, VDEnc Statistics Streamout
        VDENC_Down_Scaled_Reference_Picture_CMD  DsFwdRef04X;                                                             //!< DW37..39, DS FWD REF0 4X
        VDENC_Down_Scaled_Reference_Picture_CMD  DsFwdRef14X;                                                             //!< DW40..42, DS FWD REF1 4X
        VDENC_Down_Scaled_Reference_Picture_CMD  DsBwdRef04X;                                                             //!< DW43..45, DS BWD REF0 4X
        VDENC_Statistics_Streamout_CMD           VdencLcuPakObjCmdBuffer;                                                 //!< DW46..48, VDEnc LCU PAK OBJ CMD Buffer
        VDENC_Down_Scaled_Reference_Picture_CMD  ScaledReferenceSurfaceStage1;                                            //!< DW49..51, Scaled Reference Surface Stage 1
        VDENC_Down_Scaled_Reference_Picture_CMD  ScaledReferenceSurfaceStage2;                                            //!< DW52..54, Scaled Reference Surface stage 2
        VDENC_Colocated_MV_Picture_CMD           Vp9SegmentationMapStreaminBuffer;                                        //!< DW55..57, VP9 Segmentation Map Streamin Buffer
        VDENC_Statistics_Streamout_CMD           Vp9SegmentationMapStreamoutBuffer;                                       //!< DW58..60, VP9 Segmentation Map Streamout Buffer
        union
        {
            struct
            {
                uint32_t                 WeightsHistogramStreamoutOffset                                                  ; //!< Weights Histogram Streamout offset
            };
            uint32_t                     Value;
        } DW61;
        VDENC_Row_Store_Scratch_Buffer_Picture_CMD VdencTileRowStoreBuffer;                                               //!< DW62..64, VDENC Tile Row store Buffer
        VDENC_Statistics_Streamout_CMD           VdencCumulativeCuCountStreamoutSurface;                                  //!< DW65..67, VDENC Cumulative CU count streamout surface
        VDENC_Statistics_Streamout_CMD           VdencPaletteModeStreamoutSurface;                                        //!< DW68..70, VDENC Palette Mode streamout surface
        VDENC_Statistics_Streamout_CMD           VDENC_PIPE_BUF_ADDR_STATE_DW71_73;      
        VDENC_Statistics_Streamout_CMD           VDENC_PIPE_BUF_ADDR_STATE_DW74_76;              
        VDENC_Row_Store_Scratch_Buffer_Picture_CMD IntraPredictionRowstoreBaseAddress;                                    //!< DW77..79, Intra Prediction RowStore Base Address
        VDENC_Statistics_Streamout_CMD           VDENC_PIPE_BUF_ADDR_STATE_DW80_82;                                       
        VDENC_Colocated_MV_Picture_CMD           ColocatedMvAvcWriteBuffer;                                               //!< DW83..85, Colocated MV AVC Write Buffer
        VDENC_Down_Scaled_Reference_Picture_CMD  Additional4xDsFwdRef;                                                    //!< DW86..88, Additional x4 DS FWD REF

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
        VDENC_PIPE_BUF_ADDR_STATE_CMD()
        {
            MOS_ZeroMemory(this, sizeof(*this));

            DW0.DwordLength = __CODEGEN_OP_LENGTH(dwSize);
            DW0.Subopb      = SUBOPB_VDENCPIPEBUFADDRSTATE;
            DW0.Subopa      = SUBOPA_UNNAMED0;
            DW0.Opcode      = OPCODE_VDENCPIPE;
            DW0.Pipeline    = PIPELINE_MFXCOMMON;
            DW0.CommandType = COMMAND_TYPE_PARALLELVIDEOPIPE;
        }

        static const size_t dwSize = 89;
        static const size_t byteSize = 356;
    };

    //!
    //! \brief VDENC_PIPE_MODE_SELECT
    //! \details
    //!     Specifies which codec and hardware module is being used to encode/decode
    //!     the video data, on a per-frame basis.  The VDENC_PIPE_MODE_SELECT
    //!     command specifies which codec and hardware module is being used to
    //!     encode/decodethe video data, on a per-frame basis. It also configures
    //!     the hardware pipeline according to the activeencoder/decoder operating
    //!     mode for encoding/decoding the current picture.
    //!     
    struct VDENC_PIPE_MODE_SELECT_CMD
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
                uint32_t                 IsRandomAccess                                   : __CODEGEN_BITFIELD(18, 18)    ; //!< isRandomAccess bit
                uint32_t                 Reserved51                                       : __CODEGEN_BITFIELD(19, 19)    ; //!< Reserved
                uint32_t                 RgbEncodingEnable                                : __CODEGEN_BITFIELD(20, 20)    ; //!< RGB encoding enable
                uint32_t                 PrimaryChannelSelectionForRgbEncoding            : __CODEGEN_BITFIELD(21, 22)    ; //!< PRIMARY_CHANNEL_SELECTION_FOR_RGB_ENCODING
                uint32_t                 FirstSecondaryChannelSelectionForRgbEncoding     : __CODEGEN_BITFIELD(23, 24)    ; //!< FIRST_SECONDARY_CHANNEL_SELECTION_FOR_RGB_ENCODING
                uint32_t                 TileReplayEnable                                 : __CODEGEN_BITFIELD(25, 25)    ; //!< Tile replay enable
                uint32_t                 StreamingBufferConfig                            : __CODEGEN_BITFIELD(26, 27)    ; //!< STREAMING_BUFFER_CONFIG
                uint32_t                 Reserved60                                       : __CODEGEN_BITFIELD(28, 29)    ; //!< Reserved
                uint32_t                 VDENC_PIPE_MODE_SELECT_DW1_BIT30                 : __CODEGEN_BITFIELD(30, 30)    ; //!< 
                uint32_t                 DisableSpeedModeFetchOptimization                : __CODEGEN_BITFIELD(31, 31)    ; //!< Disable Speed Mode fetch optimization
            };
            uint32_t                     Value;
        } DW1;
        union
        {
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
            struct
            {
                uint32_t                 SourceLumaPackedDataTlbPreFetchenable            : __CODEGEN_BITFIELD( 0,  0)    ; //!< SOURCE_LUMAPACKED_DATA_TLB_PRE_FETCHENABLE
                uint32_t                 SourceChromaTlbPreFetchenable                    : __CODEGEN_BITFIELD( 1,  1)    ; //!< SOURCE_CHROMA_TLB_PRE_FETCHENABLE
                uint32_t                 VDENC_PIPE_MODE_SELECT_DW3_BIT2                  : __CODEGEN_BITFIELD( 2,  2)    ; //!< 
                uint32_t                 Reserved99                                       : __CODEGEN_BITFIELD( 3,  3)    ; //!< Reserved
                uint32_t                 Verticalshift32Minus1Src                         : __CODEGEN_BITFIELD( 4,  7)    ; //!< VERTICALSHIFT32MINUS1SRC
                uint32_t                 Hzshift32Minus1Src                               : __CODEGEN_BITFIELD( 8, 11)    ; //!< HZSHIFT32MINUS1SRC
                uint32_t                 Reserved108                                      : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 Numverticalreqminus1Src                          : __CODEGEN_BITFIELD(16, 19)    ; //!< NUMVERTICALREQMINUS1SRC
                uint32_t                 Numhzreqminus1Src                                : __CODEGEN_BITFIELD(20, 23)    ; //!< NUMHZREQMINUS1SRC
                uint32_t                 PreFetchoffsetforsource                          : __CODEGEN_BITFIELD(24, 27)    ; //!< PRE_FETCHOFFSETFORSOURCE
                uint32_t                 VDENC_PIPE_MODE_SELECT_DW3_BIT28                 : __CODEGEN_BITFIELD(28, 31)    ; 
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 Debugtilepassnum                                 : __CODEGEN_BITFIELD( 0,  3)    ; //!< DebugTilePassNum
                uint32_t                 Debugtilenum                                     : __CODEGEN_BITFIELD( 4, 11)    ; //!< DebugTileNum
                uint32_t                 VDENC_PIPE_MODE_SELECT_DW4_BIT12                 : __CODEGEN_BITFIELD(12, 14)    ;
                uint32_t                 Reserved143                                      : __CODEGEN_BITFIELD(15, 15)    ;
                uint32_t                 VDENC_PIPE_MODE_SELECT_DW4_BIT16                 : __CODEGEN_BITFIELD(16, 17)    ;
                uint32_t                 VDENC_PIPE_MODE_SELECT_DW4_BIT18                 : __CODEGEN_BITFIELD(18, 19)    ;
                uint32_t                 Reserved148                                      : __CODEGEN_BITFIELD(20, 20)    ;
                uint32_t                 VDENC_PIPE_MODE_SELECT_DW4_BIT21                 : __CODEGEN_BITFIELD(21, 21)    ;
                uint32_t                 VDENC_PIPE_MODE_SELECT_DW4_BIT22                 : __CODEGEN_BITFIELD(22, 23)    ;
                uint32_t                 VDENC_PIPE_MODE_SELECT_DW4_BIT24                 : __CODEGEN_BITFIELD(24, 25)    ;
                uint32_t                 VDENC_PIPE_MODE_SELECT_DW4_BIT26                 : __CODEGEN_BITFIELD(26, 27)    ;
                uint32_t                 VDENC_PIPE_MODE_SELECT_DW4_BIT28                 : __CODEGEN_BITFIELD(28, 29)    ;
                uint32_t                 Reserved158                                      : __CODEGEN_BITFIELD(30, 31)    ;
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            struct
            {
                uint32_t                 FrameNumber                                      : __CODEGEN_BITFIELD( 0,  3)    ; //!< Frame Number
                uint32_t                 VDENC_PIPE_MODE_SELECT_DW5_BIT4                  : __CODEGEN_BITFIELD( 4,  5)    ;
                uint32_t                 VDENC_PIPE_MODE_SELECT_DW5_BIT6                  : __CODEGEN_BITFIELD( 6,  7)    ;
                uint32_t                 VDENC_PIPE_MODE_SELECT_DW5_BIT8                  : __CODEGEN_BITFIELD( 8,  8)    ;
                uint32_t                 VDENC_PIPE_MODE_SELECT_DW5_BIT9                  : __CODEGEN_BITFIELD( 9,  9)    ;
                uint32_t                 VDENC_PIPE_MODE_SELECT_DW5_BIT10                 : __CODEGEN_BITFIELD(10, 10)    ;
                uint32_t                 CaptureMode                                      : __CODEGEN_BITFIELD(11, 12)    ; //!< CAPTURE_MODE
                uint32_t                 ParallelCaptureAndEncodeSessionId                : __CODEGEN_BITFIELD(13, 15)    ; //!< PARALLEL_CAPTURE_AND_ENCODE_SESSION_ID
                uint32_t                 VDENC_PIPE_MODE_SELECT_DW5_BIT16                 : __CODEGEN_BITFIELD(16, 16)    ;
                uint32_t                 VDENC_PIPE_MODE_SELECT_DW5_BIT17                 : __CODEGEN_BITFIELD(17, 17)    ;
                uint32_t                 Reserved185                                      : __CODEGEN_BITFIELD(18, 23)    ; //!< Reserved
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
            STANDARD_SELECT_HEVC                                             = 0, //!< No additional details
            STANDARD_SELECT_VP9                                              = 1, //!< No additional details
            STANDARD_SELECT_AVC                                              = 2, //!< No additional details
            STANDARD_SELECT_AV1                                              = 3, //!< AV1 supports only 420 8/10 bits only.
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
        //!     slice size conformance.  For HEVC standard: This bit is
        //!     used to enable dynamic slice size control.
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
        //!     VDENC_IMG_STATE.  (ROI_Enable, Fwd/Predictor0 MV Enable,
        //!     Bwd/Predictor1 MV Enable, MB Level QP Enable,
        //!     TargetSizeinWordsMB/MaxSizeinWordsMB Enable).  This bit
        //!     is valid only in AVC mode. In HEVC / VP9 mode this bit is reserved and
        //!     should be set to zero.
        enum VDENC_STREAM_IN_ENABLE
        {
            VDENC_STREAM_IN_ENABLE_DISABLE                                   = 0, //!< No additional details
            VDENC_STREAM_IN_ENABLE_ENABLE                                    = 1, //!< No additional details
        };

        //! \brief BIT_DEPTH
        //! \details
        //!     This parameter indicates the PAK bit depth. The valid values for this
        //!     are 0 / 2 in HEVC / VP9 standard. In AVC standard this field should be
        //!     set to 0.  />
        enum BIT_DEPTH
        {
            BIT_DEPTH_8BIT                                                   = 0, //!< No additional details
            BIT_DEPTH_10BIT                                                  = 2, //!< No additional details
            BIT_DEPTH_12BIT                                                  = 3, //!< No additional details
        };

        //! \brief PAK_CHROMA_SUB_SAMPLING_TYPE
        //! \details
        //!     This field is applicable only in HEVC and VP9. In AVC, this field is
        //!     ignored.  />
        enum PAK_CHROMA_SUB_SAMPLING_TYPE
        {
            PAK_CHROMA_SUB_SAMPLING_TYPE_420                                 = 1, //!< Used for Main8 and Main10 HEVC, VP9 profile0.
            PAK_CHROMA_SUB_SAMPLING_TYPE_422                                 = 2, //!< Used for HEVC RExt 422 profile.
            PAK_CHROMA_SUB_SAMPLING_TYPE_444                                 = 3, //!< HEVC RExt 444, VP9 444 profiles.
        };

        //! \brief PRIMARY_CHANNEL_SELECTION_FOR_RGB_ENCODING
        //! \details
        //!     In RGB encoding, any one of the channel could be primary. This field is
        //!     used for selecting primary channel
        enum PRIMARY_CHANNEL_SELECTION_FOR_RGB_ENCODING
        {
            PRIMARY_CHANNEL_SELECTION_FOR_RGB_ENCODING_UNNAMED0              = 0, //!< Channel R is primary channel
            PRIMARY_CHANNEL_SELECTION_FOR_RGB_ENCODING_UNNAMED1              = 1, //!< Channel G is primary channel.
            PRIMARY_CHANNEL_SELECTION_FOR_RGB_ENCODING_UNNAMED2              = 2, //!< Channel B is primary channel
        };

        //! \brief FIRST_SECONDARY_CHANNEL_SELECTION_FOR_RGB_ENCODING
        //! \details
        //!     In RGB encoding, any one of the channel could be primary. This field is
        //!     used for selecting primary channel
        enum FIRST_SECONDARY_CHANNEL_SELECTION_FOR_RGB_ENCODING
        {
            FIRST_SECONDARY_CHANNEL_SELECTION_FOR_RGB_ENCODING_UNNAMED0      = 0, //!< Channel R is first secondary channel
            FIRST_SECONDARY_CHANNEL_SELECTION_FOR_RGB_ENCODING_UNNAMED1      = 1, //!< Channel G is first secondary channel
            FIRST_SECONDARY_CHANNEL_SELECTION_FOR_RGB_ENCODING_UNNAMED2      = 2, //!< Channel B is first secondary channel.
        };

        //! \brief FORCE_8_BIT_TLB_PREFETCH_FOR_10_BIT_SOURCE
        //! \details
        //!     This bit is defined to mimmic legacy behavior
        enum FORCE_8_BIT_TLB_PREFETCH_FOR_10_BIT_SOURCE
        {
            FORCE_8_BIT_TLB_PREFETCH_FOR_10_BIT_SOURCE_DEFAULTDISABLE        = 0, //!< No additional details
        };

        //! \brief HME_REGION_PRE_FETCHENABLE
        //! \details
        //!     When this bit is set, for all reference frames HME region pages are
        //!     pre-fetched.
        enum HME_REGION_PRE_FETCHENABLE
        {
            HME_REGION_PRE_FETCHENABLE_UNNAMED0                              = 0, //!< No additional details
            HME_REGION_PRE_FETCHENABLE_UNNAMED1                              = 1, //!< No additional details
        };

        //! \brief TOPPREFETCHENABLEMODE
        //! \details
        //!     Top Pre-fetch enable Mode  <table cellpadding="1"
        //!     cellspacing="1" border="1" style="width: 500px;"><tbody><tr>
        //!        <td>Value</td><td>Description</td></tr>  
        //!      <tr><td>0</td><td>Top Pre-fetch enabled at
        //!     superslice boundary.</td></tr><tr><td>1</td>
        //!        <td>Top pre-fetch enabled always.</td></tr>   
        //!     <tr><td>2</td><td>Top pre-fetch never
        //!     performed.</td></tr><tr><td>3</td>  
        //!      <td>Reserved</td></tr></tbody></table>  />
        enum TOPPREFETCHENABLEMODE
        {
            TOPPREFETCHENABLEMODE_UNNAMED0                                   = 0, //!< Top Pre-fetch enabled at superslice boundary.
            TOPPREFETCHENABLEMODE_UNNAMED1                                   = 1, //!< Top Pre-fetch enabled always
            TOPPREFETCHENABLEMODE_UNNAMED2                                   = 2, //!< Top pre-fetch never performed.
            TOPPREFETCHENABLEMODE_UNNAMED3                                   = 3, //!< Reserved.
        };

        //! \brief LEFTPRE_FETCHATWRAPAROUND
        //! \details
        //!     Left pre-fetch enabled on wraparound
        enum LEFTPRE_FETCHATWRAPAROUND
        {
            LEFTPRE_FETCHATWRAPAROUND_UNNAMED1                               = 1, //!< No additional details
        };

        //! \brief VERTICALSHIFT32MINUS1
        //! \details
        //!     This parameter is used for determining y value for Vertical
        //!     pre-fetch.  Vertical shift is 32 * (VerticalShift32Minus1
        //!     + 1).  Example: Current pixel y position is 32,
        //!     VerticalShift32Minus1 is 0.  The y value for the
        //!     prefetches would be 0, 32, 64, 96etc.
        enum VERTICALSHIFT32MINUS1
        {
            VERTICALSHIFT32MINUS1_UNNAMED0                                   = 0, //!< No additional details
        };

        //! \brief HZSHIFT32MINUS1
        //! \details
        //!     This parameter is used for determining x value for horizontal
        //!     pre-fetch for reference.  Horizontalshift is 32 *
        //!     (HzShift32Minus1 + 1).  Example: Current pixel x position
        //!     is 32,HzShift32Minus1 is 1.  The x value for the
        //!     prefetches would be 96, 160 etc.
        enum HZSHIFT32MINUS1
        {
            HZSHIFT32MINUS1_UNNAMED3                                         = 3, //!< No additional details
        };

        //! \brief NUMVERTICALREQMINUS1
        //! \details
        //!     Number of Vertical requests in each region for a constant horizontal
        //!     position.
        enum NUMVERTICALREQMINUS1
        {
            NUMVERTICALREQMINUS1_UNNAMED11                                   = 11, //!< No additional details
        };

        //! \brief NUMHZREQMINUS1
        //! \details
        //!     Number of Horizontal Requests minus 1 at row beginning.
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
        //!     When this bit is set, Source Luma / Packed data TLB pre-fetches are
        //!     performed.
        enum SOURCE_LUMAPACKED_DATA_TLB_PRE_FETCHENABLE
        {
            SOURCE_LUMAPACKED_DATA_TLB_PRE_FETCHENABLE_UNNAMED0              = 0, //!< No additional details
            SOURCE_LUMAPACKED_DATA_TLB_PRE_FETCHENABLE_UNNAMED1              = 1, //!< No additional details
        };

        //! \brief SOURCE_CHROMA_TLB_PRE_FETCHENABLE
        //! \details
        //!     When this bit is set, Source Chroma TLB pre-fetches are performed.
        enum SOURCE_CHROMA_TLB_PRE_FETCHENABLE
        {
            SOURCE_CHROMA_TLB_PRE_FETCHENABLE_UNNAMED0                       = 0, //!< No additional details
            SOURCE_CHROMA_TLB_PRE_FETCHENABLE_UNNAMED1                       = 1, //!< This field can be set only to planar source formats.
        };

        //! \brief VERTICALSHIFT32MINUS1SRC
        //! \details
        //!     This parameter is used for determining y value for vertical
        //!     pre-fetch.  Vertical shift is 32 *
        //!     (VerticalShift32Minus1Src + 1).  Example: Current pixel
        //!     position is 32, VerticalShift32Minus1Src is zero.  The y
        //!     value for the prefetches would be 0, 32, 64, 96 etc.
        enum VERTICALSHIFT32MINUS1SRC
        {
            VERTICALSHIFT32MINUS1SRC_UNNAMED0                                = 0, //!< No additional details
        };

        //! \brief HZSHIFT32MINUS1SRC
        //! \details
        //!     This parameter is used for determining x value for horizontal
        //!     pre-fetch.  Horizontalshift is 32 * (HzShift32Minus1Src +
        //!     1).  Example: Current pixel x position is 32,
        //!     HzShift32Minus1Src is 1.  The x value for the prefetches
        //!     would be 96, 160 etc.
        enum HZSHIFT32MINUS1SRC
        {
            HZSHIFT32MINUS1SRC_UNNAMED0 = 0,  //!< No additional details
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
        //!     Pre-fetch offset for Reference in 16 pixel increment.  <p
        //!     />
        enum PRE_FETCHOFFSETFORSOURCE
        {
            PRE_FETCHOFFSETFORSOURCE_UNNAMED0  = 0,  //!< No additional details
            PRE_FETCHOFFSETFORSOURCE_UNNAMED4                                = 4, //!< This value is applicable in HEVC mode
            PRE_FETCHOFFSETFORSOURCE_UNNAMED7                                = 7, //!< This Value is applicable in AVC mode
        };


        enum CAPTURE_MODE
        {
            CAPTURE_MODE_NOPARALLEL                                          = 0, //!<   No Parallel capture
            CAPTURE_MODE_PARALLELFROMDISPLAY                                 = 1, //!< Parallel encode from Display overlay
            CAPTURE_MODE_PARALLEFROMCAMERAPIPE                               = 2, //!< Parallel encode from Camera Pipe 
        };

        //! \brief STREAMING_BUFFER_CONFIG
        //! \details
        //!     Source format support for AVC:NV12.  Source formats
        //!     support for HEVC: NV12 / P010
        //enum STREAMING_BUFFER_CONFIG
        //{
        //  STREAMING_BUFFER_CONFIG_STREAMINGBUFFERUNSUPPORTED               = 0, //!< No additional details
        //  STREAMING_BUFFER_CONFIG_STREAMINGBUFFER64                        = 1, //!< 64 pixel rows of streaming buffer is usedThis value could be used only for AVC
        //  STREAMING_BUFFER_CONFIG_STREAMINGBUFFER128                       = 2, //!< 128 pixel rows of streaming buffer is used./>
        //  STREAMING_BUFFER_CONFIG_STREAMINGBUFFER256                       = 3, //!< 256 pixel rows of streaming buffer is used./>
        //};
        enum STREAMING_BUFFER_CONFIG
        {
            STREAMING_BUFFER_UNSUPPORTED = 0,
            STREAMING_BUFFER_64          = 1,
            STREAMING_BUFFER_128         = 2,
            STREAMING_BUFFER_256         = 3,
        };
        
        //enum PARALLEL_CAPTURE_AND_ENCODE_SESSION_ID
        //{
        //    PARALLEL_CAPTURE_AND_ENCODE_SESSION_ID_DISPLAYTAILPOINTERADDRESSLOCATION00ED0H_00ED3H = 0, //!< Display tailpointer address location 00ED0h-00ED3h.  Valid for Display / Camera capture.  
        //    PARALLEL_CAPTURE_AND_ENCODE_SESSION_ID_DISPLAYTAILPOINTERADDRESSLOCATION00ED4H_00ED7H = 1, //!< Display tailpointer address location 00ED4h-00ED7h. Valid for Display / Camera capture.
        //    PARALLEL_CAPTURE_AND_ENCODE_SESSION_ID_TAILPOINTERADDRESSLOCATION00ED8H_00EDBH = 2, //!< Tailpointer address location 00ED8h-00EDBh. Valid for Camera Capture only
        //    PARALLEL_CAPTURE_AND_ENCODE_SESSION_ID_DISPLAYTAILPOINTERADDRESSLOCATION00EDCH_00EDFH = 3, //!< Display tailpointer address location 00EDCh-00EDFh.  Valid for Camera Capture only  
        //};
        enum PARALLEL_CAPTURE_AND_ENCODE_SESSION_ID
        {
            PARALLEL_CAPTURE_AND_ENCODE_SESSION_ID_UNNAMED0 = 0,  //!< Display tailpointer address location 00ED0h-00ED3h
            PARALLEL_CAPTURE_AND_ENCODE_SESSION_ID_UNNAMED1 = 1,  //!< Display tailpointer address location 00ED4h-00ED7h
            PARALLEL_CAPTURE_AND_ENCODE_SESSION_ID_UNNAMED2 = 2,  //!< Display tailpointer address location 00ED8h-00EDBh
            PARALLEL_CAPTURE_AND_ENCODE_SESSION_ID_UNNAMED3 = 3,  //!< Display tailpointer address location 00EDCh-00EDFh
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_PIPE_MODE_SELECT_CMD()
        {
            MOS_ZeroMemory(this, sizeof(*this));

            DW0.DwordLength = __CODEGEN_OP_LENGTH(dwSize);
            DW0.Subopb      = SUBOPB_VDENCPIPEMODESELECT;
            DW0.Subopa      = SUBOPA_UNNAMED0;
            DW0.Opcode      = OPCODE_VDENCPIPE;
            DW0.Pipeline    = PIPELINE_MFXCOMMON;
            DW0.CommandType = COMMAND_TYPE_PARALLELVIDEOPIPE;

            DW1.FrameStatisticsStreamOutEnable               = FRAME_STATISTICS_STREAM_OUT_ENABLE_DISABLE;
            DW1.TlbPrefetchEnable                            = TLB_PREFETCH_ENABLE_DISABLE;
            DW1.PakThresholdCheckEnable                      = PAK_THRESHOLD_CHECK_ENABLE_DISABLESTATICSLICEMODE;
            DW1.VdencStreamInEnable                          = VDENC_STREAM_IN_ENABLE_DISABLE;
            DW1.BitDepth                                     = BIT_DEPTH_8BIT;
            DW1.PrimaryChannelSelectionForRgbEncoding        = PRIMARY_CHANNEL_SELECTION_FOR_RGB_ENCODING_UNNAMED1;
            DW1.FirstSecondaryChannelSelectionForRgbEncoding = FIRST_SECONDARY_CHANNEL_SELECTION_FOR_RGB_ENCODING_UNNAMED2;
            DW1.StreamingBufferConfig                        = STREAMING_BUFFER_UNSUPPORTED;

            DW2.HmeRegionPreFetchenable                      = HME_REGION_PRE_FETCHENABLE_UNNAMED1;
            DW2.Topprefetchenablemode                        = TOPPREFETCHENABLEMODE_UNNAMED0;
            DW2.LeftpreFetchatwraparound                     = LEFTPRE_FETCHATWRAPAROUND_UNNAMED1;
            DW2.Verticalshift32Minus1                        = VERTICALSHIFT32MINUS1_UNNAMED0;
            DW2.Hzshift32Minus1                              = HZSHIFT32MINUS1_UNNAMED3;
            DW2.NumVerticalReqMinus1                         = NUMVERTICALREQMINUS1_UNNAMED11;
            DW2.Numhzreqminus1                               = NUMHZREQMINUS1_UNNAMED2;
            DW2.PreFetchOffsetForReferenceIn16PixelIncrement = PRE_FETCH_OFFSET_FOR_REFERENCE_IN_16_PIXEL_INCREMENT_UNNAMED0;

            DW3.SourceLumaPackedDataTlbPreFetchenable = SOURCE_LUMAPACKED_DATA_TLB_PRE_FETCHENABLE_UNNAMED0;
            DW3.SourceChromaTlbPreFetchenable         = SOURCE_CHROMA_TLB_PRE_FETCHENABLE_UNNAMED0;
            DW3.Verticalshift32Minus1Src              = VERTICALSHIFT32MINUS1SRC_UNNAMED0;
            DW3.Hzshift32Minus1Src                    = HZSHIFT32MINUS1SRC_UNNAMED0;
            DW3.Numverticalreqminus1Src               = NUMVERTICALREQMINUS1SRC_UNNAMED0;
            DW3.Numhzreqminus1Src                     = NUMHZREQMINUS1SRC_UNNAMED0;
            DW3.PreFetchoffsetforsource               = PRE_FETCHOFFSETFORSOURCE_UNNAMED0;

            DW5.CaptureMode                       = CAPTURE_MODE_NOPARALLEL;
            DW5.ParallelCaptureAndEncodeSessionId = PARALLEL_CAPTURE_AND_ENCODE_SESSION_ID_UNNAMED0;
        }

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
                uint32_t                 Reserved32                                                                       ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        VDENC_Surface_State_Fields_CMD           Dwords25;                                                                //!< DW2..5, Dwords 2..5

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
        VDENC_REF_SURFACE_STATE_CMD()
        {
            MOS_ZeroMemory(this, sizeof(*this));

            DW0.DwordLength = __CODEGEN_OP_LENGTH(dwSize);
            DW0.Subopb      = SUBOPB_VDENCREFSURFACESTATE;
            DW0.Subopa      = SUBOPA_UNNAMED0;
            DW0.Opcode      = OPCODE_VDENCPIPE;
            DW0.Pipeline    = PIPELINE_MFXCOMMON;
            DW0.CommandType = COMMAND_TYPE_PARALLELVIDEOPIPE;
        }

        static const size_t dwSize = 6;
        static const size_t byteSize = 24;
    };

    //!
    //! \brief VDENC_SRC_SURFACE_STATE
    //! \details
    //!     This command specifies the uncompressed original input picture to be
    //!     encoded.  The actual base address is defined in the
    //!     VDENC_PIPE_BUF_ADDR_STATE. Pitch can be wider than the Picture Width in
    //!     pixels and garbage will be there at the end of each line. The following
    //!     describes all the different formats that are supported in WLV+ VDEnc: 
    //!     NV12 - 4:2:0 only; UV interleaved; Full Pitch, U and V offset is set to
    //!     0 (the only format supported for video codec); vertical UV offset is MB
    //!     aligned; UV xoffsets = 0.
    //!     This surface state here is identical to the Surface State for
    //!     deinterlace and sample_8x8 messages described in the Shared Function
    //!     Volume and Sampler Chapter.  For non pixel data, such as row stores, DMV
    //!     and streamin/out, a linear buffer is employed. For row stores, the H/W
    //!     is designed to guarantee legal memory accesses (read and write). For the
    //!     remaining cases, indirect object base address, indirect object address
    //!     upper bound, object data start address (offset) and object data length
    //!     are used to fully specified their corresponding buffer. This mechanism
    //!     is chosen over the pixel surface type because of their variable record
    //!     sizes.  All row store surfaces are linear surface. Their addresses are
    //!     programmed in VDEnc_Pipe_Buf_Base_State.
    //!     
    struct VDENC_SRC_SURFACE_STATE_CMD
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
                uint32_t                 Reserved32                                                                       ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        VDENC_Surface_State_Fields_CMD           Dwords25;                                                                //!< DW2..5, Dwords 2..5

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
        VDENC_SRC_SURFACE_STATE_CMD()
        {
            MOS_ZeroMemory(this, sizeof(*this));
            
            DW0.DwordLength = __CODEGEN_OP_LENGTH(dwSize);
            DW0.Subopb      = SUBOPB_VDENCSRCSURFACESTATE;
            DW0.Subopa      = SUBOPA_UNNAMED0;
            DW0.Opcode      = OPCODE_VDENCPIPE;
            DW0.Pipeline    = PIPELINE_MFXCOMMON;
            DW0.CommandType = COMMAND_TYPE_PARALLELVIDEOPIPE;
        }

        static const size_t dwSize = 6;
        static const size_t byteSize = 24;
    };

    //!
    //! \brief VDENC_WALKER_STATE
    //! \details
    //!     
    //!     
    struct VDENC_WALKER_STATE_CMD
    {
        union
        {
            struct
            {
                uint32_t                 Length                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
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

        enum LENGTH
        {
            LENGTH_UNNAMED1 = 1,  //!< No additional details
        };

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
        VDENC_WALKER_STATE_CMD()
        {
            MOS_ZeroMemory(this, sizeof(*this));

            DW0.Length      = LENGTH_UNNAMED1;
            DW0.Subopb      = SUBOPB_VDENCWALKERSTATE;
            DW0.Subopa      = SUBOPA_UNNAMED0;
            DW0.Opcode      = OPCODE_VDENCPIPE;
            DW0.Pipeline    = PIPELINE_MFXCOMMON;
            DW0.CommandType = COMMAND_TYPE_PARALLELVIDEOPIPE;
        }

        static const size_t dwSize = 3;
        static const size_t byteSize = 12;
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
                uint32_t DwordLength : __CODEGEN_BITFIELD(0, 11);               //!< Dword Length
                uint32_t Reserved12 : __CODEGEN_BITFIELD(12, 15);               //!< Reserved
                uint32_t MediaInstructionCommand : __CODEGEN_BITFIELD(16, 22);  //!< Media Instruction Command
                uint32_t MediaInstructionOpcode : __CODEGEN_BITFIELD(23, 26);   //!< Media Instruction Opcode
                uint32_t PipelineType : __CODEGEN_BITFIELD(27, 28);             //!< Pipeline Type
                uint32_t CommandType : __CODEGEN_BITFIELD(29, 31);              //!< Command Type
            };
            uint32_t Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t Reserved32 : __CODEGEN_BITFIELD(0, 0);           //!< Reserved
                uint32_t VdencInitialization : __CODEGEN_BITFIELD(1, 1);  //!< VDenc Initialization
                uint32_t Reserved34 : __CODEGEN_BITFIELD(2, 31);          //!< Reserved
            };
            uint32_t Value;
        } DW1;

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_CONTROL_STATE_CMD()
        {
            MOS_ZeroMemory(this, sizeof(*this));

            DW0.DwordLength             = __CODEGEN_OP_LENGTH(dwSize);
            DW0.MediaInstructionCommand = 0xB;
            DW0.MediaInstructionOpcode  = 0x1;
            DW0.PipelineType            = 0x2;
            DW0.CommandType             = 0x3;
        }

        static const size_t dwSize   = 2;
        static const size_t byteSize = 8;
    };

    //!
    //! \brief VDENC_AVC_SLICE_STATE
    //! \details
    //!
    //!
    struct VDENC_AVC_SLICE_STATE_CMD
    {
        union
        {
            struct
            {
                uint32_t DwordLength : __CODEGEN_BITFIELD(0, 11);   //!< DWORD_LENGTH
                uint32_t Reserved12 : __CODEGEN_BITFIELD(12, 15);   //!< Reserved
                uint32_t Subopb : __CODEGEN_BITFIELD(16, 20);       //!< SUBOPB
                uint32_t Subopa : __CODEGEN_BITFIELD(21, 22);       //!< SUBOPA
                uint32_t Opcode : __CODEGEN_BITFIELD(23, 26);       //!< OPCODE
                uint32_t Pipeline : __CODEGEN_BITFIELD(27, 28);     //!< PIPELINE
                uint32_t CommandType : __CODEGEN_BITFIELD(29, 31);  //!< COMMAND_TYPE
            };
            uint32_t Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t RoundIntra : __CODEGEN_BITFIELD(0, 2);        //!< RoundIntra
                uint32_t RoundIntraEnable : __CODEGEN_BITFIELD(3, 3);  //!< RoundIntraEnable
                uint32_t RoundInter : __CODEGEN_BITFIELD(4, 6);        //!< RoundInter
                uint32_t RoundInterEnable : __CODEGEN_BITFIELD(7, 7);  //!< RoundInterEnable
                uint32_t Reserved32 : __CODEGEN_BITFIELD(8, 31);       //!< Reserved
            };
            uint32_t Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t Reserved64;  //!< Reserved
            };
            uint32_t Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t Log2WeightDenomLuma : __CODEGEN_BITFIELD(0, 2);  //!< Log 2 Weight Denom Luma
                uint32_t Reserved99 : __CODEGEN_BITFIELD(3, 31);          //!< Reserved
            };
            uint32_t Value;
        } DW3;

        //! \name Local enumerations

        enum SUBOPB
        {
            SUBOPB_VDENCAVCSLICESTATE = 12,  //!< No additional details
        };

        enum SUBOPA
        {
            SUBOPA_UNNAMED0 = 0,  //!< No additional details
        };

        enum OPCODE
        {
            OPCODE_VDENCPIPE = 1,  //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFXCOMMON = 2,  //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE = 3,  //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_AVC_SLICE_STATE_CMD()
        {
            MOS_ZeroMemory(this, sizeof(*this));

            DW0.Value = 0x708c0002;
        }

        static const size_t dwSize   = 4;
        static const size_t byteSize = 16;
    };

    struct VDENC_HEVC_VP9_TILE_SLICE_STATE_CMD
    {
        union
        {
            struct
            {
                uint32_t DwordLength : __CODEGEN_BITFIELD(0, 11);   //!< DWORD_LENGTH
                uint32_t Reserved12 : __CODEGEN_BITFIELD(12, 15);   //!< Reserved
                uint32_t Subopb : __CODEGEN_BITFIELD(16, 20);       //!< SUBOPB
                uint32_t Subopa : __CODEGEN_BITFIELD(21, 22);       //!< SUBOPA
                uint32_t Opcode : __CODEGEN_BITFIELD(23, 26);       //!< OPCODE
                uint32_t Pipeline : __CODEGEN_BITFIELD(27, 28);     //!< PIPELINE
                uint32_t CommandType : __CODEGEN_BITFIELD(29, 31);  //!< COMMAND_TYPE
            };
            uint32_t Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t Reserved32;  //!< Reserved
            };
            uint32_t Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t Reserved64;  //!< Reserved
            };
            uint32_t Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t Log2WeightDenomLuma : __CODEGEN_BITFIELD(0, 2);         //!< Log 2 Weight Denom Luma
                uint32_t Reserved99 : __CODEGEN_BITFIELD(3, 3);                  //!< Reserved
                uint32_t HevcVp9Log2WeightDenomLuma : __CODEGEN_BITFIELD(4, 6);  //!< HEVC/VP9 Log 2 Weight Denom Luma
                uint32_t Reserved103 : __CODEGEN_BITFIELD(7, 8);                 //!< Reserved
                uint32_t NumParEngine : __CODEGEN_BITFIELD(9, 10);               //!< NUM_PAR_ENGINE
                uint32_t Reserved107 : __CODEGEN_BITFIELD(11, 15);               //!< Reserved
                uint32_t TileRowStoreSelect : __CODEGEN_BITFIELD(16, 16);        //!< Tile Row store Select
                uint32_t Log2WeightDenomChroma : __CODEGEN_BITFIELD(17, 19);     //!< Log 2 Weight Denom Chroma
                uint32_t Reserved113 : __CODEGEN_BITFIELD(20, 23);               //!< Reserved
                uint32_t TileNumber : __CODEGEN_BITFIELD(24, 31);                //!< Tile number
            };
            uint32_t Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t TileStartCtbY : __CODEGEN_BITFIELD(0, 15);   //!< Tile Start CTB-Y
                uint32_t TileStartCtbX : __CODEGEN_BITFIELD(16, 31);  //!< Tile Start CTB-X
            };
            uint32_t Value;
        } DW4;
        union
        {
            struct
            {
                uint32_t TileWidth : __CODEGEN_BITFIELD(0, 15);    //!< Tile Width
                uint32_t TileHeight : __CODEGEN_BITFIELD(16, 31);  //!< Tile Height
            };
            uint32_t Value;
        } DW5;
        union
        {
            struct
            {
                uint32_t StreaminOffsetEnable : __CODEGEN_BITFIELD(0, 0);  //!< Streamin Offset enable
                uint32_t Reserved193 : __CODEGEN_BITFIELD(1, 5);           //!< Reserved
                uint32_t TileStreaminOffset : __CODEGEN_BITFIELD(6, 31);   //!< Tile Streamin Offset
            };
            uint32_t Value;
        } DW6;
        union
        {
            struct
            {
                uint32_t RowStoreOffsetEnable : __CODEGEN_BITFIELD(0, 0);  //!< Row store Offset enable
                uint32_t Reserved225 : __CODEGEN_BITFIELD(1, 5);           //!< Reserved
                uint32_t TileRowstoreOffset : __CODEGEN_BITFIELD(6, 31);   //!< Tile Rowstore Offset
            };
            uint32_t Value;
        } DW7;
        union
        {
            struct
            {
                uint32_t TileStreamoutOffsetEnable : __CODEGEN_BITFIELD(0, 0);  //!< Tile streamout offset enable
                uint32_t Reserved257 : __CODEGEN_BITFIELD(1, 5);                //!< Reserved
                uint32_t TileStreamoutOffset : __CODEGEN_BITFIELD(6, 31);       //!< Tile streamout offset
            };
            uint32_t Value;
        } DW8;
        union
        {
            struct
            {
                uint32_t LcuStreamOutOffsetEnable : __CODEGEN_BITFIELD(0, 0);  //!< LCU stream out offset enable
                uint32_t Reserved289 : __CODEGEN_BITFIELD(1, 5);               //!< Reserved
                uint32_t TileLcuStreamOutOffset : __CODEGEN_BITFIELD(6, 31);   //!< Tile LCU stream out offset
            };
            uint32_t Value;
        } DW9;
        union
        {
            struct
            {
                uint32_t Reserved320;  //!< Reserved
            };
            uint32_t Value;
        } DW10;
        union
        {
            struct
            {
                uint32_t DeltaQp : __CODEGEN_BITFIELD(0, 7);                        //!< DELTA_QP
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW11_BIT8 : __CODEGEN_BITFIELD(8, 17);
                uint32_t Reserved370 : __CODEGEN_BITFIELD(18, 23);                  //!< Reserved
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW11_BIT24 : __CODEGEN_BITFIELD(24, 31);
            };
            uint32_t Value;
        } DW11;
        union
        {
            struct
            {
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW12_BIT0 : __CODEGEN_BITFIELD(0, 2);
                uint32_t Reserved387 : __CODEGEN_BITFIELD(3, 7);                       //!< Reserved
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW12_BIT8 : __CODEGEN_BITFIELD(8, 15);
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW12_BIT16 : __CODEGEN_BITFIELD(16, 17);
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW12_BIT18 : __CODEGEN_BITFIELD(18, 22);
                uint32_t Reserved407 : __CODEGEN_BITFIELD(23, 23);                     //!< Reserved
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW12_BIT24 : __CODEGEN_BITFIELD(24, 25);
                uint32_t PaletteModeEnable : __CODEGEN_BITFIELD(26, 26);               //!< Palette Mode Enable
                uint32_t IbcControl : __CODEGEN_BITFIELD(27, 28);                      //!< IBC_CONTROL
                uint32_t Reserved413 : __CODEGEN_BITFIELD(29, 31);                     //!< Reserved
            };
            uint32_t Value;
        } DW12;
        union
        {
            struct
            {
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW13_BIT0 : __CODEGEN_BITFIELD(0, 9);
                uint32_t Reserved426 : __CODEGEN_BITFIELD(10, 15);                  //!< Reserved
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW13_BIT16 : __CODEGEN_BITFIELD(16, 25);
                uint32_t Reserved442 : __CODEGEN_BITFIELD(26, 31);                  //!< Reserved
            };
            uint32_t Value;
        } DW13;
        union
        {
            struct
            {
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW14_BIT0 : __CODEGEN_BITFIELD(0, 5);
                uint32_t Reserved454 : __CODEGEN_BITFIELD(6, 7);                  //!< Reserved
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW14_BIT8 : __CODEGEN_BITFIELD(8, 13);
                uint32_t Reserved462 : __CODEGEN_BITFIELD(14, 15);                //!< Reserved
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW14_BIT16 : __CODEGEN_BITFIELD(16, 20);
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW14_BIT21 : __CODEGEN_BITFIELD(21, 22);
                uint32_t Reserved471 : __CODEGEN_BITFIELD(23, 23);                //!< Reserved
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW14_BIT24 : __CODEGEN_BITFIELD(24, 30);
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW14_BIT31 : __CODEGEN_BITFIELD(31, 31);
            };
            uint32_t Value;
        } DW14;
        union
        {
            struct
            {
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW15_BIT0 : __CODEGEN_BITFIELD(0, 9);
                uint32_t Reserved490 : __CODEGEN_BITFIELD(10, 15);                      //!< Reserved
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW15_BIT16 : __CODEGEN_BITFIELD(16, 31);
            };
            uint32_t Value;
        } DW15;
        union
        {
            struct
            {
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW16_BIT0 : __CODEGEN_BITFIELD(0, 5);
                uint32_t Reserved518 : __CODEGEN_BITFIELD(6, 7);                      //!< Reserved
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW16_BIT8 : __CODEGEN_BITFIELD(8, 13);
                uint32_t Reserved526 : __CODEGEN_BITFIELD(14, 15);                    //!< Reserved
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW16_BIT16 : __CODEGEN_BITFIELD(16, 21);
                uint32_t Reserved534 : __CODEGEN_BITFIELD(22, 23);                    //!< Reserved
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW16_BIT24 : __CODEGEN_BITFIELD(24, 28);
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW16_BIT29 : __CODEGEN_BITFIELD(29, 29);
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW16_BIT30 : __CODEGEN_BITFIELD(30, 30);
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW16_BIT31 : __CODEGEN_BITFIELD(31, 31);
            };
            uint32_t Value;
        } DW16;
        union
        {
            struct
            {
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW17_BIT0 : __CODEGEN_BITFIELD(0, 0);
                uint32_t Reserved545 : __CODEGEN_BITFIELD(1, 5);                   //!< Reserved
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW17_BIT6 : __CODEGEN_BITFIELD(6, 31);
            };
            uint32_t Value;
        } DW17;
        union
        {
            struct
            {
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW18_BIT0 : __CODEGEN_BITFIELD(0, 0);
                uint32_t Reserved577 : __CODEGEN_BITFIELD(1, 5);                       //!< Reserved
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW18_BIT6 : __CODEGEN_BITFIELD(6, 31);
            };
            uint32_t Value;
        } DW18;
        union
        {
            struct
            {
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW19_BIT0;
            };
            uint32_t Value;
        } DW19;
        union
        {
            struct
            {
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW20_BIT0;
            };
            uint32_t Value;
        } DW20;
        union
        {
            struct
            {
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW21_BIT0;
            };
            uint32_t Value;
        } DW21;
        union
        {
            struct
            {
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW22_BIT0;
            };
            uint32_t Value;
        } DW22;
        union
        {
            struct
            {
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW23_BIT0;
            };
            uint32_t Value;
        } DW23;
        union
        {
            struct
            {
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW24_BIT0;
            };
            uint32_t Value;
        } DW24;
        union
        {
            struct
            {
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW25_BIT0;
            };
            uint32_t Value;
        } DW25;
        union
        {
            struct
            {
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW26_BIT0;
            };
            uint32_t Value;
        } DW26;
        union
        {
            struct
            {
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW27_BIT0 : __CODEGEN_BITFIELD(0, 0);
                uint32_t Reserved15 : __CODEGEN_BITFIELD(1, 5);                       //!< Reserved
                uint32_t VDENC_HEVC_VP9_TILE_SLICE_STATE_DW27_BIT6 : __CODEGEN_BITFIELD(6, 31);
            };
            uint32_t Value;
        } DW27;

        //! \brief Explicit member initialization function
        VDENC_HEVC_VP9_TILE_SLICE_STATE_CMD()
        {
            MOS_ZeroMemory(this, sizeof(*this));

            DW0.Value = 0x708d001a;

            DW14.Value = 0x3f400000;

            DW16.Value = 0x003f3f3f;
        }

        static const size_t dwSize   = 28;
        static const size_t byteSize = 112;
    };

    using VDENC_CMD1_CMD = _VDENC_CMD1_CMD;
    using VDENC_CMD2_CMD = _VDENC_CMD2_CMD;
    using VDENC_CMD3_CMD = _VDENC_CMD3_CMD;
    using VDENC_AVC_IMG_STATE_CMD = _VDENC_AVC_IMG_STATE_CMD;
MEDIA_CLASS_DEFINE_END(mhw__vdbox__vdenc__xe2_lpm_base__xe2_lpm__Cmd)
};
}  // namespace xe2_lpm
}  // namespace xe2_lpm_base
}  // namespace vdenc
}  // namespace vdbox
}  // namespace mhw

#pragma pack()

#endif  // __MHW_VDBOX_VDENC_HWCMD_XE2_LPM_H__