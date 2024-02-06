/*
* Copyright (c) 2020-2024, Intel Corporation
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
//! \file     decode_avc_downsampling_packet.cpp
//! \brief    Defines the interface for avc decode down sampling sub packet
//!
#include "decode_avc_downsampling_packet.h"
#include "decode_avc_basic_feature.h"

#ifdef _DECODE_PROCESSING_SUPPORTED

namespace decode
{

AvcDownSamplingPkt::AvcDownSamplingPkt(DecodePipeline *pipeline, CodechalHwInterfaceNext *hwInterface)
    : DecodeDownSamplingPkt(pipeline, hwInterface)
{
    m_avcPipeline = dynamic_cast<AvcPipeline*>(pipeline);
}

MOS_STATUS AvcDownSamplingPkt::Init()
{
    DECODE_CHK_STATUS(DecodeDownSamplingPkt::Init());
    DECODE_CHK_NULL(m_avcPipeline);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcDownSamplingPkt::SetSfcMode(MEDIA_SFC_INTERFACE_MODE &mode)
{
    mode.veboxSfcEnabled = 0;
    mode.vdboxSfcEnabled = 1;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcDownSamplingPkt::InitSfcParams(VDBOX_SFC_PARAMS &sfcParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    DECODE_CHK_STATUS(DecodeDownSamplingPkt::InitSfcParams(sfcParams));

    AvcBasicFeature *avcBasicFeature = dynamic_cast<AvcBasicFeature*>(m_basicFeature);
    DECODE_CHK_NULL(avcBasicFeature);

    sfcParams.input.width  = avcBasicFeature->m_width;
    sfcParams.input.height = avcBasicFeature->m_height;

    CODEC_PICTURE curPic = avcBasicFeature->m_avcPicParams->CurrPic;

    if (avcBasicFeature->m_avcPicParams->seq_fields.mb_adaptive_frame_field_flag == true)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        DECODE_ASSERTMESSAGE("SFC does not support MBAFF mode as input");
        return eStatus;
    }

    if (CodecHal_PictureIsField(curPic) && (avcBasicFeature->m_width < 128) && ((avcBasicFeature->m_height >> 1) < 128))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        DECODE_ASSERTMESSAGE("Unsupported Input Resolution for field scaling, shoule be equal to or larger than 128 for each field");
        return eStatus;
    }
    sfcParams.videoParams.avc.deblockingEnabled = avcBasicFeature->m_deblockingEnabled;
    sfcParams.videoParams.fieldParams.isFieldToInterleaved = CodecHal_PictureIsField(curPic);

    if (CodecHal_PictureIsField(curPic))
    {
        sfcParams.input.height /= 2;
        sfcParams.output.rcDst.bottom /= 2;

        if (CodecHal_PictureIsBottomField(curPic))
        {
            sfcParams.videoParams.fieldParams.isBottomField = true;
            if (avcBasicFeature->m_isSecondField)
                sfcParams.videoParams.fieldParams.isBottomFirst = false;
            else
                sfcParams.videoParams.fieldParams.isBottomFirst = true;
        }
        else
        {
            sfcParams.videoParams.fieldParams.isBottomField = false;
            if (avcBasicFeature->m_isSecondField)
                sfcParams.videoParams.fieldParams.isBottomFirst = true;
            else
                sfcParams.videoParams.fieldParams.isBottomFirst = false;
        }
    }

    return eStatus;
}

}
#endif  // !_DECODE_PROCESSING_SUPPORTED
