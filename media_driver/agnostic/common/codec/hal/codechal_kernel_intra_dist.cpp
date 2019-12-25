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
//! \file     codechal_kernel_intra_dist.cpp
//! \brief    Defines the intra distortion kernel base
//! \details  Intra distortion base includes all common functions and definitions for intra distortion
//!
#include "codechal_kernel_intra_dist.h"

CodechalKernelIntraDist::CodechalKernelIntraDist(CodechalEncoderState *encoder) :
        CodechalKernelBase(encoder)
{

}

CodechalKernelIntraDist::~CodechalKernelIntraDist()
{
    ReleaseResources();
}

MOS_STATUS CodechalKernelIntraDist::AllocateResources()
{
    // no resource need to allocate
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalKernelIntraDist::ReleaseResources()
{
    // no resource need to release
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalKernelIntraDist::SetCurbe(MHW_KERNEL_STATE *kernelState)
{
    CODECHAL_ENCODE_CHK_NULL_RETURN(kernelState);

    Curbe curbe;

    curbe.m_data.DW0.picWidthInLumaSamples   = m_curbeParam.downScaledWidthInMb4x << 4;
    curbe.m_data.DW0.picHeightInLumaSamples  = m_curbeParam.downScaledHeightInMb4x << 4;
    curbe.m_data.DW8.currPic4xBTI            = BindingTableOffset::intraDistCurrent4xY;
    curbe.m_data.DW9.intraDistSurfaceBTI     = BindingTableOffset::intraDistOutputSurf;
    curbe.m_data.DW10.vmeIntraPredSurfaceBTI = BindingTableOffset::intraDistVmeIntraPred;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernelState->m_dshRegion.AddData(&curbe.m_data, kernelState->dwCurbeOffset, Curbe::m_curbeSize));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalKernelIntraDist::SendSurfaces(PMOS_COMMAND_BUFFER cmd, MHW_KERNEL_STATE *kernelState)
{
    CODECHAL_SURFACE_CODEC_PARAMS surfaceParams;
    // 4X DS Surface
    memset(&surfaceParams, 0, sizeof(surfaceParams));
    surfaceParams.bIs2DSurface          = true;
    surfaceParams.bIsWritable           = false;
    surfaceParams.bMediaBlockRW         = true;
    surfaceParams.psSurface             = m_surfaceParam.input4xDsSurface;
    surfaceParams.dwVerticalLineStride  = m_verticalLineStride;
    surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value;
    surfaceParams.dwBindingTableOffset  = BindingTableOffset::intraDistCurrent4xY;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmd,
        &surfaceParams,
        kernelState));

    //Intra Dist Surface
    memset(&surfaceParams, 0, sizeof(surfaceParams));
    surfaceParams.bIs2DSurface          = true;
    surfaceParams.bIsWritable           = true;
    surfaceParams.bMediaBlockRW         = true;
    surfaceParams.psSurface             = m_surfaceParam.intraDistSurface;
    surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_INTRA_DISTORTION_ENCODE].Value;
    surfaceParams.dwBindingTableOffset  = BindingTableOffset::intraDistOutputSurf;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmd,
        &surfaceParams,
        kernelState));

    // 4X DS VME Surface
    memset(&surfaceParams, 0, sizeof(surfaceParams));
    surfaceParams.bUseAdvState          = true;
    surfaceParams.psSurface             = m_surfaceParam.input4xDsVmeSurface;
    surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value;
    surfaceParams.dwBindingTableOffset  = BindingTableOffset::intraDistVmeIntraPred;
    surfaceParams.ucVDirection          = CODECHAL_VDIRECTION_FRAME;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmd,
        &surfaceParams,
        kernelState));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalKernelIntraDist::AddPerfTag()
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

MHW_KERNEL_STATE *CodechalKernelIntraDist::GetActiveKernelState()
{
    auto it = m_kernelStatePool.find(KernelIndex::intraDistortion);
    if (it != m_kernelStatePool.end())
    {
        return it->second;
    }
    MHW_KERNEL_STATE *kernelState = nullptr;
    CreateKernelState(&kernelState, KernelIndex::intraDistortion, ENC_INTRA_DISTORTION, 0);

    return kernelState;
}

MOS_STATUS CodechalKernelIntraDist::InitWalkerCodecParams(CODECHAL_WALKER_CODEC_PARAMS &walkerParam)
{
    walkerParam.WalkerMode              = m_walkerMode;
    walkerParam.dwResolutionX           = m_curbeParam.downScaledWidthInMb4x;
    walkerParam.dwResolutionY           = m_curbeParam.downScaledHeightInMb4x;
    walkerParam.bNoDependency           = true;

    return MOS_STATUS_SUCCESS;
}

CODECHAL_MEDIA_STATE_TYPE CodechalKernelIntraDist::GetMediaStateType()
{
    return CODECHAL_MEDIA_STATE_ENC_I_FRAME_DIST;
}

MOS_STATUS CodechalKernelIntraDist::Execute( CurbeParam &curbeParam, SurfaceParams &surfaceParam )
{
    MOS_SecureMemcpy(&m_curbeParam, sizeof(m_curbeParam), &curbeParam, sizeof(m_curbeParam));
    MOS_SecureMemcpy(&m_surfaceParam, sizeof(m_surfaceParam), &surfaceParam, sizeof(m_surfaceParam));

    return Run();
}
