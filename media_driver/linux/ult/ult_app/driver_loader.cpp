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
    char init_func_s[] = "__vaDriverInit_1_0";
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
        //printf("INFO: Load driver success\n");
        init_func = (VADriverInit)dlsym(umdhandle, init_func_s);
        vaCmExtSendReqMsg = (CmExtSendReqMsgFunc)dlsym(umdhandle,
                                                       cm_entry_name);
        if (!init_func || !vaCmExtSendReqMsg)
        {
            return VA_STATUS_ERROR_UNKNOWN;
        }
        //printf("INFO: Found init function %s and CM entry point %s.\n", init_func_s, cm_entry_name);

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
