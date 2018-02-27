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
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>

#include "driver_loader.h"
extern char* DirverPath;
extern vector<Platform_t> g_platform;

const char *g_platformName[] = {
    "SKL",
    "BXT",
    "BDW",
    "CNL",
};

DriverDllLoader::DriverDllLoader()
{
    if (DirverPath)
        driver_path = DirverPath;
    else
        driver_path = "/opt/intel/mediasdk/lib64/iHD_drv_video.so";
    platformArray = {
#ifdef IGFX_GEN9_SKL_SUPPORTED
        igfxSKLAKE,
#endif
#ifdef IGFX_GEN9_BXT_SUPPORTED
        igfxBROXTON,
#endif
#ifdef IGFX_GEN8_BDW_SUPPORTED
        igfxBROADWELL,
#endif
#ifdef IGFX_GEN10_CNL_SUPPORTED
        igfxCANNONLAKE
#endif
    };

    if (g_platform.size() > 0)
    {
        platformArray = g_platform;
    }
}

DriverDllLoader::DriverDllLoader(char* path)
{
    DriverDllLoader();
    driver_path = path;

}

DriverDllLoader::~DriverDllLoader()
{
}

VAStatus DriverDllLoader::CloseDriver()
{
    VAStatus vaStatus;
    vaStatus = ctx.vtable->vaTerminate(&ctx);
    vaCmExtSendReqMsg = nullptr;
    if(umdhandle)
    {
        dlclose(umdhandle);
    }
    return vaStatus;
}
VAStatus DriverDllLoader::InitDriver(int platform_id)
{
    void *handle;
    VAStatus vaStatus;
    char init_func_s[256];
    const char *cm_entry_name = "vaCmExtSendReqMsg";
    VADriverInit init_func = NULL;

    //printf("Load driver from %s.\n", driver_path);
    int drm_fd = platform_id+1;//open("/dev/dri/renderD128", O_RDONLY);
    if(drm_fd<0)
        drm_fd = 1;
    drmstate.fd = drm_fd;
    drmstate.auth_type = 3;
    //putenv("DRMMode=1");

    umdhandle = dlopen(driver_path, RTLD_NOW | RTLD_GLOBAL | RTLD_NODELETE);

    if (!umdhandle)
    {
        printf("ERROR: dlopen of %s failed.\n", driver_path);
        return VA_STATUS_ERROR_UNKNOWN;
    }
    else
    {
        for (int i = 0; i <= VA_MINOR_VERSION; i++) {
            sprintf(init_func_s, "__vaDriverInit_%d_%d",VA_MAJOR_VERSION,i);
            init_func = (VADriverInit)dlsym(umdhandle, init_func_s);
            if (init_func){
                vaCmExtSendReqMsg = (CmExtSendReqMsgFunc)dlsym(umdhandle,cm_entry_name);
                Mos_SetUltFlag = (MOS_SetUltFlagFunc)dlsym(umdhandle, "Mos_SetUltFlag");
                Mos_GetMemNinjaCounter = (GetMemNinjaCounter)dlsym(umdhandle, "MOS_GetMemNinjaCounter");
                Mos_GetMemNinjaCounterGfx = (GetMemNinjaCounter)dlsym(umdhandle, "MOS_GetMemNinjaCounterGfx");
                break;
            }
        }

        if (!init_func || !vaCmExtSendReqMsg || !Mos_SetUltFlag || !Mos_GetMemNinjaCounter || !Mos_GetMemNinjaCounterGfx)
        {
            return VA_STATUS_ERROR_UNKNOWN;
        }
        //printf("INFO: Found init function %s and CM entry point %s.\n", init_func_s, cm_entry_name);

        Mos_SetUltFlag(1);

        ctx.vtable = &vtable;
        ctx.vtable_vpp = &vtable_vpp;
        ctx.drm_state = &drmstate;
        vaStatus = (*init_func)(&ctx);
        //if (VA_STATUS_SUCCESS == vaStatus)
        //{
        //    printf("INFO: Get driver DDI functions\n");
        //}
        return vaStatus;
    }
}
