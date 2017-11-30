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
//! \file     codechal_encode_csc_ds_g9.cpp
//! \brief    This file implements the Csc+Ds feature for all codecs on Gen9 platform
//!

#include "codechal_encoder.h"
#include "codechal_encoder_base.h"
#include "codechal_encode_csc_ds_g9.h"
#include "codeckrnheader.h"
#include "igcodeckrn_g9.h"
#if USE_CODECHAL_DEBUG_TOOL
#include "codechal_debug_encode_par_g9.h"
#endif

MOS_STATUS CodechalEncodeCscDsG9::SetCurbeCsc()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CscKernelCurbeData Curbe;

    Curbe.DW0_InputPictureWidth = m_curbeParams.dwInputPictureWidth;
    Curbe.DW0_InputPictureHeight = m_curbeParams.dwInputPictureHeight;

    if (m_curbeParams.bCscOrCopyOnly)
    {
        Curbe.DW1_CscDsCopyOpCode = 0;    // Copy only
    }
    else
    {
        // Enable DS kernel (0  disable, 1  enable)
        Curbe.DW1_CscDsCopyOpCode = 1;    // 0x01 to 0x7F: DS + Copy
    }

    if (cscColorNv12TileY == m_colorRawSurface ||
        cscColorNv12Linear == m_colorRawSurface)
    {
        Curbe.DW1_InputColorFormat = 0;
    }
    else if (cscColorYUY2 == m_colorRawSurface)
    {
        Curbe.DW1_InputColorFormat = 1;
    }
    else if ((cscColorARGB == m_colorRawSurface) || (cscColorABGR == m_colorRawSurface))
    {
        Curbe.DW1_InputColorFormat = 2;
    }

    if (m_curbeParams.bFlatnessCheckEnabled ||
        m_curbeParams.bMBVarianceOutputEnabled ||
        m_curbeParams.bMBPixelAverageOutputEnabled)
    {
        Curbe.DW2_FlatnessThreshold = 128;
        Curbe.DW3_EnableMBStatSurface = true;
    }
    else
    {
        Curbe.DW3_EnableMBStatSurface = false;
    }
    
    // RGB->YUV CSC coefficients
    if (m_curbeParams.inputColorSpace == ECOLORSPACE_P709)
    {
        Curbe.DW4_CscCoefficientC0 = 0xFFCD;
        Curbe.DW5_CscCoefficientC3 = 0x0080;
        Curbe.DW6_CscCoefficientC4 = 0x004F;
        Curbe.DW7_CscCoefficientC7 = 0x0010;
        Curbe.DW8_CscCoefficientC8 = 0xFFD5;
        Curbe.DW9_CscCoefficientC11 = 0x0080;
        if (cscColorARGB == m_colorRawSurface)
        {
            Curbe.DW4_CscCoefficientC1 = 0xFFFB;
            Curbe.DW5_CscCoefficientC2 = 0x0038;
            Curbe.DW6_CscCoefficientC5 = 0x0008;
            Curbe.DW7_CscCoefficientC6 = 0x0017;
            Curbe.DW8_CscCoefficientC9 = 0x0038;
            Curbe.DW9_CscCoefficientC10 = 0xFFF3;
        }
        else // cscColorABGR == m_colorRawSurface
        {
            Curbe.DW4_CscCoefficientC1 = 0x0038;
            Curbe.DW5_CscCoefficientC2 = 0xFFFB;
            Curbe.DW6_CscCoefficientC5 = 0x0017;
            Curbe.DW7_CscCoefficientC6 = 0x0008;
            Curbe.DW8_CscCoefficientC9 = 0xFFF3;
            Curbe.DW9_CscCoefficientC10 = 0x0038;
        }
    }
    else if (m_curbeParams.inputColorSpace == ECOLORSPACE_P601)
    {
        Curbe.DW4_CscCoefficientC0 = 0xFFD1;
        Curbe.DW5_CscCoefficientC3 = 0x0080;
        Curbe.DW6_CscCoefficientC4 = 0x0041;
        Curbe.DW7_CscCoefficientC7 = 0x0010;
        Curbe.DW8_CscCoefficientC8 = 0xFFDB;
        Curbe.DW9_CscCoefficientC11 = 0x0080;
        if (cscColorARGB == m_colorRawSurface)
        {
            Curbe.DW4_CscCoefficientC1 = 0xFFF7;
            Curbe.DW5_CscCoefficientC2 = 0x0038;
            Curbe.DW6_CscCoefficientC5 = 0x000D;
            Curbe.DW7_CscCoefficientC6 = 0x0021;
            Curbe.DW8_CscCoefficientC9 = 0x0038;
            Curbe.DW9_CscCoefficientC10 = 0xFFED;
        }
        else // cscColorABGR == m_colorRawSurface
        {
            Curbe.DW4_CscCoefficientC1 = 0x0038;
            Curbe.DW5_CscCoefficientC2 = 0xFFF7;
            Curbe.DW6_CscCoefficientC5 = 0x0021;
            Curbe.DW7_CscCoefficientC6 = 0x000D;
            Curbe.DW8_CscCoefficientC9 = 0xFFED;
            Curbe.DW9_CscCoefficientC10 = 0x0038;
        }
    }
    else 
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ARGB input color space = %d!", m_curbeParams.inputColorSpace);
        return MOS_STATUS_INVALID_PARAMETER;
    }

    Curbe.DW16_SrcNV12SurfYIndex = cscSrcYPlane;
    Curbe.DW17_DstYSurfIndex = cscDstDsYPlane;
    Curbe.DW18_MbStatDstSurfIndex = cscDstFlatOrMbStats;
    Curbe.DW19_CopyDstNV12SurfIndex = cscDstCopyYPlane;
    Curbe.DW20_SrcNV12SurfUVIndex = cscSrcUVPlane;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscKernelState->m_dshRegion.AddData(
        &Curbe,
        m_cscKernelState->dwCurbeOffset,
        sizeof(Curbe)));

    return eStatus;
}

MOS_STATUS CodechalEncodeCscDsG9::InitKernelStateDS()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (CODECHAL_AVC == m_standard)
    {
        m_dsBTCount[0] = ds4xNumSurfaces;
        m_dsCurbeLength[0] = 
        m_dsInlineDataLength = sizeof(Ds4xKernelCurbeData);
        m_dsBTISrcY = ds4xSrcYPlane;
        m_dsBTIDstY = ds4xDstYPlane;
        m_dsBTISrcYTopField = ds4xSrcYPlaneTopField;
        m_dsBTIDstYTopField = ds4xDstYPlaneTopField;
        m_dsBTISrcYBtmField = ds4xSrcYPlaneBtmField;
        m_dsBTIDstYBtmField = ds4xDstYPlaneBtmField;
        m_dsBTIDstMbVProc = ds4xDstMbVProc;
        m_dsBTIDstMbVProcTopField = ds4xDstMbVProcTopField;
        m_dsBTIDstMbVProcBtmField = ds4xDstMbVProcBtmField;
    }

    return CodechalEncodeCscDs::InitKernelStateDS();
}

MOS_STATUS CodechalEncodeCscDsG9::SetCurbeDS4x()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (CODECHAL_AVC != m_standard)
    {
        return CodechalEncodeCscDs::SetCurbeDS4x();
    }

    Ds4xKernelCurbeData Curbe;

    Curbe.DW0_InputPictureWidth = m_curbeParams.dwInputPictureWidth;
    Curbe.DW0_InputPictureHeight = m_curbeParams.dwInputPictureHeight;

    Curbe.DW1_InputYBTIFrame = ds4xSrcYPlane;
    Curbe.DW2_OutputYBTIFrame = ds4xDstYPlane;

    if (m_curbeParams.bFieldPicture)
    {
        Curbe.DW3_InputYBTIBottomField = ds4xSrcYPlaneBtmField;
        Curbe.DW4_OutputYBTIBottomField = ds4xDstYPlaneBtmField;
    }

    if (Curbe.DW6_EnableMBFlatnessCheck = m_curbeParams.bFlatnessCheckEnabled)
    {
        Curbe.DW5_FlatnessThreshold = 128;
    }

    // For gen10 DS kernel, If Flatness Check enabled, need enable MBVariance as well. Otherwise will not output MbIsFlat.
    Curbe.DW6_EnableMBVarianceOutput = m_curbeParams.bFlatnessCheckEnabled || m_curbeParams.bMBVarianceOutputEnabled;
    Curbe.DW6_EnableMBPixelAverageOutput = m_curbeParams.bMBPixelAverageOutputEnabled;
    Curbe.DW6_EnableBlock8x8StatisticsOutput = m_curbeParams.bBlock8x8StatisticsEnabled;

    if (Curbe.DW6_EnableMBVarianceOutput || Curbe.DW6_EnableMBPixelAverageOutput)
    {
        Curbe.DW8_MBVProcStatsBTIFrame = ds4xDstMbVProc;

        if (m_curbeParams.bFieldPicture)
        {
            Curbe.DW9_MBVProcStatsBTIBottomField = ds4xDstMbVProcBtmField;
        }
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_dsKernelState->m_dshRegion.AddData(
        &Curbe,
        m_dsKernelState->dwCurbeOffset,
        sizeof(Curbe)));

    CODECHAL_DEBUG_TOOL(
        if (m_encoder->m_encodeParState)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_encodeParState->PopulateDsParam(&Curbe));
        }
    )

    return MOS_STATUS_SUCCESS;
}

CodechalEncodeCscDsG9::CodechalEncodeCscDsG9(PCODECHAL_ENCODER pEncoder)
    : CodechalEncodeCscDs(pEncoder)
{
    m_cscKernelUID = IDR_CODEC_Downscale_Copy;
    m_cscCurbeLength = sizeof(CscKernelCurbeData);
    m_kernelBase = (uint8_t*)IGCODECKRN_G9;
    Initialize();
}

CodechalEncodeCscDsG9::CodechalEncodeCscDsG9(CodechalEncoderState* encoder)
    : CodechalEncodeCscDs(encoder)
{
    m_cscKernelUID = IDR_CODEC_Downscale_Copy;
    m_cscCurbeLength = sizeof(CscKernelCurbeData);
    m_kernelBase = (uint8_t*)IGCODECKRN_G9;
    Initialize();
}
