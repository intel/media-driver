/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     encode_avc_aqm.h
//! \brief    Defines the common interface for avc aqm
//!
#ifndef __ENCODE_AVC_AQM_H__
#define __ENCODE_AVC_AQM_H__

#include "encode_aqm_feature.h"
#include "mhw_vdbox_vdenc_itf.h"
#include "mhw_vdbox_mfx_itf.h"

namespace encode
{
class AvcEncodeAqm : public EncodeAqmFeature, public mhw::vdbox::mfx::Itf::ParSetting, public mhw::vdbox::vdenc::Itf::ParSetting
{
public:
    AvcEncodeAqm(MediaFeatureManager *featureManager,
        EncodeAllocator *             allocator,
        CodechalHwInterfaceNext *     hwInterface,
        void *                        constSettings);

    virtual ~AvcEncodeAqm(){};

    virtual MOS_STATUS Update(void *params) override;

    virtual MOS_STATUS Init(void *setting) override;

#if USE_CODECHAL_DEBUG_TOOL
    virtual MOS_STATUS UpdateFrameDisplayOrder(const uint16_t pictureCodingType, const uint32_t framePOC, const uint32_t gopPicSize) override;
#endif

    MHW_SETPAR_DECL_HDR(AQM_PIC_STATE);
    MHW_SETPAR_DECL_HDR(AQM_SLICE_STATE);
    MHW_SETPAR_DECL_HDR(MFX_AVC_IMG_STATE);

MEDIA_CLASS_DEFINE_END(encode__AvcEncodeAqm)
};

}  // namespace encode

#endif  // !__ENCODE_AVC_AQM_H__
