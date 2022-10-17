/*
* Copyright (c) 2018-2020, Intel Corporation
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
//! \file     encode_hevc_vdenc_weighted_prediction.cpp
//! \brief    Implemetation for hevc weighted prediction feature
//!

#include "encode_hevc_vdenc_weighted_prediction.h"
#include "encode_hevc_vdenc_feature_manager.h"
namespace encode
{
    HevcVdencWeightedPred::HevcVdencWeightedPred(
        MediaFeatureManager *featureManager,
        EncodeAllocator *allocator,
        CodechalHwInterfaceNext *hwInterface,
        void *constSettings) :
        MediaFeature(constSettings)
    {
        ENCODE_CHK_NULL_NO_STATUS_RETURN(featureManager);

        m_basicFeature = dynamic_cast<HevcBasicFeature *>(featureManager->GetFeature(FeatureIDs::basicFeature));
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);
    }

    MOS_STATUS HevcVdencWeightedPred::Update(void *params)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(params);

        EncoderParams *encodeParams = (EncoderParams *)params;

        auto hevcPicParams = static_cast<PCODEC_HEVC_ENCODE_PICTURE_PARAMS>(encodeParams->pPicParams);
        ENCODE_CHK_NULL_RETURN(hevcPicParams);

        m_hevcSliceParams = static_cast<PCODEC_HEVC_ENCODE_SLICE_PARAMS>(encodeParams->pSliceParams);
        ENCODE_CHK_NULL_RETURN(m_hevcSliceParams);

        if (hevcPicParams->weighted_pred_flag || hevcPicParams->weighted_bipred_flag)
        {
            m_enabled = true;
        }

        m_bEnableGPUWeightedPrediction = m_enabled && hevcPicParams->bEnableGPUWeightedPrediction;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencWeightedPred::SetHucBrcUpdateDmemBuffer(
        bool isFirstPass,
        VdencHevcHucBrcUpdateDmem &dmem)
    {
        ENCODE_FUNC_CALL();

        // 1: BRC (including ACQP), 2: Weighted prediction (should not be enabled in first pass)
        // 01: BRC, 10: WP never used,  11: BRC + WP
        dmem.OpMode_U8 = (m_bEnableGPUWeightedPrediction && !isFirstPass) ? 3 : 1;

        if (m_enabled)
        {
            dmem.LumaLog2WeightDenom_S8   = 6;
            dmem.ChromaLog2WeightDenom_S8 = 6;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencWeightedPred::SetHucBrcUpdateConstData(
        const CODEC_HEVC_ENCODE_SLICE_PARAMS &hevcSliceParams,
        uint32_t sliceIndex,
        uint32_t weightOffsetStateCmdSize,
        uint32_t &sliceLocation,
        VdencHevcHucBrcConstantData &constantData)
    {
        ENCODE_FUNC_CALL();

        uint32_t sliceType = hevcSliceParams.slice_type;

        if (m_enabled)
        {
            // 1st HCP_WEIGHTOFFSET_STATE cmd - P & B
            if (sliceType == encodeHevcPSlice || sliceType == encodeHevcBSlice)
            {
                // HCP_WEIGHTOFFSET_L0 starts in byte from beginning of the SLB.
                // 0xFFFF means unavailable in SLB
                constantData.Slice[sliceIndex].HcpWeightOffsetL0_StartInBytes =
                    (uint16_t)sliceLocation;
                sliceLocation += weightOffsetStateCmdSize;
            }

            // 2nd HCP_WEIGHTOFFSET_STATE cmd - B
            if (sliceType == encodeHevcBSlice)
            {
                // HCP_WEIGHTOFFSET_L1 starts in byte from beginning of the SLB.
                // 0xFFFF means unavailable in SLB
                constantData.Slice[sliceIndex].HcpWeightOffsetL1_StartInBytes =
                    (uint16_t)sliceLocation;
                sliceLocation += weightOffsetStateCmdSize;
            }

            constantData.Slice[sliceIndex].WeightTable_StartInBits =
                (uint16_t)hevcSliceParams.PredWeightTableBitOffset;
            constantData.Slice[sliceIndex].WeightTable_EndInBits =
                (uint16_t)(hevcSliceParams.PredWeightTableBitOffset +
                          (hevcSliceParams.PredWeightTableBitLength));
        }
        else
        {
            // 0xFFFF means unavailable in SLB
            constantData.Slice[sliceIndex].HcpWeightOffsetL0_StartInBytes = 0xFFFF;
            constantData.Slice[sliceIndex].HcpWeightOffsetL1_StartInBytes = 0xFFFF;

            // number of bits from beginning of slice header, 0xffff means not awailable
            constantData.Slice[sliceIndex].WeightTable_StartInBits = 0xFFFF;
            constantData.Slice[sliceIndex].WeightTable_EndInBits   = 0xFFFF;
        }
        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_WEIGHTSOFFSETS_STATE, HevcVdencWeightedPred)
    {
        params.denomLuma = 1;

        if (m_enabled)
        {
            char    lumaWeights[2][CODEC_MAX_NUM_REF_FRAME_HEVC];
            int16_t lumaOffsets[2][CODEC_MAX_NUM_REF_FRAME_HEVC];
            char    chromaWeights[2][CODEC_MAX_NUM_REF_FRAME_HEVC][2];
            int16_t chromaOffsets[2][CODEC_MAX_NUM_REF_FRAME_HEVC][2];

            params.denomLuma   = 1 << (m_hevcSliceParams->luma_log2_weight_denom);
            params.denomChroma = 1 << (m_hevcSliceParams->luma_log2_weight_denom + m_hevcSliceParams->delta_chroma_log2_weight_denom);

            for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
            {
                // Luma offset
                lumaOffsets[0][i] = (int16_t)m_hevcSliceParams->luma_offset[0][i];
                lumaOffsets[1][i] = (int16_t)m_hevcSliceParams->luma_offset[1][i];

                // Cb offset
                chromaOffsets[0][i][0] = (int16_t)m_hevcSliceParams->chroma_offset[0][i][0];
                chromaOffsets[1][i][0] = (int16_t)m_hevcSliceParams->chroma_offset[1][i][0];

                // Cr offset
                chromaOffsets[0][i][1] = (int16_t)m_hevcSliceParams->chroma_offset[0][i][1];
                chromaOffsets[1][i][1] = (int16_t)m_hevcSliceParams->chroma_offset[1][i][1];
            }

            // Luma Weight
            ENCODE_CHK_STATUS_MESSAGE_RETURN(MOS_SecureMemcpy(&lumaWeights[0], sizeof(lumaWeights[0]), &m_hevcSliceParams->delta_luma_weight[0], sizeof(m_hevcSliceParams->delta_luma_weight[0])), "Failed to copy luma weight 0 memory.");

            ENCODE_CHK_STATUS_MESSAGE_RETURN(MOS_SecureMemcpy(&lumaWeights[1], sizeof(lumaWeights[1]), &m_hevcSliceParams->delta_luma_weight[1], sizeof(m_hevcSliceParams->delta_luma_weight[1])), "Failed to copy luma weight 1 memory.");

            // Chroma Weight
            ENCODE_CHK_STATUS_MESSAGE_RETURN(MOS_SecureMemcpy(&chromaWeights[0], sizeof(chromaWeights[0]), &m_hevcSliceParams->delta_chroma_weight[0], sizeof(m_hevcSliceParams->delta_chroma_weight[0])), "Failed to copy chroma weight 0 memory.");

            ENCODE_CHK_STATUS_MESSAGE_RETURN(MOS_SecureMemcpy(&chromaWeights[1], sizeof(chromaWeights[1]), &m_hevcSliceParams->delta_chroma_weight[1], sizeof(m_hevcSliceParams->delta_chroma_weight[1])), "Failed to copy chroma weight 1 memory.");

            if (m_basicFeature->m_ref.IsLowDelay())
            {
                lumaWeights[1][0] = lumaWeights[0][0];
                lumaOffsets[1][0] = lumaOffsets[0][0];

                chromaWeights[1][0][0] = chromaWeights[0][0][0];
                chromaOffsets[1][0][0] = chromaOffsets[0][0][0];

                chromaWeights[1][0][1] = chromaWeights[0][0][1];
                chromaOffsets[1][0][1] = chromaOffsets[0][0][1];
            }

            for (auto i = 0; i < 3; i++)
            {
                params.offsetsLuma[0][i] = lumaOffsets[0][i];
                params.offsetsLuma[1][i] = lumaOffsets[1][i];
                params.weightsLuma[0][i] = lumaWeights[0][i];
                params.weightsLuma[1][i] = lumaWeights[1][i];

                params.offsetsChroma[0][i][0] = chromaOffsets[0][i][0];
                params.offsetsChroma[1][i][0] = chromaOffsets[1][i][0];
                params.offsetsChroma[0][i][1] = chromaOffsets[0][i][1];
                params.offsetsChroma[1][i][1] = chromaOffsets[1][i][1];

                params.weightsChroma[0][i][0] = chromaWeights[0][i][0];
                params.weightsChroma[1][i][0] = chromaWeights[1][i][0];
                params.weightsChroma[0][i][1] = chromaWeights[0][i][1];
                params.weightsChroma[1][i][1] = chromaWeights[1][i][1];
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HCP_WEIGHTOFFSET_STATE, HevcVdencWeightedPred)
    {
        ENCODE_FUNC_CALL();

        if (!m_enabled)
        {
            return MOS_STATUS_SUCCESS;
        }

        ENCODE_CHK_NULL_RETURN(m_hevcSliceParams);

        PCODEC_HEVC_ENCODE_SLICE_PARAMS pEncodeHevcSliceParams = &m_basicFeature->m_hevcSliceParams[m_basicFeature->m_curNumSlices];

        for (auto k = 0; k < 2; k++)  // k=0: LIST_0, k=1: LIST_1
        {
            // LUMA, Chroma offset
            for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
            {
                params.LumaOffsets[k][i] = pEncodeHevcSliceParams->luma_offset[k][i];
                // Cb, Cr
                for (auto j = 0; j < 2; j++)
                {
                    params.ChromaOffsets[k][i][j] = pEncodeHevcSliceParams->chroma_offset[k][i][j];
                }
            }

            // LUMA Weight
            ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                &params.LumaWeights[k],
                sizeof(params.LumaWeights[k]),
                &pEncodeHevcSliceParams->delta_luma_weight[k],
                sizeof(pEncodeHevcSliceParams->delta_luma_weight[k])));

            // Chroma Weight
            for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
            {
                for (auto j = 0; j < 2; j++)
                {
                    params.ChromaWeights[k][i][j] = pEncodeHevcSliceParams->delta_chroma_weight[k][i][j];
                }
            }
        }
        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_HEVC_VP9_TILE_SLICE_STATE, HevcVdencWeightedPred)
    {
        auto hevcFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(hevcFeature);

        params.log2WeightDenomLuma = params.hevcVp9Log2WeightDenomLuma = m_enabled ? (m_bEnableGPUWeightedPrediction ? 6 : hevcFeature->m_hevcSliceParams->luma_log2_weight_denom) : 0;
        params.log2WeightDenomChroma                                   = m_enabled ? (m_bEnableGPUWeightedPrediction ? 6 : hevcFeature->m_hevcSliceParams->luma_log2_weight_denom + hevcFeature->m_hevcSliceParams->delta_chroma_log2_weight_denom) : 0;

        return MOS_STATUS_SUCCESS;
    }
}  // namespace encode
