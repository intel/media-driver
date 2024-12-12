/*
* Copyright (c) 2021 - 2024, Intel Corporation
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
//! \file     encode_av1_basic_feature_Xe2_Lpm_base.cpp
//! \brief    Defines the Xe2_LPM+ common class for encode av1 basic feature
//!

#include "encode_av1_basic_feature_xe2_lpm_base.h"
#include "encode_av1_vdenc_const_settings_xe2_lpm_base.h"
#include "encode_av1_superres.h"

namespace encode
{

MOS_STATUS Av1BasicFeatureXe2_Lpm_Base::Update(void *params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);
    ENCODE_CHK_STATUS_RETURN(Av1BasicFeature::Update(params));

    EncoderParams* encodeParams = (EncoderParams*)params;

    // raw surface
    if (IS_RGB_FORMAT(m_rawSurface.Format))
    {
        ENCODE_CHK_STATUS_RETURN(m_allocator->UpdateResourceUsageType(&m_rawSurface.OsResource, MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE));
    }

    auto superResFeature = dynamic_cast<Av1SuperRes *>(m_featureManager->GetFeature(Av1FeatureIDs::av1SuperRes));
    ENCODE_CHK_NULL_RETURN(superResFeature);
    if (superResFeature->IsEnabled())
    {
        m_rawSurfaceToEnc          = superResFeature->GetRawSurfaceToEnc();
        m_postCdefReconSurfaceFlag = true;
    }
    if (m_roundingMethod == RoundingMethod::fixedRounding)
    {
        m_roundingMethod = RoundingMethod::lookUpTableRounding;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AVP_SURFACE_STATE, Av1BasicFeatureXe2_Lpm_Base)
{
    ENCODE_CHK_STATUS_RETURN(Av1BasicFeature::MHW_SETPAR_F(AVP_SURFACE_STATE)(params));

    if (!m_is10Bit)
    {
        params.srcFormat = mhw::vdbox::avp::SURFACE_FORMAT::SURFACE_FORMAT_PLANAR4208;
    }
    else
    {
        if (params.surfaceStateId == srcInputPic || params.surfaceStateId == origUpscaledSrc)
        {
            params.srcFormat = mhw::vdbox::avp::SURFACE_FORMAT::SURFACE_FORMAT_P010;
        }
        else
        {
            params.srcFormat = mhw::vdbox::avp::SURFACE_FORMAT::SURFACE_FORMAT_P010VARIANT;
        }
    }

    return MOS_STATUS_SUCCESS;
}

#if _MEDIA_RESERVED
#include "encode_av1_basic_feature_ext.h"
#else
static uint32_t ComputeRdMult(uint16_t qIndex, bool is10Bit)
{
    return RdMultLUT[is10Bit][qIndex];
}
#endif

MHW_SETPAR_DECL_SRC(AVP_PIC_STATE, Av1BasicFeatureXe2_Lpm_Base)
{
    ENCODE_CHK_STATUS_RETURN(Av1BasicFeature::MHW_SETPAR_F(AVP_PIC_STATE)(params));

    params.rdmult = ComputeRdMult(params.baseQindex, m_is10Bit);

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
