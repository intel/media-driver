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
//! \file     decode_hevc_aqm_packet.cpp
//! \brief    Defines the interface for hevc decode picture packet
//!
#include "decode_hevc_aqm_packet.h"
#include "codechal_debug.h"
#include "decode_common_feature_defs.h"

#ifdef _DECODE_PROCESSING_SUPPORTED

namespace decode
{
    HevcDecodeAqmPkt::~HevcDecodeAqmPkt()
    {
    }
    MOS_STATUS HevcDecodeAqmPkt::Init()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_featureManager);
        DECODE_CHK_NULL(m_hwInterface);
        DECODE_CHK_NULL(m_osInterface);
        DECODE_CHK_NULL(m_hevcPipeline);

        m_hevcBasicFeature = dynamic_cast<HevcBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        DECODE_CHK_NULL(m_hevcBasicFeature);

        m_allocator = m_pipeline->GetDecodeAllocator();
        DECODE_CHK_NULL(m_allocator);

        MediaFeatureManager* featureManager = m_pipeline->GetFeatureManager();
        DECODE_CHK_NULL(featureManager);

        m_downSampling = dynamic_cast<DecodeDownSamplingFeature*>(
            featureManager->GetFeature(DecodeFeatureIDs::decodeDownSampling));
        DECODE_CHK_NULL(m_downSampling);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcDecodeAqmPkt::Prepare()
    {
        DECODE_FUNC_CALL();

        m_hevcPicParams = m_hevcBasicFeature->m_hevcPicParams;
        DECODE_CHK_NULL(m_hevcPicParams);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcDecodeAqmPkt::CalculateCommandSize(
        uint32_t& commandBufferSize,
        uint32_t& requestedPatchListSize)
    {
        DECODE_FUNC_CALL();

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AQM_HIST_STATE, HevcDecodeAqmPkt)
    {
        params.chromaPixelBitDepth                = m_hevcPicParams->bit_depth_chroma_minus8 + 8;
        params.CodecType                          = 1; // hevc
        params.disableStatisticalSummaryHistogram = 0; // Enable
        params.frameOrTileSizeInPixels            = ((1 << (m_hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3)) * (m_hevcPicParams->PicWidthInMinCbsY)) * 
            ((1 << (m_hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3)) * (m_hevcPicParams->PicHeightInMinCbsY));
        params.initializationMode                 = 0; // Initialize all the bins with 0
        params.inputChromaSubsamplingFormat       = (m_hevcPicParams->chroma_format_idc != HCP_CHROMA_FORMAT_MONOCHROME) ? m_hevcPicParams->chroma_format_idc - 1 : 3;  // 4:2:0 - 0, 4:2:2 - 1, 4:4:4 - 2, 4:0:0 - 3
        params.lumaPixelBitDepth                  = m_hevcPicParams->bit_depth_luma_minus8 + 8;  // TBD: need to override in each generation
        params.operatingMode                      = 0; // Decoder mode
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

    MHW_SETPAR_DECL_SRC(AQM_HIST_BUFF_ADDR_STATE, HevcDecodeAqmPkt)
    {
        if (m_downSampling->m_histogramBuffer)
        {
            params.AqmYChannelHistogramOutputBuffer = &m_downSampling->m_histogramBuffer->OsResource;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AQM_HIST_FLUSH, HevcDecodeAqmPkt)
    {
        params.aqmHistFlush = 0;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AQM_FRAME_START, HevcDecodeAqmPkt)
    {
        params.aqmFrameStart = 1;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AQM_VD_CONTROL_STATE, HevcDecodeAqmPkt)
    {
        params.pipelineInitialization = 1;
        params.VDboxPipelineArchitectureClockgateDisable = 0;
        params.memoryImplicitFlush = 1;

        return MOS_STATUS_SUCCESS;
    }
}  // namespace decode

#endif //_DECODE_PROCESSING_SUPPORTED
