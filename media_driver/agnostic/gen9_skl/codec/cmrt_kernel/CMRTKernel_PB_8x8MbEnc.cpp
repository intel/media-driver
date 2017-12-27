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
//! \file     CMRTKernel_PB_8x8MbEnc.cpp
//! \brief    HEVC FEI PB 8x8 MbEnc class for GEN9 SKL.
//!

#include "CMRTKernel_PB_8x8MbEnc.h"
#include "CMRTKernel_PB_Kernel_def.h"

CMRTKernelPB8x8MbEnc::CMRTKernelPB8x8MbEnc()
{

    m_isaName           = HEVC_PB_ISA_FILE_NAME_G9;
    m_kernelName        = HEVCENCKERNELNAME_PB_MB;

    m_cmSurface2DCount   = 17;
    m_cmBufferCount      = 9;
    m_cmVmeSurfCount     = 2;
    m_cmSurfaceRef0Count = 8;
    m_cmSurfaceRef1Count = 8;

    if (m_cmSurface2DCount > 0)
    {
        m_cmSurface2D     = (CmSurface2D **)malloc(sizeof(CmSurface2D *) * m_cmSurface2DCount);
        if (m_cmSurface2D != nullptr)
        {
            memset(m_cmSurface2D, 0, sizeof(CmSurface2D *) * m_cmSurface2DCount);
        }
    }
    if (m_cmSurfaceRef0Count > 0)
    {
        m_cmSurfaceRef0   = (CmSurface2D **)malloc(sizeof(CmSurface2D *) * m_cmSurfaceRef0Count);
        if (m_cmSurfaceRef0 != nullptr)
        {
            memset(m_cmSurfaceRef0, 0, sizeof(CmSurface2D *) * m_cmSurfaceRef0Count);
        }
    }
    if (m_cmSurfaceRef1Count > 0)
    {
        m_cmSurfaceRef1   = (CmSurface2D **)malloc(sizeof(CmSurface2D *) * m_cmSurfaceRef1Count);
        if (m_cmSurfaceRef1 != nullptr)
        {
            memset(m_cmSurfaceRef1, 0, sizeof(CmSurface2D *) * m_cmSurfaceRef1Count);
        }
    }
    if (m_cmBufferCount > 0)
    {
        m_cmBuffer        = (CmBuffer **)malloc(sizeof(CmBuffer *) * m_cmBufferCount);
        if (m_cmBuffer != nullptr)
        {
            memset(m_cmBuffer, 0, sizeof(CmBuffer *) * m_cmBufferCount);
        }
    }
    if (m_cmVmeSurfCount > 0)
    {
        m_cmVmeSurf       = (SurfaceIndex **)malloc(sizeof(SurfaceIndex *) * m_cmVmeSurfCount);
        if (m_cmVmeSurf != nullptr)
        {
            memset(m_cmVmeSurf, 0, sizeof(SurfaceIndex *) * m_cmVmeSurfCount);
        }
    }

    m_surfIndex       = (SurfaceIndex **)malloc(sizeof(SurfaceIndex *) * (m_cmSurface2DCount + m_cmBufferCount + m_cmVmeSurfCount));
    if (m_surfIndex != nullptr)
    {
        memset(m_surfIndex, 0, sizeof(SurfaceIndex *) * (m_cmSurface2DCount + m_cmBufferCount + m_cmVmeSurfCount));
    }
}

CMRTKernelPB8x8MbEnc::~CMRTKernelPB8x8MbEnc()
{
    if (m_cmSurface2D != nullptr)
    {
        free(m_cmSurface2D);
    }
    if (m_cmSurfaceRef0 != nullptr)
    {
        free(m_cmSurfaceRef0);
    }
    if (m_cmSurfaceRef1 != nullptr)
    {
        free(m_cmSurfaceRef1);
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

CM_RETURN_CODE CMRTKernelPB8x8MbEnc::SetupCurbe(void *curbe)
{
    m_curbe = curbe;
    return CM_SUCCESS;
}

CM_RETURN_CODE CMRTKernelPB8x8MbEnc::CreateAndDispatchKernel(CmEvent *&cmEvent, bool destroyEvent, bool isEnqueue)
{
    CM_RETURN_CODE r = CM_SUCCESS;
    int32_t result;
    uint8_t i, idx = 0;
    uint8_t *curbe = (uint8_t *)m_curbe;
    uint32_t width, height, width_padded, height_padded, threadSpaceWidth, threadSpaceHeight, colorCount, splitCount, tempHeight;
    PBMbEncWalkParams mbEncWalkParams;

    memset(&mbEncWalkParams, 0, sizeof(PBMbEncWalkParams));

    width = ((curbe[25] & 0xFF) << 8) | (curbe[24] & 0xFF);
    height = ((curbe[27] & 0xFF) << 8) | (curbe[26] & 0xFF);
    width_padded = ((width + 16) >> 5) << 5;
    height_padded = ((height + 16) >> 5) << 5;

    colorCount = (curbe[191] >> 2) + 1;
    splitCount = (curbe[188] & 0x0F);
    if (splitCount == 0)
    {
        splitCount = 1;
    }

    threadSpaceWidth = width_padded >> 4;
    threadSpaceHeight = (colorCount > 1) ? ((curbe[211] << 8) + (curbe[210] & 0xFF)) : (height_padded >> 4);

    CM_CHK_STATUS_RETURN(m_cmKernel->SetKernelArg(idx++, CURBEDATA_SIZE_PB_MB, m_curbe));

    for (i = 0; i < NUM_MBENC_PB_MB_SURFACES; i++)
    {
        CM_CHK_STATUS_RETURN(m_cmKernel->SetKernelArg(idx++, sizeof(SurfaceIndex), m_surfIndex[i]));
    }

    if ((curbe[189] & 0x0F) == 1)
    {
        CM_CHK_STATUS_RETURN(m_cmKernel->SetThreadCount(((threadSpaceWidth + 1) & 0xFFFE) * threadSpaceHeight * colorCount));
        //create Thread Space
        result = m_cmDev->CreateThreadSpace(((threadSpaceWidth + 1) & 0xFFFE), threadSpaceHeight, m_cmThreadSpace);
        if (result != CM_SUCCESS)
        {
            printf("CM Create ThreadSpace error : %d", result);
            return (CM_RETURN_CODE)result;
        }
        SetupMwScoreboard26(mbEncWalkParams.m_walkParams, mbEncWalkParams.m_scoreboardParams, threadSpaceWidth, threadSpaceHeight, splitCount, colorCount);
    }
    else if ((curbe[189]&0x0F) == 0)
    {
        tempHeight = ((threadSpaceWidth + 3) >> 2) + (((threadSpaceWidth + 1) >> 1) + 2 * (((threadSpaceHeight + 1) >> 1) - 1) + (2 * splitCount - 1)) / (2 * splitCount);
        CM_CHK_STATUS_RETURN(m_cmKernel->SetThreadCount(((threadSpaceWidth + 3) & 0xFFFC) * 2 * tempHeight * colorCount));
        //create Thread Space
        result = m_cmDev->CreateThreadSpace(((threadSpaceWidth + 3) & 0xFFFC) >> 1, 4 * tempHeight, m_cmThreadSpace);
        SetupMwScoreboard26Zig(mbEncWalkParams.m_walkParams, mbEncWalkParams.m_scoreboardParams, threadSpaceWidth, threadSpaceHeight, splitCount, colorCount);
    }

    if (m_cmThreadSpace != nullptr)
    {
        CM_CHK_STATUS_RETURN(m_cmThreadSpace->SelectThreadDependencyVectors(mbEncWalkParams.m_scoreboardParams));
        CM_CHK_STATUS_RETURN(m_cmThreadSpace->SetThreadSpaceColorCount(colorCount));
        CM_CHK_STATUS_RETURN(m_cmThreadSpace->SelectMediaWalkingParameters(mbEncWalkParams.m_walkParams));
    }

    r = AddKernel(cmEvent, destroyEvent, isEnqueue);
    return r;
}

void CMRTKernelPB8x8MbEnc::SetupMwScoreboard26(CM_WALKING_PARAMETERS& walkParams, CM_DEPENDENCY& scoreboardParams, uint32_t width, uint32_t height, uint32_t splitCount, uint32_t colorCount)
{
    uint8_t n = 0;
    uint32_t *pDW5 = &walkParams.Value[n++];
    uint32_t *pDW6 = &walkParams.Value[n++];
    uint32_t *pDW7 = &walkParams.Value[n++];
    uint32_t *pDW8 = &walkParams.Value[n++];
    uint32_t *pDW9 = &walkParams.Value[n++];
    uint32_t *pDW10 = &walkParams.Value[n++];
    uint32_t *pDW11 = &walkParams.Value[n++];
    uint32_t *pDW12 = &walkParams.Value[n++];
    uint32_t *pDW13 = &walkParams.Value[n++];
    uint32_t *pDW14 = &walkParams.Value[n++];
    uint32_t *pDW15 = &walkParams.Value[n++];
    uint32_t *pDW16 = &walkParams.Value[n++];

    int32_t scoreBoardMask;
    int32_t globalResolutionX;
    int32_t globalResolutionY;
    int32_t globalStartX;
    int32_t globalStartY;
    int32_t globalOuterLoopStrideX;
    int32_t globalOuterLoopStrideY;
    int32_t globalInnerLoopUnitX;
    int32_t globalInnerLoopUnitY;
    int32_t blockResolutionX;
    int32_t blockResolutionY;
    int32_t localInitialStartPointX;
    int32_t localInitialStartPointY;
    int32_t localEndX;
    int32_t localEndY;
    int32_t outerLoopUnitX;
    int32_t outerLoopUnitY;
    int32_t innerLoopUnitX;
    int32_t innerLoopUnitY;
    int32_t middleLoopExtraSteps;
    int32_t middleLoopUnitX;
    int32_t middleLoopUnitY;
    int32_t globalOuterLoopExecCount;
    int32_t localOuterLoopExecCount;
    int32_t ts_width = (width + 1) & 0xfffe;
    int32_t ts_height = (height + 1) & 0xfffe;

    int32_t tmp1 = ((ts_width + 1) >> 1) + ((ts_width + ((ts_height - 1) << 1)) + (2 * splitCount - 1)) / (2 * splitCount);

    scoreBoardMask = 0x0F;
    globalResolutionX = ts_width;
    globalResolutionY = tmp1;   // ts_height;
    globalStartX = 0;
    globalStartY = 0;
    globalOuterLoopStrideX = ts_width;
    globalOuterLoopStrideY = 0;
    globalInnerLoopUnitX = 0;
    globalInnerLoopUnitY = tmp1;    // 2;
    blockResolutionX = ts_width;
    blockResolutionY = tmp1;
    localInitialStartPointX = ts_width;
    localInitialStartPointY = 0;
    localEndX = 0;
    localEndY = 0;
    outerLoopUnitX = 1;
    outerLoopUnitY = 0;
    innerLoopUnitX = -2;
    innerLoopUnitY = 1;
    middleLoopExtraSteps = 0;
    middleLoopUnitX = 0;
    middleLoopUnitY = 0;
    globalOuterLoopExecCount = 0;
    localOuterLoopExecCount = (width + (height - 1) * 2 + splitCount - 1) / splitCount;

    *pDW5 = scoreBoardMask;
    *pDW6 = ((middleLoopUnitX & 0x3) << 8) | ((middleLoopUnitY & 0x3) << 12) | ((middleLoopExtraSteps & 0x1F) << 16) | (((colorCount - 1) & 0x0F) << 24);
    *pDW7 = (localOuterLoopExecCount & 0x0FFF) | ((globalOuterLoopExecCount & 0x0FFF) << 16);
    *pDW8 = (blockResolutionX & 0x7FF) | ((blockResolutionY & 0x7FF) << 16);
    *pDW9 = (localInitialStartPointX & 0x7FF) | ((localInitialStartPointY & 0x7FF) << 16);
    *pDW10 = (localEndX & 0xFFFF) | ((localEndY & 0xFFFF) << 16);
    *pDW11 = (outerLoopUnitX & 0x0FFF) | ((outerLoopUnitY & 0x0FFF) << 16);
    *pDW12 = (innerLoopUnitX & 0x0FFF) | ((innerLoopUnitY & 0x0FFF) << 16);
    *pDW13 = (globalResolutionX & 0x7FF) | ((globalResolutionY & 0x7FF) << 16);
    *pDW14 = (globalStartX & 0x0FFF) | ((globalStartY & 0x0FFF) << 16);
    *pDW15 = (globalOuterLoopStrideX & 0x0FFF) | ((globalOuterLoopStrideY & 0x0FFF) << 16);
    *pDW16 = (globalInnerLoopUnitX & 0x0FFF) | ((globalInnerLoopUnitY & 0x0FFF) << 16);

    scoreboardParams.count = 4;

    scoreboardParams.deltaX[0] = -1;
    scoreboardParams.deltaY[0] = 0;

    scoreboardParams.deltaX[1] = -1;
    scoreboardParams.deltaY[1] = -1;

    scoreboardParams.deltaX[2] = 0;
    scoreboardParams.deltaY[2] = -1;

    scoreboardParams.deltaX[3] = 1;
    scoreboardParams.deltaY[3] = -1;
}

void CMRTKernelPB8x8MbEnc::SetupMwScoreboard26Zig(CM_WALKING_PARAMETERS& walkParams, CM_DEPENDENCY& scoreboardParams, uint32_t width, uint32_t height, uint32_t splitCount, uint32_t colorCount)
{
    uint8_t n = 0;
    uint32_t *pDW5 = &walkParams.Value[n++];
    uint32_t *pDW6 = &walkParams.Value[n++];
    uint32_t *pDW7 = &walkParams.Value[n++];
    uint32_t *pDW8 = &walkParams.Value[n++];
    uint32_t *pDW9 = &walkParams.Value[n++];
    uint32_t *pDW10 = &walkParams.Value[n++];
    uint32_t *pDW11 = &walkParams.Value[n++];
    uint32_t *pDW12 = &walkParams.Value[n++];
    uint32_t *pDW13 = &walkParams.Value[n++];
    uint32_t *pDW14 = &walkParams.Value[n++];
    uint32_t *pDW15 = &walkParams.Value[n++];
    uint32_t *pDW16 = &walkParams.Value[n++];

    int32_t scoreBoardMask;
    int32_t globalResolutionX;
    int32_t globalResolutionY;
    int32_t globalStartX;
    int32_t globalStartY;
    int32_t globalOuterLoopStrideX;
    int32_t globalOuterLoopStrideY;
    int32_t globalInnerLoopUnitX;
    int32_t globalInnerLoopUnitY;
    int32_t blockResolutionX;
    int32_t blockResolutionY;
    int32_t localInitialStartPointX;
    int32_t localInitialStartPointY;
    int32_t localEndX;
    int32_t localEndY;
    int32_t outerLoopUnitX;
    int32_t outerLoopUnitY;
    int32_t innerLoopUnitX;
    int32_t innerLoopUnitY;
    int32_t middleLoopExtraSteps;
    int32_t middleLoopUnitX;
    int32_t middleLoopUnitY;
    int32_t globalOuterLoopExecCount;
    int32_t localOuterLoopExecCount;
    int32_t ts_width = ((width + 3) & 0xFFFC) >> 1;
    int32_t LCU_width = (width + 1) >> 1;
    int32_t LCU_height = (height + 1) >> 1;

    int32_t tmp1 = ((LCU_width + 1) >> 1) + ((LCU_width + ((LCU_height - 1) << 1)) + (2 * splitCount - 1)) / (2 * splitCount);


    scoreBoardMask = 0x0FF;
    globalResolutionX = ts_width;
    globalResolutionY = 4 * tmp1;
    globalStartX = 0;
    globalStartY = 0;
    globalOuterLoopStrideX = ts_width;
    globalOuterLoopStrideY = 0;
    globalInnerLoopUnitX = 0;
    globalInnerLoopUnitY = 4 * tmp1;
    blockResolutionX = ts_width;
    blockResolutionY = 4 * tmp1;
    localInitialStartPointX = ts_width;
    localInitialStartPointY = 0;
    localEndX = 0;
    localEndY = 0;
    outerLoopUnitX = 1;
    outerLoopUnitY = 0;
    innerLoopUnitX = -2;
    innerLoopUnitY = 4;
    middleLoopExtraSteps = 3;
    middleLoopUnitX = 0;
    middleLoopUnitY = 1;
    globalOuterLoopExecCount = 0;
    localOuterLoopExecCount = 2 * ((LCU_width + (LCU_height - 1) * 2 + 2 * splitCount - 1) / (2 * splitCount)) - 1;

    *pDW5 = scoreBoardMask;
    *pDW6 = ((middleLoopUnitX & 0x3) << 8) | ((middleLoopUnitY & 0x3) << 12) | ((middleLoopExtraSteps & 0x1F) << 16) | (((colorCount - 1) & 0x0F) << 24);
    *pDW7 = (localOuterLoopExecCount & 0x0FFF) | ((globalOuterLoopExecCount & 0x0FFF) << 16);
    *pDW8 = (blockResolutionX & 0x7FF) | ((blockResolutionY & 0x7FF) << 16);
    *pDW9 = (localInitialStartPointX & 0x7FF) | ((localInitialStartPointY & 0x7FF) << 16);
    *pDW10 = (localEndX & 0xFFFF) | ((localEndY & 0xFFFF) << 16);
    *pDW11 = (outerLoopUnitX & 0x0FFF) | ((outerLoopUnitY & 0x0FFF) << 16);
    *pDW12 = (innerLoopUnitX & 0x0FFF) | ((innerLoopUnitY & 0x0FFF) << 16);
    *pDW13 = (globalResolutionX & 0x7FF) | ((globalResolutionY & 0x7FF) << 16);
    *pDW14 = (globalStartX & 0x0FFF) | ((globalStartY & 0x0FFF) << 16);
    *pDW15 = (globalOuterLoopStrideX & 0x0FFF) | ((globalOuterLoopStrideY & 0x0FFF) << 16);
    *pDW16 = (globalInnerLoopUnitX & 0x0FFF) | ((globalInnerLoopUnitY & 0x0FFF) << 16);

    scoreboardParams.count = 8;

    scoreboardParams.deltaX[0] = -1;
    scoreboardParams.deltaY[0] = 3;

    scoreboardParams.deltaX[1] = -1;
    scoreboardParams.deltaY[1] = 1;

    scoreboardParams.deltaX[2] = -1;
    scoreboardParams.deltaY[2] = -1;

    scoreboardParams.deltaX[3] = 0;
    scoreboardParams.deltaY[3] = -1;

    scoreboardParams.deltaX[4] = 0;
    scoreboardParams.deltaY[4] = -2;

    scoreboardParams.deltaX[5] = 0;
    scoreboardParams.deltaY[5] = -3;

    scoreboardParams.deltaX[6] = 1;
    scoreboardParams.deltaY[6] = -2;

    scoreboardParams.deltaX[7] = 1;
    scoreboardParams.deltaY[7] = -3;
}

CM_RETURN_CODE CMRTKernelPB8x8MbEncUMD::AllocateSurfaces(void *params)
{
    PBFrameKernelParams *PB8x8MbEncParams = (PBFrameKernelParams *)params;

    uint8_t i;

    CM_VME_SURFACE_STATE_PARAM surfaceParams;
    memset(&surfaceParams, 0, sizeof(CM_VME_SURFACE_STATE_PARAM));
    surfaceParams.width = PB8x8MbEncParams->m_width;
    surfaceParams.height = PB8x8MbEncParams->m_height; 

    CM_BUFFER_STATE_PARAM bufParams;
    memset(&bufParams, 0, sizeof(CM_BUFFER_STATE_PARAM));
    bufParams.uiSize = PB8x8MbEncParams->m_bufSize;
    bufParams.uiBaseAddressOffset = PB8x8MbEncParams->m_bufOffset;

    CM_CHK_STATUS_RETURN(m_cmDev->CreateBuffer((MOS_RESOURCE *)PB8x8MbEncParams->m_cmSurfPOCDbuf, m_cmBuffer[0]));
    CM_CHK_STATUS_RETURN(m_cmDev->CreateBufferAlias(m_cmBuffer[0], m_surfIndex[0]));
    CM_CHK_STATUS_RETURN(m_cmBuffer[0]->SetSurfaceStateParam(m_surfIndex[0], &bufParams));
    CM_CHK_STATUS_RETURN(m_cmBuffer[0]->GetIndex(m_surfIndex[1]));
    CM_CHK_STATUS_RETURN(m_cmDev->CreateSurface2D((MOS_RESOURCE *)PB8x8MbEncParams->m_cmSurfCurrY, m_cmSurface2D[0]));
    CM_CHK_STATUS_RETURN(m_cmSurface2D[0]->GetIndex(m_surfIndex[2]));
    CM_CHK_STATUS_RETURN(m_cmDev->CreateBuffer((MOS_RESOURCE *)PB8x8MbEncParams->m_cmSurfIntraDist, m_cmBuffer[1]));
    CM_CHK_STATUS_RETURN(m_cmBuffer[1]->GetIndex(m_surfIndex[3]));
    CM_CHK_STATUS_RETURN(m_cmDev->CreateSurface2D((MOS_RESOURCE *)PB8x8MbEncParams->m_cmSurfMinDist, m_cmSurface2D[1]));
    CM_CHK_STATUS_RETURN(m_cmSurface2D[1]->GetIndex(m_surfIndex[4]));
    m_surfIndex[5] = (SurfaceIndex *)CM_NULL_SURFACE;
    m_surfIndex[6] = (SurfaceIndex *)CM_NULL_SURFACE;
    CM_CHK_STATUS_RETURN(m_cmDev->CreateSurface2D((MOS_RESOURCE *)PB8x8MbEncParams->m_cmSurfSliceMap, m_cmSurface2D[4]));
    CM_CHK_STATUS_RETURN(m_cmSurface2D[4]->GetIndex(m_surfIndex[7]));
    CM_CHK_STATUS_RETURN(m_cmDev->CreateBuffer((MOS_RESOURCE *)PB8x8MbEncParams->m_cmSurfVMEIN, m_cmBuffer[2]));
    CM_CHK_STATUS_RETURN(m_cmBuffer[2]->GetIndex(m_surfIndex[8]));
    CM_CHK_STATUS_RETURN(m_cmDev->CreateSurface2D((MOS_RESOURCE *)PB8x8MbEncParams->m_cmSurfSIF, m_cmSurface2D[5]));
    CM_CHK_STATUS_RETURN(m_cmSurface2D[5]->GetIndex(m_surfIndex[9]));
    
    if (PB8x8MbEncParams->m_cmSurfColRefData == nullptr)
    {
        m_surfIndex[10] = (SurfaceIndex *)CM_NULL_SURFACE;
    }
    else
    {
        CM_CHK_STATUS_RETURN(m_cmDev->CreateBuffer((MOS_RESOURCE *)PB8x8MbEncParams->m_cmSurfColRefData, m_cmBuffer[3]));
        CM_CHK_STATUS_RETURN(m_cmBuffer[3]->GetIndex(m_surfIndex[10]));
    }

    m_surfIndex[11] = (SurfaceIndex *)CM_NULL_SURFACE;
    CM_CHK_STATUS_RETURN(m_cmDev->CreateBuffer((MOS_RESOURCE *)PB8x8MbEncParams->m_cmSurfCombinedQP, m_cmBuffer[4]));
    CM_CHK_STATUS_RETURN(m_cmBuffer[4]->GetIndex(m_surfIndex[12]));
    CM_CHK_STATUS_RETURN(m_cmDev->CreateSurface2D((MOS_RESOURCE *)PB8x8MbEncParams->m_cmLCUQPSurf, m_cmSurface2D[6]));
    CM_CHK_STATUS_RETURN(m_cmSurface2D[6]->GetIndex(m_surfIndex[13]));
    CM_CHK_STATUS_RETURN(m_cmDev->CreateSurface2D((MOS_RESOURCE *)PB8x8MbEncParams->m_cmBRCConstSurf, m_cmSurface2D[7]));
    CM_CHK_STATUS_RETURN(m_cmSurface2D[7]->GetIndex(m_surfIndex[14]));

    for (i = 0; i < PB8x8MbEncParams->m_ucRefNum0; i++)
    {
         CM_CHK_STATUS_RETURN(m_cmDev->CreateSurface2D((MOS_RESOURCE *)PB8x8MbEncParams->m_cmSurfRef0[i], m_cmSurfaceRef0[i]));
    }
    for (i = 0; i < PB8x8MbEncParams->m_ucRefNum1; i++)
    {
         CM_CHK_STATUS_RETURN(m_cmDev->CreateSurface2D((MOS_RESOURCE *)PB8x8MbEncParams->m_cmSurfRef1[i], m_cmSurfaceRef1[i]));
    }

    CM_CHK_STATUS_RETURN(m_cmDev->CreateVmeSurfaceG7_5(m_cmSurface2D[0],
                                                 m_cmSurfaceRef0, m_cmSurfaceRef1,
                                                 PB8x8MbEncParams->m_ucRefNum0, PB8x8MbEncParams->m_ucRefNum1,
                                                 m_cmVmeSurf[0]));
    CM_CHK_STATUS_RETURN(m_cmDev->SetVmeSurfaceStateParam(m_cmVmeSurf[0], &surfaceParams));
    m_surfIndex[15] = m_cmVmeSurf[0];
    CM_CHK_STATUS_RETURN(m_cmDev->CreateVmeSurfaceG7_5(m_cmSurface2D[0],
                                                 m_cmSurfaceRef1, m_cmSurfaceRef1,
                                                 PB8x8MbEncParams->m_ucRefNum1, PB8x8MbEncParams->m_ucRefNum1,
                                                 m_cmVmeSurf[1]));
    CM_CHK_STATUS_RETURN(m_cmDev->SetVmeSurfaceStateParam(m_cmVmeSurf[1], &surfaceParams));
    m_surfIndex[16] = m_cmVmeSurf[1];
    CM_CHK_STATUS_RETURN(m_cmDev->CreateSurface2D((MOS_RESOURCE *)PB8x8MbEncParams->m_cmWaveFrontMap, m_cmSurface2D[8]));
    CM_CHK_STATUS_RETURN(m_cmSurface2D[8]->GetIndex(m_surfIndex[17]));
    CM_CHK_STATUS_RETURN(m_cmDev->CreateBuffer((MOS_RESOURCE *)PB8x8MbEncParams->m_cmSurfMVIndex, m_cmBuffer[5]));
    CM_CHK_STATUS_RETURN(m_cmBuffer[5]->GetIndex(m_surfIndex[18]));
    CM_CHK_STATUS_RETURN(m_cmDev->CreateBuffer((MOS_RESOURCE *)PB8x8MbEncParams->m_cmSurfMVPred, m_cmBuffer[6]));
    CM_CHK_STATUS_RETURN(m_cmBuffer[6]->GetIndex(m_surfIndex[19]));
    m_surfIndex[20] = (SurfaceIndex *)CM_NULL_SURFACE;
    m_surfIndex[21] = (SurfaceIndex *)CM_NULL_SURFACE;
    m_surfIndex[22] = (SurfaceIndex *)CM_NULL_SURFACE;
    if (PB8x8MbEncParams->m_cmSurfMVPredictor == nullptr)
    {
        m_surfIndex[23] = (SurfaceIndex *)CM_NULL_SURFACE;
    }
    else
    {
        CM_CHK_STATUS_RETURN(m_cmDev->CreateBuffer((MOS_RESOURCE *)PB8x8MbEncParams->m_cmSurfMVPredictor, m_cmBuffer[8]));
        CM_CHK_STATUS_RETURN(m_cmBuffer[8]->GetIndex(m_surfIndex[23]));
    }
    m_surfIndex[24] = (SurfaceIndex *)CM_NULL_SURFACE;
    m_surfIndex[25] = (SurfaceIndex *)CM_NULL_SURFACE;
    m_surfIndex[26] = (SurfaceIndex *)CM_NULL_SURFACE;

    return CM_SUCCESS;
}

