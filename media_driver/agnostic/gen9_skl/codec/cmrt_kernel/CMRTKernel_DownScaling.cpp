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
//! \file     CMRTKernel_DownScaling.cpp
//! \brief    HEVC FEI 2xScaling class for GEN9 SKL.
//!

#include "CMRTKernel_DownScaling.h"
#include "CMRTKernel_DS_Kernel_def.h"

CMRTKernelDownScaling::CMRTKernelDownScaling()
{

    m_isaName           = HEVC_DS_ISA_FILE_NAME_G9;
    m_kernelName        = HEVCENCKERNELNAME_2xDS_FRAME;

    m_cmSurface2DCount  = 2;
    m_cmBufferCount     = 1;
    m_cmVmeSurfCount    = 0;

    if (m_cmSurface2DCount > 0)
    {
        m_cmSurface2D   = (CmSurface2D **)malloc(sizeof(CmSurface2D *) * m_cmSurface2DCount);
        if (m_cmSurface2D != NULL)
        {
            memset(m_cmSurface2D, 0, sizeof(CmSurface2D *) * m_cmSurface2DCount);
        }
    }

    if (m_cmBufferCount > 0)
    {
        m_cmBuffer      = (CmBuffer **)malloc(sizeof(CmBuffer *) * m_cmBufferCount);
        if (m_cmBuffer != NULL)
        {
            memset(m_cmBuffer, 0, sizeof(CmBuffer *) * m_cmBufferCount);
        }
    }

    if (m_cmVmeSurfCount > 0)
    {
        m_cmVmeSurf     = (SurfaceIndex **)malloc(sizeof(SurfaceIndex *) * m_cmVmeSurfCount);
        if (m_cmVmeSurf != NULL)
        {
            memset(m_cmVmeSurf, 0, sizeof(SurfaceIndex *) * m_cmVmeSurfCount);
        }
    }

    m_surfIndex         = (SurfaceIndex **)malloc(sizeof(SurfaceIndex *) * (m_cmSurface2DCount + m_cmBufferCount + m_cmVmeSurfCount));
    if (m_surfIndex != NULL)
    {
        memset(m_surfIndex, 0, sizeof(SurfaceIndex *) * (m_cmSurface2DCount + m_cmBufferCount + m_cmVmeSurfCount));
    }
}

CMRTKernelDownScaling::~CMRTKernelDownScaling()
{
    if (m_cmSurface2D != nullptr)
    {
        free(m_cmSurface2D);
    }

    if (m_cmBuffer != nullptr)
    {
        free(m_cmBuffer);
    }

    if (m_cmVmeSurf != nullptr)
    {
        free(m_cmVmeSurf);
    }

    if (m_surfIndex != nullptr)
    {
        free(m_surfIndex);
    }
}

CM_RETURN_CODE CMRTKernelDownScaling::SetupCurbe(void *curbe)
{
    m_curbe = curbe;
    return CM_SUCCESS;
}

CM_RETURN_CODE CMRTKernelDownScaling::CreateAndDispatchKernel(CmEvent *&cmEvent, bool destroyEvent, bool isEnqueue)
{
    CM_RETURN_CODE r = CM_SUCCESS;
    int32_t result;
    uint32_t *curbe = (uint32_t *)m_curbe;
    uint32_t reserved[7];
    uint32_t width, height, scaling2xWidth, scaling2xHeight, threadSpaceWidth, threadSpaceHeight;

    width =  curbe[0] & 0x0FFFF;
    height = (curbe[0] >> 16) & 0x0FFFF;

    scaling2xWidth = (16 * MOS_ROUNDUP_DIVIDE(width , 32));
    scaling2xHeight = (16 * MOS_ROUNDUP_DIVIDE(height , 32));

    if (scaling2xWidth < 48)
    {
        scaling2xWidth = 48;
    }

    if (scaling2xHeight < 48)
    {
        scaling2xHeight = 48;
    }

    threadSpaceWidth = MOS_ROUNDUP_DIVIDE(scaling2xWidth, 16);           // Each 16x16 pixel output is completed by 1 thread
    threadSpaceHeight = MOS_ROUNDUP_DIVIDE(scaling2xHeight, 16);         // Each 16x16 pixel output is completed by 1 thread

    m_cmKernel->SetKernelArg(0, sizeof(uint16_t), &width);                // DW0
    m_cmKernel->SetKernelArg(1, sizeof(uint16_t), &height);               // DW0
    m_cmKernel->SetKernelArg(2, 7 * sizeof(uint32_t), reserved);
    m_cmKernel->SetKernelArg(3, sizeof(SurfaceIndex), m_surfIndex[0]);            // DW8
    m_cmKernel->SetKernelArg(4, sizeof(SurfaceIndex), m_surfIndex[1]);            // DW9

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

CM_RETURN_CODE CMRTKernelDownScalingUMD::AllocateSurfaces(void *params)
{
    DownScalingKernelParams *scalingParams=(DownScalingKernelParams *)params;

    if (scalingParams->m_resetPic)
    {
        MOS_ZeroMemory(&m_mosResourceWA, sizeof(MOS_RESOURCE));
        MOS_SecureMemcpy(&m_mosResourceWA, sizeof(MOS_RESOURCE), (MOS_RESOURCE *)scalingParams->m_cmSurfDS_TopIn, sizeof(MOS_RESOURCE));

//        m_mosResourceWA.Format = CM_SURFACE_FORMAT_R8G8_SNORM;
//        m_mosResourceWA.iWidth = m_mosResourceWA.iPitch / 2;
        Mos_Specific_SetResourceFormat(&m_mosResourceWA, CM_SURFACE_FORMAT_R8G8_SNORM);
        Mos_Specific_SetResourceWidth(&m_mosResourceWA, Mos_Specific_GetResourcePitch(&m_mosResourceWA) / 2);

        CM_CHK_STATUS_RETURN(m_cmDev->CreateSurface2D(&m_mosResourceWA, m_cmSurface2D[0]));
        CM_CHK_STATUS_RETURN(m_cmSurface2D[0]->GetIndex(m_surfIndex[0]));

        CM_SURFACE2D_STATE_PARAM surfaceParams;
        memset(&surfaceParams, 0, sizeof(CM_SURFACE2D_STATE_PARAM));
        surfaceParams.width = scalingParams->m_width;

        CM_CHK_STATUS_RETURN(m_cmSurface2D[0]->SetSurfaceStateParam(m_surfIndex[0], &surfaceParams));
    }
    else
    {
        CM_CHK_STATUS_RETURN(m_cmDev->CreateSurface2D((MOS_RESOURCE *)scalingParams->m_cmSurfDS_TopIn, m_cmSurface2D[0]));
        CM_CHK_STATUS_RETURN(m_cmSurface2D[0]->GetIndex(m_surfIndex[0]));
    }

    CM_CHK_STATUS_RETURN(m_cmDev->CreateSurface2D((MOS_RESOURCE *)scalingParams->m_cmSurfDS_TopOut, m_cmSurface2D[1]));
    CM_CHK_STATUS_RETURN(m_cmSurface2D[1]->GetIndex(m_surfIndex[1]));

    if (scalingParams->m_cmSurfTopVProc != nullptr)
    {
        CM_CHK_STATUS_RETURN(m_cmDev->CreateBuffer((MOS_RESOURCE *)scalingParams->m_cmSurfTopVProc, m_cmBuffer[0]));
        CM_CHK_STATUS_RETURN(m_cmBuffer[0]->GetIndex(m_surfIndex[2]));
    }
    else
    {
        m_surfIndex[2] = nullptr;
    }

    return CM_SUCCESS;
}

