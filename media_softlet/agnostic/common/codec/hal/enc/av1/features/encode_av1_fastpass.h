/*
* Copyright (c) 2023-2024, Intel Corporation
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
//! \file     encode_av1_fast_pass.h
//! \brief    Fast Pass feature
//!

#ifndef __ENCODE_AV1_FAST_PASS_H__
#define __ENCODE_AV1_FAST_PASS_H__

#include "encode_av1_basic_feature.h"

namespace encode
{


class Av1FastPass : public MediaFeature, public mhw::vdbox::vdenc::Itf::ParSetting, public mhw::vdbox::avp::Itf::ParSetting
{
public:
    Av1FastPass(
        MediaFeatureManager     *featureManager,
        EncodeAllocator         *allocator,
        CodechalHwInterfaceNext *hwInterface,
        void                    *constSettings);

    virtual ~Av1FastPass();

    //!
    //! \brief  Update encode parameter
    //! \param  [in] params
    //!         Pointer to parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Update(void *params) override;

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_MODE_SELECT);

    MHW_SETPAR_DECL_HDR(VDENC_CMD2);

    MHW_SETPAR_DECL_HDR(VDENC_HEVC_VP9_TILE_SLICE_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_WALKER_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_REF_SURFACE_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_DS_REF_SURFACE_STATE);

    MHW_SETPAR_DECL_HDR(AVP_TILE_CODING);

    MHW_SETPAR_DECL_HDR(AVP_PIC_STATE);

    MHW_SETPAR_DECL_HDR(AVP_SURFACE_STATE);

protected:

    uint8_t m_fastPassDownScaleRatio = 0;      //!< downscale ratio for fast pass encode
    uint8_t m_fastPassDownScaleType  = 0;      //!< downscale Type for fast pass encode
    uint32_t m_dsWidth                = 0;
    uint32_t m_dsHeight               = 0;

    Av1BasicFeature *m_basicFeature = nullptr;  //!< AV1 Basic Feature

#define AV1_FAST_PASS_4X_DS(value) ((value) != 1)

MEDIA_CLASS_DEFINE_END(encode__Av1FastPass)
};
}  // namespace encode

#endif  // __ENCODE_AV1_FASTPASS_H__
