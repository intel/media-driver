/*
* Copyright (c) 2020 - 2023, Intel Corporation
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
//! \file     encode_av1_superres.h
//! \brief    Super-res feature
//!

#ifndef __ENCODE_AV1_SUPERRES_H__
#define __ENCODE_AV1_SUPERRES_H__

#include "encode_av1_basic_feature.h"

namespace encode
{
class Av1SuperRes : public MediaFeature, public mhw::vdbox::vdenc::Itf::ParSetting, public mhw::vdbox::avp::Itf::ParSetting
{
public:
    Av1SuperRes(MediaFeatureManager *featureManager,
        EncodeAllocator             *allocator,
        void                        *constSettings);

    virtual MOS_STATUS Update(void *params) override;

    uint32_t GetUpscaledWidth() const { return m_widthUpscaled; }

    MHW_SETPAR_DECL_HDR(AVP_PIC_STATE);

    MHW_SETPAR_DECL_HDR(AVP_INLOOP_FILTER_STATE);

protected:

    EncodeAllocator     *m_allocator      = nullptr;
    Av1BasicFeature     *m_basicFeature   = nullptr;  //!< EncodeBasicFeature
    TrackedBuffer       *m_trackedBuf     = nullptr;

    bool                 m_useSuperRes     = false;
    uint32_t             m_widthUpscaled   = 0;
    uint32_t             m_widthDownscaled = 0;
    uint8_t              m_superResDenom   = 0;
    int8_t               m_subsamplingX[3] = {};
    MediaFeatureManager *m_featureManager  = nullptr;

MEDIA_CLASS_DEFINE_END(encode__Av1SuperRes)
};
}  // namespace encode

#endif  // __ENCODE_AV1_SUPERRES_H__
