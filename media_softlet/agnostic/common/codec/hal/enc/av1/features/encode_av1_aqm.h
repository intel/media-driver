/*
* Copyright (c) 2021-2023, Intel Corporation
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
//! \file     encode_av1_aqm.h
//! \brief    Defines the common interface for av1 aqm
//!
#ifndef __ENCODE_AV1_AQM_H__
#define __ENCODE_AV1_AQM_H__
#include <array>

#include "encode_aqm_feature.h"
#include "mhw_vdbox_vdenc_itf.h"
#include "mhw_vdbox_avp_itf.h"
#include "codec_def_encode_av1.h"

namespace encode
{
class Av1EncodeAqm : public EncodeAqmFeature, public mhw::vdbox::avp::Itf::ParSetting, public mhw::vdbox::vdenc::Itf::ParSetting
{
public:
    Av1EncodeAqm(MediaFeatureManager *featureManager,
        EncodeAllocator *             allocator,
        CodechalHwInterfaceNext *         hwInterface,
        void *                        constSettings);

    virtual ~Av1EncodeAqm(){};

    virtual MOS_STATUS Update(void *params) override;

    MHW_SETPAR_DECL_HDR(AQM_PIC_STATE);

    MHW_SETPAR_DECL_HDR(AVP_PIC_STATE);

    MHW_SETPAR_DECL_HDR(AQM_TILE_CODING);

    MHW_SETPAR_DECL_HDR(AQM_SLICE_STATE);

    MHW_SETPAR_DECL_HDR(VD_PIPELINE_FLUSH);

    MHW_SETPAR_DECL_HDR(AQM_PIPE_BUF_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(AQM_SURFACE_STATE);
MEDIA_CLASS_DEFINE_END(encode__Av1EncodeAqm)
};

}  // namespace encode

#endif  // !__ENCODE_AV1_AQM_H__
