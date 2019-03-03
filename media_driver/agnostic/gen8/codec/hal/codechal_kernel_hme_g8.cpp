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
//! \file     codechal_kernel_hme_g8.cpp
//! \brief    Hme kernel implementation for Gen8 platform
//!
#include "codechal_kernel_hme_g8.h"

// clang-format off
const uint32_t CodechalKernelHmeG8::Curbe::m_initCurbe[39] =
{
    0x00000000, 0x00200010, 0x00003939, 0x77a43000, 0x00000000, 0x28300000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};
// clang-format on

CodechalKernelHmeG8::CodechalKernelHmeG8(
    CodechalEncoderState *encoder,
    bool     me4xDistBufferSupported)
        : CodechalKernelHme(encoder, me4xDistBufferSupported)
{
}

MOS_STATUS CodechalKernelHmeG8::SetCurbe(MHW_KERNEL_STATE *kernelState)
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
    curbe.m_data.DW6.MaxVmvR             = CodecHal_PictureIsFrame(m_curbeParam.currOriginalPic) ? m_curbeParam.maxMvLen * 4 : (m_curbeParam.maxMvLen >> 1) * 4;

    if (m_pictureCodingType == B_TYPE)
    {
        curbe.m_data.DW1.BiWeight             = 32;
        curbe.m_data.DW13.NumRefIdxL1MinusOne = m_curbeParam.numRefIdxL1Minus1;
    }

    if (m_pictureCodingType == B_TYPE ||
        m_pictureCodingType == P_TYPE)
    {
        curbe.m_data.DW13.NumRefIdxL0MinusOne = m_curbeParam.numRefIdxL0Minus1;
    }

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
    MOS_SecureMemcpy(&curbe.m_data.SpDelta, 14 * sizeof(uint32_t), codechalEncodeSearchPath[tableIndex][methodIndex], 14 * sizeof(uint32_t));

    //r5
    curbe.m_data.DW32._4xMeMvOutputDataSurfIndex      = BindingTableOffset::meOutputMvDataSurface;
    curbe.m_data.DW33._16xOr32xMeMvInputDataSurfIndex = BindingTableOffset::meInputMvDataSurface;
    curbe.m_data.DW34._4xMeOutputDistSurfIndex        = BindingTableOffset::meDistortionSurface;
    curbe.m_data.DW35._4xMeOutputBrcDistSurfIndex     = BindingTableOffset::meBrcDistortion;
    curbe.m_data.DW36.VMEFwdInterPredictionSurfIndex  = BindingTableOffset::meCurrForFwdRef;
    curbe.m_data.DW37.VMEBwdInterPredictionSurfIndex  = BindingTableOffset::meCurrForBwdRef;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernelState->m_dshRegion.AddData(&curbe.m_data, kernelState->dwCurbeOffset, Curbe::m_curbeSize));

    return MOS_STATUS_SUCCESS;
}
