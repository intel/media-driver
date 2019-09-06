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
        char path[1024] = {};
        MOS_USER_FEATURE_VALUE_DATA userFeatureData = {};

        m_instance = make_shared<MhwCmdReader>();

        userFeatureData.StringData.pStringData = path;
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_COMMAND_OVERRIDE_INPUT_FILE_PATH_ID,
            &userFeatureData);
        m_instance->SetFilePath(path);

        m_instance->PrepareCmdData();
    }
    return m_instance;
}

void MhwCmdReader::OverrideCmdDataFromFile(string cmdName, uint32_t cmdLen, uint32_t *cmd)
{
    if (!m_ready)
    {
        return;
    }

    for (const auto &e : m_longTermFields)
    {
        if (m_cmdNames[e.info.cmdIdx] == cmdName && e.info.dwordIdx < cmdLen)
        {
            AssignField(cmd, e);
        }
    }

    auto it = m_shortTermFields.begin();
    while (it != m_shortTermFields.end())
    {
        if (m_cmdNames[it->info.cmdIdx] == cmdName)
        {
            uint32_t cmdIdx   = it->info.cmdIdx;
            uint32_t dwordIdx = it->info.dwordIdx;
            uint32_t stardBit = it->info.stardBit;

            if (it->info.dwordIdx < cmdLen)
            {
                AssignField(cmd, *it);
            }

            it = m_shortTermFields.erase(it);
            while (it != m_shortTermFields.end() && it->info.cmdIdx == cmdIdx &&
                it->info.dwordIdx == dwordIdx && it->info.stardBit == stardBit)
            {
                ++it;
            }
        }
        else
        {
            ++it;
        }
    }
}

void MhwCmdReader::SetFilePath(string path)
{
    if (!path.empty())
    {
        m_path = path;
    }
}

void MhwCmdReader::GetFileType()
{
    m_fileType = FileType::BIN;

    for (auto it = m_path.rbegin(); it != m_path.rend(); ++it)
    {
        if (*it == '.')
        {
            string suffix(m_path.cbegin() + (m_path.rend() - it), m_path.cend());
            for (auto &e : suffix)
            {
                e = static_cast<char>(toupper(e));
            }

            if (suffix.compare("CSV") == 0)
            {
                m_fileType = FileType::CSV;
            }
            break;
        }
    }
}

void MhwCmdReader::PrepareCmdData()
{
    GetFileType();

    if (m_fileType == FileType::BIN)
    {
        PrepareCmdDataBin();
    }
    else
    {
        PrepareCmdDataCsv();
    }
}

void MhwCmdReader::PrepareCmdDataBin()
{
    ifstream fs(m_path, ios::binary);

    if (!fs)
    {
        return;
    }

    uint32_t fieldNum;
    fs.read((char *)&fieldNum, sizeof(fieldNum));

    CmdField field;
    for (uint32_t i = 0; i < fieldNum; i++)
    {
        fs.read((char *)&field, sizeof(CmdField));
        if (field.info.longTermUse)
        {
            m_longTermFields.push_back(field);
        }
        else
        {
            m_shortTermFields.push_back(field);
        }
    }

    string cmdName;
    while (fs)
    {
        fs >> cmdName;
        if (!cmdName.empty())
        {
            m_cmdNames.push_back(std::move(cmdName));
        }
    }

    fs.close();
    m_ready = true;
}

void MhwCmdReader::PrepareCmdDataCsv()
{
    ifstream fs(m_path);

    if (!fs)
    {
        return;
    }

    string line;
    getline(fs, line);

    set<string> names;
    vector<pair<CmdField, string>> data;
    while (getline(fs, line))
    {
        CmdField field = {};
        string name;

        size_t beg = 0;
        size_t end = line.find(',');
        name = line.substr(beg, end);
        names.insert(name);

        beg = end + 1;
        end = line.find(',', beg);
        field.info.dwordIdx = stoul(line.substr(beg, end));

        beg = end + 1;
        end = line.find(',', beg);
        field.info.stardBit = stoul(line.substr(beg, end));

        beg = end + 1;
        end = line.find(',', beg);
        field.info.endBit = stoul(line.substr(beg, end));

        beg = end + 1;
        end = line.find(',', beg);
        field.info.longTermUse = stoul(line.substr(beg, end));

        beg = end + 1;
        end = line.find(',', beg);
        field.value = stoul(line.substr(beg, end), 0, 0);

        data.push_back(make_pair(field, name));
    }

    m_cmdNames.assign(names.cbegin(), names.cend());
    sort(m_cmdNames.begin(), m_cmdNames.end());

    for (auto &e : data)
    {
        e.first.info.cmdIdx = find(m_cmdNames.begin(), m_cmdNames.end(), e.second) - m_cmdNames.begin();
        if (e.first.info.longTermUse)
        {
            m_longTermFields.push_back(e.first);
        }
        else
        {
            m_shortTermFields.push_back(e.first);
        }
    }

    fs.close();
    m_ready = true;
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
    cmd[field.info.dwordIdx] |= (field.value << field.info.stardBit);
}

#endif // _DEBUG || _RELEASE_INTERNAL
