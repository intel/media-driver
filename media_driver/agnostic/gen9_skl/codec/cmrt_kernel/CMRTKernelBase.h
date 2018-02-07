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
//! \file     CMRTKernelBase.h
//! \brief    HEVC FEI MDF structure base class for GEN9 SKL.
//!

#ifndef _CMKERNELBASE_
#define _CMKERNELBASE_

#include <stdio.h>

#ifdef ENABLE_CMRT_KERNEL_ULT
#include "cm_rt.h"
#include <cm/cm.h>
#else
#include "cm_rt_umd.h"
#endif

#define CM_CHK_STATUS_RETURN(stmt)                                              \
{                                                                               \
    CM_RETURN_CODE hr = (CM_RETURN_CODE)(stmt);                                 \
    if (hr != CM_SUCCESS)                                                       \
    {                                                                           \
        printf("the error is %d, %d, %s\n",hr, __LINE__,__FILE__);                     \
        return CM_FAILURE;                                                      \
    }                                                                           \
}                                                                               \

class CMRTKernelBase {
public:
    CmDevice        *m_cmDev;
    CmProgram       *m_cmProgram;
    CmQueue         *m_cmQueue;
    CmTask          *m_cmTask;       //Can be reused for each kernel
    CmThreadSpace   *m_cmThreadSpace;
    CmKernel        *m_cmKernel;
    CmSurface2D     **m_cmSurface2D;
    CmSurface2D     **m_cmSurfaceRef0;
    CmSurface2D     **m_cmSurfaceRef1;
    CmBuffer        **m_cmBuffer;
    SurfaceIndex    **m_cmVmeSurf;
    SurfaceIndex    **m_surfIndex;

    uint32_t        m_cmSurface2DCount;
    uint32_t        m_cmSurfaceRef0Count;
    uint32_t        m_cmSurfaceRef1Count;
    uint32_t        m_cmBufferCount;
    uint32_t        m_cmVmeSurfCount;
    void            *m_curbe;

    const char      *m_isaName;
    const char      *m_kernelName;

    CMRTKernelBase();
    virtual ~CMRTKernelBase();

    CM_RETURN_CODE LoadProgramISA(const char* sFilename, CmProgram * &program);
    CM_RETURN_CODE AddKernel(CmEvent *&cmEvent, bool destroyEvent, bool isEnqueue);
    CM_RETURN_CODE WaitAndDestroyEvent(CmEvent *&cmEvent);
    virtual CM_RETURN_CODE Init(void *osContext = nullptr, CmDevice *cmDev = nullptr, CmQueue *cmQueue = nullptr, CmTask *cmTask = nullptr, CmProgram *cmProgram = nullptr);
    virtual CM_RETURN_CODE SetupCurbe(void *curbe) = 0;
    virtual CM_RETURN_CODE AllocateSurfaces(void *params) = 0;
    virtual CM_RETURN_CODE CreateAndDispatchKernel(CmEvent *&cmEvent, bool destroyEvent, bool isEnqueue) = 0;//(EventList[i]);

    virtual void DestroySurfResources();
    virtual void DestroyKernelResources();
    virtual void DestroyProgramResources();
    virtual void Destroy();

};

#endif
