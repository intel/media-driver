/*
* Copyright (c) 2023, Intel Corporation
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
//! \file     encode_hevc_basic_feature_rsvd.h
//! \brief    Defines the common interface for encode hevc basic feature rsvd
//!
#ifndef __ENCODE_HEVC_BASIC_FEATURE_422_H__
#define __ENCODE_HEVC_BASIC_FEATURE_422_H__

#include "encode_basic_feature.h"

namespace encode
{
class HevcBasicFeature422
{
public:
    HevcBasicFeature422() { };

    ~HevcBasicFeature422() { };

    bool GetFeature422Flag(){ return m_is422; }

    MOS_STATUS Init(PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams, PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams);

    MOS_STATUS RegisterMbCodeBuffer(TrackedBuffer *trackedBuf, bool &isRegistered, uint32_t mbCodeSize);

    MOS_STATUS Update422Format(PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams, uint8_t &outputChromaFormat, MOS_FORMAT &reconFormat, bool is10Bit);

    MOS_STATUS Revert422Format(PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams, uint8_t &outputChromaFormat, MOS_FORMAT &reconFormat, bool is10Bit);

protected:
    bool m_is422 = false;

MEDIA_CLASS_DEFINE_END(encode__HevcBasicFeature422)
};
}

#endif  // !__ENCODE_BASIC_FEATURE_RSVD_H__
