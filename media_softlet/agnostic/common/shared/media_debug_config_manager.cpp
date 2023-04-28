/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     media_debug_config_manager.cpp
//! \brief    Defines the dump configuration manager.
//! \details  The debug interface dumps configuration manager file which parse attributes.
//!

#include "media_debug_interface.h"
#if USE_MEDIA_DEBUG_TOOL
#include "media_debug_config_manager.h"
#include <fstream>
#include <sstream>

MediaDebugConfigMgr::MediaDebugConfigMgr(
    std::string outputFolderPath)
    : m_outputFolderPath(outputFolderPath)
{
}

MediaDebugConfigMgr::~MediaDebugConfigMgr()
{
    if (m_debugAllConfigs != nullptr)
    {
        MOS_Delete(m_debugAllConfigs);
    }
}

MOS_STATUS MediaDebugConfigMgr::ParseConfig(MOS_CONTEXT_HANDLE mosCtx)
{
    std::string               configFileName;
    bool                      isGenCfgEnabled   = false;

    configFileName    = InitFileName(m_mediaFunction);
    configFileName    = m_outputFolderPath + configFileName;
    std::ifstream configStream(configFileName);

    if (!configStream.good())
    {
        ReadUserSettingForDebug(
            GetUserSettingInstance(),
            isGenCfgEnabled,
            __MEDIA_USER_FEATURE_VALUE_MEDIA_DEBUG_CFG_GENERATION,
            MediaUserSetting::Group::Device);
        if (isGenCfgEnabled)
        {
            GenerateDefaultConfig(configFileName);
        }
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
        auto        pos     = line.find_first_not_of("\t ");
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
                        m_mediaFunction == MEDIA_FUNCTION_ENCODE) ||
                    (!strncmp(newline.substr(pos1, 6).c_str(), "decode", 6) &&
                        (m_mediaFunction == MEDIA_FUNCTION_DECODE ||
                         m_mediaFunction == MEDIA_FUNCTION_CENC_DECODE)) ||
                    (!strncmp(newline.substr(pos1, 2).c_str(), "vp", 2) &&
                        (m_mediaFunction == MEDIA_FUNCTION_VP)))
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
                        m_debugAllConfigs = MOS_New(MediaDbgCfg);
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

uint32_t MediaDebugConfigMgr::GetFrameConfig(uint32_t frameIdx)
{
    uint32_t pos;
    for (pos = 0; pos < m_debugFrameConfigs.size(); pos++)
    {
        if (m_debugFrameConfigs[pos].frameIndex == frameIdx)
        {
            return pos;
        }
    }
    MediaDbgCfg frameConfig;
    frameConfig.frameIndex = frameIdx;
    m_debugFrameConfigs.push_back(frameConfig);
    return pos;
}

void MediaDebugConfigMgr::StoreDebugAttribs(std::string line, MediaDbgCfg *dbgCfg)
{
    MediaDbgCfg *config = dbgCfg != nullptr ? dbgCfg : m_debugAllConfigs;
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

void MediaDebugConfigMgr::ParseKernelAttribs(std::string line, MediaDbgCfg *dbgCfg)
{
    MediaDbgCfg *config = dbgCfg != nullptr ? dbgCfg : m_debugAllConfigs;
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
        else if (!strncmp(attrName.c_str(), MediaDbgAttr::attrDsh, sizeof(MediaDbgAttr::attrDsh) - 1))
        {
            config->kernelAttribs[kernelName].dumpDsh = true;
        }
        else if (!strncmp(attrName.c_str(), MediaDbgAttr::attrSsh, sizeof(MediaDbgAttr::attrSsh) - 1))
        {
            config->kernelAttribs[kernelName].dumpSsh = true;
        }
        else if (!strncmp(attrName.c_str(), MediaDbgAttr::attrIsh, sizeof(MediaDbgAttr::attrIsh) - 1))
        {
            config->kernelAttribs[kernelName].dumpIsh = true;
        }
        else if (!strncmp(attrName.c_str(), MediaDbgAttr::attrCurbe, sizeof(MediaDbgAttr::attrCurbe) - 1))
        {
            config->kernelAttribs[kernelName].dumpCurbe = true;
        }
        else if (!strncmp(attrName.c_str(), MediaDbgAttr::attrCmdBuffer, sizeof(MediaDbgAttr::attrCmdBuffer) - 1))
        {
            config->kernelAttribs[kernelName].dumpCmdBuffer = true;
        }
        else if (!strncmp(attrName.c_str(), MediaDbgAttr::attr2ndLvlBatch, sizeof(MediaDbgAttr::attr2ndLvlBatch) - 1))
        {
            config->kernelAttribs[kernelName].dump2ndLvlBatch = true;
        }
        else if (!strncmp(attrName.c_str(), MediaDbgAttr::attrInput, sizeof(MediaDbgAttr::attrInput) - 1))
        {
            config->kernelAttribs[kernelName].dumpInput = true;
        }
        else if (!strncmp(attrName.c_str(), MediaDbgAttr::attrOutput, sizeof(MediaDbgAttr::attrOutput) - 1))
        {
            config->kernelAttribs[kernelName].dumpOutput = true;
        }

    } while (iss);
}

MOS_STATUS MediaDebugConfigMgr::DeleteCfgNode(uint32_t frameIdx)
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

bool MediaDebugConfigMgr::AttrIsEnabled(std::string attrName)
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
        if (it.frameIndex == GetDumpFrameNum())
        {
            int attrValue = it.cmdAttribs[attrName];
            return attrValue > 0;
        }
    }

    return false;
}

bool MediaDebugConfigMgr::KernelAttrEnabled(MediaKernelDumpConfig kernelConfig, std::string attrName)
{
    if (!strncmp(attrName.c_str(), MediaDbgAttr::attrDsh, sizeof(MediaDbgAttr::attrDsh) - 1))
    {
        return kernelConfig.dumpDsh;
    }
    else if (!strncmp(attrName.c_str(), MediaDbgAttr::attrSsh, sizeof(MediaDbgAttr::attrSsh) - 1))
    {
        return kernelConfig.dumpSsh;
    }
    else if (!strncmp(attrName.c_str(), MediaDbgAttr::attrIsh, sizeof(MediaDbgAttr::attrIsh) - 1))
    {
        return kernelConfig.dumpIsh;
    }
    else if (!strncmp(attrName.c_str(), MediaDbgAttr::attrCurbe, sizeof(MediaDbgAttr::attrCurbe) - 1))
    {
        return kernelConfig.dumpCurbe;
    }
    else if (!strncmp(attrName.c_str(), MediaDbgAttr::attrCmdBuffer, sizeof(MediaDbgAttr::attrCmdBuffer) - 1))
    {
        return kernelConfig.dumpCmdBuffer;
    }
    else if (!strncmp(attrName.c_str(), MediaDbgAttr::attr2ndLvlBatch, sizeof(MediaDbgAttr::attr2ndLvlBatch) - 1))
    {
        return kernelConfig.dump2ndLvlBatch;
    }
    else if (!strncmp(attrName.c_str(), MediaDbgAttr::attrInput, sizeof(MediaDbgAttr::attrInput) - 1))
    {
        return kernelConfig.dumpInput;
    }
    else if (!strncmp(attrName.c_str(), MediaDbgAttr::attrOutput, sizeof(MediaDbgAttr::attrOutput) - 1))
    {
        return kernelConfig.dumpOutput;
    }
    else
        return false;
}

void MediaDebugConfigMgr::GenerateDefaultConfig(std::string configFileName)
{
    std::ofstream ofs(configFileName);

    ofs << "###################################################################" << std::endl;
    ofs << "## - White space should be agnostic." << std::endl;
    ofs << "## - '#' as first non-white space char denotes line as comment" << std::endl;
    ofs << "## - '@frame ALL' MUST be present as the first directive, even if " << std::endl;
    ofs << "##   nothing is dumped for every frame." << std::endl;
    ofs << "###################################################################" << std::endl;
    ofs << std::endl;

    ofs << "###################################################################" << std::endl;
    ofs << "## key words defined under @mode ALL works for encode decode vp" << std::endl;
    ofs << "###################################################################" << std::endl;
    ofs << std::endl;
    ofs << "@mode ALL" << std::endl;
    ofs << "@Frame ALL" << std::endl;
    ofs << std::endl;

    ofs << "#" << MediaDbgAttr::attrPicParams << ":0" << std::endl;
    ofs << "#" << MediaDbgAttr::attrSlcParams << ":0" << std::endl;
    ofs << "#" << MediaDbgAttr::attrSubsetsParams << ":0" << std::endl;
    ofs << "#" << MediaDbgAttr::attrIqParams << ":0" << std::endl;
    ofs << "#" << MediaDbgAttr::attrDecodeBitstream << ":0" << std::endl;
    ofs << "#" << MediaDbgAttr::attrBitstream << ":0" << std::endl;
    ofs << "#" << MediaDbgAttr::attrHucRegions << ":0" << std::endl;
    ofs << "#" << MediaDbgAttr::attrHuCDmem << ":0" << std::endl;
    ofs << "#" << MediaDbgAttr::attrCmdBufferMfx << ":0" << std::endl;
    ofs << "#" << MediaDbgAttr::attr2ndLvlBatchMfx << ":0" << std::endl;
    ofs << "#" << MediaDbgAttr::attrSurfaceInfo << ":0" << std::endl;
    ofs << "#" << MediaDbgAttr::attrHuffmanTbl << ":0" << std::endl;
    ofs << "#" << MediaDbgAttr::attrScanParams << ":0" << std::endl;
    ofs << "#" << MediaDbgAttr::attrDriverUltDump << ":0" << std::endl;
    ofs << "#" << MediaDbgAttr::attrDumpBufferInBinary << ":0" << std::endl;
    ofs << "#" << MediaDbgAttr::attrDumpToThreadFolder << ":0" << std::endl;
    ofs << "#" << MediaDbgAttr::attrDumpCmdBufInBinary << ":0" << std::endl;
    ofs << "#" << MediaDbgAttr::attrEnableFastDump << ":1" << std::endl;
    ofs << "#" << MediaDbgAttr::attrStatusReport << ":0" << std::endl;
    ofs << std::endl;

    ofs << "##" << MediaDbgAttr::attrStreamOut << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrStreamIn << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrResidualDifference << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrDeblocking << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrMvData << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrForceYUVDumpWithMemcpy << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrDisableSwizzleForDumps << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrSfcOutputSurface << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrSfcBuffers << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrDecodeReferenceSurfaces << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrReferenceSurfaces << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrEncodeRawInputSurface << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrReconstructedSurface << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrPakInput << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrPakOutput << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrUpsamlingInput << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrResidualSurface << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrStCoeff << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrCoeffPredCs << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrMbRecord << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrPakObjStreamout << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrTileBasedStats << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrOverwriteCommands << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrForceCmdDumpLvl << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrForceCurbeDumpLvl << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrFrameState << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrBrcPakStats << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrCUStreamout << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrImageState << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrSliceSizeStreamout << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrCoeffProb << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrROISurface << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrHuCStitchDataBuf << ":0" << std::endl;

    // MD5 attributes
    ofs << "##" << MediaDbgAttr::attrMD5HashEnable << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrMD5FlushInterval << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrMD5PicWidth << ":0" << std::endl;
    ofs << "##" << MediaDbgAttr::attrMD5PicHeight << ":0" << std::endl;
    ofs << std::endl;

    ofs << "###################################################################" << std::endl;
    ofs << "## key words defined under @mode decode works for decode only" << std::endl;
    ofs << "###################################################################" << std::endl;
    ofs << std::endl;
    ofs << "#@mode decode" << std::endl;
    ofs << "#@Frame ALL" << std::endl;
    ofs << std::endl;

    ofs << "#" << MediaDbgAttr::attrSegmentParams << ":0" << std::endl;
    ofs << "#" << MediaDbgAttr::attrMbParams << ":0" << std::endl;
    ofs << "#" << MediaDbgAttr::attrVc1Bitplane << ":0" << std::endl;
    ofs << "#" << MediaDbgAttr::attrCoefProb << ":0" << std::endl;
    ofs << "#" << MediaDbgAttr::attrSegId << ":0" << std::endl;
    ofs << "#" << MediaDbgAttr::attrDecodeOutputSurface << ":0" << std::endl;
    ofs << "#" << MediaDbgAttr::attrDecodeAuxSurface << ":0" << std::endl;
    ofs << "#" << MediaDbgAttr::attrDecodeBltOutput << ":0" << std::endl;
    ofs << "#" << MediaDbgAttr::attrDecodeProcParams << ":0" << std::endl;
    ofs << std::endl;

    ofs << "###############################################################" << std::endl;
    ofs << "## key words defined under @mode encode works for encode only" << std::endl;
    ofs << "###############################################################" << std::endl;
    ofs << std::endl;
    ofs << "#@mode encode" << std::endl;
    ofs << "#@Frame ALL" << std::endl;
    ofs << std::endl;

    ofs << "#" << MediaDbgAttr::attrFeiPicParams << ":0" << std::endl;
    ofs << "#" << MediaDbgAttr::attrSeqParams << ":0" << std::endl;
    ofs << "#" << MediaDbgAttr::attrVuiParams << ":0" << std::endl;
    ofs << "#" << MediaDbgAttr::attrDumpEncodePar << ":0" << std::endl;
    ofs << "#" << MediaDbgAttr::attrVdencOutput << ":0" << std::endl;
    ofs << std::endl;

    ofs << "### Encode plug-in ###" << std::endl;
    ofs << "#@force" << std::endl;
    ofs << "#ForceCmpDumpLvl:3" << std::endl;
    ofs << "#@OverwriteCurbe" << std::endl;
    ofs << "#ForceCurbeDumpLvl:3" << std::endl;

    ofs.close();
}

#endif  // USE_MEDIA_DEBUG_TOOL
