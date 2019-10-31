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
//! \file     codechal_kernel_hme_g12.cpp
//! \brief    Hme kernel implementation using MDF RT for Gen12 platform
//!
#include "codechal_kernel_hme_mdf_g12.h"
#include "Gen12LP_hme_genx.h"

// clang-format off
const uint32_t CodechalKernelHmeMdfG12::Curbe::m_initCurbe[40] =
{
    0x00000000, 0x00200010, 0x00003939, 0x77a43000, 0x00000000, 0x28300000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000200,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
};
// clang-format on

CodechalKernelHmeMdfG12::CodechalKernelHmeMdfG12(
    CodechalEncoderState *encoder,
    bool     me4xDistBufferSupported)
        : CodechalKernelHme(encoder, me4xDistBufferSupported)
{
}

CodechalKernelHmeMdfG12::~CodechalKernelHmeMdfG12()
{
}

MOS_STATUS CodechalKernelHmeMdfG12::ReleaseResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    DestroyYUVSurfaces(m_HME4xYUVInfo);
    DestroyYUVSurfaces(m_HME16xYUVInfo);
    DestroyYUVSurfaces(m_HME32xYUVInfo);

    CmDevice* &cmDev = m_encoder->m_cmDev;

    if (m_HME4xDistortionSurface)
    {
        cmDev->DestroySurface(m_HME4xDistortionSurface);
        m_HME4xDistortionSurface = nullptr;
    }

    if (m_HME4xMVSurface)
    {
        cmDev->DestroySurface(m_HME4xMVSurface);
        m_HME4xMVSurface = nullptr;
    }

    if (m_HME16xMVSurface)
    {
        cmDev->DestroySurface(m_HME16xMVSurface);
        m_HME16xMVSurface = nullptr;
    }

    if (m_HME32xMVSurface)
    {
        cmDev->DestroySurface(m_HME32xMVSurface);
        m_HME32xMVSurface = nullptr;
    }

    if (m_VdencStreamInBuffer)
    {
        cmDev->DestroySurface(m_VdencStreamInBuffer);
        m_VdencStreamInBuffer = nullptr;
    }

    if (m_SumMvandDistortionBuffer)
    {
        cmDev->DestroySurface(m_SumMvandDistortionBuffer);
        m_SumMvandDistortionBuffer = nullptr;
    }

    if (m_threadSpace4x)
    {
        cmDev->DestroyThreadSpace(m_threadSpace4x);
        m_threadSpace4x = nullptr;
    }

    if (m_threadSpace16x)
    {
        cmDev->DestroyThreadSpace(m_threadSpace16x);
        m_threadSpace16x = nullptr;
    }

    if (m_threadSpace32x)
    {
        cmDev->DestroyThreadSpace(m_threadSpace32x);
        m_threadSpace32x = nullptr;
    }

    if (m_cmKrnME4xP)
    {
        (cmDev->DestroyKernel(m_cmKrnME4xP));
        m_cmKrnME4xP = nullptr;
    }

    if (m_cmKrnME16xP)
    {
        (cmDev->DestroyKernel(m_cmKrnME16xP));
        m_cmKrnME16xP = nullptr;
    }

    if (m_cmKrnME32xP)
    {
        (cmDev->DestroyKernel(m_cmKrnME32xP));
        m_cmKrnME32xP = nullptr;
    }

    if (m_cmKrnME4xB)
    {
        (cmDev->DestroyKernel(m_cmKrnME4xB));
        m_cmKrnME4xB = nullptr;
    }

    if (m_cmKrnME16xB)
    {
        (cmDev->DestroyKernel(m_cmKrnME16xB));
        m_cmKrnME16xB = nullptr;
    }

    if (m_cmKrnME32xB)
    {
        (cmDev->DestroyKernel(m_cmKrnME32xB));
        m_cmKrnME32xB = nullptr;
    }

    if (m_cmProgramME)
    {
        (cmDev->DestroyProgram(m_cmProgramME));
        m_cmProgramME = nullptr;
    }

    return MOS_STATUS_SUCCESS;
}
MOS_STATUS CodechalKernelHmeMdfG12::DestroyYUVSurfaces(HmeYUVInfo& YUVInfo)
{
    CmDevice* &cmDev = m_encoder->m_cmDev;
    if (YUVInfo.SrcSurface)
    {
        YUVInfo.SrcSurface->NotifyUmdResourceChanged(nullptr);
        cmDev->DestroySurface(YUVInfo.SrcSurface);
        YUVInfo.SrcSurface = nullptr;
    }

    for (uint8_t i = 0; i < MAX_HME_BWD_REF; i++)
    {
        if (YUVInfo.BwdReference[i])
        {
            YUVInfo.BwdReference[i]->NotifyUmdResourceChanged(nullptr);
            cmDev->DestroySurface(YUVInfo.BwdReference[i]);
            YUVInfo.BwdReference[i] = nullptr;
        }
    }

    for (uint8_t i = 0; i < MAX_HME_FWD_REF; i++)
    {
        if (YUVInfo.FwdReference[i])
        {
            YUVInfo.FwdReference[i]->NotifyUmdResourceChanged(nullptr);
            cmDev->DestroySurface(YUVInfo.FwdReference[i]);
            YUVInfo.FwdReference[i] = nullptr;
        }
    }

    return MOS_STATUS_SUCCESS;
}


MOS_STATUS CodechalKernelHmeMdfG12::Execute(CurbeParam &curbeParam, SurfaceParams &surfaceParam, HmeLevel hmeLevel)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_4xMeInUse = Is4xMeEnabled() ? (hmeLevel & HmeLevel::hmeLevel4x) != 0 : false;
    m_16xMeInUse = Is16xMeEnabled() ? (hmeLevel & HmeLevel::hmeLevel16x) != 0 : false;
    m_32xMeInUse = Is32xMeEnabled() ? (hmeLevel & HmeLevel::hmeLevel32x) != 0 : false;

    CmDevice* &cmDev = m_encoder->m_cmDev;

    MOS_SecureMemcpy(&m_curbeParam, sizeof(m_curbeParam), &curbeParam, sizeof(m_curbeParam));
    MOS_SecureMemcpy(&m_surfaceParam, sizeof(m_surfaceParam), &surfaceParam, sizeof(m_surfaceParam));

    InitKernelState((void *)GEN12LP_HME_GENX, GEN12LP_HME_GENX_SIZE);

    SetupSurfaces();

    AddPerfTag();

    uint32_t scalingFactor = m_32xMeInUse ? scalingFactor32X : m_16xMeInUse ? scalingFactor16X : scalingFactor4X;
    uint32_t xResolution = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth / scalingFactor);
    uint32_t yResolution = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameHeight / scalingFactor);

    CmThreadSpace          *threadSpace = nullptr;
    CmKernel               *cmKrn = nullptr;

    uint32_t threadCount = xResolution * yResolution;

    if (m_16xMeInUse)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->CreateThreadSpace(
            xResolution,
            yResolution,
            m_threadSpace16x));
        threadSpace = m_threadSpace16x;
        cmKrn = m_cmKrnME16x;
    }
    else if (m_32xMeInUse)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->CreateThreadSpace(
            xResolution,
            yResolution,
            m_threadSpace32x));
        threadSpace = m_threadSpace32x;
        cmKrn = m_cmKrnME32x;
    }
    else
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->CreateThreadSpace(
            xResolution,
            yResolution,
            m_threadSpace4x));
        threadSpace = m_threadSpace4x;
        cmKrn = m_cmKrnME4x;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(cmKrn->SetThreadCount(threadCount));

    if (m_groupIdSelectSupported)
    {
        threadSpace->SetMediaWalkerGroupSelect((CM_MW_GROUP_SELECT)m_groupId);
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(cmKrn->AssociateThreadSpace(threadSpace));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupKernelArgs(cmKrn));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmTask->AddKernel(cmKrn));

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

CmSurface2D* CodechalKernelHmeMdfG12::GetCmSurface(uint32_t surfaceId)
{
    switch (surfaceId)
    {
        case SurfaceId::me4xMvDataBuffer:
            return m_HME4xMVSurface;
            break;
        case SurfaceId::me16xMvDataBuffer:
            return m_HME16xMVSurface;
            break;
        case SurfaceId::me32xMvDataBuffer:
            return m_HME32xMVSurface;
            break;
        case SurfaceId::me4xDistortionBuffer:
            return m_HME4xDistortionSurface;
            break;
     };
    return nullptr;
}

MOS_STATUS CodechalKernelHmeMdfG12::AllocateResources()
{
    MOS_ALLOC_GFXRES_PARAMS allocParamsForBuffer2D;
    PMOS_SURFACE allocSurface = nullptr;
    CmDevice* &cmDev = m_encoder->m_cmDev;

    if (m_4xMeSupported)
    {
        if (!m_HME4xMVSurface)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->CreateSurface2D(
                MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 32), 64),  // MediaBlockRW requires pitch multiple of 64 bytes when linear.,
                (m_downscaledHeightInMb4x * 2 * 4 * CODECHAL_ENCODE_ME_DATA_SIZE_MULTIPLIER),
                CM_SURFACE_FORMAT_A8,
                m_HME4xMVSurface));
        }

        if (m_4xMeDistortionBufferSupported)
        {
            uint32_t ajustedHeight =
                m_downscaledHeightInMb4x * CODECHAL_MACROBLOCK_HEIGHT * SCALE_FACTOR_4x;
            uint32_t downscaledFieldHeightInMB4x =
                CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(((ajustedHeight + 1) >> 1) / 4);
            if (!m_HME4xDistortionSurface)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->CreateSurface2D(
                    MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 8), 64),
                    (2 * MOS_ALIGN_CEIL((downscaledFieldHeightInMB4x * 4 * 10), 8)),
                    CM_SURFACE_FORMAT_A8,
                    m_HME4xDistortionSurface));
            }
        }
    }

    if (m_16xMeSupported)
    {
        if (!m_HME16xMVSurface)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->CreateSurface2D(
                MOS_ALIGN_CEIL((m_downscaledWidthInMb16x * 32), 64),  // MediaBlockRW requires pitch multiple of 64 bytes when linear,
                (m_downscaledHeightInMb16x * 2 * 4 * CODECHAL_ENCODE_ME_DATA_SIZE_MULTIPLIER),
                CM_SURFACE_FORMAT_A8,
                m_HME16xMVSurface));
        }
    }

    if (m_32xMeSupported)
    {
        if (!m_HME32xMVSurface)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->CreateSurface2D(
                MOS_ALIGN_CEIL((m_downscaledWidthInMb32x * 32), 64),  // MediaBlockRW requires pitch multiple of 64 bytes when linear
                (m_downscaledHeightInMb32x * 2 * 4 * CODECHAL_ENCODE_ME_DATA_SIZE_MULTIPLIER),
                CM_SURFACE_FORMAT_A8,
                m_HME32xMVSurface));
        }
    }
    return MOS_STATUS_SUCCESS;
}


MOS_STATUS CodechalKernelHmeMdfG12::InitKernelState(void *kernelIsa, uint32_t kernelIsaSize)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (!m_cmProgramME)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->LoadProgram(kernelIsa,
            kernelIsaSize,
            m_cmProgramME,
            "-nojitter"));

        if (m_vdencEnabled)
        {
            if (m_standard == CODECHAL_AVC)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->CreateKernel(m_cmProgramME, "HME_VDENC_STREAMIN", m_cmKrnME4xP));
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->CreateKernel(m_cmProgramME, "HME_VDENC_STREAMIN", m_cmKrnME4xB));
            }
            else
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->CreateKernel(m_cmProgramME, "HME_VDENC_STREAMIN_HEVC", m_cmKrnME4xP));
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->CreateKernel(m_cmProgramME, "HME_VDENC_STREAMIN_HEVC", m_cmKrnME4xB));
            }
        }
        else 
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->CreateKernel(m_cmProgramME, "HME_P", m_cmKrnME4xP));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->CreateKernel(m_cmProgramME, "HME_B", m_cmKrnME4xB));
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->CreateKernel(m_cmProgramME, "HME_B", m_cmKrnME16xB));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->CreateKernel(m_cmProgramME, "HME_B", m_cmKrnME32xB));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->CreateKernel(m_cmProgramME, "HME_P", m_cmKrnME16xP));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->CreateKernel(m_cmProgramME, "HME_P", m_cmKrnME32xP));
    }

    if ((m_pictureCodingType == B_TYPE) && (!m_noMEKernelForPFrame))
    {
        m_cmKrnME4x = m_cmKrnME4xB;
        m_cmKrnME16x = m_cmKrnME16xB;
        m_cmKrnME32x = m_cmKrnME32xB;
    }
    else
    {
        m_cmKrnME4x = m_cmKrnME4xP;
        m_cmKrnME16x = m_cmKrnME16xP;
        m_cmKrnME32x = m_cmKrnME32xP;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalKernelHmeMdfG12::SetMECurbe(Curbe&  curbe)
{

    uint32_t  mvShiftFactor       = 0;
    uint32_t  prevMvReadPosFactor = 0;
    uint32_t  scaleFactor;
    bool      useMvFromPrevStep;
    bool      writeDistortions;

    if (m_32xMeInUse)
    {
        useMvFromPrevStep   = false;
        writeDistortions    = false;
        scaleFactor         = scalingFactor32X;
        mvShiftFactor       = 1;
        prevMvReadPosFactor = 0;
    }
    else if (m_16xMeInUse)
    {
        useMvFromPrevStep   = Is32xMeEnabled() ? true : false;
        writeDistortions    = false;
        scaleFactor         = scalingFactor16X;
        mvShiftFactor       = 2;
        prevMvReadPosFactor = 1;
    }
    else if (m_4xMeInUse)
    {
        useMvFromPrevStep   = Is16xMeEnabled() ? true : false;
        writeDistortions    = true;
        scaleFactor         = scalingFactor4X;
        mvShiftFactor       = 2;
        prevMvReadPosFactor = 0;
    }
    else
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    curbe.m_data.DW3.SubPelMode = m_curbeParam.subPelMode;

    if (m_fieldScalingOutputInterleaved)
    {
        curbe.m_data.DW3.SrcAccess = curbe.m_data.DW3.RefAccess = CodecHal_PictureIsField(m_curbeParam.currOriginalPic);
        curbe.m_data.DW7.SrcFieldPolarity                = CodecHal_PictureIsBottomField(m_curbeParam.currOriginalPic);
    }
    curbe.m_data.DW4.PictureHeightMinus1 = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameFieldHeight / scaleFactor) - 1;
    curbe.m_data.DW4.PictureWidth        = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameWidth / scaleFactor);
    curbe.m_data.DW5.QpPrimeY            = m_curbeParam.qpPrimeY;
    curbe.m_data.DW6.WriteDistortions    = writeDistortions;
    curbe.m_data.DW6.UseMvFromPrevStep   = useMvFromPrevStep;
    curbe.m_data.DW6.SuperCombineDist    = SuperCombineDist[m_curbeParam.targetUsage];
    curbe.m_data.DW6.MaxVmvR = CodecHal_PictureIsFrame(m_curbeParam.currOriginalPic) ? m_curbeParam.maxMvLen * 4 : (m_curbeParam.maxMvLen >> 1) * 4;

    if (m_pictureCodingType == B_TYPE)
    {
        curbe.m_data.DW1.BiWeight             = 32;
        curbe.m_data.DW13.NumRefIdxL1MinusOne = m_curbeParam.numRefIdxL1Minus1;
    }

    if (m_pictureCodingType == B_TYPE || m_pictureCodingType == P_TYPE)
    {
        if (m_vdencEnabled && Is16xMeEnabled())
        {
            curbe.m_data.DW30.ActualMBHeight = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameFieldHeight);
            curbe.m_data.DW30.ActualMBWidth = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameWidth);
        }
        curbe.m_data.DW13.NumRefIdxL0MinusOne = m_curbeParam.numRefIdxL0Minus1;
    }

    curbe.m_data.DW13.RefStreaminCost = 5;
    // This flag is to indicate the ROI source type instead of indicating ROI is enabled or not
    curbe.m_data.DW13.ROIEnable = 0;

    if (!CodecHal_PictureIsFrame(m_curbeParam.currOriginalPic))
    {
        if (m_pictureCodingType != I_TYPE)
        {
            curbe.m_data.DW14.List0RefID0FieldParity = m_curbeParam.list0RefID0FieldParity;
            curbe.m_data.DW14.List0RefID1FieldParity = m_curbeParam.list0RefID1FieldParity;
            curbe.m_data.DW14.List0RefID2FieldParity = m_curbeParam.list0RefID2FieldParity;
            curbe.m_data.DW14.List0RefID3FieldParity = m_curbeParam.list0RefID3FieldParity;
            curbe.m_data.DW14.List0RefID4FieldParity = m_curbeParam.list0RefID4FieldParity;
            curbe.m_data.DW14.List0RefID5FieldParity = m_curbeParam.list0RefID5FieldParity;
            curbe.m_data.DW14.List0RefID6FieldParity = m_curbeParam.list0RefID6FieldParity;
            curbe.m_data.DW14.List0RefID7FieldParity = m_curbeParam.list0RefID7FieldParity;
        }
        if (m_pictureCodingType == B_TYPE)
        {
            curbe.m_data.DW14.List1RefID0FieldParity = m_curbeParam.list1RefID0FieldParity;
            curbe.m_data.DW14.List1RefID1FieldParity = m_curbeParam.list1RefID1FieldParity;
        }
    }
    curbe.m_data.DW15.MvShiftFactor       = mvShiftFactor;
    curbe.m_data.DW15.PrevMvReadPosFactor = prevMvReadPosFactor;

    if (m_4xMeInUse && m_curbeParam.brcEnable) // HME kernel generates Sum MV and Distortion for Hevc dual pipe
    {
        curbe.m_data.DW5.SumMVThreshold = m_curbeParam.sumMVThreshold; // As per kernel requirement, used only when BRC is on/LTR is on
        curbe.m_data.DW6.BRCEnable      = m_curbeParam.brcEnable;
    }

    // r3 & r4
    uint8_t methodIndex = 0; // kernel requirement
    uint8_t tableIndex  = (m_pictureCodingType == B_TYPE) ? 1 : 0;

    MOS_SecureMemcpy(&curbe.m_data.SpDelta, 14 * sizeof(uint32_t), codechalEncodeSearchPath[tableIndex][methodIndex], 14 * sizeof(uint32_t));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalKernelHmeMdfG12::SetupSurfaces()
{
    if (!(m_4xMeInUse || m_16xMeInUse || m_32xMeInUse))
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (m_surfaceParam.vdencStreamInEnabled)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_surfaceParam.meVdencStreamInBuffer);
    }
    else
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_surfaceParam.meBrcDistortionBuffer);
    }

    CmDevice* &cmDev = m_encoder->m_cmDev;

    PMOS_SURFACE currScaledSurface;

    uint32_t     refScaledBottomFieldOffset = 0;
    bool         currFieldPicture = CodecHal_PictureIsField(*(m_surfaceParam.currOriginalPic)) ? true : false;
    bool         currBottomField = CodecHal_PictureIsBottomField(*(m_surfaceParam.currOriginalPic)) ? true : false;
    uint8_t      currVDirection = (!currFieldPicture) ? CODECHAL_VDIRECTION_FRAME : ((currBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);

    HmeYUVInfo *YuvInfo = nullptr;
    PMOS_SURFACE psurface = nullptr;
   
    if (m_32xMeInUse)
    {
        currScaledSurface = m_encoder->m_trackedBuf->Get32xDsSurface(CODEC_CURR_TRACKED_BUFFER);
        YuvInfo = &m_HME32xYUVInfo;
    }
    else if (m_16xMeInUse)
    {
        currScaledSurface = m_encoder->m_trackedBuf->Get16xDsSurface(CODEC_CURR_TRACKED_BUFFER);
        YuvInfo = &m_HME16xYUVInfo;
    }
    else
    {
        currScaledSurface = m_encoder->m_trackedBuf->Get4xDsSurface(CODEC_CURR_TRACKED_BUFFER);
        YuvInfo = &m_HME4xYUVInfo;
    }

    // Current Picture Y - VME
    CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->UpdateSurface2D(
        &currScaledSurface->OsResource,
        YuvInfo->SrcSurface));

    // Setup references 1...n
    // LIST 0 references
    CODEC_PICTURE refPic;
    // NOTE; keeping some of the legacy comments below. This may help if MDF RT is extended to AVC

    // Reference height and width information should be taken from the current scaled surface rather
    // than from the reference scaled surface in the case of PAFF.
    MOS_SURFACE refScaledSurface = *currScaledSurface;
    for (uint8_t refIdx = 0; refIdx <= m_surfaceParam.numRefIdxL0ActiveMinus1; refIdx++)
    {
        refPic = m_surfaceParam.refL0List[refIdx];

        if (!CodecHal_PictureIsInvalid(refPic) && m_surfaceParam.picIdx[refPic.FrameIdx].bValid)
        {
            bool    refBottomField = (CodecHal_PictureIsBottomField(refPic)) ? 1 : 0;
            uint8_t refPicIdx = m_surfaceParam.picIdx[refPic.FrameIdx].ucPicIdx;
            uint8_t scaledIdx = m_surfaceParam.refList[refPicIdx]->ucScalingIdx;
            if (m_32xMeInUse)
            {
                MOS_SURFACE* p32xSurface = m_encoder->m_trackedBuf->Get32xDsSurface(scaledIdx);
                if (p32xSurface != nullptr)
                {
                    refScaledSurface.OsResource = p32xSurface->OsResource;
                }
                else
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("NULL pointer of DsSurface");
                }
            }
            else if (m_16xMeInUse)
            {
                MOS_SURFACE* p16xSurface = m_encoder->m_trackedBuf->Get16xDsSurface(scaledIdx);
                if (p16xSurface != nullptr)
                {
                    refScaledSurface.OsResource = p16xSurface->OsResource;
                }
                else
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("NULL pointer of DsSurface");
                }
            }
            else
            {
                MOS_SURFACE* p4xSurface = m_encoder->m_trackedBuf->Get4xDsSurface(scaledIdx);
                if (p4xSurface != nullptr)
                {
                    refScaledSurface.OsResource = p4xSurface->OsResource;
                }
                else
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("NULL pointer of DsSurface");
                }
            }
            // L0 Reference Picture Y - VME
            CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->UpdateSurface2D(
                &refScaledSurface.OsResource,
                YuvInfo->FwdReference[refIdx]));
        }
    }

    if (YuvInfo->VMEFwdIdx)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->DestroyVmeSurfaceG7_5(YuvInfo->VMEFwdIdx));
        YuvInfo->VMEFwdIdx = nullptr;
    }
    CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->CreateVmeSurfaceG7_5(
            YuvInfo->SrcSurface,
            YuvInfo->FwdReference,
            YuvInfo->FwdReference,
            (m_surfaceParam.numRefIdxL0ActiveMinus1 + 1),
            (m_surfaceParam.numRefIdxL0ActiveMinus1 + 1),
            YuvInfo->VMEFwdIdx));
    // Setup references 1...n
    // LIST 1 references
    for (uint8_t refIdx = 0; refIdx <= m_surfaceParam.numRefIdxL1ActiveMinus1; refIdx++)
    {
        refPic = m_surfaceParam.refL1List[refIdx];
        if (!CodecHal_PictureIsInvalid(refPic) && m_surfaceParam.picIdx[refPic.FrameIdx].bValid)
        {
            bool    refBottomField = (CodecHal_PictureIsBottomField(refPic)) ? true : false;
            uint8_t refPicIdx = m_surfaceParam.picIdx[refPic.FrameIdx].ucPicIdx;
            uint8_t scaledIdx = m_surfaceParam.refList[refPicIdx]->ucScalingIdx;
            if (m_32xMeInUse)
            {
                MOS_SURFACE* p32xSurface = m_encoder->m_trackedBuf->Get32xDsSurface(scaledIdx);
                if (p32xSurface != nullptr)
                {
                    refScaledSurface.OsResource = p32xSurface->OsResource;
                }
                else
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("NULL pointer of DsSurface");
                }
            }
            else if (m_16xMeInUse)
            {
                MOS_SURFACE* p16xSurface = m_encoder->m_trackedBuf->Get16xDsSurface(scaledIdx);
                if (p16xSurface != nullptr)
                {
                    refScaledSurface.OsResource = p16xSurface->OsResource;
                }
                else
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("NULL pointer of DsSurface");
                }
            }
            else
            {
                MOS_SURFACE* p4xSurface = m_encoder->m_trackedBuf->Get4xDsSurface(scaledIdx);
                if (p4xSurface != nullptr)
                {
                    refScaledSurface.OsResource = p4xSurface->OsResource;
                }
                else
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("NULL pointer of DsSurface");
                }
            }
            // L1 Reference Picture Y - VME
            CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->UpdateSurface2D(
                &refScaledSurface.OsResource,
                YuvInfo->BwdReference[refIdx]));
        }
    }
    if (YuvInfo->VMEBwdIdx)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->DestroyVmeSurfaceG7_5(YuvInfo->VMEBwdIdx));
        YuvInfo->VMEBwdIdx = nullptr;
    }
    //  HME L1, L1 references are provided to kernel/VME as L0
    CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->CreateVmeSurfaceG7_5(
        YuvInfo->SrcSurface,
        YuvInfo->BwdReference,
        YuvInfo->BwdReference,
        (m_surfaceParam.numRefIdxL1ActiveMinus1 + 1),
        (m_surfaceParam.numRefIdxL1ActiveMinus1 + 1),
        YuvInfo->VMEBwdIdx));

    CODECHAL_MEDIA_STATE_TYPE  mediaStateType = (m_32xMeInUse) ? CODECHAL_MEDIA_STATE_32X_ME :
        m_16xMeInUse ? CODECHAL_MEDIA_STATE_16X_ME : CODECHAL_MEDIA_STATE_4X_ME;

    if ( m_surfaceParam.vdencStreamInEnabled && mediaStateType == CODECHAL_MEDIA_STATE_4X_ME)
    {
        mediaStateType = CODECHAL_MEDIA_STATE_ME_VDENC_STREAMIN;
    }

    if (mediaStateType == CODECHAL_MEDIA_STATE_ME_VDENC_STREAMIN)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->UpdateBuffer(
                m_surfaceParam.meVdencStreamInBuffer,
                m_VdencStreamInBuffer));
    }

    if (m_curbeParam.brcEnable && m_4xMeInUse)
    {
        if (!m_SumMvandDistortionBuffer)
        {
          CODECHAL_ENCODE_CHK_STATUS_RETURN(cmDev->CreateBuffer(
              &m_surfaceParam.meSumMvandDistortionBuffer.sResource,
              m_SumMvandDistortionBuffer));
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalKernelHmeMdfG12::SetupKernelArgs(CmKernel *cmKrn)
{
    int idx = 0;
    Curbe curbe;
    if (!(m_4xMeInUse || m_16xMeInUse || m_32xMeInUse))
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    SetMECurbe(curbe);
    cmKrn->SetKernelArg(idx++, sizeof(Curbe), &curbe);
    HmeYUVInfo *YuvInfo = nullptr;
    SurfaceIndex * pSurfIndex = nullptr;

    CODECHAL_DEBUG_TOOL(
        CODECHAL_MEDIA_STATE_TYPE  mediaStateType = (m_32xMeInUse) ? CODECHAL_MEDIA_STATE_32X_ME :
            m_16xMeInUse ? CODECHAL_MEDIA_STATE_16X_ME : CODECHAL_MEDIA_STATE_4X_ME;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpMDFCurbe(
            mediaStateType,
            (uint8_t *)&curbe,
            sizeof(curbe)));)

    if (m_32xMeInUse)
    {
        m_HME32xMVSurface->GetIndex(pSurfIndex);
        YuvInfo = &m_HME32xYUVInfo;
    }
    else if (m_16xMeInUse)
    {
        m_HME16xMVSurface->GetIndex(pSurfIndex);
        YuvInfo = &m_HME16xYUVInfo;
    }
    else
    {
        m_HME4xMVSurface->GetIndex(pSurfIndex);
        YuvInfo = &m_HME4xYUVInfo;
    }
    cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex), pSurfIndex);

    if (m_16xMeInUse && Is32xMeEnabled())
    {
        // Pass 32x MV to 16x ME operation
        m_HME32xMVSurface->GetIndex(pSurfIndex);

    }
    else if (Is16xMeEnabled() && !m_32xMeInUse)
    {
        // Pass 16x MV to 4x ME
        m_HME16xMVSurface->GetIndex(pSurfIndex);
    }
    // else pass same surface index as dummy
    cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex), pSurfIndex);

    // Insert Distortion buffers only for 4xMe case
    if (m_4xMeInUse && m_4xMeDistortionBufferSupported)
    {
        m_HME4xDistortionSurface->GetIndex(pSurfIndex);
    }
    cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex), pSurfIndex);

    if (m_4xMeInUse && !m_surfaceParam.vdencStreamInEnabled)
    {
        m_surfaceParam.meBrcDistortionSurface->GetIndex(pSurfIndex);
    }
    cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex), pSurfIndex);

    CODECHAL_ENCODE_CHK_NULL_RETURN(YuvInfo->VMEFwdIdx)

    cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex), YuvInfo->VMEFwdIdx);

    if (m_pictureCodingType == B_TYPE)
    {
        cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex), YuvInfo->VMEBwdIdx);
    }
    else
    {
        // bwd ref surfaces. if not provided, set to previous surface
        cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex), YuvInfo->VMEFwdIdx);
    }

    if (m_surfaceParam.vdencStreamInEnabled && m_4xMeInUse)
    {
        m_VdencStreamInBuffer->GetIndex(pSurfIndex);
    }
    //set surface for vdenc streamin
    cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex), pSurfIndex);

    // hevc vdenc streamin. no separate buffer created for now
    cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex), pSurfIndex);


    if (m_curbeParam.brcEnable && m_4xMeInUse)
    {
      m_SumMvandDistortionBuffer->GetIndex(pSurfIndex);
    }
    //set surface for Sum MV distortion buffer
    cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex), pSurfIndex);


    return MOS_STATUS_SUCCESS;
}
