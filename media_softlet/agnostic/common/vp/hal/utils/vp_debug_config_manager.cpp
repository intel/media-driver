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
//! \file     vp_debug_config_manager.cpp
//! \brief    Defines the dump configuration manager.
//! \details  The debug interface dumps configuration manager file which parse attributes.
//!
#include "vp_debug_config_manager.h"
#if USE_VP_DEBUG_TOOL

VpDebugConfigMgr::VpDebugConfigMgr(
    VpDebugInterface *debugInterface,
    std::string       outputFolderPath)
    : MediaDebugConfigMgr(outputFolderPath),
      m_debugInterface(debugInterface)
{
    m_mediaFunction = MEDIA_FUNCTION_VP;
}

uint32_t VpDebugConfigMgr::GetDumpFrameNum()
{
    VP_FUNC_CALL();

    return (uint32_t)m_debugInterface->m_bufferDumpFrameNum;
}

std::string VpDebugConfigMgr::InitFileName(MediaDbgFunction mediaFunction)
{
    VP_FUNC_CALL();

    return "VpDbgSetting.cfg";
}

MediaUserSettingSharedPtr VpDebugConfigMgr::GetUserSettingInstance()
{
    VP_FUNC_CALL();
    return m_debugInterface->m_userSettingPtr;
}

VpDebugConfigMgr::~VpDebugConfigMgr()
{
}

#endif  // USE_VP_DEBUG_TOOL

