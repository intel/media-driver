/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     codechal_kernel_intra_dist.cpp
//! \brief    Defines the intra distortion kernel base
//! \details  Intra distortion base includes all common functions and definitions for intra distortion
//!
#include "codechal_kernel_intra_dist_mdf_g12.h"

CodechalKernelIntraDistMdfG12::CodechalKernelIntraDistMdfG12(CodechalEncoderState *encoder) :
    CodechalKernelBase(encoder)
{
}

CodechalKernelIntraDistMdfG12::~CodechalKernelIntraDistMdfG12()
{
    ReleaseResources();
}

MOS_STATUS CodechalKernelIntraDistMdfG12::AllocateResources()
{
    // no resource need to allocate
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalKernelIntraDistMdfG12::ReleaseResources()
{
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_encoder->m_cmDev);
    if (m_vmeIdx)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->DestroyVmeSurfaceG7_5(m_vmeIdx));
        m_vmeIdx = nullptr;
    }

    if (m_src4xSurface)
    {
        m_src4xSurface->NotifyUmdResourceChanged(nullptr);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->DestroySurface(m_src4xSurface));
        m_src4xSurface = nullptr;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalKernelIntraDistMdfG12::InitializeKernelIsa(void *kernelIsa, uint32_t kernelIsaSize)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_encoder->m_cmDev);

    if (!m_cmProgram)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->LoadProgram(kernelIsa,
            kernelIsaSize,
            m_cmProgram,
            "-nojitter"));
    }
    if (!m_cmKrn)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->CreateKernel(m_cmProgram,
            "Coarse_Intra",
            m_cmKrn));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalKernelIntraDistMdfG12::SetCurbe(MHW_KERNEL_STATE *kernelState)
{
    // Do nothing in the MDF path
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalKernelIntraDistMdfG12::SendSurfaces(PMOS_COMMAND_BUFFER cmd, MHW_KERNEL_STATE *kernelState)
{
    // Do nothing in the MDF path
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalKernelIntraDistMdfG12::AddPerfTag()
{
    PerfTagSetting perfTag;

    perfTag.Value             = 0;
    perfTag.Mode              = (uint16_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
    perfTag.CallType          = CODECHAL_ENCODE_PERFTAG_CALL_INTRA_DIST;
    perfTag.PictureCodingType = m_pictureCodingType > 3 ? 0 : m_pictureCodingType;
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);
    m_osInterface->pfnResetPerfBufferID(m_osInterface);

    return MOS_STATUS_SUCCESS;
}

MHW_KERNEL_STATE *CodechalKernelIntraDistMdfG12::GetActiveKernelState()
{
    // Do nothing in the MDF path
    return nullptr;
}

MOS_STATUS CodechalKernelIntraDistMdfG12::InitWalkerCodecParams(CODECHAL_WALKER_CODEC_PARAMS &walkerParam)
{
    // Do nothing in the MDF path
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalKernelIntraDistMdfG12::SetIntraDistCurbe(Curbe&  curbe)
{
    curbe.m_data.DW0.picWidthInLumaSamples = m_curbeParam.downScaledWidthInMb4x << 4;
    curbe.m_data.DW0.picHeightInLumaSamples = m_curbeParam.downScaledHeightInMb4x << 4;

    return MOS_STATUS_SUCCESS;
}

CODECHAL_MEDIA_STATE_TYPE CodechalKernelIntraDistMdfG12::GetMediaStateType()
{
    return CODECHAL_MEDIA_STATE_ENC_I_FRAME_DIST;
}

MOS_STATUS CodechalKernelIntraDistMdfG12::SetupSurfaces()
{
    CmDevice* &cmDev = m_encoder->m_cmDev;
    bool       currFieldPicture = CodecHal_PictureIsField(m_encoder->m_currOriginalPic);
    bool       currBottomField  = CodecHal_PictureIsBottomField(m_encoder->m_currOriginalPic) ? true : false;

    if (m_vmeIdx)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->DestroyVmeSurfaceG7_5(m_vmeIdx));
        m_vmeIdx = nullptr;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->UpdateSurface2D(
        &m_surfaceParam.input4xDsSurface->OsResource,
        m_src4xSurface));
    if (currFieldPicture)
    {
        m_src4xSurface->SetProperty(currBottomField ? CM_BOTTOM_FIELD : CM_TOP_FIELD);
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->CreateVmeSurfaceG7_5(
        m_src4xSurface,
        nullptr,
        nullptr,
        0,
        0,
        m_vmeIdx));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalKernelIntraDistMdfG12::SetupKernelArgs()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    int idx = 0;
    Curbe curbe;
    SurfaceIndex * pSurfIndex = nullptr;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetIntraDistCurbe(curbe));
    m_cmKrn->SetKernelArg(idx++, sizeof(curbe), &curbe);

    m_src4xSurface->GetIndex(pSurfIndex);
    m_cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex), pSurfIndex);

    m_surfaceParam.intraDistSurface->GetIndex(pSurfIndex);
    m_cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex), pSurfIndex);

    m_cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex), m_vmeIdx);

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpMDFCurbe(
            CODECHAL_MEDIA_STATE_ENC_I_FRAME_DIST,
            (uint8_t *)&curbe,
            sizeof(curbe)));)

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalKernelIntraDistMdfG12::Execute( CurbeParam &curbeParam, SurfaceParams &surfaceParam )
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_SecureMemcpy(&m_curbeParam, sizeof(m_curbeParam), &curbeParam, sizeof(m_curbeParam));
    MOS_SecureMemcpy(&m_surfaceParam, sizeof(m_surfaceParam), &surfaceParam, sizeof(m_surfaceParam));

    CmDevice* &cmDev = m_encoder->m_cmDev;

    SetupSurfaces();

    AddPerfTag();

    CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->CreateThreadSpace(
        m_curbeParam.downScaledWidthInMb4x,
        m_curbeParam.downScaledHeightInMb4x,
        m_threadSpace));

    uint32_t threadCount = m_curbeParam.downScaledWidthInMb4x * m_curbeParam.downScaledHeightInMb4x;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmKrn->SetThreadCount(threadCount));

    if (m_groupIdSelectSupported)
    {
        m_threadSpace->SetMediaWalkerGroupSelect((CM_MW_GROUP_SELECT)m_groupId);
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmKrn->AssociateThreadSpace(m_threadSpace));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupKernelArgs());
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmTask->AddKernel(m_cmKrn));

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CmEvent * event = CM_NO_EVENT;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmQueue->EnqueueFast(m_encoder->m_cmTask, event));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmTask->Reset());
        m_lastTaskInPhase = false;
    }
    else
    {
        m_encoder->m_cmTask->AddSync();
    }

    return MOS_STATUS_SUCCESS;
}
