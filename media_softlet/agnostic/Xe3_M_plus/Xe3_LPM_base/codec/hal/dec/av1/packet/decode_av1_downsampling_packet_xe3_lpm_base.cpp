/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     decode_av1_downsampling_packet_xe3_lpm_base.cpp
//! \brief    Defines the interface for av1 decode down sampling sub packet
//!
#include "decode_av1_downsampling_packet_xe3_lpm_base.h"
#include "decode_av1_basic_feature.h"

#ifdef _DECODE_PROCESSING_SUPPORTED

namespace decode
{
Av1DownSamplingPktXe3_Lpm_Base::Av1DownSamplingPktXe3_Lpm_Base(DecodePipeline *pipeline, CodechalHwInterfaceNext *hwInterface)
    : DecodeDownSamplingPkt(pipeline, hwInterface)
{
}

MOS_STATUS Av1DownSamplingPktXe3_Lpm_Base::Init()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(DecodeDownSamplingPkt::Init());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1DownSamplingPktXe3_Lpm_Base::SetSfcMode(MEDIA_SFC_INTERFACE_MODE &mode)
{
    mode.veboxSfcEnabled = 0;
    mode.vdboxSfcEnabled = 1;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1DownSamplingPktXe3_Lpm_Base::InitSfcParams(VDBOX_SFC_PARAMS &sfcParams)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(DecodeDownSamplingPkt::InitSfcParams(sfcParams));

    Av1BasicFeature *av1BasicFeature = dynamic_cast<Av1BasicFeature *>(m_basicFeature);
    DECODE_CHK_NULL(av1BasicFeature);
    CodecAv1PicParams *av1PicParams = av1BasicFeature->m_av1PicParams;
    DECODE_CHK_NULL(av1PicParams);
    int16_t tileIdx                         = av1BasicFeature->m_tileCoding.m_curTile;

    sfcParams.input.width                   = (uint32_t)av1PicParams->m_superResUpscaledWidthMinus1 + 1;
    sfcParams.input.height                  = (uint32_t)av1PicParams->m_superResUpscaledHeightMinus1 + 1;
    sfcParams.videoParams.av1.lcuSize       = av1PicParams->m_seqInfoFlags.m_fields.m_use128x128Superblock ? 128 : 64;
    sfcParams.videoParams.av1.lossless      = av1PicParams->m_losslessMode;
    sfcParams.videoParams.av1.superResInuse = av1PicParams->m_picInfoFlags.m_fields.m_useSuperres && (av1PicParams->m_superresScaleDenominator != av1ScaleNumerator);
    sfcParams.videoParams.av1.intraBC       = av1PicParams->m_picInfoFlags.m_fields.m_allowIntrabc;
    sfcParams.videoParams.av1.tileCols      = av1BasicFeature->m_tileCoding.m_tileDesc[tileIdx].m_tileColumn;
    sfcParams.videoParams.av1.tileRows      = av1BasicFeature->m_tileCoding.m_tileDesc[tileIdx].m_tileRow;
    return MOS_STATUS_SUCCESS;
}

}  // namespace decode

#endif  // !_DECODE_PROCESSING_SUPPORTED