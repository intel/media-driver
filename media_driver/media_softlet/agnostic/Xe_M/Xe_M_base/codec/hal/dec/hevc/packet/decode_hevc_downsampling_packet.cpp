/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     decode_hevc_downsampling_packet.cpp
//! \brief    Defines the interface for hevc decode down sampling sub packet
//!
#include "decode_hevc_downsampling_packet.h"
#include "decode_hevc_basic_feature.h"

#ifdef _DECODE_PROCESSING_SUPPORTED

namespace decode
{

HevcDownSamplingPkt::HevcDownSamplingPkt(DecodePipeline *pipeline, CodechalHwInterfaceNext *hwInterface)
    : DecodeDownSamplingPkt(pipeline, hwInterface)
{}

MOS_STATUS HevcDownSamplingPkt::Init()
{
    DECODE_CHK_STATUS(DecodeDownSamplingPkt::Init());

#if (_DEBUG || _RELEASE_INTERNAL)
    m_dbgOvrdWidthInMinCb =
        ReadUserFeature(m_pipeline->GetUserSetting(), "Scalability Split Width In MinCb", MediaUserSetting::Group::Sequence).Get<bool>();
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDownSamplingPkt::SetSfcMode(MEDIA_SFC_INTERFACE_MODE &mode)
{
    mode.veboxSfcEnabled = 0;
    mode.vdboxSfcEnabled = 1;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDownSamplingPkt::InitSfcParams(VDBOX_SFC_PARAMS &sfcParams)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_STATUS(DecodeDownSamplingPkt::InitSfcParams(sfcParams));

    sfcParams.input.width  = m_basicFeature->m_width;
    sfcParams.input.height = m_basicFeature->m_height;

    HevcBasicFeature *hevcBasicFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
    DECODE_CHK_NULL(hevcBasicFeature);
    sfcParams.videoParams.hevc.lcuSize = hevcBasicFeature->m_ctbSize;

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

MOS_STATUS HevcDownSamplingPkt::InitSfcScalabParams(SCALABILITY_PARAMS &scalabilityParams)
{
    DECODE_FUNC_CALL();

    HevcPipeline *hevcPipeline = dynamic_cast<HevcPipeline *>(m_pipeline);
    DECODE_CHK_NULL(hevcPipeline);
    HevcBasicFeature *hevcBasicFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
    DECODE_CHK_NULL(hevcBasicFeature);

    CodecRectangle &inputSurfaceRegion  = m_downSampling->m_inputSurfaceRegion;
    CodecRectangle &outputSurfaceRegion = m_downSampling->m_outputSurfaceRegion;

    auto curPipe = hevcPipeline->GetCurrentPipe();
    auto numPipe = hevcPipeline->GetPipeNum();
    auto curPass = hevcPipeline->GetCurrentPass();

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
        *hevcPipeline, *hevcBasicFeature, scalabilityParams, tileColIndex, tileColCount));

    DECODE_CHK_STATUS(InitSfcScalabDstParams(
        *hevcPipeline, *hevcBasicFeature, scalabilityParams, tileColIndex, tileColCount));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDownSamplingPkt::InitSfcScalabSrcParams(
    HevcPipeline &hevcPipeline, HevcBasicFeature &hevcBasicFeature,
    SCALABILITY_PARAMS &scalabilityParams, uint32_t &tileColIndex, uint32_t &tileColCount)
{
    DECODE_FUNC_CALL();

    CodecRectangle &inputSurfaceRegion  = m_downSampling->m_inputSurfaceRegion;
    CodecRectangle &outputSurfaceRegion = m_downSampling->m_outputSurfaceRegion;

    auto curPipe = hevcPipeline.GetCurrentPipe();
    auto numPipe = hevcPipeline.GetPipeNum();
    auto curPass = hevcPipeline.GetCurrentPass();

    if (hevcPipeline.GetDecodeMode() == HevcPipeline::realTileDecodeMode)
    {
        scalabilityParams.tileType = MhwSfcInterface::sfcScalabRealTile;

        DECODE_CHK_NULL(hevcBasicFeature.m_hevcPicParams);
        PCODEC_HEVC_PIC_PARAMS hevcPicParams = hevcBasicFeature.m_hevcPicParams;
        tileColIndex = curPipe + curPass * numPipe;
        tileColCount = hevcPicParams->num_tile_columns_minus1 + 1;

        if (hevcPicParams->uniform_spacing_flag)
        {
            scalabilityParams.srcStartX = tileColIndex * hevcBasicFeature.m_widthInCtb / tileColCount *
                hevcBasicFeature.m_ctbSize;
            scalabilityParams.srcEndX   = (tileColIndex + 1) * hevcBasicFeature.m_widthInCtb / tileColCount *
                hevcBasicFeature.m_ctbSize - 1;
        }
        else
        {
            scalabilityParams.srcStartX = 0;
            for (uint32_t i = 0; i < tileColIndex; i++)
            {
                scalabilityParams.srcStartX +=
                    (hevcPicParams->column_width_minus1[i] + 1) * hevcBasicFeature.m_ctbSize;
            }
            if (tileColIndex == hevcPicParams->num_tile_columns_minus1)
            {
                scalabilityParams.srcEndX = inputSurfaceRegion.m_x + inputSurfaceRegion.m_width - 1;
            }
            else
            {
                scalabilityParams.srcEndX = scalabilityParams.srcStartX +
                    (hevcPicParams->column_width_minus1[tileColIndex] + 1) * hevcBasicFeature.m_ctbSize - 1;
            }
        }
    }
    else if (hevcPipeline.GetDecodeMode() == HevcPipeline::virtualTileDecodeMode)
    {
        scalabilityParams.tileType = MhwSfcInterface::sfcScalabVirtualTile;

        tileColIndex = curPipe;
        tileColCount = numPipe;

        scalabilityParams.srcStartX = (tileColIndex * hevcBasicFeature.m_widthInCtb) / tileColCount *
            hevcBasicFeature.m_ctbSize;
        if (tileColIndex == tileColCount - 1)
        {
            scalabilityParams.srcEndX = inputSurfaceRegion.m_x + inputSurfaceRegion.m_width - 1;
        }
        else
        {
            scalabilityParams.srcEndX = (tileColIndex + 1) * hevcBasicFeature.m_widthInCtb / tileColCount *
                hevcBasicFeature.m_ctbSize - 1;
        }
#if (_DEBUG || _RELEASE_INTERNAL)
        if (m_dbgOvrdWidthInMinCb > 0 && numPipe == 2)
        {
            if (curPipe == 1)
            {
                scalabilityParams.srcStartX = m_dbgOvrdWidthInMinCb * hevcBasicFeature.m_minCtbSize;
                scalabilityParams.srcEndX   = inputSurfaceRegion.m_x + inputSurfaceRegion.m_width - 1;
            }
            else
            {
                scalabilityParams.srcStartX = 0;
                scalabilityParams.srcEndX   = m_dbgOvrdWidthInMinCb * hevcBasicFeature.m_minCtbSize - 1;
            }
        }
#endif
    }
    else
    {
        DECODE_ASSERTMESSAGE("Invalid decode mode, expect real tile mode and virtual tile mode.");
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

MOS_STATUS HevcDownSamplingPkt::InitSfcScalabDstParams(
    HevcPipeline &hevcPipeline, HevcBasicFeature &hevcBasicFeature,
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
        tileOffsetX = (hevcBasicFeature.m_hevcPicParams->chroma_format_idc == HCP_CHROMA_FORMAT_YUV444) ?
            m_tileOffsetX444 : m_tileOffsetXBasic;
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

#endif  // !_DECODE_PROCESSING_SUPPORTED
