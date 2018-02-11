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
//! \file     CMRTKernel_I_8x8.cpp
//! \brief    HEVC FEI I 8x8 class for GEN9 SKL.
//!

#include "CMRTKernel_I_8x8.h"
#include "CMRTKernel_I_Kernel_def.h"

CMRTKernelI8x8::CMRTKernelI8x8()
{

    m_isaName         = HEVCENCFEI_I_GEN9;
    m_isaSize         = HEVCENCFEI_I_GEN9_SIZE;
    m_kernelName      = HEVCENCKERNELNAME_I_8x8;

    m_cmSurface2DCount = 6;
    m_cmBufferCount    = 3;
    m_cmVmeSurfCount   = 0;

    if (m_cmSurface2DCount > 0)
    {
        m_cmSurface2D     = (CmSurface2D **)malloc(sizeof(CmSurface2D *) * m_cmSurface2DCount);
        if (m_cmSurface2D != NULL)
        {
            memset(m_cmSurface2D, 0, sizeof(CmSurface2D *) * m_cmSurface2DCount);
        }
    }

    if (m_cmBufferCount > 0)
    {
        m_cmBuffer        = (CmBuffer **)malloc(sizeof(CmBuffer *) * m_cmBufferCount);
        if (m_cmBuffer != NULL)
        {
            memset(m_cmBuffer, 0, sizeof(CmBuffer *) * m_cmBufferCount);
        }
    }

    if (m_cmVmeSurfCount > 0)
    {
        m_cmVmeSurf       = (SurfaceIndex **)malloc(sizeof(SurfaceIndex *) * m_cmVmeSurfCount);
        if (m_cmVmeSurf != NULL)
        {
            memset(m_cmVmeSurf, 0, sizeof(SurfaceIndex *) * m_cmVmeSurfCount);
        }
    }

    m_surfIndex       = (SurfaceIndex **)malloc(sizeof(SurfaceIndex *) * (m_cmSurface2DCount + m_cmBufferCount + m_cmVmeSurfCount));
    if (m_surfIndex != NULL)
    {
        memset(m_surfIndex, 0, sizeof(SurfaceIndex *) * (m_cmSurface2DCount + m_cmBufferCount + m_cmVmeSurfCount));
    }
}

CMRTKernelI8x8::~CMRTKernelI8x8()
{
    if (m_cmSurface2D != NULL)
    {
        free(m_cmSurface2D);
    }

    if (m_cmBuffer != NULL)
    {
        free(m_cmBuffer);
    }

    if (m_cmVmeSurf != NULL)
    {
        free(m_cmVmeSurf);
    }

    if (m_surfIndex != NULL)
    {
        free(m_surfIndex);
    }
}

CM_RETURN_CODE CMRTKernelI8x8::SetupCurbe(void *curbe)
{
    m_curbe = curbe;
    return CM_SUCCESS;
}

CM_RETURN_CODE CMRTKernelI8x8::CreateAndDispatchKernel(CmEvent *&cmEvent, bool destroyEvent, bool isEnqueue)
{
    CM_RETURN_CODE r = CM_SUCCESS;
    int32_t result;
    uint8_t i, idx = 0;
    uint32_t width, height, threadSpaceWidth, threadSpaceHeight;
    uint32_t *curbe = (uint32_t *)m_curbe;

    width = curbe[0] & 0x0FFFF;
    height = (curbe[0] >> 16) & 0x0FFFF;

    threadSpaceWidth = width >> 3;
    threadSpaceHeight = height >> 3;

    CM_CHK_STATUS_RETURN(m_cmKernel->SetKernelArg(idx++, CURBEDATA_SIZE_I_8X8_PU, m_curbe));

    for (i = 0; i < NUM_MBENC_I_8x8_PU_SURFACES; i++)
    {
       CM_CHK_STATUS_RETURN(m_cmKernel->SetKernelArg(idx++, sizeof(SurfaceIndex), m_surfIndex[i]));
    }

    CM_CHK_STATUS_RETURN(m_cmKernel->SetThreadCount(threadSpaceWidth * threadSpaceHeight));
    //create Thread Space
    result = m_cmDev->CreateThreadSpace(threadSpaceWidth, threadSpaceHeight, m_cmThreadSpace);
    if (result != CM_SUCCESS)
    {
        printf("CM Create ThreadSpace error : %d", result);
        return (CM_RETURN_CODE)result;
    }

    r = AddKernel(cmEvent, destroyEvent, isEnqueue);
    return r;
}

CM_RETURN_CODE CMRTKernelI8x8UMD::AllocateSurfaces(void *params)
{
    IFrameKernelParams *ResourceParams = (IFrameKernelParams *)params;

    CM_CHK_STATUS_RETURN(m_cmDev->CreateSurface2D((MOS_RESOURCE *)ResourceParams->m_cmSurfCurrY, m_cmSurface2D[0]));
    CM_CHK_STATUS_RETURN(m_cmSurface2D[0]->GetIndex(m_surfIndex[0]));
    CM_CHK_STATUS_RETURN(m_cmDev->CreateSurface2D((MOS_RESOURCE *)ResourceParams->m_cmSurfSliceMap, m_cmSurface2D[1]));
    CM_CHK_STATUS_RETURN(m_cmSurface2D[1]->GetIndex(m_surfIndex[1]));
    CM_CHK_STATUS_RETURN(m_cmDev->CreateBuffer((MOS_RESOURCE *)ResourceParams->m_cmSurfVMEMode, m_cmBuffer[0]));
    CM_CHK_STATUS_RETURN(m_cmBuffer[0]->GetIndex(m_surfIndex[2]));
    CM_CHK_STATUS_RETURN(m_cmDev->CreateBuffer((MOS_RESOURCE *)ResourceParams->m_cmSurfMode, m_cmBuffer[1]));
    CM_CHK_STATUS_RETURN(m_cmBuffer[1]->GetIndex(m_surfIndex[3]));
    CM_CHK_STATUS_RETURN(m_cmDev->CreateBuffer((MOS_RESOURCE *)ResourceParams->m_cmSurfCombinedQP, m_cmBuffer[2]));
    CM_CHK_STATUS_RETURN(m_cmBuffer[2]->GetIndex(m_surfIndex[4]));
    CM_CHK_STATUS_RETURN(m_cmDev->CreateSurface2D((MOS_RESOURCE *)ResourceParams->m_cmSurfSIF, m_cmSurface2D[2]));
    CM_CHK_STATUS_RETURN(m_cmSurface2D[2]->GetIndex(m_surfIndex[5]));
    CM_CHK_STATUS_RETURN(m_cmDev->CreateSurface2D((MOS_RESOURCE *)ResourceParams->m_cmLCUQPSurf, m_cmSurface2D[3]));
    CM_CHK_STATUS_RETURN(m_cmSurface2D[3]->GetIndex(m_surfIndex[6]));
    CM_CHK_STATUS_RETURN(m_cmDev->CreateSurface2D((MOS_RESOURCE *)ResourceParams->m_cmBRCConstSurf, m_cmSurface2D[4]));
    CM_CHK_STATUS_RETURN(m_cmSurface2D[4]->GetIndex(m_surfIndex[7]));
    m_surfIndex[8] = (SurfaceIndex *)CM_NULL_SURFACE;

    return CM_SUCCESS;
}

