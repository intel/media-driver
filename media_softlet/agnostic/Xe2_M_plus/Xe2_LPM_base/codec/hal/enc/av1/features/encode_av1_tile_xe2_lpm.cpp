/*
* Copyright (c) 2023, Intel Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,Av1EncodeTile
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
//! \file     encode_av1_tile_xe2_lpm.cpp
//! \brief    Defines the common interface for av1 tile
//!

#include "encode_av1_tile_xe2_lpm.h"
#include "encode_av1_vdenc_feature_manager_xe2_lpm_base.h"
#include "codec_def_common_av1.h"
#include "encode_av1_basic_feature_xe2_lpm_base.h"

namespace encode
{
    Av1EncodeTile_Xe2_Lpm::Av1EncodeTile_Xe2_Lpm(
        MediaFeatureManager *featureManager,
        EncodeAllocator *allocator,
        CodechalHwInterfaceNext *hwInterface,
        void *constSettings) : Av1EncodeTile(featureManager, allocator, hwInterface, constSettings)
    {
        auto encFeatureManager = dynamic_cast<EncodeAv1VdencFeatureManagerXe2_Lpm_Base *>(featureManager);
        ENCODE_CHK_NULL_NO_STATUS_RETURN(encFeatureManager);

        m_basicFeature = dynamic_cast<Av1BasicFeatureXe2_Lpm_Base *>(encFeatureManager->GetFeature(Av1FeatureIDs::basicFeature));
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);
    }

    MHW_SETPAR_DECL_SRC(VDENC_CMD2, Av1EncodeTile_Xe2_Lpm)
    {
        ENCODE_FUNC_CALL();
        auto av1BasicFeature = dynamic_cast<Av1BasicFeature *>(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(av1BasicFeature);
        auto av1Picparams = av1BasicFeature->m_av1PicParams;
        ENCODE_CHK_NULL_RETURN(av1Picparams);
        auto av1Seqparams = av1BasicFeature->m_av1SeqParams;
        ENCODE_CHK_NULL_RETURN(av1Seqparams);

        // for BRC, adaptive rounding is done in HuC, so we can skip it here.
        if (av1BasicFeature->m_roundingMethod == RoundingMethod::adaptiveRounding && !IsRateControlBrc(av1Seqparams->RateControlMethod))
        {
            // Assue encoding 8K frame, Width * Height is still less than 0xFFFFFFFF(max of uint32_t), no overflow risk.
            uint32_t frameSizeIn8x8Units1 = ((av1BasicFeature->m_oriFrameWidth + 63) >> 6) * ((av1BasicFeature->m_oriFrameHeight + 63) >> 6);
            uint32_t frameSizeIn8x8Units2 = ((av1BasicFeature->m_oriFrameWidth + 7) >> 3) * ((av1BasicFeature->m_oriFrameHeight + 7) >> 3);
            av1BasicFeature->m_par65Inter = 2;

            if (av1BasicFeature->m_encodedFrameNum > 0)
            {
                MOS_RESOURCE* buf = nullptr;

                ENCODE_CHK_STATUS_RETURN(GetTileBasedStatisticsBuffer(m_prevStatisticsBufIndex, buf));
                ENCODE_CHK_NULL_RETURN(buf);

                //will be optimized to avoid perf degradation when async > 1
                const auto* bufData = (VdencStatistics*)((uint8_t*)m_allocator->LockResourceForRead(buf) + m_av1TileStatsOffset.uiVdencStatistics);
                ENCODE_CHK_NULL_RETURN(bufData);

                //No risk of dividing by 0.
                uint32_t newFCost1 = bufData->DW1.IntraCuCountNormalized / frameSizeIn8x8Units1;

                m_allocator->UnLock(buf);

                if (newFCost1 >= 2)
                {
                    av1BasicFeature->m_par65Inter = 3;
                }
                else if (newFCost1 == 0)
                {
                    av1BasicFeature->m_par65Inter = 1;
                }
            }

            av1BasicFeature->m_par65Intra = 7;

            if (av1Picparams->base_qindex <= 100 || frameSizeIn8x8Units2 < 5000)
            {
                av1BasicFeature->m_par65Intra = 6;
            }

            if (av1Picparams->PicFlags.fields.frame_type != keyFrame)
            {
                av1BasicFeature->m_par65Intra = 5;
            }
        }
        else if(av1BasicFeature->m_roundingMethod == RoundingMethod::fixedRounding)
        {
            av1BasicFeature->m_par65Inter = 2;
            if (av1BasicFeature->m_av1SeqParams->GopRefDist == 1)
            {
                av1BasicFeature->m_par65Intra = 6;
            }
            else
            {
                av1BasicFeature->m_par65Intra = 5;
            }
        }


        if(av1Picparams->PicFlags.fields.frame_type == keyFrame)
        {
            av1BasicFeature->m_par65Inter = av1BasicFeature->m_par65Intra;
        }
        
        if (av1BasicFeature->m_roundingMethod == RoundingMethod::adaptiveRounding
            || av1BasicFeature->m_roundingMethod == RoundingMethod::fixedRounding)
        {
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
