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
//! \file     encode_av1_fastpass_xe2_hpm.cpp
//! \brief    Fast Pass feature
//!

#include "encode_av1_fastpass_xe2_hpm.h"

namespace encode
{
Av1FastPass_Xe2_Hpm::Av1FastPass_Xe2_Hpm(MediaFeatureManager *featureManager,
    EncodeAllocator                          *allocator,
    CodechalHwInterfaceNext                  *hwInterface,
    void                                     *constSettings): 
    Av1FastPass(featureManager,
    allocator,
    hwInterface,
    constSettings)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_NO_STATUS_RETURN(featureManager);

        m_basicFeature = dynamic_cast<Av1BasicFeatureXe2_Hpm *>(featureManager->GetFeature(Av1FeatureIDs::basicFeature));
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);

        //regkey to control fast pass encode settings
        MediaUserSetting::Value outValue;
        ReadUserSetting(m_userSettingPtr,
            outValue,
            "Enable Fast Pass Encode",
            MediaUserSetting::Group::Sequence);

        m_enabled = outValue.Get<bool>();

        if (m_enabled)
        {
            MediaUserSetting::Value outValue_ratio;
            MediaUserSetting::Value outValue_type;
#if (_DEBUG || _RELEASE_INTERNAL)
            ReadUserSetting(m_userSettingPtr,
                outValue_type,
                "Fast Pass Encode Downscale Type",
                MediaUserSetting::Group::Sequence);
#endif
            // Xe2_Hpm HW restrictions, only 4x downscale
            m_fastPassDownScaleRatio = 2;
            m_fastPassDownScaleType  = (uint8_t)outValue_type.Get<int32_t>();
        }
    }

    Av1FastPass_Xe2_Hpm::~Av1FastPass_Xe2_Hpm()
    {
        ENCODE_FUNC_CALL();
    }

    MOS_STATUS Av1FastPass_Xe2_Hpm::Update(void *params)
    {
        ENCODE_CHK_NULL_RETURN(m_basicFeature);
        PCODEC_AV1_ENCODE_PICTURE_PARAMS av1PicParams = m_basicFeature->m_av1PicParams;
        ENCODE_CHK_NULL_RETURN(av1PicParams);

        // Xe2_Hpm HW restrictions for input surfaces, post-downscale 64x32 aligned
        m_aligned_Width = MOS_ALIGN_FLOOR(av1PicParams->frame_width_minus1 + 1, 256);
        m_aligned_Height = MOS_ALIGN_FLOOR(av1PicParams->frame_height_minus1 + 1, 128);
        m_dsWidth = m_aligned_Width >> m_fastPassDownScaleRatio;
        m_dsHeight = m_aligned_Height >> m_fastPassDownScaleRatio;
     
        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_SRC_SURFACE_STATE, Av1FastPass_Xe2_Hpm)
    {
        ENCODE_FUNC_CALL();

        if (!m_enabled)
        {
            return MOS_STATUS_SUCCESS;
        }

        params.width  = m_aligned_Width;
        params.height = m_aligned_Height;

        return MOS_STATUS_SUCCESS;
    }

}  // namespace encode
