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
//! \file     CMRTKernel_PB_8x8Pak.cpp
//! \brief    HEVC FEI PB 8x8 PAK class for GEN9 SKL.
//!

#include "CMRTKernel_PB_8x8Pak.h"
#include "CMRTKernel_PB_Kernel_def.h"

CMRTKernelPB8x8Pak::CMRTKernelPB8x8Pak()
{

    m_isaName         = HEVCENCFEI_PB_GEN9;
    m_isaSize         = HEVCENCFEI_PB_GEN9_SIZE;
    m_kernelName      = HEVCENCKERNELNAME_PB_PAK;

    m_cmSurface2DCount = 7;
    m_cmBufferCount    = 4;
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

CMRTKernelPB8x8Pak::~CMRTKernelPB8x8Pak()
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

CM_RETURN_CODE CMRTKernelPB8x8Pak::SetupCurbe(void *curbe)
{
    m_curbe = curbe;
    return CM_SUCCESS;
}

CM_RETURN_CODE CMRTKernelPB8x8Pak::CreateAndDispatchKernel(CmEvent *&cmEvent, bool destroyEvent, bool isEnqueue)
{
    CM_RETURN_CODE r = CM_SUCCESS;
    int32_t result;
    uint8_t i, idx = 0;
    uint32_t width, height, width_padded, height_padded, threadSpaceWidth, threadSpaceHeight;
    uint32_t *curbe = (uint32_t *)m_curbe;

    width = curbe[0] & 0x0FFFF;
    height = (curbe[0] >> 16) & 0x0FFFF;
    width_padded = ((width + 16) >> 5) << 5;
    height_padded = ((height + 16) >> 5) << 5;

    threadSpaceWidth = width_padded >> 5;
    threadSpaceHeight = height_padded >> 5;

    CM_CHK_STATUS_RETURN(m_cmKernel->SetKernelArg(idx++, CURBEDATA_SIZE_PB_PAK, m_curbe));

    for (i = 0; i < NUM_MBENC_PB_PAK_SURFACES; i++)
    {
        CM_CHK_STATUS_RETURN(m_cmKernel->SetKernelArg(idx++, sizeof(SurfaceIndex), m_surfIndex[i]));
    }

    CM_CHK_STATUS_RETURN(m_cmKernel->SetThreadCount(threadSpaceWidth * threadSpaceHeight));
    //create Thread Space
    result = CreateThreadSpace(threadSpaceWidth, threadSpaceHeight);
    if (result != CM_SUCCESS)
    {
        printf("CM Create ThreadSpace error : %d", result);
        return (CM_RETURN_CODE)result;
    }

    r = AddKernel(cmEvent, destroyEvent, isEnqueue);
    return r;
}

CM_RETURN_CODE CMRTKernelPB8x8PakUMD::AllocateSurfaces(void *params)
{
    PBFrameKernelParams *PB8x8PakParams = (PBFrameKernelParams *)params;

    CM_BUFFER_STATE_PARAM bufParams;
    memset(&bufParams, 0, sizeof(CM_BUFFER_STATE_PARAM));
    bufParams.uiSize = PB8x8PakParams->m_bufSize;
    bufParams.uiBaseAddressOffset = PB8x8PakParams->m_bufOffset;

    CM_CHK_STATUS_RETURN(m_cmDev->CreateBuffer((MOS_RESOURCE *)PB8x8PakParams->m_cmSurfPOCDbuf, m_cmBuffer[0]));
    CM_CHK_STATUS_RETURN(m_cmDev->CreateBufferAlias(m_cmBuffer[0], m_surfIndex[0]));
    CM_CHK_STATUS_RETURN(m_cmBuffer[0]->SetSurfaceStateParam(m_surfIndex[0], &bufParams));
    CM_CHK_STATUS_RETURN(m_cmBuffer[0]->GetIndex(m_surfIndex[1]));
    CM_CHK_STATUS_RETURN(m_cmDev->CreateSurface2D((MOS_RESOURCE *)PB8x8PakParams->m_cmSurfSliceMap, m_cmSurface2D[0]));
    CM_CHK_STATUS_RETURN(m_cmSurface2D[0]->GetIndex(m_surfIndex[2]));
    CM_CHK_STATUS_RETURN(m_cmDev->CreateBuffer((MOS_RESOURCE *)PB8x8PakParams->m_cmSurfCombinedQP, m_cmBuffer[1]));
    CM_CHK_STATUS_RETURN(m_cmBuffer[1]->GetIndex(m_surfIndex[3]));
    CM_CHK_STATUS_RETURN(m_cmDev->CreateSurface2D((MOS_RESOURCE *)PB8x8PakParams->m_cmLCUQPSurf, m_cmSurface2D[1]));
    CM_CHK_STATUS_RETURN(m_cmSurface2D[1]->GetIndex(m_surfIndex[4]));
    CM_CHK_STATUS_RETURN(m_cmDev->CreateSurface2D((MOS_RESOURCE *)PB8x8PakParams->m_cmBRCConstSurf, m_cmSurface2D[2]));
    CM_CHK_STATUS_RETURN(m_cmSurface2D[2]->GetIndex(m_surfIndex[5]));
    CM_CHK_STATUS_RETURN(m_cmDev->CreateBuffer((MOS_RESOURCE *)PB8x8PakParams->m_cmSurfMVIndex, m_cmBuffer[2]));
    CM_CHK_STATUS_RETURN(m_cmBuffer[2]->GetIndex(m_surfIndex[6]));
    CM_CHK_STATUS_RETURN(m_cmDev->CreateBuffer((MOS_RESOURCE *)PB8x8PakParams->m_cmSurfMVPred, m_cmBuffer[3]));
    CM_CHK_STATUS_RETURN(m_cmBuffer[3]->GetIndex(m_surfIndex[7]));
    m_surfIndex[8] = (SurfaceIndex *)CM_NULL_SURFACE;
    m_surfIndex[9] = (SurfaceIndex *)CM_NULL_SURFACE;
    m_surfIndex[10] = (SurfaceIndex *)CM_NULL_SURFACE;

    return CM_SUCCESS;
}

