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
//! \file     encode_avc_vdenc_fast_pass_xe2_hpm.cpp
//! \brief    Defines the common interface for encode avc Xe2_HPM fast pass feature
//!

#include "encode_avc_vdenc_fastpass_xe2_hpm.h"
#include "encode_avc_vdenc_feature_manager_xe2_hpm.h"

namespace encode
{

AvcVdencFastPass_Xe2_Hpm::AvcVdencFastPass_Xe2_Hpm(
    MediaFeatureManager *featureManager,
    EncodeAllocator *allocator,
    CodechalHwInterfaceNext *hwInterface,
    void *constSettings) :
    AvcVdencFastPass(featureManager, allocator, hwInterface, constSettings)
{
    auto encFeatureManager = dynamic_cast<EncodeAvcVdencFeatureManagerXe2_Hpm *>(featureManager);
    ENCODE_CHK_NULL_NO_STATUS_RETURN(encFeatureManager);

    m_basicFeature = dynamic_cast<AvcBasicFeature *>(encFeatureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);

    if (hwInterface)
    {
        m_userSettingPtr = hwInterface->GetOsInterface()->pfnGetUserSettingInstance(hwInterface->GetOsInterface());
    }
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
        m_fastPassShiftIndex = 2; //xe2_hpm HW restriction, only 4x downscaling
        m_fastPassDownScaleType = (uint8_t)outValue_type.Get<int32_t>();
    }
}

MOS_STATUS AvcVdencFastPass_Xe2_Hpm::Update(void *params)
{
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS avcSeqParams = m_basicFeature->m_seqParam;
    ENCODE_CHK_NULL_RETURN(avcSeqParams);

    if (avcSeqParams->GopRefDist > 1)  //xe2_hpm HW restriction, no B frame
    {
        m_enabled = false;
    }

    if (!m_enabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    //xe2_hpm HW input surface restriction
    m_aligned_Width  = MOS_ALIGN_FLOOR(m_basicFeature->m_seqParam->FrameWidth, 256);
    m_aligned_Height = MOS_ALIGN_FLOOR(m_basicFeature->m_seqParam->FrameHeight, 128);

    m_dsWidth  = m_aligned_Width >> m_fastPassShiftIndex;
    m_dsHeight = m_aligned_Height >> m_fastPassShiftIndex;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_SRC_SURFACE_STATE, AvcVdencFastPass_Xe2_Hpm)
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

MHW_SETPAR_DECL_SRC(VDENC_REF_SURFACE_STATE, AvcVdencFastPass_Xe2_Hpm)
{
    ENCODE_FUNC_CALL();

    if (!m_enabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    params.width  = m_dsWidth;
    params.height = m_dsHeight;    

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode