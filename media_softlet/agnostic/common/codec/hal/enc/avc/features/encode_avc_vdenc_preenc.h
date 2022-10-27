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
//! \file     encode_avc_vdenc_preenc.h
//! \brief    Defines the interface for encode avc preenc feature
//!
#ifndef __ENCODE_AVC_VDENC_PREENC_H__
#define __ENCODE_AVC_VDENC_PREENC_H__

#include "mhw_vdbox_vdenc_itf.h"
#include "encode_mem_compression.h"
#include "encode_avc_basic_feature.h"
#include "encode_preenc_basic_feature.h"

namespace encode
{
class AvcVdencPreEnc : public PreEncBasicFeature
{
public:
    AvcVdencPreEnc(MediaFeatureManager *featureManager,
        EncodeAllocator                 *allocator,
        CodechalHwInterfaceNext         *hwInterface,
        TrackedBuffer                   *trackedBuf,
        RecycleResource                 *recycleBuf,
        void                            *constSettings = nullptr);

    virtual ~AvcVdencPreEnc();

    virtual MOS_STATUS CalculatePreEncInfo(uint32_t width, uint32_t height, PreEncInfo &preEncInfo) override;

protected:
    virtual MOS_STATUS PreparePreEncConfig(void *params) override;

    const uint8_t HevcSliceType[10] = {encodeHevcBSlice, encodeHevcBSlice, encodeHevcISlice, encodeHevcBSlice, encodeHevcISlice, encodeHevcBSlice, encodeHevcBSlice, encodeHevcISlice, encodeHevcBSlice, encodeHevcISlice};

    EncodeAllocator *m_allocator = nullptr;

    AvcBasicFeature *m_basicFeature = nullptr;  //!< EncodeBasicFeature

    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS m_avcSeqParams   = nullptr;  //!< Pointer to sequence parameter
    PCODEC_AVC_ENCODE_PIC_PARAMS      m_avcPicParams   = nullptr;  //!< Pointer to picture parameter
    PCODEC_AVC_ENCODE_SLICE_PARAMS    m_avcSliceParams = nullptr;  //!< Pointer to slice parameter

    MEDIA_CLASS_DEFINE_END(encode__AvcVdencPreEnc)
};

}  // namespace encode

#endif  // !__ENCODE_AVC_VDENC_PREENC_H__
