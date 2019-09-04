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

class MhwCmdReader final
{
private:
    enum SPECIAL_OPCODES
    {
        MI_NOOP             = 0x00000000,
        MI_BATCH_BUFFER_END = 0x05000000,
        MI_STORE_DATA_IMM   = 0x10000000,
        MI_FLUSH_DW         = 0x13000000,
        MFX_WAIT            = 0x68000000,
    };

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

    void OverrideCmdBuf(uint32_t *cmdBuf, uint32_t dwLen);

private:
    static std::string TrimSpace(const std::string &str);

    void SetFilePath(std::string path);

    void PrepareCmdData();

    void ParseCmdHeader(uint32_t cmdHdr, uint32_t &opcode, uint32_t &dwSize) const;

    void AssignField(uint32_t *cmd, const CmdField &field) const;

    void OverrideOneCmd(uint32_t *cmd, uint32_t opcode, uint32_t dwLen);

private:
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

#endif  //__MHW_CMD_READER_H__
