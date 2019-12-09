/*
* Copyright (c) 2019, Intel Corporation
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
//! \file      mhw_cmd_reader.h
//! \brief     This modules implements read command fields from file to override driver settings, the input file is binary, starts from a 32-bit
//!            integer indicates number of CmdField, followed by CmdField and space separated strings which are command names
//!

#ifndef __MHW_CMD_READER_H__
#define __MHW_CMD_READER_H__

#if (_DEBUG || _RELEASE_INTERNAL)

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <list>
#include <set>

#define OPCODE_DEF(cmd, opcode) cmd = (opcode)

class MhwCmdReader
{
public:
    enum OPCODE
    {
        // MI commands
        OPCODE_DEF(MI_NOOP              , 0x00000000),
        OPCODE_DEF(MI_BATCH_BUFFER_END  , 0x05000000),
        OPCODE_DEF(MI_FORCE_WAKEUP      , 0x0e800000),
        OPCODE_DEF(MI_STORE_DATA_IMM    , 0x10000000),
        OPCODE_DEF(MI_LOAD_REGISTER_IMM , 0x11000000),
        OPCODE_DEF(MI_STORE_REGISTER_MEM, 0x12000000),
        OPCODE_DEF(MI_FLUSH_DW          , 0x13000000),
        OPCODE_DEF(MI_LOAD_REGISTER_MEM , 0x14800000),
        OPCODE_DEF(MI_ATOMIC            , 0x17800000),
        OPCODE_DEF(MI_BATCH_BUFFER_START, 0x18800000),

        // HCP commands

        // HUC commands
        OPCODE_DEF(HUC_PIPE_MODE_SELECT       , 0x75800000),
        OPCODE_DEF(HUC_IMEM_STATE             , 0x75810000),
        OPCODE_DEF(HUC_DMEM_STATE             , 0x75820000),
        OPCODE_DEF(HUC_VIRTUAL_ADDR_STATE     , 0x75840000),
        OPCODE_DEF(HUC_IND_OBJ_BASE_ADDR_STATE, 0x75850000),

        // MFX commands
        OPCODE_DEF(MFX_PIPE_MODE_SELECT       , 0x70000000),
        OPCODE_DEF(MFX_SURFACE_STATE          , 0x70010000),
        OPCODE_DEF(MFX_PIPE_BUF_ADDR_STATE    , 0x70020000),
        OPCODE_DEF(MFX_IND_OBJ_BASE_ADDR_STATE, 0x70030000),
        OPCODE_DEF(MFX_BSP_BUF_BASE_ADDR_STATE, 0x70040000),
        OPCODE_DEF(MFX_QM_STATE               , 0x70070000),
        OPCODE_DEF(MFX_FQM_STATE              , 0x70080000),
        OPCODE_DEF(MFX_PAK_INSERT_OBJECT      , 0x70480000),
        OPCODE_DEF(MFX_AVC_IMG_STATE          , 0x71000000),
        OPCODE_DEF(MFX_AVC_DIRECTMODE_STATE   , 0x71020000),
        OPCODE_DEF(MFX_AVC_SLICE_STATE        , 0x71030000),
        OPCODE_DEF(MFX_AVC_REF_IDX_STATE      , 0x71040000),

        // VDENC commands
        OPCODE_DEF(MFX_WAIT                       , 0x68000000),
        OPCODE_DEF(VDENC_PIPE_MODE_SELECT         , 0x70800000),
        OPCODE_DEF(VDENC_SRC_SURFACE_STATE        , 0x70810000),
        OPCODE_DEF(VDENC_REF_SURFACE_STATE        , 0x70820000),
        OPCODE_DEF(VDENC_DS_REF_SURFACE_STATE     , 0x70830000),
        OPCODE_DEF(VDENC_PIPE_BUF_ADDR_STATE      , 0x70840000),
        OPCODE_DEF(VDENC_AVC_IMG_STATE            , 0x70850000),
        OPCODE_DEF(VDENC_WALKER_STATE             , 0x70870000),
        OPCODE_DEF(VDENC_WEIGHTSOFFSETS_STATE     , 0x70880000),
        OPCODE_DEF(VDENC_HEVC_VP9_IMAGE_STATE     , 0x70890000),
        OPCODE_DEF(VDENC_CONTROL_STATE            , 0x708b0000),
        OPCODE_DEF(VDENC_AVC_SLICE_STATE          , 0x708c0000),
        OPCODE_DEF(VDENC_HEVC_VP9_TILE_SLICE_STATE, 0x708d0000),
        OPCODE_DEF(VD_CONTROL_STATE               , 0x738a0000),
        OPCODE_DEF(VD_PIPELINE_FLUSH              , 0x77800000),
    };

protected:
    struct CmdField
    {
        union
        {
            struct
            {
                uint32_t opcode;

                uint32_t dwordIdx : 21;
                uint32_t stardBit : 5;
                uint32_t endBit : 5;
                uint32_t longTerm : 1;
            };
            uint64_t value;
        } info;

        uint32_t overrideData;
    };

public:
    static std::shared_ptr<MhwCmdReader> GetInstance();

    MhwCmdReader() = default;

    ~MhwCmdReader() = default;

    std::pair<uint32_t *, uint32_t> FindCmd(uint32_t *cmdBuf, uint32_t dwLen, OPCODE opcode) const;

    void OverrideCmdBuf(uint32_t *cmdBuf, uint32_t dwLen);

protected:
    static std::string GetOverrideDataPath();

    static std::string TrimSpace(const std::string &str);

    void SetFilePath(std::string path);

    void PrepareCmdData();

    void ParseCmdHeader(uint32_t cmdHdr, uint32_t &opcode, uint32_t &dwSize) const;

    void AssignField(uint32_t *cmd, const CmdField &field) const;

    void OverrideOneCmd(uint32_t *cmd, uint32_t opcode, uint32_t dwLen);

protected:
    static std::shared_ptr<MhwCmdReader> m_instance;

    bool                  m_ready = false;
    std::string           m_path;
    std::vector<CmdField> m_longTermFields;
    std::list<CmdField>   m_shortTermFields;
};

#define OVERRIDE_CMD_DATA(cmdBuf, dwLen) MhwCmdReader::GetInstance()->OverrideCmdBuf(cmdBuf, dwLen)

#else  // _RELEASE

#define OVERRIDE_CMD_DATA(cmdBuf, dwLen)

#endif  // _DEBUG || _RELEASE_INTERNAL

#endif  // __MHW_CMD_READER_H__
