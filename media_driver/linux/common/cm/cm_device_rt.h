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
//!
//! \file      cm_device_rt.h
//! \brief     Contains CmDeviceRT declarations.
//!

#ifndef MEDIADRIVER_LINUX_COMMON_CM_CMDEVICERT_H_
#define MEDIADRIVER_LINUX_COMMON_CM_CMDEVICERT_H_

#include "cm_device_rt_base.h"

#include "cm_array.h"
#include "cm_csync.h"
#include "cm_hal.h"
#include "cm_log.h"
#include "cm_program.h"
#include "cm_notifier.h"

#if USE_EXTENSION_CODE
#include "cm_gtpin.h"
#endif

namespace CMRT_UMD
{
class CmQueueRT;
class CmSurfaceManager;
class CmSampler8x8State_RT;

//! \brief    Class CmDeviceRT definitions for Linux
class CmDeviceRT: public CmDeviceRTBase
{
public:
    static int32_t
    Create(MOS_CONTEXT *umdContext,
           CmDeviceRT* &device,
           uint32_t options = CM_DEVICE_CREATE_OPTION_DEFAULT);
    static int32_t Destroy(CmDeviceRT* &device);

    CM_RT_API int32_t CreateSurface2D(VASurfaceID vaSurface,
                                      VADriverContext *vaDriverCtx,
                                      CmSurface2D* &surface);

    CM_RT_API virtual  int32_t CreateSurface2D(uint32_t width,
                                               uint32_t height,
                                               CM_SURFACE_FORMAT format,
                                               CmSurface2D* & pSurface);

    CM_RT_API virtual  int32_t CreateSurface2D(PMOS_RESOURCE pMosResource,
                                               CmSurface2D* & pSurface);
 
    virtual int32_t GetJITCompileFnt(pJITCompile &jitCompile);

    virtual int32_t GetJITCompileFntV2(pJITCompile_v2 &fJITCompile_v2);

    virtual int32_t GetFreeBlockFnt(pFreeBlock &freeBlock);

    virtual int32_t GetJITVersionFnt(pJITVersion &jitVersion);

    virtual int32_t LoadJITDll();

    virtual int32_t CreateSurface2D(PMOS_RESOURCE pMosResource,
                                    bool bIsCmCreated,
                                    CmSurface2D* & pSurface);

    int32_t SetVaCtxID(uint32_t vaCtxID);

    int32_t GetVaCtxID(uint32_t &vaCtxID);

    int32_t RegisterCallBack(pCallBackReleaseVaSurface callBack);

    int32_t ReleaseVASurface(void *vaDisplay, void *vaSurfaceID);

protected:

    virtual int32_t InitializeOSSpecific(MOS_CONTEXT *mosContext);

    virtual void ConstructOSSpecific(uint32_t devCreateOption);

    virtual CM_RETURN_CODE QueryGPUInfoInternal(PCM_QUERY_CAPS pQueryCaps);

    virtual int32_t ReadVtuneProfilingFlag();

    virtual int32_t QuerySurface2DFormats(void *capValue, uint32_t &capValueSize);

    virtual CM_RETURN_CODE QuerySurface2DFormatsInternal(PCM_QUERY_CAPS queryCaps);

    int32_t CreateAuxDevice(MOS_CONTEXT *mosContext);

    int32_t DestroyAuxDevice();

    CM_RETURN_CODE GetCapsInternalOSSpecific(PCM_QUERY_CAPS queryCaps);

    CmDeviceRT(uint32_t option);

    ~CmDeviceRT();    

    inline bool IsMediaResetNeeded(uint32_t options)
    {
        UNUSED(options);
        return false;
    }

    inline HANDLE QueryRegHandleInternal(PCM_HAL_STATE cmHalState)
    {
        UNUSED(cmHalState);
        return (HANDLE)nullptr;
    }


    VAContextID m_vaCtxID;
    pCallBackReleaseVaSurface  m_pfnReleaseVaSurface;

private:
    CmDeviceRT(const CmDeviceRT& other);

    CmDeviceRT& operator= (const CmDeviceRT& other);
};
}; //namespace

#endif  // #ifndef MEDIADRIVER_LINUX_COMMON_CM_CMDEVICERT_H_
