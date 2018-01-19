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
//! \file     codechal_kernel_hme_g10.cpp
//! \brief    Hme kernel implementation for Gen10 platform
//!
#include "codechal_kernel_hme_g10.h"

// clang-format off
const uint32_t CodechalKernelHmeG10::Curbe::m_initCurbe[48] =
{
    0x00000000, 0x00200010, 0x00003939, 0x77a43000, 0x00000000, 0x28300000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000200,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};
// clang-format on

CodechalKernelHmeG10::CodechalKernelHmeG10(
    CodechalEncoderState *encoder,
    bool     me4xDistBufferSupported)
        : CodechalKernelHme(encoder, me4xDistBufferSupported)
{
}

MOS_STATUS CodechalKernelHmeG10::SetCurbe(MHW_KERNEL_STATE *kernelState)
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
    if (m_vdencEnabled &&
        (m_standard == CODECHAL_HEVC ||
            m_standard == CODECHAL_VP9))
    {
        curbe.m_data.DW6.SuperCombineDist = 5;  //SuperCombineDist[m_curbeParam.targetUsage]; harded coded in KCM
    }
    else
    {
        curbe.m_data.DW6.SuperCombineDist = SuperCombineDist[m_curbeParam.targetUsage];
    }
    curbe.m_data.DW6.MaxVmvR = CodecHal_PictureIsFrame(m_curbeParam.currOriginalPic) ? m_curbeParam.maxMvLen * 4 : (m_curbeParam.maxMvLen >> 1) * 4;

    if (m_pictureCodingType == B_TYPE)
    {
        curbe.m_data.DW1.BiWeight             = 32;
        curbe.m_data.DW13.NumRefIdxL1MinusOne = m_curbeParam.numRefIdxL1Minus1;
    }

    if (m_pictureCodingType == B_TYPE || m_pictureCodingType == P_TYPE)
    {
        curbe.m_data.DW13.NumRefIdxL0MinusOne = m_curbeParam.numRefIdxL0Minus1;
    }

    if (Is16xMeEnabled() && m_surfaceParam.vdencStreamInEnabled)
    {
        curbe.m_data.DW30.ActualMBHeight = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameFieldHeight);
        curbe.m_data.DW30.ActualMBWidth  = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameWidth);
    }

    curbe.m_data.DW13.RefStreaminCost = 0;
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

    // r3 & r4
    uint8_t methodIndex;
    if (m_pictureCodingType == B_TYPE)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_bmeMethodTable);
        methodIndex = m_curbeParam.bmeMethodTable ?
            m_curbeParam.bmeMethodTable[m_curbeParam.targetUsage] : m_bmeMethodTable[m_curbeParam.targetUsage];
    }
    else
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_meMethodTable);
        methodIndex = m_curbeParam.meMethodTable ?
            m_curbeParam.meMethodTable[m_curbeParam.targetUsage] : m_meMethodTable[m_curbeParam.targetUsage];
    }

    uint8_t tableIndex = (m_pictureCodingType == B_TYPE) ? 1 : 0;
    memcpy_s(&curbe.m_data.SpDelta, 14 * sizeof(uint32_t), codechalEncodeSearchPath[tableIndex][methodIndex], 14 * sizeof(uint32_t));

    if (m_4xMeInUse && m_vdencEnabled &&
        (m_standard == CODECHAL_HEVC ||
            m_standard == CODECHAL_VP9))
    {
        curbe.m_data.DW6.LCUSize           = 1;  // only LCU64 is supported by VDEnc HW
        curbe.m_data.DW6.InputStreamInEn   = 0;
        curbe.m_data.DW31.NumImePredictors = 8;
        curbe.m_data.DW31.MaxCuSize        = 3;
        curbe.m_data.DW31.MaxTuSize        = 3;

        switch (m_curbeParam.targetUsage)
        {
        case 1:
        case 4:
            curbe.m_data.DW36.NumMergeCandCu64x64 = 4;
            curbe.m_data.DW36.NumMergeCandCu32x32 = 3;
            curbe.m_data.DW36.NumMergeCandCu16x16 = 2;
            curbe.m_data.DW36.NumMergeCandCu8x8   = 1;
            break;
        case 7:
            curbe.m_data.DW36.NumMergeCandCu64x64 = 2;
            curbe.m_data.DW36.NumMergeCandCu32x32 = 2;
            curbe.m_data.DW36.NumMergeCandCu16x16 = 2;
            curbe.m_data.DW36.NumMergeCandCu8x8   = 0;
            break;
        default:
            break;
        }
    }

    //r5
    curbe.m_data.DW40._4xMeMvOutputDataSurfIndex      = BindingTableOffset::meOutputMvDataSurface;
    curbe.m_data.DW41._16xOr32xMeMvInputDataSurfIndex = BindingTableOffset::meInputMvDataSurface;
    curbe.m_data.DW42._4xMeOutputDistSurfIndex        = BindingTableOffset::meDistortionSurface;
    curbe.m_data.DW43._4xMeOutputBrcDistSurfIndex     = BindingTableOffset::meBrcDistortion;
    curbe.m_data.DW44.VMEFwdInterPredictionSurfIndex  = BindingTableOffset::meCurrForFwdRef;
    curbe.m_data.DW45.VMEBwdInterPredictionSurfIndex  = BindingTableOffset::meCurrForBwdRef;
    curbe.m_data.DW46.VDEncStreamInOutputSurfIndex    = BindingTableOffset::meVdencStreamInOutputBuffer;
    curbe.m_data.DW47.VDEncStreamInInputSurfIndex     = BindingTableOffset::meVdencStreamInInputBuffer;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernelState->m_dshRegion.AddData(&curbe.m_data, kernelState->dwCurbeOffset, Curbe::m_curbeSize));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalKernelHmeG10::SendSurfaces(PMOS_COMMAND_BUFFER cmd, MHW_KERNEL_STATE *kernelState)
{
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalKernelHme::SendSurfaces(cmd, kernelState));

    if (m_vdencEnabled && m_4xMeInUse)
    {
        CODECHAL_SURFACE_CODEC_PARAMS surfaceParams;
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
    }
    return MOS_STATUS_SUCCESS;
}

MHW_KERNEL_STATE *CodechalKernelHmeG10::GetActiveKernelState()
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
            kernelIndex  = KernelIndex::hmeVDEncStreamIn;
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

CODECHAL_MEDIA_STATE_TYPE CodechalKernelHmeG10::GetMediaStateType()
{
    CODECHAL_MEDIA_STATE_TYPE mediaStateType;
    mediaStateType = m_32xMeInUse ? CODECHAL_MEDIA_STATE_32X_ME : m_16xMeInUse ? CODECHAL_MEDIA_STATE_16X_ME : CODECHAL_MEDIA_STATE_4X_ME;
    if (m_4xMeInUse && m_vdencEnabled && m_standard == CODECHAL_AVC)
    {
        mediaStateType = CODECHAL_MEDIA_STATE_ME_VDENC_STREAMIN;
    }

    return mediaStateType;
}
