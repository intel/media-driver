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
//! \file     encode_av1_vdenc_preenc.h
//! \brief    Defines the interface for encode av1 preenc feature
//!
#ifndef __ENCODE_AV1_VDENC_PREENC_H__
#define __ENCODE_AV1_VDENC_PREENC_H__

#include "mhw_vdbox_vdenc_itf.h"
#include "encode_mem_compression.h"
#include "encode_preenc_basic_feature.h"
#include "encode_av1_basic_feature.h"
#include "codechal_debug.h"

namespace encode
{

class Av1VdencPreEnc : public PreEncBasicFeature
{
public:
    Av1VdencPreEnc(MediaFeatureManager *featureManager,
                    EncodeAllocator     *allocator,
                    CodechalHwInterfaceNext *hwInterface,
                    TrackedBuffer       *trackedBuf,
                    RecycleResource     *recycleBuf,
                    void                *constSettings = nullptr);

    virtual ~Av1VdencPreEnc() {};

protected:
    virtual MOS_STATUS PreparePreEncConfig(void *params) override;

    Av1BasicFeature *m_basicFeature = nullptr;  //!< AV1 paramter

    const uint8_t HevcSliceType[3] = {encodeHevcISlice, encodeHevcBSlice, encodeHevcBSlice};

    uint8_t m_orderHintOffset[ENCODE_AV1_ORDER_HINT_SIZE];

MEDIA_CLASS_DEFINE_END(encode__Av1VdencPreEnc)
};

}  // namespace encode

#endif  // !__ENCODE_AV1_VDENC_PREENC_H__
