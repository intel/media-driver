/*===================== begin_copyright_notice ==================================

# Copyright (c) 2020-2021, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file     codechal_kernel_olp_mdf_xe_xpm.cpp
//! \brief    Implements the MDF OLP kernel for Xe_XPM VC1.
//! \details  Implements the MDF OLP kernel for Xe_XPM VC1.
//!
#include "codechal_kernel_olp_mdf_xe_xpm.h"
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
#include "Xe_XPM_VC1_OLP.h"
#endif

MOS_STATUS CodechalKernelOlpMdf::Init(PMOS_INTERFACE osInterface)
{
    CODECHAL_DECODE_FUNCTION_ENTER;
    CODECHAL_DECODE_CHK_NULL_RETURN(osInterface);
    m_osInterface = osInterface;
    if (m_cmDevice)
    {
        return MOS_STATUS_SUCCESS;
    }

    osInterface->pfnNotifyStreamIndexSharing(osInterface);

    uint32_t devCreateOption = CM_DEVICE_CREATE_OPTION_SCRATCH_SPACE_DISABLE;
    CODECHAL_DECODE_CHK_STATUS_RETURN(osInterface->pfnCreateCmDevice(
        osInterface->pOsContext,
        m_cmDevice,
        devCreateOption,
        CM_DEVICE_CREATE_PRIORITY_DEFAULT));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmDevice->CreateQueue(m_cmQueue));
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmDevice->LoadProgram(
        (void *)XE_HP_VC1_OLP,
        XE_HP_VC1_OLP_SIZE,
        m_cmProgram,
        "-nojitter"));
#endif
    for (int i = 0; i < 2; i++)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmDevice->CreateKernel(
            m_cmProgram,
            "VC1_OLP_NV12",
            m_cmKernels[i]));
    }
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmDevice->CreateTask(m_cmTask));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalKernelOlpMdf::SetupSurfaces(PMOS_SURFACE src, uint16_t *srcMemory_object_control, PMOS_SURFACE dst, uint16_t *dstMemory_object_control)
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(src);
    CODECHAL_DECODE_CHK_NULL_RETURN(dst);

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmDevice->UpdateSurface2D(&src->OsResource, m_cmSurfSrc));
    if (!m_srcYIndex)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmDevice->CreateSurface2DAlias(m_cmSurfSrc, m_srcYIndex));
    }
    if (!m_srcUVIndex)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmDevice->CreateSurface2DAlias(m_cmSurfSrc, m_srcUVIndex));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmDevice->UpdateSurface2D(&dst->OsResource, m_cmSurfDst));
    if (!m_dstYIndex)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmDevice->CreateSurface2DAlias(m_cmSurfDst, m_dstYIndex));
    }
    if (!m_dstUVIndex)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmDevice->CreateSurface2DAlias(m_cmSurfDst, m_dstUVIndex));
    }

    CM_SURFACE2D_STATE_PARAM srcSurfParams = {};
    CM_SURFACE2D_STATE_PARAM dstSurfParams  = {};
    srcSurfParams.format                                      = CM_SURFACE_FORMAT_R8_UNORM;
    srcSurfParams.memory_object_control                       = *srcMemory_object_control;
    dstSurfParams.format                                      = CM_SURFACE_FORMAT_R8_UNORM;
    dstSurfParams.memory_object_control                       = *dstMemory_object_control;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmSurfSrc->SetSurfaceStateParam(m_srcYIndex, &srcSurfParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmSurfDst->SetSurfaceStateParam(m_dstYIndex, &dstSurfParams));

    srcSurfParams.format = CM_SURFACE_FORMAT_R8G8_UNORM;
    srcSurfParams.width  = src->dwWidth / 2;
    srcSurfParams.height = src->dwHeight / 2;
    dstSurfParams.format = CM_SURFACE_FORMAT_R8G8_UNORM;
    dstSurfParams.width  = src->dwWidth / 2;
    dstSurfParams.height = src->dwHeight / 2;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmSurfSrc->SetSurfaceStateParam(m_srcUVIndex, &srcSurfParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmSurfDst->SetSurfaceStateParam(m_dstUVIndex, &dstSurfParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalKernelOlpMdf::SetKernelArgs(uint16_t flags, bool uv)
{
    CODECHAL_DECODE_FUNCTION_ENTER;
    uint32_t rsvd        = 0;
    uint16_t cmpFlags    = uv ? 16 : 0;
    uint16_t blockWidth  = 16;
    uint16_t blockHeight = 16;

    CmKernel *cmKernel = m_cmKernels[uv ? 1 : 0];
    CODECHAL_DECODE_CHK_STATUS_RETURN(cmKernel->SetKernelArg(0, sizeof(uint32_t), &rsvd));
    CODECHAL_DECODE_CHK_STATUS_RETURN(cmKernel->SetKernelArg(1, sizeof(uint16_t), &blockWidth));
    CODECHAL_DECODE_CHK_STATUS_RETURN(cmKernel->SetKernelArg(2, sizeof(uint16_t), &blockHeight));
    CODECHAL_DECODE_CHK_STATUS_RETURN(cmKernel->SetKernelArg(3, sizeof(uint16_t), &flags));
    CODECHAL_DECODE_CHK_STATUS_RETURN(cmKernel->SetKernelArg(4, sizeof(uint16_t), &cmpFlags));
    CODECHAL_DECODE_CHK_STATUS_RETURN(cmKernel->SetKernelArg(5, sizeof(uint16_t), &rsvd));
    if (uv)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(cmKernel->SetKernelArg(6, sizeof(SurfaceIndex), m_srcUVIndex));
        CODECHAL_DECODE_CHK_STATUS_RETURN(cmKernel->SetKernelArg(7, sizeof(SurfaceIndex), m_dstUVIndex));
    }
    else
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(cmKernel->SetKernelArg(6, sizeof(SurfaceIndex), m_srcYIndex));
        CODECHAL_DECODE_CHK_STATUS_RETURN(cmKernel->SetKernelArg(7, sizeof(SurfaceIndex), m_dstYIndex));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalKernelOlpMdf::Execute(PMOS_SURFACE src, uint16_t *srcMemory_object_control, PMOS_SURFACE dst, uint16_t *dstMemory_object_control, uint16_t flags)
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    CmEvent *event = CM_NO_EVENT;

    uint32_t threadWidth  = MOS_ALIGN_CEIL(src->dwWidth, 16) / 16;
    uint32_t threadHeight = MOS_ALIGN_CEIL(src->dwHeight, 16) / 16;

    CODECHAL_DECODE_CHK_STATUS_RETURN(SetupSurfaces(src, srcMemory_object_control, dst, dstMemory_object_control));

    CODECHAL_DECODE_CHK_STATUS_RETURN(SetKernelArgs(flags, false));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmDevice->CreateThreadGroupSpace(1, 1, threadWidth, threadHeight, m_threadGroupSpaces[0]));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmKernels[0]->AssociateThreadGroupSpace(m_threadGroupSpaces[0]));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmTask->AddKernel(m_cmKernels[0]));

    auto delete_event = [&]()
    {
        MOS_Delete(event);
    };

    if (!m_SingleTaskPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_WITH_DESTROY_RETURN(m_cmQueue->EnqueueWithGroup(m_cmTask, event), delete_event);
        CODECHAL_ENCODE_CHK_STATUS_WITH_DESTROY_RETURN(m_cmTask->Reset(), delete_event);
    }

    threadWidth  = MOS_ALIGN_CEIL(src->dwWidth, 16) / 16;
    threadHeight = MOS_ALIGN_CEIL(src->dwHeight / 2, 16) / 16;

    CODECHAL_DECODE_CHK_STATUS_RETURN(SetKernelArgs(flags, true));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmDevice->CreateThreadGroupSpace(1, 1, threadWidth, threadHeight, m_threadGroupSpaces[1]));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmKernels[1]->AssociateThreadGroupSpace(m_threadGroupSpaces[1]));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_cmTask->AddKernel(m_cmKernels[1]));
    CODECHAL_ENCODE_CHK_STATUS_WITH_DESTROY_RETURN(m_cmQueue->EnqueueWithGroup(m_cmTask, event), delete_event);
    CODECHAL_ENCODE_CHK_STATUS_WITH_DESTROY_RETURN(m_cmTask->Reset(), delete_event);


    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalKernelOlpMdf::UnInit()
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    if (m_cmSurfSrc)
    {
        m_cmDevice->DestroySurface(m_cmSurfSrc);
    }
    if (m_cmSurfDst)
    {
        m_cmDevice->DestroySurface(m_cmSurfDst);
    }

    for (int i = 0; i < 2; i++)
    {
        if (m_threadGroupSpaces[i])
        {
            m_cmDevice->DestroyThreadGroupSpace(m_threadGroupSpaces[i]);
            m_threadGroupSpaces[i] = nullptr;
        }
        if (m_cmKernels[i])
        {
            m_cmDevice->DestroyKernel(m_cmKernels[i]);
            m_cmKernels[i] = nullptr;
        }
    }
    if (m_cmTask)
    {
        m_cmDevice->DestroyTask(m_cmTask);
        m_cmTask = nullptr;
    }
    if (m_cmProgram)
    {
        m_cmDevice->DestroyProgram(m_cmProgram);
        m_cmProgram = nullptr;
    }
    if (m_osInterface != nullptr)
    {
        m_osInterface->pfnDestroyCmDevice(m_cmDevice);
    }

    return MOS_STATUS_SUCCESS;
}
