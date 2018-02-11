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
#pragma once
#include <vector>
using namespace std;
#include "va/va_backend.h"
#include "va/va_backend_vpp.h"
#include "va/va_drmcommon.h"

#include "devconfig.h"

typedef struct _FeatureID{
    VAProfile profile;
    VAEntrypoint entrypoint;

    bool operator == (_FeatureID g2){return (profile==g2.profile)&&(entrypoint==g2.entrypoint);}
    bool operator < (const _FeatureID &g2) const
    {
        if ( profile < g2.profile )
        {
            return true;
        }
        else if ( profile == g2.profile)
        {
            if( entrypoint < g2.entrypoint)
                return true;
            else
                return false;
        }else
        {
            return false;
        }
    }
}FeatureID;
typedef struct _CompBufConif{
    VABufferType BufType;
    uint32_t     BufSize;
    void*        pData;
    VABufferID   BufID;

    bool operator = (_CompBufConif g2){
        BufType = g2.BufType;
        BufSize = g2.BufSize;
        pData   = g2.pData;
        BufID   = g2.BufID;
        return true;
    }
}CompBufConif;

typedef VAStatus (*CmExtSendReqMsgFunc)(VADisplay dpy,
                                        void *moduleType,
                                        uint32_t *inputFunId,
                                        void *inputData,
                                        uint32_t *inputDataLen,
                                        uint32_t *outputFunId,
                                        void *outputData,
                                        uint32_t *outputDataLen);

typedef void (*MOS_SetUltFlagFunc)(uint8_t ultFlag);

typedef int32_t (*GetMemNinjaCounter)();

class DriverDllLoader
{
public:
    DriverDllLoader();
    DriverDllLoader(char* path);
    ~DriverDllLoader();

    vector<Platform_t>& GetPlatforms() { return platformArray; }
    int GetPlatformNum(){return platformArray.size();}
    //int GetPlatformNum(){return sizeof(DeviceConfigTable)/sizeof(DeviceConfig_t);}
    VAStatus InitDriver(int platform_id);
    VAStatus CloseDriver();

    VADriverContext ctx;
    VADriverVTable vtable;
    VADriverVTableVPP vtable_vpp;
    CmExtSendReqMsgFunc vaCmExtSendReqMsg;
    MOS_SetUltFlagFunc Mos_SetUltFlag;
    GetMemNinjaCounter Mos_GetMemNinjaCounter;
    GetMemNinjaCounter Mos_GetMemNinjaCounterGfx;

private:
    const char *driver_path;
    void *umdhandle;
    vector<Platform_t> platformArray;

    struct drm_state drmstate;
};
