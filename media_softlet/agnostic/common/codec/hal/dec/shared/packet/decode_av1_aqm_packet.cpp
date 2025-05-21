/*
* Copyright (c) 2025, Intel Corporation
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
//! \file     decode_av1_aqm_packet.cpp
//! \brief    Defines the interface for av1 decode picture packet
//!
#include "decode_av1_aqm_packet.h"
#include "codechal_debug.h"
#include "decode_common_feature_defs.h"

#ifdef _DECODE_PROCESSING_SUPPORTED

namespace decode
{
    Av1DecodeAqmPkt::~Av1DecodeAqmPkt()
    {
    }
    MOS_STATUS Av1DecodeAqmPkt::Init()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_featureManager);
        DECODE_CHK_NULL(m_hwInterface);
        DECODE_CHK_NULL(m_osInterface);
        DECODE_CHK_NULL(m_av1Pipeline);

        m_av1BasicFeature = dynamic_cast<Av1BasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        DECODE_CHK_NULL(m_av1BasicFeature);

        m_allocator = m_pipeline->GetDecodeAllocator();
        DECODE_CHK_NULL(m_allocator);

        MediaFeatureManager *featureManager = m_pipeline->GetFeatureManager();
        DECODE_CHK_NULL(featureManager);

        m_downSampling = dynamic_cast<DecodeDownSamplingFeature *>(
            featureManager->GetFeature(DecodeFeatureIDs::decodeDownSampling));
        DECODE_CHK_NULL(m_downSampling);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodeAqmPkt::Prepare()
    {
        DECODE_FUNC_CALL();

        m_av1PicParams = m_av1BasicFeature->m_av1PicParams;
        DECODE_CHK_NULL(m_av1PicParams);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodeAqmPkt::CalculateCommandSize(
        uint32_t& commandBufferSize,
        uint32_t& requestedPatchListSize)
    {
        DECODE_FUNC_CALL();

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AQM_HIST_STATE, Av1DecodeAqmPkt)
    {
        params.chromaPixelBitDepth                = m_av1PicParams->m_bitDepthIdx == 0 ? 8: m_av1PicParams->m_bitDepthIdx == 1? 10: 12;  // TBD: need to override in each generation
        params.CodecType                          = 2; // AV1
        params.disableStatisticalSummaryHistogram = 0; // Enable
        params.frameOrTileSizeInPixels            = (m_av1PicParams->m_superResUpscaledWidthMinus1 + 1) * (m_av1PicParams->m_superResUpscaledHeightMinus1 + 1);
        params.initializationMode                 = 0; // Initialize all the bins with 0
        if (m_av1PicParams->m_profile == 0)
        {
            params.inputChromaSubsamplingFormat = 0; // 4:2:0
        }
        else if (m_av1PicParams->m_profile == 2) // TBD: move to xe3 file
        {
            params.inputChromaSubsamplingFormat = 2; // 4:4:4
        }
        params.lumaPixelBitDepth = params.chromaPixelBitDepth;  // TBD: need to override in each generation
        params.operatingMode     = 0; // Decoder mode

        if (m_downSampling->m_histogramBuffer)
        {
            params.yHistogramEnable = 1;  // Enable Y channel histogram
        }

        if (m_downSampling->m_histogramBufferU)
        {
            params.uHistogramEnable = 1;  // Enable U channel histogram
        }

        if (m_downSampling->m_histogramBufferV)
        {
            params.vHistogramEnable = 1;  // Enable V channel histogram
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AQM_HIST_BUFF_ADDR_STATE, Av1DecodeAqmPkt)
    {
        if (m_downSampling->m_histogramBuffer)
        {
            params.AqmYChannelHistogramOutputBuffer = &m_downSampling->m_histogramBuffer->OsResource;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AQM_HIST_FLUSH, Av1DecodeAqmPkt)
    {
        params.aqmHistFlush = 0;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AQM_FRAME_START, Av1DecodeAqmPkt)
    {
        params.aqmFrameStart = 1;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AQM_VD_CONTROL_STATE, Av1DecodeAqmPkt)
    {
        params.pipelineInitialization                    = 1;
        params.VDboxPipelineArchitectureClockgateDisable = 0;
        params.memoryImplicitFlush                       = 1;

        return MOS_STATUS_SUCCESS;
    }

    }  // namespace decode

#endif  //_DECODE_PROCESSING_SUPPORTED