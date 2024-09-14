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
//! \file     encode_avc_vdenc_fastpass.h
//! \brief    Defines the common interface for encode avc Xe2_HPM+ basic feature
//!
#ifndef __ENCODE_AVC_VDENC_FASTPASS_H__
#define __ENCODE_AVC_VDENC_FASTPASS_H__

#include "media_feature.h"
#include "encode_allocator.h"
#include "encode_basic_feature.h"
#include "mhw_vdbox_vdenc_itf.h"
#include "encode_avc_basic_feature.h"
#include "encode_avc_vdenc_const_settings.h"

namespace encode
{

class AvcVdencFastPass : public MediaFeature, public mhw::vdbox::vdenc::Itf::ParSetting, public mhw::vdbox::mfx::Itf::ParSetting
{
public:
    AvcVdencFastPass(MediaFeatureManager *featureManager,
                      EncodeAllocator *allocator,
                      CodechalHwInterfaceNext *hwInterface,
                      void *constSettings);

    virtual ~AvcVdencFastPass() {}

    //!
    //! \brief  Update encode parameter
    //! \param  [in] params
    //!         Pointer to parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Update(void *params) override;

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_MODE_SELECT);

    MHW_SETPAR_DECL_HDR(VDENC_REF_SURFACE_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_AVC_IMG_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_WALKER_STATE);

    MHW_SETPAR_DECL_HDR(MFX_AVC_IMG_STATE);

    MHW_SETPAR_DECL_HDR(MFX_AVC_SLICE_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_DS_REF_SURFACE_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_CMD3);

protected:
    PMOS_INTERFACE m_osInterface              = nullptr;  //!< Os Inteface
    MOS_RESOURCE   m_vdencRecDownScaledBuffer = {};
    uint32_t       m_dsWidth                  = 0;
    uint32_t       m_dsHeight                 = 0;

    uint8_t m_fastPassShiftIndex     = 0;      //!< shift index for fast pass encode
    uint8_t m_fastPassDownScaleType  = 0;      //!< downscaleType for fast pass encode

    AvcBasicFeature *m_basicFeature = nullptr;  //!< AvcBasicFeature
    AvcVdencCMD3ConstSettings m_CMD3Settings;

MEDIA_CLASS_DEFINE_END(encode__AvcVdencFastPass)
};

}  // namespace encode

#endif  // !__ENCODE_AVC_VDENC_FASTPASS_H__