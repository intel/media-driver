/*
* Copyright (c) 2023-2026, Intel Corporation
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
//! \file     encode_av1_tile_xe3p_lpm_base.cpp
//! \brief    Defines the common interface for av1 tile
//!

#include "encode_av1_tile_xe3p_lpm_base.h"
#include "encode_av1_vdenc_feature_manager_xe3p_lpm_base.h"
#include "codec_def_common_av1.h"
#include "encode_av1_basic_feature_xe3p_lpm_base.h"

namespace encode
{
    Av1EncodeTile_Xe3P_Lpm_Base::Av1EncodeTile_Xe3P_Lpm_Base(
        MediaFeatureManager *featureManager,
        EncodeAllocator *allocator,
        CodechalHwInterfaceNext *hwInterface,
        void *constSettings) : Av1EncodeTile(featureManager, allocator, hwInterface, constSettings)
    {
        ENCODE_FUNC_CALL();
        auto encFeatureManager = dynamic_cast<EncodeAv1VdencFeatureManager *>(featureManager);
        ENCODE_CHK_NULL_NO_STATUS_RETURN(encFeatureManager);

        m_basicFeature = dynamic_cast<Av1BasicFeature *>(encFeatureManager->GetFeature(Av1FeatureIDs::basicFeature));
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);
    }

    MHW_SETPAR_DECL_SRC(VDENC_CMD2, Av1EncodeTile_Xe3P_Lpm_Base)
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

                uint32_t newFCost1 = bufData->DW1.IntraCuCountNormalized / frameSizeIn8x8Units1;

                m_allocator->UnLock(buf);

                if (newFCost1 >= 2)
                {
                    av1BasicFeature->m_par65Inter = 7;
                }
                else if (newFCost1 == 0)
                {
                    av1BasicFeature->m_par65Inter = 3;
                }
            }

            av1BasicFeature->m_par65Intra = 15;

            if (av1Picparams->base_qindex <= 100 || frameSizeIn8x8Units2 < 5000)
            {
                av1BasicFeature->m_par65Intra = 13;
            }

            if (av1Picparams->PicFlags.fields.frame_type != keyFrame)
            {
                av1BasicFeature->m_par65Intra = 11;
            }
        }
        else if (av1BasicFeature->m_roundingMethod == RoundingMethod::fixedRounding)
        {
            av1BasicFeature->m_par65Inter = 5;
            av1BasicFeature->m_par65Intra = 13;
            if (av1Picparams->PicFlags.fields.frame_type != keyFrame)
            {
                av1BasicFeature->m_par65Intra = 11;
            }
        }


        if(av1Picparams->PicFlags.fields.frame_type == keyFrame)
        {
            av1BasicFeature->m_par65Inter = av1BasicFeature->m_par65Intra;
        }
        
        // m_par65Inter/m_par65Intra are passed to HuC via DMEM.

        return MOS_STATUS_SUCCESS;
    }

    }  // namespace encode
