/*
* Copyright (c) 2018, Intel Corporation
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

class MhwCmdReader final
{
private:

    struct CmdField
    {
        union
        {
            struct
            {
                uint32_t cmdIdx      : 8;
                uint32_t dwordIdx    : 8;
                uint32_t stardBit    : 5;
                uint32_t endBit      : 5;
                uint32_t longTermUse : 1;
                uint32_t reserved    : 5;
            };
            uint32_t value;
        } info;

        uint32_t value;
    };

public:

    static std::shared_ptr<MhwCmdReader> GetInstance();

    MhwCmdReader() = default;

    ~MhwCmdReader() = default;

    void OverrideCmdDataFromFile(std::string cmdName, uint32_t cmdLen, uint32_t *cmd);

private:

    void SetFilePath(std::string path);

    void PrepareCmdData();

    void AssignField(uint32_t *cmd, const CmdField &field) const;

private:

    static std::shared_ptr<MhwCmdReader> m_instance;

    bool                     m_ready = false;
    std::string              m_path;
    std::vector<CmdField>    m_longTermFields;
    std::list<CmdField>      m_shortTermFields;
    std::vector<std::string> m_cmdNames;
};

#define OVERRIDE_CMD_DATA(cmdName, dwordSize, cmd) MhwCmdReader::GetInstance()->OverrideCmdDataFromFile(cmdName, dwordSize, cmd)

#else // _RELEASE

#define OVERRIDE_CMD_DATA(cmdName, dwordSize, cmd)

#endif // _DEBUG || _RELEASE_INTERNAL

#endif //__MHW_CMD_READER_H__
