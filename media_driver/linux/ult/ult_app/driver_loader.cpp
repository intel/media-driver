/*
* Copyright (c) 2018-2021, Intel Corporation
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
#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "driver_loader.h"
#include "mos_util_debug.h"
#include "memory_leak_detector.h"

using namespace std;

#if MOS_MESSAGES_ENABLED
void MosUtilDebug::MosMessage(
    MOS_MESSAGE_LEVEL level,
    MOS_COMPONENT_ID  compID,
    uint8_t           subCompID,
    const PCCHAR      functionName,
    int32_t           lineNum,
    const PCCHAR      message,
    ...)
{
}
#endif

void UltGetCmdBuf(PMOS_COMMAND_BUFFER pCmdBuffer);

extern char               *g_driverPath;
extern vector<Platform_t> g_platform;

const char *g_platformName[] = {
    "SKL",
    "BXT",
    "BDW",
};

DriverDllLoader::DriverDllLoader()
{
    if (g_driverPath)
    {
        m_driver_path = g_driverPath;
    }
    else
    {
        m_driver_path = "/opt/intel/mediasdk/lib64/iHD_drv_video.so";
    }

    m_platformArray = {
#ifdef IGFX_GEN9_SKL_SUPPORTED
        igfxSKLAKE,
#endif
#ifdef IGFX_GEN9_BXT_SUPPORTED
        igfxBROXTON,
#endif
#ifdef IGFX_GEN8_BDW_SUPPORTED
        igfxBROADWELL,
#endif
    };

    if (g_platform.size() > 0)
    {
        m_platformArray = g_platform;
    }
}

DriverDllLoader::DriverDllLoader(char *path)
{
    DriverDllLoader();
    m_driver_path = path;
}

VAStatus DriverDllLoader::CloseDriver(bool detectMemLeak)
{
    VAStatus vaStatus = m_ctx.vtable->vaTerminate(&m_ctx);

    if (detectMemLeak)
    {
        MemoryLeakDetector::Detect(m_drvSyms.MOS_GetMemNinjaCounter(),
                                m_drvSyms.MOS_GetMemNinjaCounterGfx(),
                                m_currentPlatform);
    }

    m_drvSyms = {};

    if(m_umdhandle)
    {
        dlclose(m_umdhandle);
        m_umdhandle = nullptr;
    }

    return vaStatus;
}

VAStatus DriverDllLoader::InitDriver(Platform_t platform_id)
{
    int drm_fd           = platform_id + 1 < 0 ? 1 : platform_id + 1;
    m_drmstate.fd        = drm_fd;
    m_drmstate.auth_type = 3;
    m_ctx.vtable         = &m_vtable;
    m_ctx.vtable_vpp     = &m_vtable_vpp;
#if VA_CHECK_VERSION(1,11,0)
    m_ctx.vtable_prot    = &m_vtable_prot;
#endif
    m_ctx.drm_state      = &m_drmstate;
    m_currentPlatform    = platform_id;
    m_ctx.vtable_tpi     = nullptr;

    if (LoadDriverSymbols() != VA_STATUS_SUCCESS)
    {
        return VA_STATUS_ERROR_UNKNOWN;
    }
    m_drvSyms.MOS_SetUltFlag(1);
    *m_drvSyms.ppfnUltGetCmdBuf = UltGetCmdBuf;
    return m_drvSyms.__vaDriverInit_(&m_ctx);
}

VAStatus DriverDllLoader::LoadDriverSymbols()
{
    const int buf_len         = 256;
    char init_func_s[buf_len] = {};

    m_umdhandle = dlopen(m_driver_path, RTLD_NOW | RTLD_GLOBAL);
    if (!m_umdhandle)
    {
        printf("ERROR: dlopen of %s failed.\n", m_driver_path);
        char* pErrorStr = dlerror();
        if(nullptr != pErrorStr)
        {
            printf("ERROR: %s\n", pErrorStr);
        }
        return VA_STATUS_ERROR_UNKNOWN;
    }

    for (int i = 0; i <= VA_MINOR_VERSION; i++)
    {
        sprintf_s(init_func_s, buf_len, "__vaDriverInit_%d_%d", VA_MAJOR_VERSION, i);
        m_drvSyms.__vaDriverInit_ = (VADriverInit)dlsym(m_umdhandle, init_func_s);
        if (m_drvSyms.__vaDriverInit_)
        {
            m_drvSyms.vaCmExtSendReqMsg         = (CmExtSendReqMsgFunc)dlsym(m_umdhandle, "vaCmExtSendReqMsg");
            m_drvSyms.MOS_SetUltFlag            = (MOS_SetUltFlagFunc)dlsym(m_umdhandle, "MOS_SetUltFlag");
            m_drvSyms.MOS_GetMemNinjaCounter    = (MOS_GetMemNinjaCounterFunc)dlsym(m_umdhandle, "MOS_GetMemNinjaCounter");
            m_drvSyms.MOS_GetMemNinjaCounterGfx = (MOS_GetMemNinjaCounterFunc)dlsym(m_umdhandle, "MOS_GetMemNinjaCounterGfx");
            m_drvSyms.ppfnUltGetCmdBuf          = (UltGetCmdBufFunc *)dlsym(m_umdhandle, "pfnUltGetCmdBuf");
            break;
        }
    }

    if (!m_drvSyms.Initialized())
    {
        printf("ERROR: not all driver symbols are successfully loaded.\n");
        return VA_STATUS_ERROR_UNKNOWN;
    }

    return VA_STATUS_SUCCESS;
}
