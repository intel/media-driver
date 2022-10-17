/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     decode_vp9_downsampling_packet.cpp
//! \brief    Defines the interface for vp9 decode down sampling sub packet
//!
#include "decode_vp9_downsampling_packet.h"
#include "decode_vp9_basic_feature.h"

#ifdef _DECODE_PROCESSING_SUPPORTED

namespace decode
{

Vp9DownSamplingPkt::Vp9DownSamplingPkt(DecodePipeline *pipeline, CodechalHwInterfaceNext *hwInterface)
    : DecodeDownSamplingPkt(pipeline, hwInterface)
{}

MOS_STATUS Vp9DownSamplingPkt::Init()
{
    DECODE_CHK_STATUS(DecodeDownSamplingPkt::Init());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DownSamplingPkt::SetSfcMode(MEDIA_SFC_INTERFACE_MODE &mode)
{
    mode.veboxSfcEnabled = 0;
    mode.vdboxSfcEnabled = 1;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DownSamplingPkt::InitSfcParams(VDBOX_SFC_PARAMS &sfcParams)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_STATUS(DecodeDownSamplingPkt::InitSfcParams(sfcParams));

    Vp9BasicFeature *vp9BasicFeature = dynamic_cast<Vp9BasicFeature *>(m_basicFeature);
    DECODE_CHK_NULL(vp9BasicFeature);

    sfcParams.input.width  = (uint32_t)vp9BasicFeature->m_vp9PicParams->FrameWidthMinus1 + 1;
    sfcParams.input.height = (uint32_t)vp9BasicFeature->m_vp9PicParams->FrameHeightMinus1 + 1;

    SCALABILITY_PARAMS &scalabilityParams = sfcParams.videoParams.scalabilityParams;
    if (m_pipeline->GetPipeNum() <= 1)
    {
        scalabilityParams.numPipe    = 1;
        scalabilityParams.curPipe    = 0;
        scalabilityParams.engineMode = MhwSfcInterface::sfcScalabModeSingle;
    }
    else
    {
        DECODE_CHK_STATUS(InitSfcScalabParams(scalabilityParams));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DownSamplingPkt::InitSfcScalabParams(SCALABILITY_PARAMS &scalabilityParams)
{
    DECODE_FUNC_CALL();

    Vp9Pipeline *vp9Pipeline = dynamic_cast<Vp9Pipeline *>(m_pipeline);
    DECODE_CHK_NULL(vp9Pipeline);
    Vp9BasicFeature *vp9BasicFeature = dynamic_cast<Vp9BasicFeature *>(m_basicFeature);
    DECODE_CHK_NULL(vp9BasicFeature);

    CodecRectangle &inputSurfaceRegion  = m_downSampling->m_inputSurfaceRegion;
    CodecRectangle &outputSurfaceRegion = m_downSampling->m_outputSurfaceRegion;

    auto curPipe = vp9Pipeline->GetCurrentPipe();
    auto numPipe = vp9Pipeline->GetPipeNum();
    auto curPass = vp9Pipeline->GetCurrentPass();

    scalabilityParams.numPipe = numPipe;
    scalabilityParams.curPipe = curPipe;

    if (curPipe == 0)
    {
        scalabilityParams.engineMode = MhwSfcInterface::sfcScalabModeLeftCol;
    }
    else if (curPipe == numPipe - 1)
    {
        scalabilityParams.engineMode = MhwSfcInterface::sfcScalabModeRightCol;
    }
    else
    {
        scalabilityParams.engineMode = MhwSfcInterface::sfcScalabModeMiddleCol;
    }

    uint32_t tileColIndex = 0;
    uint32_t tileColCount = 0;

    DECODE_CHK_STATUS(InitSfcScalabSrcParams(
        *vp9Pipeline, *vp9BasicFeature, scalabilityParams, tileColIndex, tileColCount));

    DECODE_CHK_STATUS(InitSfcScalabDstParams(
        *vp9Pipeline, *vp9BasicFeature, scalabilityParams, tileColIndex, tileColCount));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DownSamplingPkt::InitSfcScalabSrcParams(
    Vp9Pipeline &vp9Pipeline, Vp9BasicFeature &vp9BasicFeature,
    SCALABILITY_PARAMS &scalabilityParams, uint32_t &tileColIndex, uint32_t &tileColCount)
{
    DECODE_FUNC_CALL();

    CodecRectangle &inputSurfaceRegion  = m_downSampling->m_inputSurfaceRegion;
    CodecRectangle &outputSurfaceRegion = m_downSampling->m_outputSurfaceRegion;

    auto curPipe = vp9Pipeline.GetCurrentPipe();
    auto numPipe = vp9Pipeline.GetPipeNum();
    auto curPass = vp9Pipeline.GetCurrentPass();

    if (vp9Pipeline.GetDecodeMode() == Vp9Pipeline::virtualTileDecodeMode)
    {
        scalabilityParams.tileType = MhwSfcInterface::sfcScalabVirtualTile;

        tileColIndex = curPipe;
        tileColCount = numPipe;

        scalabilityParams.srcStartX = (tileColIndex * vp9BasicFeature.m_allocatedWidthInSb) / tileColCount *
            CODEC_VP9_SUPER_BLOCK_WIDTH;
        if (tileColIndex == tileColCount - 1)
        {
            scalabilityParams.srcEndX = inputSurfaceRegion.m_x + inputSurfaceRegion.m_width - 1;
        }
        else
        {
            scalabilityParams.srcEndX = (tileColIndex + 1) * vp9BasicFeature.m_allocatedWidthInSb / tileColCount *
                CODEC_VP9_SUPER_BLOCK_WIDTH - 1;
        }
#if (_DEBUG || _RELEASE_INTERNAL)
        if (m_dbgOvrdWidthInMinCb > 0 && numPipe == 2)
        {
            if (curPipe == 1)
            {
                scalabilityParams.srcStartX = m_dbgOvrdWidthInMinCb * CODEC_VP9_MIN_BLOCK_WIDTH;
                scalabilityParams.srcEndX   = inputSurfaceRegion.m_x + inputSurfaceRegion.m_width - 1;
            }
            else
            {
                scalabilityParams.srcStartX = 0;
                scalabilityParams.srcEndX   = m_dbgOvrdWidthInMinCb * CODEC_VP9_MIN_BLOCK_WIDTH - 1;
            }
        }
#endif
    }
    else
    {
        DECODE_ASSERTMESSAGE("Invalid decode mode, expect virtual tile mode.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // Clamp srcStartX, srcEndX into source region
    if (scalabilityParams.srcStartX < inputSurfaceRegion.m_x)
    {
        scalabilityParams.srcStartX = inputSurfaceRegion.m_x;
    }
    if (scalabilityParams.srcEndX > inputSurfaceRegion.m_x + inputSurfaceRegion.m_width - 1)
    {
        scalabilityParams.srcEndX = inputSurfaceRegion.m_x + inputSurfaceRegion.m_width - 1;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DownSamplingPkt::InitSfcScalabDstParams(
    Vp9Pipeline &vp9Pipeline, Vp9BasicFeature &vp9BasicFeature,
    SCALABILITY_PARAMS &scalabilityParams, const uint32_t &tileColIndex, const uint32_t &tileColCount)
{
    DECODE_FUNC_CALL();

    CodecRectangle &inputSurfaceRegion  = m_downSampling->m_inputSurfaceRegion;
    CodecRectangle &outputSurfaceRegion = m_downSampling->m_outputSurfaceRegion;

    if (tileColIndex == 0)
    {
        m_firstValidTileIndex = 0;
        m_lastValidTileIndex  = tileColCount - 1;
        m_dstXLandingCount    = 0;
    }

    // start ildb offset correction
    uint32_t srcEndXCorr = scalabilityParams.srcEndX - m_ildbXOffset;

    uint32_t tileOffsetX;
    uint32_t tileEndX;
    if ((inputSurfaceRegion.m_x + inputSurfaceRegion.m_width - 1) <= srcEndXCorr)
    {
        tileOffsetX = 0;
        tileEndX    = inputSurfaceRegion.m_x + inputSurfaceRegion.m_width;
    }
    else
    {
        bool isInput444 = (vp9BasicFeature.m_vp9PicParams->subsampling_x == 0 &&
                           vp9BasicFeature.m_vp9PicParams->subsampling_y == 0);
        tileOffsetX = isInput444 ? m_tileOffsetX444 : m_tileOffsetXBasic;
        tileEndX    = srcEndXCorr;
    }

    uint32_t dstEndX = 0;
    if ((srcEndXCorr - inputSurfaceRegion.m_x) >= (tileOffsetX + 1))
    {
        constexpr int32_t phaseShiftMin = -(1 << (4 + 19));
        constexpr int32_t phaseShiftMax = (1 << (4 + 19)) - 1;
        int32_t xPhaseShift = MOS_F_ROUND((double(inputSurfaceRegion.m_width) / outputSurfaceRegion.m_width - 1.0) / 2.0 * 524288.0F);
        uint32_t xPhaseShiftClamped = MOS_CLAMP_MIN_MAX(xPhaseShift, phaseShiftMin, phaseShiftMax);
        uint64_t oneBySf = uint64_t(double(uint64_t(inputSurfaceRegion.m_width) * 524288 / outputSurfaceRegion.m_width));

        while (true)
        {
            if (m_dstXLandingCount == 0)
            {
                m_firstValidTileIndex = tileColIndex;
            }

            double tempDestCntx = (((double)m_dstXLandingCount * (double)oneBySf) + xPhaseShiftClamped);
            if (tempDestCntx < 0)
            {
                tempDestCntx = 0;
            }
            double xLandingPoint = double((tempDestCntx + (double(1 << (m_oneBySfFractionPrecision - m_betaPrecision - 1)))) / 524288
                + inputSurfaceRegion.m_x);

            if (xLandingPoint >= double(tileEndX - tileOffsetX))
            {
                dstEndX = m_dstXLandingCount - 1;
                break;
            }
            else
            {
                m_dstXLandingCount++;
            }
        }
    }

    if (tileOffsetX == 0)
    {
        m_lastValidTileIndex = tileColIndex;
    }

    // Last column end at destination region right border.
    if (tileColIndex == m_lastValidTileIndex)
    {
        dstEndX = outputSurfaceRegion.m_x + outputSurfaceRegion.m_width - 1;
    }

    if (tileColIndex <= m_firstValidTileIndex)
    {
        scalabilityParams.dstStartX = 0;
        scalabilityParams.dstEndX   = dstEndX;
    }
    else if (tileColIndex <= m_lastValidTileIndex)
    {
        scalabilityParams.dstStartX = m_lastDstEndX + 1;
        scalabilityParams.dstEndX   = dstEndX;
    }
    else
    {
        scalabilityParams.dstStartX = 0;
        scalabilityParams.dstEndX   = 0;
    }

    m_lastDstEndX = dstEndX;

    return MOS_STATUS_SUCCESS;
}

}

#endif
