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
//! \file     encode_hevc_vdenc_fastpass_xe2_hpm_base.cpp
//! \brief    Defines the Xe2_HPM+ Base interface for hevc encode fastpass features
//!

#include "encode_hevc_vdenc_fastpass_xe2_hpm_base.h"
#include "encode_hevc_vdenc_feature_manager.h"

namespace encode
{

HevcVdencFastPass_Xe2_Hpm_Base::HevcVdencFastPass_Xe2_Hpm_Base(
    MediaFeatureManager     *featureManager,
    EncodeAllocator         *allocator,
    CodechalHwInterfaceNext *hwInterface,
    void                    *constSettings) : HevcVdencFastPass(featureManager, allocator, hwInterface, constSettings)
{
    ENCODE_FUNC_CALL();
    auto encFeatureManager = dynamic_cast<EncodeHevcVdencFeatureManager *>(featureManager);
    ENCODE_CHK_NULL_NO_STATUS_RETURN(encFeatureManager);

    m_hevcFeature = dynamic_cast<HevcBasicFeature *>(encFeatureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_hevcFeature);

    //regkey to control fast pass encode settings
    MediaUserSetting::Value outValue;
    ReadUserSetting(m_userSettingPtr,
        outValue,
        "Enable Fast Pass Encode",
        MediaUserSetting::Group::Sequence);

    m_enableFastPass = outValue.Get<bool>();
    if (m_enableFastPass)
    {
        MediaUserSetting::Value outValue_ratio;
        MediaUserSetting::Value outValue_type;
#if (_DEBUG || _RELEASE_INTERNAL)
        ReadUserSetting(m_userSettingPtr,
            outValue_type,
            "Fast Pass Encode Downscale Type",
            MediaUserSetting::Group::Sequence);
#endif
        m_fastPassShiftIndex = 2; //Xe2_Hpm HW restrictions.
        m_fastPassDownScaleType = (uint8_t)outValue_type.Get<int32_t>();
    }

}

MOS_STATUS HevcVdencFastPass_Xe2_Hpm_Base::Update(void *params)
{
    if (!m_enableFastPass)
    {
        return MOS_STATUS_SUCCESS;
    }

    ENCODE_CHK_NULL_RETURN(m_hevcFeature);
    m_hevcSeqParams = m_hevcFeature->m_hevcSeqParams;
    ENCODE_CHK_NULL_RETURN(m_hevcSeqParams);
    
    //Xe2_Hpm HW restrictions.
    m_aligned_Width = MOS_ALIGN_FLOOR((m_hevcSeqParams->wFrameWidthInMinCbMinus1 + 1) << (m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3), 256);
    m_aligned_Height = MOS_ALIGN_FLOOR((m_hevcSeqParams->wFrameHeightInMinCbMinus1 + 1) << (m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3), 128);

    m_dsWidth  = m_aligned_Width >> m_fastPassShiftIndex;
    m_dsHeight = m_aligned_Height >> m_fastPassShiftIndex;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_SRC_SURFACE_STATE, HevcVdencFastPass_Xe2_Hpm_Base)
{
    ENCODE_FUNC_CALL();

    if (!m_enableFastPass)
    {
        return MOS_STATUS_SUCCESS;
    }

    params.width  = m_aligned_Width;
    params.height = m_aligned_Height;

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
