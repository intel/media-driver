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
//! \file     media_debug_config_manager.h
//! \brief    Defines the dump configuration manager.
//! \details  The debug interface dumps configuration manager file which parse attributes.
#ifndef __MEDIA_DEBUG_CONFIG_MANAGER_H__
#define __MEDIA_DEBUG_CONFIG_MANAGER_H__
#if USE_MEDIA_DEBUG_TOOL
#include "media_debug_utils.h"
#include "media_common_defs.h"
#include "media_debug_interface.h"

struct MediaKernelDumpConfig
{
    bool dumpDsh         = false;
    bool dumpSsh         = false;
    bool dumpIsh         = false;
    bool dumpCurbe       = false;
    bool dumpCmdBuffer   = false;
    bool dump2ndLvlBatch = false;
    bool dumpInput       = false;
    bool dumpOutput      = false;
};

struct MediaDbgCfg
{
    MediaDbgCfg()
    {
        cmdAttribs[MediaDbgAttr::attrEnableFastDump] = 1;
    }

    int32_t                                      frameIndex;
    std::map<std::string, int32_t>               cmdAttribs;
    std::map<std::string, MediaKernelDumpConfig> kernelAttribs;
    std::vector<std::string>                     forceCmds;
};

enum MediaDbgFunction
{
    MEDIA_FUNCTION_DEFAULT     = 0,
    MEDIA_FUNCTION_ENCODE      = 1,
    MEDIA_FUNCTION_DECODE      = 2,
    MEDIA_FUNCTION_CENC_DECODE = 3,
    MEDIA_FUNCTION_VP          = 4,
};

class MediaDebugInterface;
class CodechalDebugInterface;
class VpDebugInterface;
class MediaDebugConfigMgr
{
public:
    MediaDebugConfigMgr(std::string outputFolderPath);
    virtual ~MediaDebugConfigMgr();

    MOS_STATUS ParseConfig(MOS_CONTEXT_HANDLE mosCtx);
    MOS_STATUS DeleteCfgNode(uint32_t frameIdx);

    bool AttrIsEnabled(std::string attrName);

 protected:
    void     GenerateDefaultConfig(std::string configFileName);
    void     StoreDebugAttribs(std::string line, MediaDbgCfg *dbgCfg);
    void     ParseKernelAttribs(std::string line, MediaDbgCfg *dbgCfg);
    bool     KernelAttrEnabled(MediaKernelDumpConfig kernelConfig, std::string attrName);
    uint32_t GetFrameConfig(uint32_t frameIdx);

    virtual uint32_t GetDumpFrameNum() = 0;
    virtual std::string InitFileName(MediaDbgFunction mediaFunction) = 0;
    virtual MediaUserSettingSharedPtr GetUserSettingInstance() = 0;

protected:
    MediaDbgFunction         m_mediaFunction;
    std::string              m_outputFolderPath;
    std::vector<MediaDbgCfg> m_debugFrameConfigs;
    MediaDbgCfg *            m_debugAllConfigs = nullptr;
MEDIA_CLASS_DEFINE_END(MediaDebugConfigMgr)
};

#endif  //USE_MEDIA_DEBUG_TOOL
#endif  /* __MEDIA_DEBUG_CONFIG_MANAGER_H__ */
