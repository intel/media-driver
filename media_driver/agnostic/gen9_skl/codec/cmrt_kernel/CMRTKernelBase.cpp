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
//! \file     CMRTKernelBase.cpp
//! \brief    HEVC FEI MDF structure base class for GEN9 SKL.
//!

#include "CMRTKernelBase.h"

CMRTKernelBase::CMRTKernelBase()
{
    m_cmDev              = nullptr;
    m_cmProgram          = nullptr;
    m_cmQueue            = nullptr;
    m_cmTask             = nullptr;       //Can be reused for each kernel
    m_cmThreadSpace      = nullptr;
    m_cmKernel           = nullptr;
    m_cmSurface2D        = nullptr;
    m_cmSurfaceRef0      = nullptr;
    m_cmSurfaceRef1      = nullptr;
    m_cmBuffer           = nullptr;
    m_cmVmeSurf          = nullptr;
    m_surfIndex          = nullptr;
    m_isaName            = nullptr;
    m_kernelName         = nullptr;
    m_curbe              = nullptr;
    m_isaSize            = 0;
    m_cmSurface2DCount   = 0;
    m_cmSurfaceRef0Count = 0;
    m_cmSurfaceRef1Count = 0;
    m_cmBufferCount      = 0;
    m_cmVmeSurfCount     = 0;
}

CMRTKernelBase::~CMRTKernelBase()
{
}

CM_RETURN_CODE CMRTKernelBase::LoadProgramISA(const uint32_t *isaCode, uint32_t isaSize, CmProgram * &program)
{
    int32_t result;

    // Load Program
    result = m_cmDev->LoadProgram((void *)isaCode, isaSize, program, "-nojitter");
    if (result != CM_SUCCESS)
    {
        printf("MDF LoadProgram error: %d\n", result);
    }

    return CM_SUCCESS;

}

CM_RETURN_CODE CMRTKernelBase::Init(void *osContext, CmDevice *cmDev, CmQueue *cmQueue, CmTask *cmTask, CmProgram *cmProgram)
{
    int32_t result;

    if (cmDev)
    {
        m_cmDev = cmDev;
    }
    else
    {
#ifdef ENABLE_CMRT_KERNEL_ULT
        uint32_t version = 0;
        result = CreateCmDevice(m_cmDev, version);
#else
        result = CreateCmDevice((PMOS_CONTEXT)osContext, m_cmDev, CM_DEVICE_CREATE_OPTION_FOR_HEVC);
#endif
        if (result != CM_SUCCESS)
        {
            printf("CmDevice creation error\n");
            return CM_FAILURE;
        }
    }

    if (cmQueue)
    {
        m_cmQueue = cmQueue;
    }
    else
    {
#ifdef ENABLE_CMRT_KERNEL_ULT
        result = m_cmDev->CreateQueue(m_cmQueue);
#else
        result = m_cmDev->CreateQueue(m_cmQueue);
#endif
        if (result != CM_SUCCESS)
        {
            printf("CM CreateQueue error\n");
            return CM_FAILURE;
        }
    }

    if (cmTask)
    {
        m_cmTask = cmTask;
    }
    else
    {
#ifdef ENABLE_CMRT_KERNEL_ULT
        result = m_cmDev->CreateTask(m_cmTask);
#else
        result = m_cmDev->CreateTask(m_cmTask);
#endif
        if (result != CM_SUCCESS)
        {
            printf("CmDevice CreateTask error\n");
            return CM_FAILURE;
        }
    }

    if (cmProgram)
    {
        m_cmProgram = cmProgram;
    }
    else
    {
        result = LoadProgramISA(m_isaName, m_isaSize, m_cmProgram);
        if (result != CM_SUCCESS)
        {
            printf("CmDevice LoadProgramISA error\n");
            return CM_FAILURE;
        }
    }

    result = m_cmDev->CreateKernel(m_cmProgram, m_kernelName, m_cmKernel);
    if (result != CM_SUCCESS)
    {
        printf("CmDevice CreateKernel error\n");
        return CM_FAILURE;
    }

    return CM_SUCCESS;
}

int32_t CMRTKernelBase::CreateThreadSpace(uint32_t threadSpaceWidth, uint32_t threadSpaceHeight)
{
    int32_t result = CM_SUCCESS;

    if (!m_cmThreadSpace)
    {
        result = m_cmDev->CreateThreadSpace(threadSpaceWidth, threadSpaceHeight, m_cmThreadSpace);
    }
    else
    {
        //Destory thread space used before
        result = m_cmDev->DestroyThreadSpace(m_cmThreadSpace);
        if (result != CM_SUCCESS)
        {
            printf("CM Destroy ThreadSpace error : %d", result);
            return (CM_RETURN_CODE)result;
        }
        result = m_cmDev->CreateThreadSpace(threadSpaceWidth, threadSpaceHeight, m_cmThreadSpace);
    }

    return result;
}

CM_RETURN_CODE CMRTKernelBase::AddKernel(CmEvent *&cmEvent, bool destroyEvent, bool isEnqueue)
{
    if (m_cmTask == nullptr)
    {
        CM_CHK_STATUS_RETURN(m_cmDev->CreateTask(m_cmTask));
    }

    if (m_cmQueue == nullptr)
    {
        CM_CHK_STATUS_RETURN(m_cmDev->CreateQueue(m_cmQueue));//CreateQueue is just get queue of CmDev, so just need call once.
    }

    CM_CHK_STATUS_RETURN(m_cmKernel->AssociateThreadSpace(m_cmThreadSpace));
    CM_CHK_STATUS_RETURN(m_cmTask->AddKernel(m_cmKernel));

    if (isEnqueue)
    {
        CM_CHK_STATUS_RETURN(m_cmQueue->Enqueue(m_cmTask, cmEvent));
        CM_CHK_STATUS_RETURN(m_cmTask->Reset());
        if(destroyEvent)
        {
           CM_CHK_STATUS_RETURN(m_cmQueue->DestroyEvent(cmEvent));
        }
    }
    else
    {
        CM_CHK_STATUS_RETURN(m_cmTask->AddSync());
    }

    return CM_SUCCESS;
}

CM_RETURN_CODE CMRTKernelBase::WaitAndDestroyEvent(CmEvent *&cmEvent)
{
    int32_t dwTimeOutMs = -1;

    CM_CHK_STATUS_RETURN(cmEvent->WaitForTaskFinished(dwTimeOutMs));

    CM_CHK_STATUS_RETURN(m_cmQueue->DestroyEvent(cmEvent));
    cmEvent = nullptr;

    return CM_SUCCESS;
}

//This is kernelbased, We need to call for all the kernels in the kernellist.
void CMRTKernelBase::DestroySurfResources()
{
    uint32_t i = 0;

    for (i = 0; i < m_cmSurface2DCount; i++)
    {
        if (m_cmSurface2D[i])
        {
            m_cmDev->DestroySurface(m_cmSurface2D[i]);
            m_cmSurface2D[i] = nullptr;
        }
    }

    for (i = 0; i < m_cmSurfaceRef0Count; i++)
    {
        if (m_cmSurfaceRef0[i])
        {
            m_cmDev->DestroySurface(m_cmSurfaceRef0[i]);
            m_cmSurfaceRef0[i] = nullptr;
        }
    }

    for (i = 0; i < m_cmSurfaceRef1Count; i++)
    {
        if (m_cmSurfaceRef1[i])
        {
            m_cmDev->DestroySurface(m_cmSurfaceRef1[i]);
            m_cmSurfaceRef1[i] = nullptr;
        }
    }

    for (i = 0; i < m_cmBufferCount; i++)
    {
        if (m_cmBuffer[i])
        {
            m_cmDev->DestroySurface(m_cmBuffer[i]);
            m_cmBuffer[i] = nullptr;
        }
    }
    for (i = 0; i < m_cmVmeSurfCount; i++)
    {
        if (m_cmVmeSurf[i])
        {
            m_cmDev->DestroyVmeSurfaceG7_5(m_cmVmeSurf[i]);
            m_cmVmeSurf[i] = nullptr;
        }
    }
}

//IF called init for /kernel/program/TS per frame, then we call this per frame;
//if called init for all the kernels per process, then call this per process.
//We can also init every first the kernel is called, then we just need to call this per process for all the kernels in kernelList.
void CMRTKernelBase::DestroyKernelResources()
{
    if(m_cmKernel)
    {
        m_cmDev->DestroyKernel(m_cmKernel);
        m_cmKernel = nullptr;
    }

    if (m_cmThreadSpace)
    {
        m_cmDev->DestroyThreadSpace(m_cmThreadSpace);
        m_cmThreadSpace = nullptr;
    }
}

void CMRTKernelBase::DestroyProgramResources()
{
    if(m_cmProgram)
    {
        m_cmDev->DestroyProgram(m_cmProgram);
        m_cmProgram = nullptr;
    }
}

void CMRTKernelBase::Destroy()
{
    //Only keep one Task/Dev/Queue in one Process.
    if (m_cmTask)
    {
        m_cmDev->DestroyTask(m_cmTask);
        m_cmTask = nullptr;
    }

    if(m_cmDev)
    {
        DestroyCmDevice(m_cmDev);
        m_cmDev = nullptr;
    }
}

