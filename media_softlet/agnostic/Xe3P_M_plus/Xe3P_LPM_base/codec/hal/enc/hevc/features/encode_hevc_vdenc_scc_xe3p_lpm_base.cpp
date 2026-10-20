/*
* Copyright (c) 2023-2026, Intel Corporation
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
//! \file     encode_hevc_vdenc_scc_xe3p_lpm_base.cpp
//! \brief    Defines the Xe3P_LPM_Base interface for hevc encode scc features
//!

#include "encode_hevc_basic_feature.h"
#include "encode_hevc_vdenc_scc_xe3p_lpm_base.h"
#include "encode_hevc_vdenc_feature_manager.h"
namespace encode
{

HevcVdencSccXe3P_Lpm_Base::HevcVdencSccXe3P_Lpm_Base(
    MediaFeatureManager *featureManager,
    EncodeAllocator *allocator,
    CodechalHwInterfaceNext *hwInterface,
    void *constSettings) : HevcVdencScc(featureManager, allocator, hwInterface, constSettings)
{
}

MOS_STATUS HevcVdencSccXe3P_Lpm_Base::UpdateSlotForRecNotFiltered()
{
    ENCODE_FUNC_CALL();

    if (m_enableSCC)
    {
        auto hevcFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(hevcFeature);
        if (hevcFeature->m_hevcPicParams &&
            hevcFeature->m_hevcPicParams->pps_curr_pic_ref_enabled_flag)
        {
            ENCODE_CHK_STATUS_RETURN(
                hevcFeature->m_ref.SetSlotForRecNotFiltered(m_slotForRecNotFiltered));
        }
    }
    return MOS_STATUS_SUCCESS;
}

bool HevcVdencSccXe3P_Lpm_Base::IsCompressFlagNeeded()
{
    auto skuTable = m_osInterface->pfnGetSkuTable(m_osInterface);
    if (skuTable && MEDIA_IS_SKU(skuTable, FtrXe2Compression))
    {
        return false;
    }
    return true;
}

}  // namespace encode
