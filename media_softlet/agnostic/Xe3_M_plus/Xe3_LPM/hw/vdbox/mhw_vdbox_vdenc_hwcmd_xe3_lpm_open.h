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
//! \file     mhw_vdbox_vdenc_hwcmd_xe3_lpm_open.h
//! \brief    
//! \details  VDENC mhw hwcmd file
//!           
//!

#ifndef __MHW_VDBOX_VDENC_HWCMD_XE3_LPM_OPEN_H__
#define __MHW_VDBOX_VDENC_HWCMD_XE3_LPM_OPEN_H__

#ifndef _MEDIA_RESERVED
#include "mhw_hwcmd.h"

#pragma once
#pragma pack(1)


namespace mhw
{
namespace vdbox
{
namespace vdenc
{
namespace xe3_lpm_base
{
namespace xe3_lpm
{

class Cmd
{
public:
    // Internal Macros
    #define __CODEGEN_MAX(_a, _b) (((_a) > (_b)) ? (_a) : (_b))
    #define __CODEGEN_BITFIELD(l, h) (h) - (l) + 1
    #define __CODEGEN_OP_LENGTH_BIAS 2
    #define __CODEGEN_OP_LENGTH(x) (uint32_t)((__CODEGEN_MAX(x, __CODEGEN_OP_LENGTH_BIAS)) - __CODEGEN_OP_LENGTH_BIAS)

    static uint32_t GetOpLength(uint32_t uiLength) { return __CODEGEN_OP_LENGTH(uiLength); }

    struct VDENC_CONTROL_STATE_CMD{};

    struct VDENC_PIPE_MODE_SELECT_CMD{};

    struct VDENC_SRC_SURFACE_STATE_CMD{};

    struct VDENC_REF_SURFACE_STATE_CMD{};

    struct VDENC_DS_REF_SURFACE_STATE_CMD{};

    struct VDENC_PIPE_BUF_ADDR_STATE_CMD{};

    struct VDENC_WEIGHTSOFFSETS_STATE_CMD{};

    struct VDENC_HEVC_VP9_TILE_SLICE_STATE_CMD{};

    struct VDENC_WALKER_STATE_CMD{};

    struct VDENC_AVC_SLICE_STATE_CMD{};

    struct VDENC_AVC_IMG_STATE_CMD{};

    struct VDENC_CMD1_CMD{};

    struct VDENC_CMD2_CMD{};

    struct VDENC_CMD3_CMD{};

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
                uint32_t VdaqmPipelineDone : __CODEGEN_BITFIELD(7, 7);            //!< VDAQM pipeline Done
                uint32_t VvcpPipelineDone : __CODEGEN_BITFIELD(8, 8);             //!< VVCP Pipeline Done
                uint32_t Reserved40 : __CODEGEN_BITFIELD(9, 15);                  //!< Reserved
                uint32_t HevcPipelineCommandFlush : __CODEGEN_BITFIELD(16, 16);   //!< HEVC pipeline command flush
                uint32_t VdencPipelineCommandFlush : __CODEGEN_BITFIELD(17, 17);  //!< VD-ENC pipeline command flush
                uint32_t MflPipelineCommandFlush : __CODEGEN_BITFIELD(18, 18);    //!< MFL pipeline command flush
                uint32_t MfxPipelineCommandFlush : __CODEGEN_BITFIELD(19, 19);    //!< MFX pipeline command flush
                uint32_t HucPipelineCommandFlush : __CODEGEN_BITFIELD(20, 20);    //!< HuC pipeline command flush
                uint32_t AvpPipelineCommandFlush : __CODEGEN_BITFIELD(21, 21);    //!< AVP Pipeline command Flush
                uint32_t VdaqmPipelineCommandFlush : __CODEGEN_BITFIELD(22, 22);  //!< VDAQM pipeline command flush
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
MEDIA_CLASS_DEFINE_END(mhw__vdbox__vdenc__xe3_lpm_base__xe3_lpm__Cmd)
};
}  // namespace xe3_lpm
}  // namespace xe3_lpm_base
}  // namespace vdenc
}  // namespace vdbox
}  // namespace mhw

#pragma pack()

#endif  // _MEDIA_RESERVED
#endif  // __MHW_VDBOX_VDENC_HWCMD_XE3_LPM_OPEN_H__
