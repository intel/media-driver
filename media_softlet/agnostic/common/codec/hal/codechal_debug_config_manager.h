/*
* Copyright (c) 2017-2020, Intel Corporation
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
//! \file     codechal_debug_config_manager.h
//! \brief    Defines the dump configuration manager.
//! \details  The debug interface dumps configuration manager file which parse attributes.
//!
#ifndef __CODECHAL_DEBUG_CONFIG_MANAGER_H__
#define __CODECHAL_DEBUG_CONFIG_MANAGER_H__

#include "codechal_debug.h"
#include "codechal_debug_kernel.h"
#if USE_CODECHAL_DEBUG_TOOL

class CodecDebugConfigMgr : public MediaDebugConfigMgr
{
public:
    CodecDebugConfigMgr(
        CodechalDebugInterface *debugInterface,
        CODECHAL_FUNCTION       codecFunction,
        std::string             outputFolderPath);
    virtual ~CodecDebugConfigMgr();

    std::string GetMediaStateStr(CODECHAL_MEDIA_STATE_TYPE mediaState);
    bool AttrIsEnabled(CODECHAL_MEDIA_STATE_TYPE mediaState, std::string attrName);
    bool AttrIsEnabled(std::string attrName);

protected:
    void     GetFunctionType();
    uint32_t GetDumpFrameNum() override;
    std::string InitFileName(MediaDbgFunction mediaFunction) override;
    MediaUserSettingSharedPtr GetUserSettingInstance() override;

protected:
    CodechalDebugInterface *m_debugInterface = nullptr;
    CODECHAL_FUNCTION       m_codecFunction;

MEDIA_CLASS_DEFINE_END(CodecDebugConfigMgr)
};

#endif  //USE_CODECHAL_DEBUG_TOOL
#endif  /* __CODECHAL_DEBUG_CONFIG_MANAGER_H__ */
