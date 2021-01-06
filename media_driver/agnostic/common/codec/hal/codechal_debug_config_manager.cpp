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
//!
//! \file     codechal_debug_config_manager.cpp
//! \brief    Defines the dump configuration manager.
//! \details  The debug interface dumps configuration manager file which parse attributes.
//!
#include "codechal_debug_config_manager.h"
#if USE_CODECHAL_DEBUG_TOOL

CodecDebugConfigMgr::CodecDebugConfigMgr(
    CodechalDebugInterface *debugInterface,
    CODECHAL_FUNCTION       codecFunction,
    std::string             outputFolderPath)
    : MediaDebugConfigMgr(outputFolderPath),
      m_debugInterface(debugInterface),
      m_codecFunction(codecFunction)
{
    GetFunctionType();
}

void CodecDebugConfigMgr::GetFunctionType()
{
    switch (m_codecFunction)
    {
    case CODECHAL_FUNCTION_CENC_DECODE:
        m_mediaFunction = MEDIA_FUNCTION_CENC_DECODE;
        break;
    case CODECHAL_FUNCTION_DECODE:
        m_mediaFunction = MEDIA_FUNCTION_DECODE;
        break;
    case CODECHAL_FUNCTION_ENC:
    case CODECHAL_FUNCTION_PAK:
    case CODECHAL_FUNCTION_ENC_PAK:
    case CODECHAL_FUNCTION_HYBRIDPAK:
    case CODECHAL_FUNCTION_ENC_VDENC_PAK:
    case CODECHAL_FUNCTION_DEMO_COPY:
    case CODECHAL_FUNCTION_FEI_PRE_ENC:
    case CODECHAL_FUNCTION_FEI_ENC:
    case CODECHAL_FUNCTION_FEI_PAK:
    case CODECHAL_FUNCTION_FEI_ENC_PAK:
        m_mediaFunction = MEDIA_FUNCTION_ENCODE;
        break;
    default:
        m_mediaFunction = MEDIA_FUNCTION_DEFAULT;
        break;
    }
}

std::string CodecDebugConfigMgr::InitFileName(MediaDbgFunction mediaFunction)
{
    return "CodecDbgSetting.cfg";
}

CodecDebugConfigMgr::~CodecDebugConfigMgr()
{
}

uint32_t CodecDebugConfigMgr::GetDumpFrameNum()
{
    return (uint32_t)m_debugInterface->m_bufferDumpFrameNum;
}

#endif  // USE_CODECHAL_DEBUG_TOOL

