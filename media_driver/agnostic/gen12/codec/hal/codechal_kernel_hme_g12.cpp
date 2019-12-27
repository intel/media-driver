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
//! \brief    Hme kernel implementation for Gen12 platform
//!
#include "codechal_kernel_hme_g12.h"
#if USE_CODECHAL_DEBUG_TOOL
#include "codechal_debug_encode_par_g12.h"
#endif

// clang-format off
const uint32_t CodechalKernelHmeG12::Curbe::m_initCurbe[49] =
{
    0x00000000, 0x00200010, 0x00003939, 0x77a43000, 0x00000000, 0x28300000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000200,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff
};
// clang-format on

CodechalKernelHmeG12::CodechalKernelHmeG12(
    CodechalEncoderState *encoder,
    bool     me4xDistBufferSupported)
        : CodechalKernelHme(encoder, me4xDistBufferSupported)
{
}

MOS_STATUS CodechalKernelHmeG12::SetCurbe(MHW_KERNEL_STATE *kernelState)
{
    CODECHAL_ENCODE_CHK_NULL_RETURN(kernelState);

    Curbe     curbe;
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
        //Different values are used by 4xMe among codecs
        if (m_4xMeInUse && m_useNonLegacyStreamIn)
        {
            curbe.m_data.DW30.ActualMBHeight = m_frameHeight;
            curbe.m_data.DW30.ActualMBWidth = m_frameWidth;
        }
        else if (m_vdencEnabled && Is16xMeEnabled())
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
    uint8_t methodIndex = 0;  // kernel requirement
    uint8_t tableIndex  = (m_pictureCodingType == B_TYPE) ? 1 : 0;

    MOS_SecureMemcpy(&curbe.m_data.SpDelta, 14 * sizeof(uint32_t), codechalEncodeSearchPath[tableIndex][methodIndex], 14 * sizeof(uint32_t));

    //r5
    curbe.m_data.DW40._4xMeMvOutputDataSurfIndex      = BindingTableOffset::meOutputMvDataSurface;
    curbe.m_data.DW41._16xOr32xMeMvInputDataSurfIndex = BindingTableOffset::meInputMvDataSurface;
    curbe.m_data.DW42._4xMeOutputDistSurfIndex        = BindingTableOffset::meDistortionSurface;
    curbe.m_data.DW43._4xMeOutputBrcDistSurfIndex     = BindingTableOffset::meBrcDistortion;
    curbe.m_data.DW44.VMEFwdInterPredictionSurfIndex  = BindingTableOffset::meCurrForFwdRef;
    curbe.m_data.DW45.VMEBwdInterPredictionSurfIndex  = BindingTableOffset::meCurrForBwdRef;
    curbe.m_data.DW46.VDEncStreamInOutputSurfIndex    = BindingTableOffset::meVdencStreamInOutputBuffer;
    curbe.m_data.DW47.VDEncStreamInInputSurfIndex     = BindingTableOffset::meVdencStreamInInputBuffer;
    
    //r6
    curbe.m_data.DW48.SumMVandDistortionOutputSurfIndex = BindingTableOffset::meSumMvandDistortionBuffer;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernelState->m_dshRegion.AddData(&curbe.m_data, kernelState->dwCurbeOffset, Curbe::m_curbeSize));

    CODECHAL_DEBUG_TOOL(
        if (m_encoder->m_encodeParState)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_encodeParState->PopulateHmeParam(Is16xMeEnabled(), Is32xMeEnabled(), methodIndex, &curbe));
        }
    )

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalKernelHmeG12::SendSurfaces(PMOS_COMMAND_BUFFER cmd, MHW_KERNEL_STATE *kernelState)
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

    PMOS_SURFACE currScaledSurface;
    uint32_t     refScaledBottomFieldOffset = 0;
    bool         currFieldPicture = CodecHal_PictureIsField(*(m_surfaceParam.currOriginalPic)) ? true : false;
    bool         currBottomField = CodecHal_PictureIsBottomField(*(m_surfaceParam.currOriginalPic)) ? true : false;
    uint8_t      currVDirection = (!currFieldPicture) ? CODECHAL_VDIRECTION_FRAME : ((currBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);

    CODECHAL_SURFACE_CODEC_PARAMS surfaceParams;
    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));

    surfaceParams.bIs2DSurface = true;
    surfaceParams.bMediaBlockRW = true;
    surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
    surfaceParams.dwBindingTableOffset = BindingTableOffset::meOutputMvDataSurface;
    surfaceParams.bIsWritable = true;
    surfaceParams.bRenderTarget = true;

    if (m_32xMeInUse)
    {
        currScaledSurface = m_encoder->m_trackedBuf->Get32xDsSurface(CODEC_CURR_TRACKED_BUFFER);
        surfaceParams.psSurface = GetSurface(SurfaceId::me32xMvDataBuffer);
        surfaceParams.dwOffset = m_32xMeMvBottomFieldOffset;
    }
    else if (m_16xMeInUse)
    {
        currScaledSurface = m_encoder->m_trackedBuf->Get16xDsSurface(CODEC_CURR_TRACKED_BUFFER);
        surfaceParams.psSurface = GetSurface(SurfaceId::me16xMvDataBuffer);
        surfaceParams.dwOffset = m_16xMeMvBottomFieldOffset;
    }
    else
    {
        currScaledSurface = m_encoder->m_trackedBuf->Get4xDsSurface(CODEC_CURR_TRACKED_BUFFER);
        surfaceParams.psSurface = GetSurface(SurfaceId::me4xMvDataBuffer);
        surfaceParams.dwOffset = m_4xMeMvBottomFieldOffset;
    }

    // Force the values
    surfaceParams.psSurface->dwWidth = MOS_ALIGN_CEIL(m_surfaceParam.downScaledWidthInMb * 32, 64);
    surfaceParams.psSurface->dwHeight = m_surfaceParam.downScaledHeightInMb * 4 * CODECHAL_ENCODE_ME_DATA_SIZE_MULTIPLIER;
    surfaceParams.psSurface->dwPitch = surfaceParams.psSurface->dwWidth;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmd,
        &surfaceParams,
        kernelState));

    if (m_16xMeInUse && Is32xMeEnabled())
    {
        // Pass 32x MV to 16x ME operation
        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.bIs2DSurface = true;
        surfaceParams.bMediaBlockRW = true;
        surfaceParams.psSurface = GetSurface(SurfaceId::me32xMvDataBuffer);
        surfaceParams.dwOffset = currBottomField ? m_32xMeMvBottomFieldOffset : 0;
        surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
        surfaceParams.dwBindingTableOffset = BindingTableOffset::meInputMvDataSurface;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmd,
            &surfaceParams,
            kernelState));
    }
    else if (Is16xMeEnabled() && !m_32xMeInUse)
    {
        // Pass 16x MV to 4x ME operation
        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.bIs2DSurface = true;
        surfaceParams.bMediaBlockRW = true;
        surfaceParams.psSurface = GetSurface(SurfaceId::me16xMvDataBuffer);
        surfaceParams.dwOffset = currBottomField ? m_16xMeMvBottomFieldOffset : 0;
        surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
        surfaceParams.dwBindingTableOffset = BindingTableOffset::meInputMvDataSurface;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmd,
            &surfaceParams,
            kernelState));
    }

    // Insert Distortion buffers only for 4xMe case
    if (m_4xMeInUse)
    {
        if (!m_surfaceParam.vdencStreamInEnabled)
        {
            MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
            surfaceParams.bIs2DSurface = true;
            surfaceParams.bMediaBlockRW = true;
            surfaceParams.psSurface = m_surfaceParam.meBrcDistortionBuffer;
            surfaceParams.dwOffset = m_surfaceParam.meBrcDistortionBottomFieldOffset;
            surfaceParams.dwBindingTableOffset = BindingTableOffset::meBrcDistortion;
            surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value;
            surfaceParams.bIsWritable = true;
            surfaceParams.bRenderTarget = true;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmd,
                &surfaceParams,
                kernelState));
        }

        if (m_4xMeDistortionBufferSupported)
        {
            MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
            surfaceParams.bIs2DSurface = true;
            surfaceParams.bMediaBlockRW = true;
            surfaceParams.psSurface = GetSurface(SurfaceId::me4xDistortionBuffer);
            surfaceParams.psSurface->dwHeight = m_surfaceParam.downScaledHeightInMb * 4 * 10;
            surfaceParams.dwOffset = m_meDistortionBottomFieldOffset;
            surfaceParams.dwBindingTableOffset = BindingTableOffset::meDistortionSurface;
            surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value;
            surfaceParams.bIsWritable = true;
            surfaceParams.bRenderTarget = true;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmd,
                &surfaceParams,
                kernelState));
        }
    }

    // Setup references 1...n
    // LIST 0 references
    CODEC_PICTURE refPic;
    // Reference height and width information should be taken from the current scaled surface rather
    // than from the reference scaled surface in the case of PAFF.
    MOS_SURFACE refScaledSurface = *currScaledSurface;

    uint8_t fwdRefBTOffset[8];
    fwdRefBTOffset[0] = BindingTableOffset::meFwdRefIdx0;
    fwdRefBTOffset[1] = BindingTableOffset::meFwdRefIdx1;
    fwdRefBTOffset[2] = BindingTableOffset::meFwdRefIdx2;
    fwdRefBTOffset[3] = BindingTableOffset::meFwdRefIdx3;
    fwdRefBTOffset[4] = BindingTableOffset::meFwdRefIdx4;
    fwdRefBTOffset[5] = BindingTableOffset::meFwdRefIdx5;
    fwdRefBTOffset[6] = BindingTableOffset::meFwdRefIdx6;
    fwdRefBTOffset[7] = BindingTableOffset::meFwdRefIdx7;

    for (uint8_t refIdx = 0; refIdx <= m_surfaceParam.numRefIdxL0ActiveMinus1; refIdx++)
    {
        refPic = m_surfaceParam.refL0List[refIdx];

        if (!CodecHal_PictureIsInvalid(refPic) && m_surfaceParam.picIdx[refPic.FrameIdx].bValid)
        {
            if (refIdx == 0)
            {
                // Current Picture Y - VME
                MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
                surfaceParams.bUseAdvState = true;
                surfaceParams.psSurface = currScaledSurface;
                surfaceParams.dwOffset = currBottomField ? m_surfaceParam.downScaledBottomFieldOffset : 0;
                surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value;
                surfaceParams.dwBindingTableOffset = BindingTableOffset::meCurrForFwdRef;
                surfaceParams.ucVDirection = currVDirection;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmd,
                    &surfaceParams,
                    kernelState));
            }

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
                refScaledBottomFieldOffset = refBottomField ? m_surfaceParam.downScaledBottomFieldOffset : 0;
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
                refScaledBottomFieldOffset = refBottomField ? m_surfaceParam.downScaledBottomFieldOffset : 0;
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
                refScaledBottomFieldOffset = refBottomField ? m_surfaceParam.downScaledBottomFieldOffset : 0;
            }

            // L0 Reference Picture Y - VME
            MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
            surfaceParams.bUseAdvState = true;
            surfaceParams.psSurface = &refScaledSurface;
            surfaceParams.dwOffset = refBottomField ? refScaledBottomFieldOffset : 0;
            surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value;
            surfaceParams.dwBindingTableOffset = fwdRefBTOffset[refIdx];
            surfaceParams.ucVDirection = !currFieldPicture ? CODECHAL_VDIRECTION_FRAME :
                ((refBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmd,
                &surfaceParams,
                kernelState));

            // To avoid fatal error
            surfaceParams.dwBindingTableOffset = fwdRefBTOffset[refIdx] + 1;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmd,
                &surfaceParams,
                kernelState));
        }
    }

    // Setup references 1...n
    // LIST 1 references
    uint8_t bwdRefBTOffset[2];
    bwdRefBTOffset[0] = BindingTableOffset::meBwdRefIdx0;
    bwdRefBTOffset[1] = BindingTableOffset::meBwdRefIdx1;

    for (uint8_t refIdx = 0; refIdx <= m_surfaceParam.numRefIdxL1ActiveMinus1; refIdx++)
    {
        refPic = m_surfaceParam.refL1List[refIdx];

        if (!CodecHal_PictureIsInvalid(refPic) && m_surfaceParam.picIdx[refPic.FrameIdx].bValid)
        {
            if (refIdx == 0)
            {
                // Current Picture Y - VME
                MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
                surfaceParams.bUseAdvState = true;
                surfaceParams.psSurface = currScaledSurface;
                surfaceParams.dwOffset = currBottomField ? m_surfaceParam.downScaledBottomFieldOffset : 0;
                surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value;
                surfaceParams.dwBindingTableOffset = BindingTableOffset::meCurrForBwdRef;
                surfaceParams.ucVDirection = currVDirection;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmd,
                    &surfaceParams,
                    kernelState));
            }

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
                refScaledBottomFieldOffset = refBottomField ? m_surfaceParam.downScaledBottomFieldOffset : 0;
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
                refScaledBottomFieldOffset = refBottomField ? m_surfaceParam.downScaledBottomFieldOffset : 0;
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
                refScaledBottomFieldOffset = refBottomField ? m_surfaceParam.downScaledBottomFieldOffset : 0;
            }
            // L1 Reference Picture Y - VME
            MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
            surfaceParams.bUseAdvState = true;
            surfaceParams.psSurface = &refScaledSurface;
            surfaceParams.dwOffset = refBottomField ? refScaledBottomFieldOffset : 0;
            surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value;
            surfaceParams.dwBindingTableOffset = bwdRefBTOffset[refIdx];
            surfaceParams.ucVDirection = (!currFieldPicture) ? CODECHAL_VDIRECTION_FRAME : ((refBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmd,
                &surfaceParams,
                kernelState));

            surfaceParams.dwBindingTableOffset = bwdRefBTOffset[refIdx] + 1;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmd,
                &surfaceParams,
                kernelState));
        }
    }

    CODECHAL_MEDIA_STATE_TYPE  mediaStateType = (m_32xMeInUse) ? CODECHAL_MEDIA_STATE_32X_ME :
        m_16xMeInUse ? CODECHAL_MEDIA_STATE_16X_ME : CODECHAL_MEDIA_STATE_4X_ME;

    if ( m_surfaceParam.vdencStreamInEnabled && mediaStateType == CODECHAL_MEDIA_STATE_4X_ME)
    {
        mediaStateType = CODECHAL_MEDIA_STATE_ME_VDENC_STREAMIN;
    }

    if (mediaStateType == CODECHAL_MEDIA_STATE_ME_VDENC_STREAMIN)
    {
        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.dwSize                = m_surfaceParam.vdencStreamInSurfaceSize;
        surfaceParams.bIs2DSurface          = false;
        surfaceParams.presBuffer            = m_surfaceParam.meVdencStreamInBuffer;
        surfaceParams.dwBindingTableOffset  = BindingTableOffset::meVdencStreamInOutputBuffer;
        surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_ME_DISTORTION_ENCODE].Value;
        surfaceParams.bIsWritable           = true;
        surfaceParams.bRenderTarget         = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmd,
            &surfaceParams,
            kernelState));

        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.dwSize = m_surfaceParam.vdencStreamInSurfaceSize;
        surfaceParams.bIs2DSurface = false;
        surfaceParams.presBuffer = m_surfaceParam.meVdencStreamInBuffer;
        surfaceParams.dwBindingTableOffset = BindingTableOffset::meVdencStreamInInputBuffer;
        surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_ME_DISTORTION_ENCODE].Value;
        surfaceParams.bIsWritable = true;
        surfaceParams.bRenderTarget = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmd,
            &surfaceParams,
            kernelState));
    }
    
    if (m_curbeParam.brcEnable && m_4xMeInUse)
    {
        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.dwSize = m_surfaceParam.meSumMvandDistortionBuffer.dwSize;
        surfaceParams.bIs2DSurface = false;
        surfaceParams.presBuffer = &m_surfaceParam.meSumMvandDistortionBuffer.sResource;
        surfaceParams.dwBindingTableOffset = BindingTableOffset::meSumMvandDistortionBuffer;
        surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value;
        surfaceParams.bIsWritable = true;
        surfaceParams.bRenderTarget = true;
        surfaceParams.bRawSurface = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmd,
            &surfaceParams,
            kernelState));
    }
    
    return MOS_STATUS_SUCCESS;
}

MHW_KERNEL_STATE *CodechalKernelHmeG12::GetActiveKernelState()
{
    EncOperation operation;

    uint32_t kernelOffset = 0;
    uint32_t kernelIndex;

    if (m_pictureCodingType == P_TYPE)
    {
        kernelIndex  = KernelIndex::hmeP;
        operation    = ENC_ME;
        kernelOffset = 0;
    }
    else
    {
        kernelIndex  = KernelIndex::hmeB;
        operation    = ENC_ME;
        kernelOffset = 1;
        if (m_noMEKernelForPFrame || m_surfaceParam.refL1List->PicFlags == PICTURE_INVALID)
        {
            kernelOffset = 0; 
            kernelIndex = KernelIndex::hmeP;
        }
    }
    if (m_vdencEnabled && m_4xMeInUse)
    {
        if (m_standard == CODECHAL_AVC)
        {
            kernelIndex  = KernelIndex::hmeVDEncStreamIn;
            operation    = VDENC_ME;
            kernelOffset = 0;
        }
        else
        {
            kernelIndex  = KernelIndex::hmeVDEncHevcVp9StreamIn;
            operation    = VDENC_STREAMIN;
            kernelOffset = 0;
        }
    }

    auto it = m_kernelStatePool.find(kernelIndex);
    if (it != m_kernelStatePool.end())
    {
        return it->second;
    }
    MHW_KERNEL_STATE *kernelState = nullptr;
    CreateKernelState(&kernelState, kernelIndex, operation, kernelOffset);

    return kernelState;
}

CODECHAL_MEDIA_STATE_TYPE CodechalKernelHmeG12::GetMediaStateType()
{
    CODECHAL_MEDIA_STATE_TYPE mediaStateType;
    mediaStateType = m_32xMeInUse ? CODECHAL_MEDIA_STATE_32X_ME : m_16xMeInUse ? CODECHAL_MEDIA_STATE_16X_ME : CODECHAL_MEDIA_STATE_4X_ME;
    if (m_4xMeInUse && m_vdencEnabled && m_standard == CODECHAL_AVC)
    {
        mediaStateType = CODECHAL_MEDIA_STATE_ME_VDENC_STREAMIN;
    }
    return mediaStateType;
}
