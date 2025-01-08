/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     encode_av1_rounding_table.cpp
//! \brief    Defines the interface for rounding table settings
//!

#include "encode_av1_rounding_table.h"
#include "encode_av1_basic_feature.h"
#include "encode_av1_vdenc_feature_manager.h"

namespace encode
{

uint16_t GetRandomValue(uint16_t v0, uint16_t v1, uint16_t v2, uint16_t v3)
{
    uint16_t tmpVar = (v0 + v3) / v1;
    return (tmpVar > v2) ? v2 : tmpVar;
}

bool ConditionVaild(uint8_t v0, uint8_t v1, uint8_t v2, uint8_t v3)
{
    return v1 == v3 && v2 < v0;
}

Av1EncodeRoundingTable::Av1EncodeRoundingTable(MediaFeatureManager *featureManager,
    CodechalHwInterfaceNext                                        *hwInterface,
    void                                                           *constSettings)
    : MediaFeature(constSettings, hwInterface ? hwInterface->GetOsInterface() : nullptr)
{
    m_hwInterface    = hwInterface;
    m_featureManager = featureManager;
    auto encFeatureManager = dynamic_cast<EncodeAv1VdencFeatureManager *>(featureManager);
    ENCODE_CHK_NULL_NO_STATUS_RETURN(encFeatureManager);

    m_basicFeature = dynamic_cast<EncodeBasicFeature *>(encFeatureManager->GetFeature(Av1FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);
}

MHW_SETPAR_DECL_SRC(VDENC_CMD2, Av1EncodeRoundingTable)
{
    ENCODE_FUNC_CALL();
    auto av1BasicFeature = dynamic_cast<Av1BasicFeature *>(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(av1BasicFeature);

    if (av1BasicFeature->m_roundingMethod == 2)
    {
        ENCODE_CHK_NULL_RETURN(av1BasicFeature->m_av1PicParams);
        ENCODE_CHK_NULL_RETURN(av1BasicFeature->m_av1SeqParams);

        // Need optimize if random value is not reasonable
        uint16_t rdmVal       = GetRandomValue(av1BasicFeature->m_av1PicParams->base_qindex, 5, 51, 2);
        uint8_t  var0         = av1BasicFeature->m_av1SeqParams->GopRefDist;
        uint8_t  var1         = av1BasicFeature->m_av1PicParams->order_hint;
        uint8_t  var2         = var0 != 0 ? var1 % var0 : 0;
        uint8_t  var3         = var0 <= 1 ? 1 : 0;
        bool     isValidFType = av1BasicFeature->m_av1PicParams->PicFlags.fields.frame_type;
        bool     isLowDelay   = av1BasicFeature->m_ref.IsLowDelay();
        // Optimize threshold if needed
        uint8_t  threshold0   = 0;
        uint8_t  threshold1   = 1;
        uint8_t  threshold2   = 8;

        int idx = 0;
        
        if (!isValidFType)
        {}
        else if (isLowDelay)
        {
            if (ConditionVaild(threshold2, var3, var0, threshold1))
            {
                idx = 1;
            }
            else
            {
                idx = (var0 < threshold2) ? ((var2 == threshold0) ? 2 : 3) : ((var2 == threshold0) ? 4 : 5);
            }
        }
        else
        {
            idx = 6;
        }

        if (!idx)
        {
            av1BasicFeature->m_par65Inter = uint8_t((~Par65Values[rdmVal]) >> 2);
            av1BasicFeature->m_par65Intra = uint8_t((~Par65Values[rdmVal]) >> 2);
        }
        else if (5 == idx || 6 == idx)
        {
            av1BasicFeature->m_par65Inter = uint8_t((~Par65Values[rdmVal + 260]) >> 2);
            av1BasicFeature->m_par65Intra = uint8_t((~Par65Values[rdmVal + 520]) >> 2);
        }
        else
        {
            av1BasicFeature->m_par65Inter = uint8_t((~Par65Values[rdmVal + idx * (51 + 1)]) >> 2);
            av1BasicFeature->m_par65Intra = uint8_t((~Par65Values[rdmVal + 624 - (7 - idx) * (51 + 1)]) >> 2);
        }

#if _MEDIA_RESERVED
        for (auto i = 0; i < 3; i++)
        {
            params.vdencCmd2Par65[i][0][0] = av1BasicFeature->m_par65Intra;
            params.vdencCmd2Par65[i][0][1] = av1BasicFeature->m_par65Intra;
            params.vdencCmd2Par65[i][1][0] = av1BasicFeature->m_par65Inter;
            params.vdencCmd2Par65[i][1][1] = av1BasicFeature->m_par65Inter;
            params.vdencCmd2Par65[i][2][0] = av1BasicFeature->m_par65Inter;
            params.vdencCmd2Par65[i][2][1] = av1BasicFeature->m_par65Inter;
        }
#else
        params.extSettings.emplace_back(
            [av1BasicFeature](uint32_t *data) {
                uint8_t tmp0 = av1BasicFeature->m_par65Intra & 0xf;
                uint8_t tmp1 = av1BasicFeature->m_par65Inter & 0xf;
                data[32] |= (tmp1 << 16);
                data[32] |= (tmp1 << 20);
                data[32] |= (tmp0 << 24);
                data[32] |= (tmp0 << 28);
                data[33] |= tmp1;
                data[33] |= (tmp1 << 4);
                data[33] |= (tmp1 << 8);
                data[33] |= (tmp1 << 12);
                data[33] |= (tmp0 << 16);
                data[33] |= (tmp0 << 20);
                data[33] |= (tmp1 << 24);
                data[33] |= (tmp1 << 28);
                data[34] |= tmp1;
                data[34] |= (tmp1 << 4);
                data[34] |= (tmp0 << 8);
                data[34] |= (tmp0 << 12);
                data[34] |= (tmp1 << 16);
                data[34] |= (tmp1 << 20);
                return MOS_STATUS_SUCCESS;
            });
#endif  // _MEDIA_RESERVED
    }

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode