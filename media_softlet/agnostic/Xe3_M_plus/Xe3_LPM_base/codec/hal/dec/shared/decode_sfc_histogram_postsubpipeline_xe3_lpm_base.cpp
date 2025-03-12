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
//! \file     decode_sfc_histogram_postsubpipeline_xe3_lpm_base.cpp
//! \brief    Defines the postSubpipeline for decode SFC histogram
//! \details  Defines the postSubpipline for decode SFC histogram
//!

#include "decode_pipeline.h"
#include "decode_common_feature_defs.h"
#include "decode_sfc_histogram_postsubpipeline_xe3_lpm_base.h"

#ifdef _DECODE_PROCESSING_SUPPORTED

namespace decode {
    DecodeSfcHistogramSubPipelineXe3LpmBase::DecodeSfcHistogramSubPipelineXe3LpmBase(DecodePipeline* pipeline,
        MediaTask* task,
        uint8_t numVdbox) :
        DecodeSfcHistogramSubPipeline(pipeline, task, numVdbox)
    {}

    MOS_STATUS DecodeSfcHistogramSubPipelineXe3LpmBase::Init(CodechalSetting &settings)
    {
        DECODE_FUNC_CALL();

        auto featureManger = m_pipeline->GetFeatureManager();
        DECODE_CHK_NULL(featureManger);
        m_basicFeature = dynamic_cast<DecodeBasicFeature *>(featureManger->GetFeature(FeatureIDs::basicFeature));
        DECODE_CHK_NULL(m_basicFeature);

        m_downsampFeature = dynamic_cast<Av1DownSamplingFeatureXe3_Lpm_Base *>(featureManger->GetFeature(
            DecodeFeatureIDs::decodeDownSampling));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS DecodeSfcHistogramSubPipelineXe3LpmBase::Prepare(DecodePipelineParams& params)
    {
        DECODE_FUNC_CALL();
        DecodeSfcHistogramSubPipeline::Prepare(params);

        if (params.m_pipeMode == decodePipeModeProcess &&
            m_downsampFeature != nullptr && m_downsampFeature->m_histogramBufferU != nullptr)  //m_downsampFeature could be null if downsampling is not enabled
        {
            DECODE_CHK_NULL(params.m_params);
            DECODE_CHK_NULL(m_basicFeature);
            DECODE_CHK_NULL(m_downsampFeature->m_histogramBufferU);

            CodechalDecodeParams *decodeParams = params.m_params;
            PMOS_RESOURCE         srcU         = &m_downsampFeature->m_histogramBufferU->OsResource;
            PMOS_RESOURCE         destU        = &decodeParams->m_histogramSurfaceU.OsResource;
            uint32_t              destOffsetU  = decodeParams->m_histogramSurfaceU.dwOffset;
            if (!m_allocator->ResourceIsNull(srcU) &&
                !m_allocator->ResourceIsNull(destU))
            {
                DECODE_CHK_STATUS(CopyHistogramToDestBuf(srcU, destU, destOffsetU));
            }
        }

        if (params.m_pipeMode == decodePipeModeProcess &&
            m_downsampFeature != nullptr && m_downsampFeature->m_histogramBufferV != nullptr)  //m_downsampFeature could be null if downsampling is not enabled
        {
            DECODE_CHK_NULL(params.m_params);
            DECODE_CHK_NULL(m_basicFeature);
            DECODE_CHK_NULL(m_downsampFeature->m_histogramBufferV);

            CodechalDecodeParams *decodeParams = params.m_params;
            PMOS_RESOURCE         srcV         = &m_downsampFeature->m_histogramBufferV->OsResource;
            PMOS_RESOURCE         destV        = &decodeParams->m_histogramSurfaceV.OsResource;
            uint32_t              destOffsetV  = decodeParams->m_histogramSurfaceV.dwOffset;
            if (!m_allocator->ResourceIsNull(srcV) &&
                !m_allocator->ResourceIsNull(destV))
            {
                DECODE_CHK_STATUS(CopyHistogramToDestBuf(srcV, destV, destOffsetV));
            }
        }

        return MOS_STATUS_SUCCESS;
    }
}  // namespace decode
#endif  //_DECODE_PROCESSING_SUPPORTED