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
//! \file     codechal_decode_nv12top010.cpp
//! \brief    Implements the conversion from NV12 to P010 format.
//! \details  This file is for the base implemetation which is shared by all platforms.
//!

#include "codechal_decoder.h"
#include "codechal_decode_nv12top010.h"

#include "cm_surface_2d_rt.h"

MOS_STATUS CodechalDecodeNV12ToP010::Init(PMOS_INTERFACE osInterface)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(osInterface);
    m_osInterface = osInterface;
    uint32_t devCreateOption = CM_DEVICE_CREATE_OPTION_FOR_HEVC;
    devCreateOption &= (~CM_DEVICE_CONFIG_GPUCONTEXT_ENABLE);

    osInterface->pfnNotifyStreamIndexSharing(osInterface);
    CODECHAL_DECODE_CHK_STATUS_RETURN(osInterface->pfnCreateCmDevice(
        osInterface->pOsContext,
        m_cmDevice,
        devCreateOption,
        CM_DEVICE_CREATE_PRIORITY_DEFAULT));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmDevice->CreateQueue(
        m_cmQueue));

    CmProgram *nv12ToP010Program;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmDevice->LoadProgram(
        (void *)m_nv12ToP010KernelBinary,
        m_nv12ToP010KernelSize,
        nv12ToP010Program,
        "-nojitter"));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmDevice->CreateKernel(
        nv12ToP010Program,
        "NV12ToP010",
        m_cmKernel));

    return eStatus;
}

MOS_STATUS CodechalDecodeNV12ToP010::Execute(
    PMOS_RESOURCE srcResource,
    PMOS_RESOURCE dstResource)
{
    MOS_STATUS eStatus= MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(m_cmDevice);

    MOS_TraceEventExt(EVENT_CODEC_NV12ToP010, EVENT_TYPE_START, nullptr, 0, nullptr, 0);

    CmSurface2D *srcCmSurface2D = nullptr;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmDevice->CreateSurface2D(srcResource, srcCmSurface2D));
    CODECHAL_DECODE_CHK_NULL_RETURN(srcCmSurface2D);
    CmSurface2D *dstCmSurface2D = nullptr;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmDevice->CreateSurface2D(dstResource, dstCmSurface2D));
    CODECHAL_DECODE_CHK_NULL_RETURN(dstCmSurface2D);

    uint32_t surfaceWidth, surfaceHeight;
    MOS_FORMAT format;
    uint32_t sizePerPixel;
    CmSurface2DRT *surfaceRT = static_cast<CmSurface2DRT *>(srcCmSurface2D);
    surfaceRT->GetSurfaceDesc(surfaceWidth, surfaceHeight, format, sizePerPixel);

    SurfaceIndex *srcSurfaceIndex;
    CODECHAL_DECODE_CHK_STATUS_RETURN(srcCmSurface2D->GetIndex(srcSurfaceIndex));
    SurfaceIndex *dstSurfaceIndex;
    CODECHAL_DECODE_CHK_STATUS_RETURN(dstCmSurface2D->GetIndex(dstSurfaceIndex));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmDevice->CreateTask(m_cmTask));
    CODECHAL_DECODE_CHK_NULL_RETURN(m_cmTask);

    uint32_t threadWidth  = MOS_ALIGN_CEIL(surfaceWidth, 8) / 8;
    uint32_t threadHeight = MOS_ALIGN_CEIL(surfaceHeight, 16) / 16;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmDevice->CreateThreadSpace(
        threadWidth,
        threadHeight,
        m_cmThreadSpace));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmKernel->SetThreadCount(threadWidth * threadHeight));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmKernel->AssociateThreadSpace(m_cmThreadSpace));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmKernel->SetKernelArg(0, sizeof(SurfaceIndex), srcSurfaceIndex));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmKernel->SetKernelArg(1, sizeof(SurfaceIndex), dstSurfaceIndex));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmTask->AddSync());
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmTask->AddKernel(m_cmKernel));

    CmEvent *eventKernel;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmQueue->Enqueue(
        m_cmTask, eventKernel));

    CODECHAL_DECODE_CHK_NULL_RETURN(eventKernel);
    eventKernel->WaitForTaskFinished();

    if (srcCmSurface2D != nullptr)
    {
        m_cmDevice->DestroySurface(srcCmSurface2D);
    }

    if (dstCmSurface2D != nullptr)
    {
        m_cmDevice->DestroySurface(dstCmSurface2D);
    }

    MOS_TraceEventExt(EVENT_CODEC_NV12ToP010, EVENT_TYPE_END, &surfaceWidth, sizeof(uint32_t), &surfaceHeight, sizeof(uint32_t));

    return eStatus;
}

CodechalDecodeNV12ToP010::~CodechalDecodeNV12ToP010()
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_NO_STATUS_RETURN(m_cmDevice);

    if (m_cmKernel != nullptr)
    {
        m_cmDevice->DestroyKernel(m_cmKernel);
        m_cmKernel = nullptr;
    }
    if (m_osInterface != nullptr)
    {
        m_osInterface->pfnDestroyCmDevice(m_cmDevice);
    }
}

