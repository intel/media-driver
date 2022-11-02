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
//! \file     vp_debug_config.manager.h
//! \brief    Defines the dump configuration manager.
//! \details  The debug interface dumps configuration manager file which parse attributes.
//!
#ifndef __VP_DEBUG_CONFIG_MANAGER_H__
#define __VP_DEBUG_CONFIG_MANAGER_H__

#include "vp_debug_interface.h"
#if USE_VP_DEBUG_TOOL

class VpDebugConfigMgr : public MediaDebugConfigMgr
{
public:
    VpDebugConfigMgr(
        VpDebugInterface *debugInterface,
        std::string       outputFolderPath);
    virtual  ~VpDebugConfigMgr();

protected:
    uint32_t GetDumpFrameNum() override;

protected:
    VpDebugInterface *m_debugInterface = nullptr;
    std::string       InitFileName(MediaDbgFunction mediaFunction) override;
    MediaUserSettingSharedPtr GetUserSettingInstance() override;

MEDIA_CLASS_DEFINE_END(VpDebugConfigMgr)
};

#endif  //USE_VP_DEBUG_TOOL
#endif  /* __VP_DEBUG_CONFIG_MANAGER_H__ */
