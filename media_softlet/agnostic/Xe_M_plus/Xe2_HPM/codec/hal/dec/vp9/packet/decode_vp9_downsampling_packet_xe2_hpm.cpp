/*
* Copyright (c) 2021-2024, Intel Corporation
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
//! \file     decode_vp9_downsampling_packet_xe2_hpm.cpp
//! \brief    Defines the interface for vp9 decode down sampling sub packet
//!
#include "decode_vp9_downsampling_packet_xe2_hpm.h"
#include "decode_vp9_basic_feature.h"

#ifdef _DECODE_PROCESSING_SUPPORTED

namespace decode
{

Vp9DownSamplingPktXe2_Hpm::Vp9DownSamplingPktXe2_Hpm(DecodePipeline *pipeline, CodechalHwInterfaceNext *hwInterface)
    : Vp9DownSamplingPkt(pipeline, hwInterface)
{}

MOS_STATUS Vp9DownSamplingPktXe2_Hpm::InitSfcParams(VDBOX_SFC_PARAMS &sfcParams)
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
}

#endif
