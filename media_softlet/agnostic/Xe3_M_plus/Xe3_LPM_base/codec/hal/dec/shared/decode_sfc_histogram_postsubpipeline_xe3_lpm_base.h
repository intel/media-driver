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
//! \file     decode_sfc_histogram_postsubpipeline_xe3_lpm_base.h
//! \brief    Defines the postSubpipeline for decode SFC histogram
//! \details  Defines the postSubpipline for decode SFC histogram
//!

#ifndef __DECODE_SFC_HISTOGRAM_POSTSUBPIPELINE_XE3_LPM_BASE_H__
#define __DECODE_SFC_HISTOGRAM_POSTSUBPIPELINE_XE3_LPM_BASE_H__

#include "decode_sfc_histogram_postsubpipeline.h"
#include "decode_av1_downsampling_feature_xe3_lpm_base.h"

#ifdef _DECODE_PROCESSING_SUPPORTED
namespace decode
{

class DecodeSfcHistogramSubPipelineXe3LpmBase : public DecodeSfcHistogramSubPipeline
{
public:
    //!
    //! \brief  Decode SFC histogram constructor
    //!
    DecodeSfcHistogramSubPipelineXe3LpmBase(DecodePipeline *pipeline, MediaTask *task, uint8_t numVdbox);

    //!
    //! \brief  Decode SFC histogram destructor
    //!
    virtual ~DecodeSfcHistogramSubPipelineXe3LpmBase() {}

    //!
    //! \brief  Initialize the bitstream context
    //!
    //! \param  [in] settings
    //!         Reference to the Codechal settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init(CodechalSetting &settings) override;

    //!
    //! \brief  Prepare interal parameters
    //! \param  [in] params
    //!         Reference to decode pipeline parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Prepare(DecodePipelineParams &params) override;

    protected:
    Av1DownSamplingFeatureXe3_Lpm_Base *m_downsampFeature = nullptr;  //!< Downsampling feature
MEDIA_CLASS_DEFINE_END(decode__DecodeSfcHistogramSubPipelineXe3LpmBase)
};
}
#endif //_DECODE_PROCESSING_SUPPORTED
#endif //__DECODE_SFC_HISTOGRAM_POSTSUBPIPELINE_XE3_LPM_BASE_H__
