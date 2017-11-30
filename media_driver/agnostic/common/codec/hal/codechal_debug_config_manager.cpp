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
//!
//! \file     codechal_debug_config_manager.cpp
//! \brief    Defines the dump configuration manager.
//! \details  The debug interface dumps configuration manager file which parse attributes.
//!
#include "codechal_debug.h"
#if USE_CODECHAL_DEBUG_TOOL
#include "codechal_debug_config_manager.h"
#include <fstream>
#include <sstream>

CodechalDebugConfigMgr::CodechalDebugConfigMgr(
    CodechalDebugInterface *debugInterface,
    CODECHAL_FUNCTION       codecFunction,
    std::string             outputFolderPath)
    : m_debugInterface(debugInterface),
    m_codecFunction(codecFunction),
    m_outputFolderPath(outputFolderPath)
{
}

CodechalDebugConfigMgr::~CodechalDebugConfigMgr()
{
    if (m_debugAllConfigs)
    {
        MOS_Delete(m_debugAllConfigs);
    }
}

MOS_STATUS CodechalDebugConfigMgr::ParseConfig()
{
    std::string   configFilePath = m_outputFolderPath + "CodecDbgSetting.cfg";
    std::ifstream configStream(configFilePath);

    if (!configStream.good())
    {
        CODECHAL_DEBUG_VERBOSEMESSAGE("CodecDbgSetting.cfg is not valid");
        return MOS_STATUS_SUCCESS;
    }

    std::string line;

    bool    parseEnable = false;
    int32_t minFrameNum = -1;
    int32_t maxFrameNum = -1;

    while (std::getline(configStream, line))
    {
        if (line.empty())
        {
            continue;
        }
        auto        pos = line.find_first_not_of("\t ");
        std::string newline = line.substr(pos);
        if (newline[0] == '\n' || newline[0] == '#')
        {
            continue;
        }
        else if (newline[0] == '@')  // parse keyword
        {
            if (!strncmp(newline.substr(1, 4).c_str(), "mode", 4))
            {
                auto pos1 = newline.find_first_not_of("\t ", 5);
                if (pos1 == std::string::npos)
                {
                    continue;
                }
                if ((!strncmp(newline.substr(pos1, 3).c_str(), "ALL", 3)) ||
                    (!strncmp(newline.substr(pos1, 6).c_str(), "encode", 6) &&
                        m_codecFunction != CODECHAL_FUNCTION_DECODE) ||
                    (!strncmp(newline.substr(pos1, 6).c_str(), "decode", 6) &&
                    (m_codecFunction == CODECHAL_FUNCTION_DECODE || m_codecFunction == CODECHAL_FUNCTION_CENC_DECODE)))
                {
                    parseEnable = true;
                }
                else
                {
                    parseEnable = false;
                }
                continue;
            }

            if (!parseEnable)
            {
                continue;
            }

            if (!strncmp(newline.substr(1, 5).c_str(), "Frame", 5))
            {
                // start with new frame config
                auto pos1 = newline.find_first_not_of("\t ", 6);
                if (pos1 == std::string::npos)
                    continue;

                minFrameNum = maxFrameNum = -1;
                if (!strncmp(newline.substr(pos1, 3).c_str(), "ALL", 3))
                {
                    // coonfig applid to all frames
                    if (nullptr == m_debugAllConfigs)
                    {
                        m_debugAllConfigs = MOS_New(CodechalDbgCfg);
                    }
                }
                else
                {
                    auto pos2 = newline.find_first_of("-", 6);
                    if (pos2 != std::string::npos)  // handle frame range
                    {
                        minFrameNum = std::stoi(newline.substr(pos1, pos2 - pos1));
                        maxFrameNum = std::stoi(newline.substr(pos2 + 1));
                    }
                    else
                    {
                        minFrameNum = maxFrameNum = std::stoi(newline.substr(pos1));
                    }
                }
            }
            else if (!strncmp(newline.substr(1, 5).c_str(), "Force", 5))
            {

            }
            else if (!strncmp(newline.substr(1, 14).c_str(), "OverwriteCurbe", 14))
            {

            }
            else
            {
                if (minFrameNum < 0)
                {
                    ParseKernelAttribs(newline, nullptr);
                }
                else
                {
                    for (int32_t i = minFrameNum; i <= maxFrameNum; i++)
                    {
                        ParseKernelAttribs(newline, &m_debugFrameConfigs.at(GetFrameConfig(i)));
                    }
                }
            }
        }
        else if (parseEnable)
        {
            if (minFrameNum < 0)
            {
                StoreDebugAttribs(newline, nullptr);
            }
            else
            {
                for (int32_t i = minFrameNum; i <= maxFrameNum; i++)
                {
                    StoreDebugAttribs(newline, &m_debugFrameConfigs.at(GetFrameConfig(i)));
                }
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

uint32_t CodechalDebugConfigMgr::GetFrameConfig(uint32_t frameIdx)
{
    uint32_t pos;
    for (pos = 0; pos < m_debugFrameConfigs.size(); pos++)
    {
        if (m_debugFrameConfigs[pos].frameIndex == frameIdx)
        {
            return pos;
        }
    }
    CodechalDbgCfg frameConfig;
    frameConfig.frameIndex = frameIdx;
    m_debugFrameConfigs.push_back(frameConfig);
    return pos;
}

void CodechalDebugConfigMgr::StoreDebugAttribs(std::string line, CodechalDbgCfg *dbgCfg)
{
    CodechalDbgCfg *config = dbgCfg != nullptr ? dbgCfg : m_debugAllConfigs;
    if (config == nullptr)
    {
        return;
    }

    auto delimeterPos = line.find(':');
    if (delimeterPos == std::string::npos)
    {
        return;
    }

    auto        attrEndPos = line.substr(0, delimeterPos).find_last_not_of("\t ");
    std::string attrName   = line.substr(0, attrEndPos + 1);
    int32_t     attrValue  = std::stoi(line.substr(delimeterPos + 1));

    auto it = config->cmdAttribs.find(attrName);
    if (it != config->cmdAttribs.end())
    {
        config->cmdAttribs.erase(it);
    }

    config->cmdAttribs[attrName] = attrValue;
}

void CodechalDebugConfigMgr::ParseKernelAttribs(std::string line, CodechalDbgCfg *dbgCfg)
{
    CodechalDbgCfg *config = dbgCfg != nullptr ? dbgCfg : m_debugAllConfigs;
    if (config == nullptr)
    {
        return;
    }

    auto        kernelNameEndPos = line.find_first_of("\t ", 1);
    std::string kernelName       = line.substr(1, kernelNameEndPos - 1);

    std::istringstream iss(line.substr(kernelNameEndPos));
    do
    {
        std::string attrName;
        iss >> attrName;
        if (attrName.empty())
        {
            continue;
        }

        if (!strncmp(attrName.c_str(), "ALL", 3))
        {
            config->kernelAttribs[kernelName].dumpDsh         = true;
            config->kernelAttribs[kernelName].dumpSsh         = true;
            config->kernelAttribs[kernelName].dumpIsh         = true;
            config->kernelAttribs[kernelName].dumpCurbe       = true;
            config->kernelAttribs[kernelName].dumpCmdBuffer   = true;
            config->kernelAttribs[kernelName].dump2ndLvlBatch = true;
            config->kernelAttribs[kernelName].dumpInput       = true;
            config->kernelAttribs[kernelName].dumpOutput      = true;
            break;
        }
        else if (!strncmp(attrName.c_str(), CodechalDbgAttr::attrDsh, sizeof(CodechalDbgAttr::attrDsh) - 1))
        {
            config->kernelAttribs[kernelName].dumpDsh = true;
        }
        else if (!strncmp(attrName.c_str(), CodechalDbgAttr::attrSsh, sizeof(CodechalDbgAttr::attrSsh) - 1))
        {
            config->kernelAttribs[kernelName].dumpSsh = true;
        }
        else if (!strncmp(attrName.c_str(), CodechalDbgAttr::attrIsh, sizeof(CodechalDbgAttr::attrIsh) - 1))
        {
            config->kernelAttribs[kernelName].dumpIsh = true;
        }
        else if (!strncmp(attrName.c_str(), CodechalDbgAttr::attrCurbe, sizeof(CodechalDbgAttr::attrCurbe) - 1))
        {
            config->kernelAttribs[kernelName].dumpCurbe = true;
        }
        else if (!strncmp(attrName.c_str(), CodechalDbgAttr::attrCmdBuffer, sizeof(CodechalDbgAttr::attrCmdBuffer) - 1))
        {
            config->kernelAttribs[kernelName].dumpCmdBuffer = true;
        }
        else if (!strncmp(attrName.c_str(), CodechalDbgAttr::attr2ndLvlBatch, sizeof(CodechalDbgAttr::attr2ndLvlBatch) - 1))
        {
            config->kernelAttribs[kernelName].dump2ndLvlBatch = true;
        }
        else if (!strncmp(attrName.c_str(), CodechalDbgAttr::attrInput, sizeof(CodechalDbgAttr::attrInput) - 1))
        {
            config->kernelAttribs[kernelName].dumpInput = true;
        }
        else if (!strncmp(attrName.c_str(), CodechalDbgAttr::attrOutput, sizeof(CodechalDbgAttr::attrOutput) - 1))
        {
            config->kernelAttribs[kernelName].dumpOutput = true;
        }

    } while (iss);
}

MOS_STATUS CodechalDebugConfigMgr::DeleteCfgNode(uint32_t frameIdx)
{
    for (auto it = m_debugFrameConfigs.begin(); it != m_debugFrameConfigs.end(); it++)
    {
        if (it->frameIndex == frameIdx)
        {
            m_debugFrameConfigs.erase(it);
            break;
        }
    }
    return MOS_STATUS_SUCCESS;
}

std::string CodechalDebugConfigMgr::GetMediaStateStr(CODECHAL_MEDIA_STATE_TYPE mediaState)
{
    CodechalDbgKernel::KernelStateMap::kernelMapType &kernelMap = CodechalDbgKernel::KernelStateMap::GetKernelStateMap();
    auto                                              it        = kernelMap.find(mediaState);
    if (it != kernelMap.end())
    {
        return it->second;
    }

    return nullptr;    
}

bool CodechalDebugConfigMgr::AttrIsEnabled(std::string attrName)
{
    if (nullptr != m_debugAllConfigs)
    {
        int attrValue = m_debugAllConfigs->cmdAttribs[attrName];
        if (attrValue > 0)
        {
            return true;
        }
    }

    for (auto it : m_debugFrameConfigs)
    {
        if (it.frameIndex == m_debugInterface->dwBufferDumpFrameNum)
        {
            int attrValue = it.cmdAttribs[attrName];
            return attrValue > 0;
        }
    }

    return false;
}

bool CodechalDebugConfigMgr::AttrIsEnabled(
    CODECHAL_MEDIA_STATE_TYPE mediaState,
    std::string               attrName)
{
    std::string kernelName = GetMediaStateStr(mediaState);
    if (kernelName.empty())
    {
        return false;
    }

    if (nullptr != m_debugAllConfigs)
    {
        KernelDumpConfig attrs   = m_debugAllConfigs->kernelAttribs[kernelName];
        bool             enabled = KernelAttrEnabled(attrs, attrName);
        if (enabled)
        {
            return enabled;
        }
    }

    for (auto it : m_debugFrameConfigs)
    {
        if (it.frameIndex == m_debugInterface->dwBufferDumpFrameNum)
        {
            KernelDumpConfig attrs = it.kernelAttribs[kernelName];
            return KernelAttrEnabled(attrs, attrName);
        }
    }
    return false;
}

bool CodechalDebugConfigMgr::KernelAttrEnabled(KernelDumpConfig kernelConfig, std::string attrName)
{
    if (!strncmp(attrName.c_str(), CodechalDbgAttr::attrDsh, sizeof(CodechalDbgAttr::attrDsh) - 1))
    {
        return kernelConfig.dumpDsh;
    }
    else if (!strncmp(attrName.c_str(), CodechalDbgAttr::attrSsh, sizeof(CodechalDbgAttr::attrSsh) - 1))
    {
        return kernelConfig.dumpSsh;
    }
    else if (!strncmp(attrName.c_str(), CodechalDbgAttr::attrIsh, sizeof(CodechalDbgAttr::attrIsh) - 1))
    {
        return kernelConfig.dumpIsh;
    }
    else if (!strncmp(attrName.c_str(), CodechalDbgAttr::attrCurbe, sizeof(CodechalDbgAttr::attrCurbe) - 1))
    {
        return kernelConfig.dumpCurbe;
    }
    else if (!strncmp(attrName.c_str(), CodechalDbgAttr::attrCmdBuffer, sizeof(CodechalDbgAttr::attrCmdBuffer) - 1))
    {
        return kernelConfig.dumpCmdBuffer;
    }
    else if (!strncmp(attrName.c_str(), CodechalDbgAttr::attr2ndLvlBatch, sizeof(CodechalDbgAttr::attr2ndLvlBatch) - 1))
    {
        return kernelConfig.dump2ndLvlBatch;
    }
    else if (!strncmp(attrName.c_str(), CodechalDbgAttr::attrInput, sizeof(CodechalDbgAttr::attrInput) - 1))
    {
        return kernelConfig.dumpInput;
    }
    else if (!strncmp(attrName.c_str(), CodechalDbgAttr::attrOutput, sizeof(CodechalDbgAttr::attrOutput) - 1))
    {
        return kernelConfig.dumpOutput;
    }
    else
        return false;
}

#endif  // USE_CODECHAL_DEBUG_TOOL
