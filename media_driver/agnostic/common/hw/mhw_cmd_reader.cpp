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
//! \file      mhw_cmd_reader.cpp
//! \brief     Implementation of class MhwCmdReader
//!

#if (_DEBUG || _RELEASE_INTERNAL)

#include "mhw_cmd_reader.h"
#include "mos_utilities.h"
#include <algorithm>
#include <fstream>
#include <sstream>

using namespace std;

shared_ptr<MhwCmdReader> MhwCmdReader::m_instance = nullptr;

shared_ptr<MhwCmdReader> MhwCmdReader::GetInstance()
{
    if (!m_instance)
    {
        m_instance = make_shared<MhwCmdReader>();
        m_instance->SetFilePath(GetOverrideDataPath());
        m_instance->PrepareCmdData();
    }
    return m_instance;
}

pair<uint32_t *, uint32_t> MhwCmdReader::FindCmd(uint32_t *cmdBuf, uint32_t dwLen, OPCODE opcode) const
{
    uint32_t                   currOpcode, currLen;
    uint32_t                   offset = 0;
    pair<uint32_t *, uint32_t> cmd(nullptr, 0);

    while (offset < dwLen)
    {
        ParseCmdHeader(cmdBuf[offset], currOpcode, currLen);

        if (currOpcode == MI_BATCH_BUFFER_END || currOpcode == opcode)
        {
            if (currOpcode == opcode)
            {
                cmd.first  = &cmdBuf[offset];
                cmd.second = currLen;
            }
            return cmd;
        }

        offset += currLen;
    }

    return cmd;
}

void MhwCmdReader::OverrideCmdBuf(uint32_t *cmdBuf, uint32_t dwLen)
{
    if (!m_ready)
    {
        return;
    }

    uint32_t opcode, len;
    uint32_t offset = 0;

    while (offset < dwLen)
    {
        ParseCmdHeader(cmdBuf[offset], opcode, len);

        if (opcode == MI_BATCH_BUFFER_END)
        {
            break;
        }

        OverrideOneCmd(&cmdBuf[offset], opcode, len);
        offset += len;
    }
}

string MhwCmdReader::GetOverrideDataPath()
{
    char                        path[1024]      = {};
    MOS_USER_FEATURE_VALUE_DATA userFeatureData = {};

    userFeatureData.StringData.pStringData = path;
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_COMMAND_OVERRIDE_INPUT_FILE_PATH_ID,
        &userFeatureData);

    return path;
}

string MhwCmdReader::TrimSpace(const string &str)
{
    int32_t sz  = static_cast<int32_t>(str.size());
    int32_t beg = 0;
    while (beg < sz)
    {
        if (str[beg] != ' ')
        {
            break;
        }
        ++beg;
    }

    int32_t end = sz - 1;
    while (end >= beg)
    {
        if (str[end] != ' ')
        {
            break;
        }
        --end;
    }

    if (beg < sz)
    {
        return str.substr(beg, static_cast<int64_t>(end) - beg + 1);
    }
    else
    {
        return string();
    }
}

void MhwCmdReader::SetFilePath(string path)
{
    if (!path.empty())
    {
        m_path = path;
    }
}

void MhwCmdReader::PrepareCmdData()
{
    ifstream fs(m_path);

    if (!fs)
    {
        return;
    }

    string line;
    getline(fs, line);  // ignore the first line

    while (getline(fs, line))
    {
        CmdField field = {};

        size_t beg        = 0;
        size_t end        = line.find(',');
        field.info.opcode = stoul(TrimSpace(line.substr(beg, end - beg)), 0, 16);

        beg                 = end + 1;
        end                 = line.find(',', beg);
        field.info.dwordIdx = stoul(TrimSpace(line.substr(beg, end - beg)));

        beg                 = end + 1;
        end                 = line.find(',', beg);
        field.info.stardBit = stoul(TrimSpace(line.substr(beg, end - beg)));

        beg               = end + 1;
        end               = line.find(',', beg);
        field.info.endBit = stoul(TrimSpace(line.substr(beg, end - beg)));

        beg                 = end + 1;
        end                 = line.find(',', beg);
        field.info.longTerm = stoul(TrimSpace(line.substr(beg, end - beg)));

        beg                = end + 1;
        end                = line.find(',', beg);
        field.overrideData = stoul(TrimSpace(line.substr(beg, end - beg)), 0, 0);

        if (field.info.longTerm)
        {
            m_longTermFields.push_back(field);
        }
        else
        {
            m_shortTermFields.push_back(field);
        }
    }

    fs.close();
    m_ready = true;
}

void MhwCmdReader::ParseCmdHeader(uint32_t cmdHdr, uint32_t &opcode, uint32_t &dwSize) const
{
    if (cmdHdr >> 29 == 0)  // MI commands
    {
        opcode = (cmdHdr >> 23) << 23;

        if (opcode == MI_NOOP || opcode == MI_BATCH_BUFFER_END)
        {
            dwSize = 1;
        }
        else if (opcode == MI_FLUSH_DW)
        {
            dwSize = (cmdHdr & 0x1f) + 2;
        }
        else if (opcode == MI_STORE_DATA_IMM)
        {
            dwSize = (cmdHdr & 0x3ff) + 2;
        }
        else
        {
            dwSize = (cmdHdr & 0xff) + 2;
        }
    }
    else if (cmdHdr >> 29 == 3)  // VDBOX commands
    {
        opcode = (cmdHdr >> 16) << 16;

        if (opcode == MFX_WAIT)
        {
            dwSize = 1;
        }
        else
        {
            dwSize = (cmdHdr & 0xfff) + 2;
        }
    }
    else  // unknown opcodes
    {
        opcode = cmdHdr;
        dwSize = 1;
    }
}

void MhwCmdReader::AssignField(uint32_t *cmd, const CmdField &field) const
{
    if (field.info.stardBit > field.info.endBit)
    {
        return;
    }

    uint32_t mask = 0;
    for (uint32_t i = 0; i < field.info.stardBit; i++)
    {
        mask |= (1 << i);
    }
    for (uint32_t i = 31; i > field.info.endBit; i--)
    {
        mask |= (1 << i);
    }

    cmd[field.info.dwordIdx] &= mask;
    cmd[field.info.dwordIdx] |= (field.overrideData << field.info.stardBit);
}

void MhwCmdReader::OverrideOneCmd(uint32_t *cmd, uint32_t opcode, uint32_t dwLen)
{
    for (const auto &e : m_longTermFields)
    {
        if (e.info.opcode == opcode && e.info.dwordIdx < dwLen)
        {
            AssignField(cmd, e);
        }
    }

    auto it = m_shortTermFields.begin();
    while (it != m_shortTermFields.end())
    {
        if (it->info.opcode == opcode)
        {
            if (it->info.dwordIdx == 0x1fffff)  // 0x1fffff is a delimiter which indicates following override data are for next time adding this command
            {
                m_shortTermFields.erase(it);
                break;
            }
            else
            {
                if (it->info.dwordIdx < dwLen)
                {
                    AssignField(cmd, *it);
                }
                it = m_shortTermFields.erase(it);
            }
        }
        else
        {
            ++it;
        }
    }
}

#endif  // _DEBUG || _RELEASE_INTERNAL
