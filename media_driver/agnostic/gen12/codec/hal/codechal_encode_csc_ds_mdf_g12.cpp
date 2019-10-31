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
//! \file     codechal_encode_csc_ds_mdf_g12.cpp
//! \brief    This file implements the Csc feature using MDF RT
//!


#include "codechal_encode_csc_ds_mdf_g12.h"
#include "Gen12LP_DS_Convert_genx.h"


MOS_STATUS CodechalEncodeCscDsMdfG12::SetCurbeCscforMDF(CMRT_UMD::vector<uint32_t, 10> & curbeData)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CscKernelCurbeData curbe;

    curbe.DW0_OutputBitDepthForChroma = m_curbeParams.ucEncBitDepthChroma;
    curbe.DW0_OutputBitDepthForLuma = m_curbeParams.ucEncBitDepthLuma;
    curbe.DW0_RoundingEnable = 1;

    curbe.DW1_PictureFormat = (uint8_t)((m_colorRawSurface == cscColorABGR) ? cscColorARGB : m_colorRawSurface); // Use cscColorARGB for ABGR CSC, just switch B and R coefficients
    curbe.DW1_ConvertFlag = m_curbeParams.bConvertFlag;
    curbe.DW1_DownscaleStage = (uint8_t)m_curbeParams.downscaleStage;
    curbe.DW1_MbStatisticsDumpFlag = (m_curbeParams.downscaleStage == dsStage4x || m_curbeParams.downscaleStage == dsStage2x4x);
    curbe.DW1_YUY2ConversionFlag = (m_colorRawSurface == cscColorYUY2) && m_cscRequireColor;
    curbe.DW1_HevcEncHistorySum = m_curbeParams.bHevcEncHistorySum;
    curbe.DW1_LCUSize = m_curbeParams.bUseLCU32;

    curbe.DW2_OriginalPicWidthInSamples = m_curbeParams.dwInputPictureWidth;
    curbe.DW2_OriginalPicHeightInSamples = m_curbeParams.dwInputPictureHeight;

    // RGB->YUV CSC coefficients
    if (m_curbeParams.inputColorSpace == ECOLORSPACE_P709)
    {
        curbe.DW4_CSC_Coefficient_C0 = 0xFFCD;
        curbe.DW5_CSC_Coefficient_C3 = 0x0080;
        curbe.DW6_CSC_Coefficient_C4 = 0x004F;
        curbe.DW7_CSC_Coefficient_C7 = 0x0010;
        curbe.DW8_CSC_Coefficient_C8 = 0xFFD5;
        curbe.DW9_CSC_Coefficient_C11 = 0x0080;
        if (cscColorARGB == m_colorRawSurface)
        {
            curbe.DW4_CSC_Coefficient_C1 = 0xFFFB;
            curbe.DW5_CSC_Coefficient_C2 = 0x0038;
            curbe.DW6_CSC_Coefficient_C5 = 0x0008;
            curbe.DW7_CSC_Coefficient_C6 = 0x0017;
            curbe.DW8_CSC_Coefficient_C9 = 0x0038;
            curbe.DW9_CSC_Coefficient_C10 = 0xFFF3;
        }
        else // cscColorABGR == m_colorRawSurface
        {
            curbe.DW4_CSC_Coefficient_C1 = 0x0038;
            curbe.DW5_CSC_Coefficient_C2 = 0xFFFB;
            curbe.DW6_CSC_Coefficient_C5 = 0x0017;
            curbe.DW7_CSC_Coefficient_C6 = 0x0008;
            curbe.DW8_CSC_Coefficient_C9 = 0xFFF3;
            curbe.DW9_CSC_Coefficient_C10 = 0x0038;
        }
    }
    else if (m_curbeParams.inputColorSpace == ECOLORSPACE_P601)
    {
        curbe.DW4_CSC_Coefficient_C0 = 0xFFD1;
        curbe.DW5_CSC_Coefficient_C3 = 0x0080;
        curbe.DW6_CSC_Coefficient_C4 = 0x0041;
        curbe.DW7_CSC_Coefficient_C7 = 0x0010;
        curbe.DW8_CSC_Coefficient_C8 = 0xFFDB;
        curbe.DW9_CSC_Coefficient_C11 = 0x0080;
        if (cscColorARGB == m_colorRawSurface)
        {
            curbe.DW4_CSC_Coefficient_C1 = 0xFFF7;
            curbe.DW5_CSC_Coefficient_C2 = 0x0038;
            curbe.DW6_CSC_Coefficient_C5 = 0x000D;
            curbe.DW7_CSC_Coefficient_C6 = 0x0021;
            curbe.DW8_CSC_Coefficient_C9 = 0x0038;
            curbe.DW9_CSC_Coefficient_C10 = 0xFFED;
        }
        else // cscColorABGR == m_colorRawSurface
        {
            curbe.DW4_CSC_Coefficient_C1 = 0x0038;
            curbe.DW5_CSC_Coefficient_C2 = 0xFFF7;
            curbe.DW6_CSC_Coefficient_C5 = 0x0021;
            curbe.DW7_CSC_Coefficient_C6 = 0x000D;
            curbe.DW8_CSC_Coefficient_C9 = 0xFFED;
            curbe.DW9_CSC_Coefficient_C10 = 0x0038;
        }
    }
    else
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ARGB input color space = %d!", m_curbeParams.inputColorSpace);
        return MOS_STATUS_INVALID_PARAMETER;
    }

    curbeData[0] = curbe.DW0;
    curbeData[1] = curbe.DW1;
    curbeData[2] = curbe.DW2;
    curbeData[3] = curbe.DW3_MBFlatnessThreshold;
    curbeData[4] = curbe.DW4;
    curbeData[5] = curbe.DW5;
    curbeData[6] = curbe.DW6;
    curbeData[7] = curbe.DW7;
    curbeData[8] = curbe.DW8;
    curbeData[9] = curbe.DW9;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeCscDsMdfG12::SetupKernelArgsCSC(CmKernel *cmKrnCSCDS, SurfaceParamsCscMdf* surfaceparams)
{
    int idx = 0;
    CMRT_UMD::vector<uint32_t, 10> curbeData;

    SetCurbeCscforMDF(curbeData);

    cmKrnCSCDS->SetKernelArg(idx++, curbeData.get_size_data(), curbeData.get_addr_data());

    SurfaceIndex * pSurfIndex = nullptr;

    CODECHAL_ENCODE_ASSERT(surfaceparams->psInputSurface != nullptr)

    surfaceparams->psInputSurface->GetIndex(pSurfIndex);
    cmKrnCSCDS->SetKernelArg(idx++, sizeof(SurfaceIndex), pSurfIndex);

    if (surfaceparams->psOutputCopiedSurface)
    {
        surfaceparams->psOutputCopiedSurface->GetIndex(pSurfIndex);
    }
    cmKrnCSCDS->SetKernelArg(idx++, sizeof(SurfaceIndex), pSurfIndex);

    if (surfaceparams->psOutput4xDsSurface)
    {
        surfaceparams->psOutput4xDsSurface->GetIndex(pSurfIndex);
    }
    cmKrnCSCDS->SetKernelArg(idx++, sizeof(SurfaceIndex), pSurfIndex);

    if (surfaceparams->presMBVProcStatsBuffer)
    {
        surfaceparams->presMBVProcStatsBuffer->GetIndex(pSurfIndex);
    }
    cmKrnCSCDS->SetKernelArg(idx++, sizeof(SurfaceIndex), pSurfIndex);

    if (surfaceparams->psOutput2xDsSurface)
    {
        surfaceparams->psOutput2xDsSurface->GetIndex(pSurfIndex);
    }

    cmKrnCSCDS->SetKernelArg(idx++, sizeof(SurfaceIndex), pSurfIndex);

    if (m_surfaceParamsCsc.hevcExtParams)
    {
        cmKrnCSCDS->SetKernelArg(idx++, sizeof(SurfaceIndex), surfaceparams->pHistBufSurfIdx);
        cmKrnCSCDS->SetKernelArg(idx++, sizeof(SurfaceIndex), surfaceparams->pHistSumBufSurfIdx);
        cmKrnCSCDS->SetKernelArg(idx++, sizeof(SurfaceIndex), surfaceparams->pMTTaskSumBufSurfIdx);
    }
    else
    {
        cmKrnCSCDS->SetKernelArg(idx++, sizeof(SurfaceIndex), pSurfIndex);
        cmKrnCSCDS->SetKernelArg(idx++, sizeof(SurfaceIndex), pSurfIndex);
        cmKrnCSCDS->SetKernelArg(idx++, sizeof(SurfaceIndex), pSurfIndex);
    }

    return  MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeCscDsMdfG12::DestroySurfaces(SurfaceParamsCscMdf& SurfaceParamsCsc)
{
    CmDevice* &cmDev = m_encoder->m_cmDev;
    // Input YUV
    if (SurfaceParamsCsc.psInputSurface)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->DestroySurface(SurfaceParamsCsc.psInputSurface));
        SurfaceParamsCsc.psInputSurface = nullptr;
    }
    if (SurfaceParamsCsc.psOutputCopiedSurface)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->DestroySurface(SurfaceParamsCsc.psOutputCopiedSurface));
        SurfaceParamsCsc.psOutputCopiedSurface = nullptr;
    }
    if (SurfaceParamsCsc.psOutput4xDsSurface)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->DestroySurface(SurfaceParamsCsc.psOutput4xDsSurface));
        SurfaceParamsCsc.psOutput4xDsSurface = nullptr;
    }
    if (SurfaceParamsCsc.psOutput2xDsSurface)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->DestroySurface(SurfaceParamsCsc.psOutput2xDsSurface));
        SurfaceParamsCsc.psOutput2xDsSurface = nullptr;
    }
    if (SurfaceParamsCsc.presMBVProcStatsBuffer)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->DestroySurface(SurfaceParamsCsc.presMBVProcStatsBuffer));
        SurfaceParamsCsc.presMBVProcStatsBuffer = nullptr;
    }
    if (SurfaceParamsCsc.presHistoryBuffer)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->DestroySurface(SurfaceParamsCsc.presHistoryBuffer));
        SurfaceParamsCsc.presHistoryBuffer = nullptr;
    }
    if (SurfaceParamsCsc.presHistorySumBuffer)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->DestroySurface(SurfaceParamsCsc.presHistorySumBuffer));
        SurfaceParamsCsc.presHistorySumBuffer = nullptr;
    }
    if (SurfaceParamsCsc.presMultiThreadTaskBuffer)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->DestroySurface(SurfaceParamsCsc.presMultiThreadTaskBuffer));
        SurfaceParamsCsc.presMultiThreadTaskBuffer = nullptr;
    }
    return  MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeCscDsMdfG12::ReleaseResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    CmDevice* &cmDev = m_encoder->m_cmDev;
    DestroySurfaces(m_cmSurfParamsCscDs4x);
    DestroySurfaces(m_cmSurfParamsCscDs16x);
    DestroySurfaces(m_cmSurfParamsCscDs32x);

    if (m_cmKrnCSCDS4x)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->DestroyKernel(m_cmKrnCSCDS4x));
        m_cmKrnCSCDS4x = nullptr;
    }

    if (m_cmKrnCSCDS16x)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->DestroyKernel(m_cmKrnCSCDS16x));
        m_cmKrnCSCDS16x = nullptr;
    }

    if (m_cmKrnCSCDS32x)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->DestroyKernel(m_cmKrnCSCDS32x));
        m_cmKrnCSCDS32x = nullptr;
    }

    if (m_cmProgramCSCDS)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->DestroyProgram(m_cmProgramCSCDS));
        m_cmProgramCSCDS = nullptr;
    }
    if (m_threadSpace4x)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->DestroyThreadSpace(m_threadSpace4x));
        m_threadSpace4x = nullptr;
    }

    if (m_threadSpace16x)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->DestroyThreadSpace(m_threadSpace16x));
        m_threadSpace16x = nullptr;
    }

    if (m_threadSpace32x)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->DestroyThreadSpace(m_threadSpace32x));
        m_threadSpace32x = nullptr;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeCscDsMdfG12::SetupSurfacesCSC(SurfaceParamsCscMdf& SurfaceParamsCsc)
{
    CmDevice* &cmDev = m_encoder->m_cmDev;
    // Input YUV
    CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->UpdateSurface2D(
        &m_surfaceParamsCsc.psInputSurface->OsResource,
        SurfaceParamsCsc.psInputSurface));

    // Change input surface interpretation from NV12 to A8
    if (GMM_FORMAT_NV12_TYPE == m_surfaceParamsCsc.psInputSurface->OsResource.pGmmResInfo->GetResourceFormat())
    {
        uint32_t   width = 0, height = 0;
        MOS_FORMAT format = Format_Invalid;
        m_encoder->m_cscDsState->GetCscAllocation(width, height, format);

        CM_SURFACE2D_STATE_PARAM param;
        MOS_ZeroMemory(&param, sizeof(param));
        param.width  = width;
        param.height = (height * 3) / 2;
        param.format = CM_SURFACE_FORMAT_A8;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SurfaceParamsCsc.psInputSurface->SetSurfaceStateParam(nullptr, &param));
    }

    // Output to Enc
    if (m_surfaceParamsCsc.psOutputCopiedSurface)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->UpdateSurface2D(
            &m_surfaceParamsCsc.psOutputCopiedSurface->OsResource,
            SurfaceParamsCsc.psOutputCopiedSurface));
    }

    if (m_surfaceParamsCsc.psOutput4xDsSurface)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->UpdateSurface2D(
            &m_surfaceParamsCsc.psOutput4xDsSurface->OsResource,
            SurfaceParamsCsc.psOutput4xDsSurface));
    }

    if (m_surfaceParamsCsc.psOutput2xDsSurface)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->UpdateSurface2D(
            &m_surfaceParamsCsc.psOutput2xDsSurface->OsResource,
            SurfaceParamsCsc.psOutput2xDsSurface));
    }

    if (m_surfaceParamsCsc.presMBVProcStatsBuffer)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->UpdateBuffer(
            m_surfaceParamsCsc.presMBVProcStatsBuffer,
            SurfaceParamsCsc.presMBVProcStatsBuffer));
    }

    if (m_surfaceParamsCsc.hevcExtParams)
    {
        auto hevcExtParams = (HevcExtKernelParams*)m_surfaceParamsCsc.hevcExtParams;
        CM_BUFFER_STATE_PARAM bufParams;

        if (SurfaceParamsCsc.presHistoryBuffer)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->DestroySurface(SurfaceParamsCsc.presHistoryBuffer));
            SurfaceParamsCsc.presHistoryBuffer = nullptr;
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->CreateBuffer(
            hevcExtParams->presHistoryBuffer,
            SurfaceParamsCsc.presHistoryBuffer));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->CreateBufferAlias(
            SurfaceParamsCsc.presHistoryBuffer,
            SurfaceParamsCsc.pHistBufSurfIdx));
        bufParams.uiBaseAddressOffset = hevcExtParams->dwOffsetHistoryBuffer;
        bufParams.uiSize              = hevcExtParams->dwSizeHistoryBuffer;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SurfaceParamsCsc.presHistoryBuffer->SetSurfaceStateParam(
            SurfaceParamsCsc.pHistBufSurfIdx,
            &bufParams));

        if (SurfaceParamsCsc.presHistorySumBuffer)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->DestroySurface(SurfaceParamsCsc.presHistorySumBuffer));
            SurfaceParamsCsc.presHistorySumBuffer = nullptr;
        }
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->CreateBuffer(
            hevcExtParams->presHistorySumBuffer,
            SurfaceParamsCsc.presHistorySumBuffer));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->CreateBufferAlias(
            SurfaceParamsCsc.presHistorySumBuffer,
            SurfaceParamsCsc.pHistSumBufSurfIdx));
        bufParams.uiBaseAddressOffset = hevcExtParams->dwOffsetHistorySumBuffer;
        bufParams.uiSize = hevcExtParams->dwSizeHistorySumBuffer;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SurfaceParamsCsc.presHistorySumBuffer->SetSurfaceStateParam(
            SurfaceParamsCsc.pHistSumBufSurfIdx,
            &bufParams));

        if (SurfaceParamsCsc.presMultiThreadTaskBuffer)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->DestroySurface(SurfaceParamsCsc.presMultiThreadTaskBuffer));
            SurfaceParamsCsc.presMultiThreadTaskBuffer = nullptr;
        }
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->CreateBuffer(
            hevcExtParams->presMultiThreadTaskBuffer,
            SurfaceParamsCsc.presMultiThreadTaskBuffer));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->CreateBufferAlias(
            SurfaceParamsCsc.presMultiThreadTaskBuffer,
            SurfaceParamsCsc.pMTTaskSumBufSurfIdx));
        bufParams.uiBaseAddressOffset = hevcExtParams->dwOffsetMultiThreadTaskBuffer;
        bufParams.uiSize = hevcExtParams->dwSizeMultiThreadTaskBuffer;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SurfaceParamsCsc.presMultiThreadTaskBuffer->SetSurfaceStateParam(
            SurfaceParamsCsc.pMTTaskSumBufSurfIdx,
            &bufParams));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeCscDsMdfG12::InitKernelStateCsc(KernelParams* pParams)
{

    CODECHAL_ENCODE_FUNCTION_ENTER;
    CmDevice* &cmDev = m_encoder->m_cmDev;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->LoadProgram((void *)GEN12LP_DS_CONVERT_GENX,
        GEN12LP_DS_CONVERT_GENX_SIZE,
        m_cmProgramCSCDS,
        "-nojitter"));

    if (!m_cmKrnCSCDS4x)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->CreateKernel(m_cmProgramCSCDS,
            "DS_Convert",
            m_cmKrnCSCDS4x));
    }

    if (m_16xMeSupported && !m_cmKrnCSCDS16x)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->CreateKernel(m_cmProgramCSCDS,
            "DS_Convert",
            m_cmKrnCSCDS16x));
    }

    if (m_32xMeSupported && !m_cmKrnCSCDS32x)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->CreateKernel(m_cmProgramCSCDS,
            "DS_Convert",
            m_cmKrnCSCDS32x));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeCscDsMdfG12::CscKernel(
    KernelParams* pParams)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams);

    if (!m_cscKernelState)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_cscKernelState = MOS_New(MHW_KERNEL_STATE));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateCsc(pParams));
    }

    // allocate CSC surface (existing surfaces will be re-used when associated frame goes out of RefList)
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateSurfaceCsc());

    if (m_scalingEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_trackedBuf->AllocateSurfaceDS());
    }

    if (m_2xScalingEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_trackedBuf->AllocateSurface2xDS());
    }

    if (m_encoder->m_trackedBuf->GetWaitCsc())
    {
        // in case PAK hasn't yet consumed a surface sent earlier, wait before we can re-use
        CODECHAL_ENCODE_CHK_STATUS_RETURN(WaitCscSurface(m_renderContext, false));
    }
    // setup kernel params
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetKernelParamsCsc(pParams));

    PerfTagSetting perfTag;
    perfTag.Value = 0;
    perfTag.Mode = (uint16_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
    perfTag.CallType = CODECHAL_ENCODE_PERFTAG_CALL_DS_CONVERSION_KERNEL;
    perfTag.PictureCodingType = m_encoder->m_pictureCodingType;
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);
    // Each scaling kernel buffer counts as a separate perf task
    m_osInterface->pfnResetPerfBufferID(m_osInterface);


    uint32_t threadCount = m_walkerResolutionX * m_walkerResolutionY;
    CmThreadSpace          *threadSpace = nullptr;
    CmKernel               *cmKrnCSCDS = nullptr;
    SurfaceParamsCscMdf     *surfaceparams = nullptr;

    if (pParams->stageDsConversion == dsStage16x)
    {
        // MDf surface states
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupSurfacesCSC(m_cmSurfParamsCscDs16x));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmKrnCSCDS16x->SetThreadCount(threadCount));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->CreateThreadSpace(
            m_walkerResolutionX,
            m_walkerResolutionY,
            m_threadSpace16x));
         threadSpace = m_threadSpace16x;
         cmKrnCSCDS = m_cmKrnCSCDS16x;
         surfaceparams = &m_cmSurfParamsCscDs16x;
    }
    else if(pParams->stageDsConversion == dsStage32x)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupSurfacesCSC(m_cmSurfParamsCscDs32x));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmKrnCSCDS32x->SetThreadCount(threadCount));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->CreateThreadSpace(
            m_walkerResolutionX,
            m_walkerResolutionY,
            m_threadSpace32x));
        threadSpace = m_threadSpace32x;
        cmKrnCSCDS = m_cmKrnCSCDS32x;
        surfaceparams = &m_cmSurfParamsCscDs32x;
    }
    else
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupSurfacesCSC(m_cmSurfParamsCscDs4x));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmKrnCSCDS4x->SetThreadCount(threadCount));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->CreateThreadSpace(
            m_walkerResolutionX,
            m_walkerResolutionY,
            m_threadSpace4x));
           threadSpace = m_threadSpace4x;
           cmKrnCSCDS = m_cmKrnCSCDS4x;
           surfaceparams = &m_cmSurfParamsCscDs4x;
    }

    if (m_groupIdSelectSupported)
    {
        threadSpace->SetMediaWalkerGroupSelect((CM_MW_GROUP_SELECT)m_groupId);
    }
    CODECHAL_ENCODE_CHK_STATUS_RETURN(cmKrnCSCDS->AssociateThreadSpace(threadSpace));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupKernelArgsCSC(cmKrnCSCDS,surfaceparams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmTask->AddKernel(cmKrnCSCDS));

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

    if (dsDisabled == pParams->stageDsConversion)
    {
        // send appropriate surface to Enc/Pak depending on different CSC operation type
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesToEncPak());
    }

    return MOS_STATUS_SUCCESS;
}

CodechalEncodeCscDsMdfG12::CodechalEncodeCscDsMdfG12(CodechalEncoderState* encoder)
    : CodechalEncodeCscDsG12(encoder)
{
}

CodechalEncodeCscDsMdfG12::~CodechalEncodeCscDsMdfG12()
{
    ReleaseResources();
}
