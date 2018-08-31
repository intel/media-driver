/*
* Copyright (c) 2018, Intel Corporation
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
#ifndef __DRIVER_LOADER_H__
#define __DRIVER_LOADER_H__

#include <vector>
#include "devconfig.h"
#include "mos_defs_specific.h"
#include "mos_os.h"
#include "va/va_drmcommon.h"
#include "va/va_backend.h"
#include "va/va_backend_vpp.h"

struct FeatureID
{
    VAProfile    profile;
    VAEntrypoint entrypoint;

    bool operator==(const FeatureID &g2) const
    {
        return (profile == g2.profile) && (entrypoint == g2.entrypoint);
    }

    bool operator<(const FeatureID &g2) const
    {
        if (profile != g2.profile)
        {
            return profile < g2.profile;
        }
        else
        {
             return entrypoint < g2.entrypoint;
        }
    }
};

struct CompBufConif
{
    VABufferType bufType;
    uint32_t     bufSize;
    void*        pData;
    VABufferID   bufID;
};

typedef VAStatus (*CmExtSendReqMsgFunc)(
                                VADisplay dpy,
                                void      *moduleType,
                                uint32_t  *inputFunId,
                                void      *inputData,
                                uint32_t  *inputDataLen,
                                uint32_t  *outputFunId,
                                void      *outputData,
                                uint32_t  *outputDataLen);

typedef void (*MOS_SetUltFlagFunc)(uint8_t ultFlag);

typedef int32_t (*MOS_GetMemNinjaCounterFunc)();

typedef void (*UltGetCmdBufFunc)(PMOS_COMMAND_BUFFER pCmdBuffer);

struct DriverSymbols
{
    bool Initialized() const
    {
        if (!__vaDriverInit_           ||
            !vaCmExtSendReqMsg         ||
            !MOS_SetUltFlag            ||
            !MOS_GetMemNinjaCounter    ||
            !MOS_GetMemNinjaCounterGfx ||
            !ppfnUltGetCmdBuf)
        {
            return false;
        }
        return true;
    }
    
    // Functions
    VADriverInit                __vaDriverInit_;
    CmExtSendReqMsgFunc         vaCmExtSendReqMsg;
    MOS_SetUltFlagFunc          MOS_SetUltFlag;
    MOS_GetMemNinjaCounterFunc  MOS_GetMemNinjaCounter;
    MOS_GetMemNinjaCounterFunc  MOS_GetMemNinjaCounterGfx;

    // Data
    UltGetCmdBufFunc            *ppfnUltGetCmdBuf;
};

class DriverDllLoader
{
public:

    DriverDllLoader();

    DriverDllLoader(char *path);

    const std::vector<Platform_t> &GetPlatforms() const { return m_platformArray; }

    int GetPlatformNum() const { return m_platformArray.size(); }

    const DriverSymbols &GetDriverSymbols() const { return m_drvSyms; }

    VAStatus InitDriver(Platform_t platform_id);

    VAStatus CloseDriver(bool detectMemLeak = true);

public:

    VADriverContext             m_ctx;
    VADriverVTable              m_vtable;
    VADriverVTableVPP           m_vtable_vpp;

private:

    VAStatus LoadDriverSymbols();

private:

    const char                  *m_driver_path    = nullptr;
    void                        *m_umdhandle      = nullptr;
    DriverSymbols               m_drvSyms         = {};
    drm_state                   m_drmstate        = {};
    Platform_t                  m_currentPlatform = igfxSKLAKE;
    std::vector<Platform_t>     m_platformArray;
};

#endif // __DRIVER_LOADER_H__
