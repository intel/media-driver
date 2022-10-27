/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     encode_avc_vdenc_weighted_prediction.h
//! \brief    Defines for avc weighted prediction feature
//!
#ifndef __ENCODE_AVC_VDENC_WEIGHTED_PREDICTION_H__
#define __ENCODE_AVC_VDENC_WEIGHTED_PREDICTION_H__

#include "media_feature.h"
#include "encode_allocator.h"
#include "codec_hw_next.h"
#include "encode_avc_basic_feature.h"
#include "mhw_vdbox_vdenc_itf.h"
#include "mhw_vdbox_mfx_itf.h"

namespace encode
{

class AvcVdencWeightedPred : public MediaFeature, public mhw::vdbox::vdenc::Itf::ParSetting, public mhw::vdbox::mfx::Itf::ParSetting
{
public:
    AvcVdencWeightedPred(
        MediaFeatureManager *featureManager,
        EncodeAllocator *allocator,
        CodechalHwInterfaceNext *hwInterface,
        void *constSettings);

    virtual ~AvcVdencWeightedPred() {}

    MHW_SETPAR_DECL_HDR(VDENC_WEIGHTSOFFSETS_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_AVC_IMG_STATE);

    MHW_SETPAR_DECL_HDR(MFX_AVC_WEIGHTOFFSET_STATE);

protected:
    AvcBasicFeature *m_basicFeature = nullptr;  //!< EncodeBasicFeature

MEDIA_CLASS_DEFINE_END(encode__AvcVdencWeightedPred)
};

}  // namespace encode

#endif  // !__ENCODE_AVC_VDENC_WEIGHTED_PREDICTION_H__
