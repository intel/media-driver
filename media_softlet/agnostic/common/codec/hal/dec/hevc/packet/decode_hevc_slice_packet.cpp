/*
* Copyright (c) 2022-2024, Intel Corporation
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
//! \file     decode_hevc_slice_packet.cpp
//! \brief    Defines the interface for hevc decode slice packet
//!
#include "decode_hevc_slice_packet.h"

namespace decode
{
    HevcDecodeSlcPkt::~HevcDecodeSlcPkt()
    {
    }

    MOS_STATUS HevcDecodeSlcPkt::Init()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_featureManager);
        DECODE_CHK_NULL(m_hwInterface);
        DECODE_CHK_NULL(m_osInterface);
        DECODE_CHK_NULL(m_miItf);
        DECODE_CHK_NULL(m_hevcPipeline);

        m_hevcBasicFeature = dynamic_cast<HevcBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        DECODE_CHK_NULL(m_hevcBasicFeature);

        m_allocator = m_pipeline ->GetDecodeAllocator();
        DECODE_CHK_NULL(m_allocator);

        m_decodecp = m_pipeline->GetDecodeCp();

        DECODE_CHK_STATUS(CalculateSliceStateCommandSize());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcDecodeSlcPkt::Prepare()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_hevcBasicFeature->m_hevcPicParams);
        DECODE_CHK_NULL(m_hevcBasicFeature->m_hevcSliceParams);

        m_hevcPicParams       = m_hevcBasicFeature->m_hevcPicParams;
        m_hevcSliceParams     = m_hevcBasicFeature->m_hevcSliceParams;
        m_hevcRextPicParams   = m_hevcBasicFeature->m_hevcRextPicParams;
        m_hevcRextSliceParams = m_hevcBasicFeature->m_hevcRextSliceParams;
        m_hevcSccPicParams    = m_hevcBasicFeature->m_hevcSccPicParams;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcDecodeSlcPkt::ValidateSubTileIdx(
        const HevcTileCoding::SliceTileInfo &sliceTileInfo,
        uint32_t                             subTileIdx)
    {
        DECODE_FUNC_CALL();

        if (sliceTileInfo.numTiles > 0)
        {
            DECODE_CHK_COND(subTileIdx >= sliceTileInfo.numTiles, "sub tile index exceeds number of tiles!");
        }
        else
        {
            DECODE_CHK_COND(subTileIdx > 0, "sub tile index exceeds number of tiles!");
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcDecodeSlcPkt::AddCmd_HCP_PALETTE_INITIALIZER_STATE(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t sliceIdx)
    {
        DECODE_FUNC_CALL();

        const HevcTileCoding::SliceTileInfo *sliceTileInfo = m_hevcBasicFeature->m_tileCoding.GetSliceTileInfo(sliceIdx);
        DECODE_CHK_NULL(sliceTileInfo);

        bool sccPaletteMode   = m_hevcBasicFeature->m_isSCCPLTMode;
        bool firstSliceOfTile = sliceTileInfo->firstSliceOfTile;
        bool independentSlice = m_hevcBasicFeature->IsIndependentSlice(sliceIdx);

        if (sccPaletteMode && (firstSliceOfTile || independentSlice))
        {
            auto &params = m_hcpItf->MHW_GETPAR_F(HCP_PALETTE_INITIALIZER_STATE)();
            params       = {};

            uint32_t yentryIdx = 0;
            uint32_t yentry = 0, cbEntry = 0, crEntry = 0;

            params.predictorPaletteSize = m_hevcSccPicParams->PredictorPaletteSize;
            params.hevcSccPaletteSize   = m_HevcSccPaletteSize;
            for (uint32_t i = 0; i < m_HevcSccPaletteSize; i += 3)
            {
                // First 64 color entries
                yentryIdx                                    = i * 2 / 3;
                params.predictorPaletteEntries[0][yentryIdx] = m_hevcSccPicParams->PredictorPaletteEntries[0][yentryIdx];  // Y
                params.predictorPaletteEntries[1][yentryIdx] = m_hevcSccPicParams->PredictorPaletteEntries[1][yentryIdx];

                params.predictorPaletteEntries[2][yentryIdx]     = m_hevcSccPicParams->PredictorPaletteEntries[2][yentryIdx];  // Cr
                params.predictorPaletteEntries[0][yentryIdx + 1] = m_hevcSccPicParams->PredictorPaletteEntries[0][yentryIdx + 1];

                params.predictorPaletteEntries[1][yentryIdx + 1] = m_hevcSccPicParams->PredictorPaletteEntries[1][yentryIdx + 1];  // Cb
                params.predictorPaletteEntries[2][yentryIdx + 1] = m_hevcSccPicParams->PredictorPaletteEntries[2][yentryIdx + 1];

                // Second 64 color entries
                yentryIdx += 64;
                params.predictorPaletteEntries[0][yentryIdx] = m_hevcSccPicParams->PredictorPaletteEntries[0][yentryIdx];  // Y
                params.predictorPaletteEntries[1][yentryIdx] = m_hevcSccPicParams->PredictorPaletteEntries[1][yentryIdx];

                params.predictorPaletteEntries[2][yentryIdx]     = m_hevcSccPicParams->PredictorPaletteEntries[2][yentryIdx];  // Cr
                params.predictorPaletteEntries[0][yentryIdx + 1] = m_hevcSccPicParams->PredictorPaletteEntries[0][yentryIdx + 1];

                params.predictorPaletteEntries[1][yentryIdx + 1] = m_hevcSccPicParams->PredictorPaletteEntries[1][yentryIdx + 1];  // Cb
                params.predictorPaletteEntries[2][yentryIdx + 1] = m_hevcSccPicParams->PredictorPaletteEntries[2][yentryIdx + 1];
            }

            DECODE_CHK_STATUS(m_hcpItf->MHW_ADDCMD_F(HCP_PALETTE_INITIALIZER_STATE)(&cmdBuffer));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcDecodeSlcPkt::SET_HCP_SLICE_STATE(uint32_t sliceIdx, uint32_t subTileIdx)
    {
        DECODE_FUNC_CALL();

        const HevcTileCoding::SliceTileInfo *sliceTileInfo = m_hevcBasicFeature->m_tileCoding.GetSliceTileInfo(sliceIdx);
        DECODE_CHK_NULL(sliceTileInfo);

        CODEC_HEVC_SLICE_PARAMS *sliceParams = m_hevcSliceParams + sliceIdx;
        DECODE_CHK_NULL(sliceParams);

        DECODE_CHK_NULL(m_hevcPicParams);

        uint32_t dwSliceIndex   = sliceIdx;
        bool     bLastSlice     = m_hevcBasicFeature->IsLastSlice(sliceIdx);
        int8_t * pRefIdxMapping = m_hevcBasicFeature->m_refFrames.m_refIdxMapping;

        auto &params                 = m_hcpItf->MHW_GETPAR_F(HCP_SLICE_STATE)();
        params.lastSliceInTile       = sliceTileInfo->lastSliceOfTile;
        params.lastSliceInTileColumn = sliceTileInfo->lastSliceOfTile && (sliceTileInfo->sliceTileY == m_hevcPicParams->num_tile_rows_minus1);

        uint32_t ctbSize    = 1 << (m_hevcPicParams->log2_diff_max_min_luma_coding_block_size + m_hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3);
        uint32_t widthInPix = (1 << (m_hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3)) * (m_hevcPicParams->PicWidthInMinCbsY);
        uint32_t widthInCtb = MOS_ROUNDUP_DIVIDE(widthInPix, ctbSize);

        // It is a hardware requirement that the first HCP_SLICE_STATE of the workload starts at LCU X,Y = 0,0.
        // If first slice doesn't starts from (0,0), that means this is error bitstream.
        if (dwSliceIndex == 0)
        {
            params.slicestartctbxOrSliceStartLcuXEncoder = 0;
            params.slicestartctbyOrSliceStartLcuYEncoder = 0;
        }
        else
        {
            params.slicestartctbxOrSliceStartLcuXEncoder = sliceParams->slice_segment_address % widthInCtb;
            params.slicestartctbyOrSliceStartLcuYEncoder = sliceParams->slice_segment_address / widthInCtb;
        }

        if (bLastSlice)
        {
            params.nextslicestartctbxOrNextSliceStartLcuXEncoder = 0;
            params.nextslicestartctbyOrNextSliceStartLcuYEncoder = 0;
        }
        else
        {
            params.nextslicestartctbxOrNextSliceStartLcuXEncoder = (sliceParams + 1)->slice_segment_address % widthInCtb;
            params.nextslicestartctbyOrNextSliceStartLcuYEncoder = (sliceParams + 1)->slice_segment_address / widthInCtb;
        }

        params.sliceType                  = sliceParams->LongSliceFlags.fields.slice_type;
        params.lastsliceofpic             = bLastSlice;
        params.dependentSliceFlag         = sliceParams->LongSliceFlags.fields.dependent_slice_segment_flag;
        params.sliceTemporalMvpEnableFlag = sliceParams->LongSliceFlags.fields.slice_temporal_mvp_enabled_flag;
        params.sliceCbQpOffset            = sliceParams->slice_cb_qp_offset;
        params.sliceCrQpOffset            = sliceParams->slice_cr_qp_offset;

        params.deblockingFilterDisable       = sliceParams->LongSliceFlags.fields.slice_deblocking_filter_disabled_flag;
        params.tcOffsetDiv2                  = sliceParams->slice_tc_offset_div2;
        params.betaOffsetDiv2                = sliceParams->slice_beta_offset_div2;
        params.loopFilterAcrossSlicesEnabled = sliceParams->LongSliceFlags.fields.slice_loop_filter_across_slices_enabled_flag;
        params.saoChromaFlag                 = sliceParams->LongSliceFlags.fields.slice_sao_chroma_flag;
        params.saoLumaFlag                   = sliceParams->LongSliceFlags.fields.slice_sao_luma_flag;
        params.mvdL1ZeroFlag                 = sliceParams->LongSliceFlags.fields.mvd_l1_zero_flag;

        uint8_t isLowDelay = 1;

        if (sliceParams->LongSliceFlags.fields.slice_type == SLICE_TYPE_I_SLICE)
        {
            isLowDelay = 0;
        }
        else
        {
            for (uint8_t i = 0; i < sliceParams->num_ref_idx_l0_active_minus1 + 1; i++)
            {
                uint8_t refFrameID = sliceParams->RefPicList[0][i].FrameIdx;
                if (m_hevcPicParams->PicOrderCntValList[refFrameID] > m_hevcPicParams->CurrPicOrderCntVal)
                {
                    isLowDelay = 0;
                    break;
                }
            }

            if (sliceParams->LongSliceFlags.fields.slice_type == SLICE_TYPE_B_SLICE)
            {
                for (uint8_t i = 0; i < sliceParams->num_ref_idx_l1_active_minus1 + 1; i++)
                {
                    uint8_t refFrameID = sliceParams->RefPicList[1][i].FrameIdx;
                    if (m_hevcPicParams->PicOrderCntValList[refFrameID] > m_hevcPicParams->CurrPicOrderCntVal)
                    {
                        isLowDelay = 0;
                        break;
                    }
                }
            }
        }

        params.isLowDelay            = isLowDelay & 0x1;
        params.collocatedFromL0Flag  = sliceParams->LongSliceFlags.fields.collocated_from_l0_flag;
        params.chromalog2Weightdenom = sliceParams->luma_log2_weight_denom + sliceParams->delta_chroma_log2_weight_denom;
        params.lumaLog2WeightDenom   = sliceParams->luma_log2_weight_denom;
        params.cabacInitFlag         = sliceParams->LongSliceFlags.fields.cabac_init_flag;
        params.maxmergeidx           = 5 - sliceParams->five_minus_max_num_merge_cand - 1;

        uint8_t collocatedRefIndex = 0, collocatedFrameIdx = 0, collocatedFromL0Flag = 0;

        if (sliceParams->LongSliceFlags.fields.slice_temporal_mvp_enabled_flag == 1)
        {
            // Get Collocated Picture POC
            collocatedRefIndex   = sliceParams->collocated_ref_idx;
            collocatedFrameIdx   = 0;
            collocatedFromL0Flag = sliceParams->LongSliceFlags.fields.collocated_from_l0_flag;
            if (sliceParams->LongSliceFlags.fields.slice_type == SLICE_TYPE_P_SLICE)
            {
                collocatedFrameIdx = sliceParams->RefPicList[0][collocatedRefIndex].FrameIdx;
            }
            else if (sliceParams->LongSliceFlags.fields.slice_type == SLICE_TYPE_B_SLICE)
            {
                collocatedFrameIdx = sliceParams->RefPicList[!collocatedFromL0Flag][collocatedRefIndex].FrameIdx;
            }

            if (sliceParams->LongSliceFlags.fields.slice_type == SLICE_TYPE_I_SLICE)
            {
                params.collocatedrefidx = 0;
            }
            else
            {
                MHW_CHK_COND(*(pRefIdxMapping + collocatedFrameIdx) < 0, "Invalid parameter");
                params.collocatedrefidx = *(pRefIdxMapping + collocatedFrameIdx);
            }
        }
        else
        {
            params.collocatedrefidx = 0;
        }

        static uint8_t ucFirstInterSliceCollocatedFrameIdx   = 0;
        static uint8_t ucFirstInterSliceCollocatedFromL0Flag = 0;
        static bool    bFinishFirstInterSlice                = false;

        // Need to save the first interSlice collocatedRefIdx value to use on subsequent intra slices
        // this is a HW requrement as collcoated ref fetching from memory may not be complete yet
        if (dwSliceIndex == 0)
        {
            ucFirstInterSliceCollocatedFrameIdx   = 0;
            ucFirstInterSliceCollocatedFromL0Flag = 0;
            bFinishFirstInterSlice                = false;
        }

        if ((!bFinishFirstInterSlice) &&
            (sliceParams->LongSliceFlags.fields.slice_type != SLICE_TYPE_I_SLICE) &&
            (sliceParams->LongSliceFlags.fields.slice_temporal_mvp_enabled_flag == 1))
        {
            ucFirstInterSliceCollocatedFrameIdx   = params.collocatedrefidx;
            ucFirstInterSliceCollocatedFromL0Flag = params.collocatedFromL0Flag;
            bFinishFirstInterSlice                = true;
        }

        if (bFinishFirstInterSlice &&
            ((sliceParams->LongSliceFlags.fields.slice_type == SLICE_TYPE_I_SLICE) ||
                (sliceParams->LongSliceFlags.fields.slice_temporal_mvp_enabled_flag == 0)))
        {
            params.collocatedrefidx     = ucFirstInterSliceCollocatedFrameIdx;
            params.collocatedFromL0Flag = ucFirstInterSliceCollocatedFromL0Flag;
        }

        params.sliceheaderlength = sliceParams->ByteOffsetToSliceData;

        int32_t sliceQP        = sliceParams->slice_qp_delta + m_hevcPicParams->init_qp_minus26 + 26;
        params.sliceqpSignFlag = (sliceQP >= 0) ? 0 : 1;
        params.sliceqp         = ABS(sliceQP);

        bool bTileInSlice = sliceTileInfo->numTiles > 1;
        if (bTileInSlice)
        {
            if (bLastSlice)
            {
                params.nextslicestartctbxOrNextSliceStartLcuXEncoder = 0;
                params.nextslicestartctbyOrNextSliceStartLcuYEncoder = 0;
            }
        }
        else
        {
            params.slicestartctbxOrSliceStartLcuXEncoder = sliceParams->slice_segment_address % widthInCtb;
            params.slicestartctbyOrSliceStartLcuYEncoder = sliceParams->slice_segment_address / widthInCtb;
        }

        auto hevcExtSliceParams = m_hevcRextSliceParams + sliceIdx;
        if (m_hevcRextPicParams && hevcExtSliceParams)
        {
            // DW3[23]
            if (m_hevcRextPicParams->PicRangeExtensionFlags.fields.chroma_qp_offset_list_enabled_flag)
            {
                params.cuChromaQpOffsetEnable = hevcExtSliceParams->cu_chroma_qp_offset_enabled_flag;
            }
        }

        if (!bTileInSlice)
        {
            params.sliceheaderlength = sliceParams->ByteOffsetToSliceData;
        }

        if ((dwSliceIndex == 0) || !params.dependentSliceFlag)
        {
            params.originalSliceStartCtbX = sliceParams->slice_segment_address % widthInCtb;
            params.originalSliceStartCtbY = sliceParams->slice_segment_address / widthInCtb;
        }
        else
        {
            params.originalSliceStartCtbX = sliceTileInfo->origCtbX;
            params.originalSliceStartCtbY = sliceTileInfo->origCtbY;
        }

        if (m_hevcSccPicParams && hevcExtSliceParams)
        {
            if (m_hevcSccPicParams->PicSCCExtensionFlags.fields.pps_slice_act_qp_offsets_present_flag)
            {
                // DW12
                params.sliceActYQpOffset  = hevcExtSliceParams->slice_act_y_qp_offset;
                params.sliceActCbQpOffset = hevcExtSliceParams->slice_act_cb_qp_offset;
                params.sliceActCrQpOffset = hevcExtSliceParams->slice_act_cr_qp_offset;
            }

            params.useIntegerMvFlag = hevcExtSliceParams->use_integer_mv_flag;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcDecodeSlcPkt::SET_HCP_REF_IDX_STATE(uint32_t sliceIdx)
    {
        DECODE_FUNC_CALL();

        auto &params        = m_hcpItf->MHW_GETPAR_F(HCP_REF_IDX_STATE)();
        params.bDecodeInUse = true;

        CODEC_HEVC_SLICE_PARAMS *sliceParams                           = m_hevcSliceParams + sliceIdx;
        int8_t                  *pRefIdxMapping                        = 0;
        int                      pocCurrPic                            = 0;
        int32_t                  pocList[CODEC_MAX_NUM_REF_FRAME_HEVC] = {};
        uint16_t                 refFieldPicFlag                       = 0;
        uint16_t                 refBottomFieldFlag                    = 0;
        CODEC_PICTURE            refPicList[2][CODEC_MAX_NUM_REF_FRAME_HEVC] = {};

        if (!m_hcpItf->IsHevcISlice(sliceParams->LongSliceFlags.fields.slice_type))
        {
            HevcReferenceFrames &refFrames = m_hevcBasicFeature->m_refFrames;
            DECODE_CHK_STATUS(refFrames.FixSliceRefList(*m_hevcPicParams, *sliceParams));

            CODEC_PICTURE currPic  = m_hevcPicParams->CurrPic;
            if (params.ucList != 1)
            {
                params.ucNumRefForList = sliceParams->num_ref_idx_l0_active_minus1 + 1;
            }

            DECODE_CHK_STATUS(MOS_SecureMemcpy(&refPicList, sizeof(refPicList), &sliceParams->RefPicList, sizeof(sliceParams->RefPicList)));

            void **hevcRefList = (void **)refFrames.m_refList;
            pocCurrPic         = m_hevcPicParams->CurrPicOrderCntVal;

            for (uint8_t i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
            {
                pocList[i] = m_hevcPicParams->PicOrderCntValList[i];
            }

            pRefIdxMapping     = refFrames.m_refIdxMapping;
            refFieldPicFlag    = m_hevcPicParams->RefFieldPicFlag;
            refBottomFieldFlag = m_hevcPicParams->RefBottomFieldFlag;

            // Need to add an empty HCP_REF_IDX_STATE_CMD for dummy reference on I-Frame
            // ucNumRefForList could be 0 for encode
            MHW_ASSERT(currPic.FrameIdx != 0x7F);

            params.numRefIdxLRefpiclistnumActiveMinus1 = params.ucNumRefForList - 1;

            for (uint8_t i = 0; i < params.ucNumRefForList; i++)
            {
                uint8_t refFrameIDx = refPicList[params.ucList][i].FrameIdx;
                if (refFrameIDx < CODEC_MAX_NUM_REF_FRAME_HEVC)
                {
                    MHW_ASSERT(*(pRefIdxMapping + refFrameIDx) >= 0);

                    params.listEntryLxReferencePictureFrameIdRefaddr07[i] = *(pRefIdxMapping + refFrameIDx);
                    int32_t pocDiff                                       = pocCurrPic - pocList[refFrameIDx];
                    params.referencePictureTbValue[i]                     = CodecHal_Clip3(-128, 127, pocDiff);
                    CODEC_REF_LIST **refList                              = (CODEC_REF_LIST **)hevcRefList;
                    params.longtermreference[i]                           = CodecHal_PictureIsLongTermRef(refList[currPic.FrameIdx]->RefList[refFrameIDx]);
                    params.fieldPicFlag[i]                                = (refFieldPicFlag >> refFrameIDx) & 0x01;
                    params.bottomFieldFlag[i]                             = ((refBottomFieldFlag >> refFrameIDx) & 0x01) ? 0 : 1;
                }
                else
                {
                    params.listEntryLxReferencePictureFrameIdRefaddr07[i] = 0;
                    params.referencePictureTbValue[i]                     = 0;
                    params.longtermreference[i]                           = false;
                    params.fieldPicFlag[i]                                = 0;
                    params.bottomFieldFlag[i]                             = 0;
                }
            }
        }
        else if (m_hevcBasicFeature->m_useDummyReference && !m_osInterface->bSimIsActive)
        {
            params.bDummyReference = true;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcDecodeSlcPkt::SET_HCP_WEIGHTOFFSET_STATE(uint32_t sliceIdx)
    {
        DECODE_FUNC_CALL();

        auto &params = m_hcpItf->MHW_GETPAR_F(HCP_WEIGHTOFFSET_STATE)();

        CODEC_HEVC_SLICE_PARAMS *sliceParams = m_hevcSliceParams + sliceIdx;

        bool weightedPred   = m_hevcPicParams->weighted_pred_flag   && m_hcpItf->IsHevcPSlice(sliceParams->LongSliceFlags.fields.slice_type);
        bool weightedBiPred = m_hevcPicParams->weighted_bipred_flag && m_hcpItf->IsHevcBSlice(sliceParams->LongSliceFlags.fields.slice_type);
    
        if (weightedPred || weightedBiPred)
        {
            params.ucList = 0;

            DECODE_CHK_STATUS(MOS_SecureMemcpy(
                &params.LumaWeights[0], sizeof(params.LumaWeights[0]), &sliceParams->delta_luma_weight_l0, sizeof(sliceParams->delta_luma_weight_l0)));

            DECODE_CHK_STATUS(MOS_SecureMemcpy(
                &params.LumaWeights[1], sizeof(params.LumaWeights[1]), &sliceParams->delta_luma_weight_l1, sizeof(sliceParams->delta_luma_weight_l1)));

            PCODEC_HEVC_EXT_SLICE_PARAMS slcRextParams = (m_hevcRextSliceParams == nullptr) ? nullptr : (m_hevcRextSliceParams + sliceIdx);
            
            if (slcRextParams != nullptr)
            {
                DECODE_CHK_STATUS(MOS_SecureMemcpy(
                    &params.LumaOffsets[0], sizeof(params.LumaOffsets[0]), &slcRextParams->luma_offset_l0, sizeof(slcRextParams->luma_offset_l0)));

                DECODE_CHK_STATUS(MOS_SecureMemcpy(
                    &params.LumaOffsets[1], sizeof(params.LumaOffsets[1]), &slcRextParams->luma_offset_l1, sizeof(slcRextParams->luma_offset_l1)));

                DECODE_CHK_STATUS(MOS_SecureMemcpy(
                    &params.ChromaOffsets[0], sizeof(params.ChromaOffsets[0]), &slcRextParams->ChromaOffsetL0, sizeof(slcRextParams->ChromaOffsetL0)));

                DECODE_CHK_STATUS(MOS_SecureMemcpy(
                    &params.ChromaOffsets[1], sizeof(params.ChromaOffsets[1]), &slcRextParams->ChromaOffsetL1, sizeof(slcRextParams->ChromaOffsetL1)));
            }
            else
            {
                for (uint32_t i = 0; i < 15; i++)
                {
                    params.LumaOffsets[0][i] = (int16_t)sliceParams->luma_offset_l0[i];
                    params.LumaOffsets[1][i] = (int16_t)sliceParams->luma_offset_l1[i];

                    for (uint32_t j = 0; j < 2; j++)
                    {
                        params.ChromaOffsets[0][i][j] = (int16_t)sliceParams->ChromaOffsetL0[i][j];
                        params.ChromaOffsets[1][i][j] = (int16_t)sliceParams->ChromaOffsetL1[i][j];
                    }
                }
            }

            DECODE_CHK_STATUS(MOS_SecureMemcpy(
                &params.ChromaWeights[0], sizeof(params.ChromaWeights[0]), &sliceParams->delta_chroma_weight_l0, sizeof(sliceParams->delta_chroma_weight_l0)));

            DECODE_CHK_STATUS(MOS_SecureMemcpy(
                &params.ChromaWeights[1], sizeof(params.ChromaWeights[1]), &sliceParams->delta_chroma_weight_l1, sizeof(sliceParams->delta_chroma_weight_l1)));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcDecodeSlcPkt::AddCmd_HCP_WEIGHTOFFSET_STATE(
        MOS_COMMAND_BUFFER &cmdBuffer,
        uint32_t            sliceIdx)
    {
        DECODE_FUNC_CALL();

        CODEC_HEVC_SLICE_PARAMS *sliceParams  = m_hevcSliceParams + sliceIdx;

        bool weightedPred   = m_hevcPicParams->weighted_pred_flag   && m_hcpItf->IsHevcPSlice(sliceParams->LongSliceFlags.fields.slice_type);
        bool weightedBiPred = m_hevcPicParams->weighted_bipred_flag && m_hcpItf->IsHevcBSlice(sliceParams->LongSliceFlags.fields.slice_type);

        if (weightedPred || weightedBiPred)
        {
            auto &params = m_hcpItf->MHW_GETPAR_F(HCP_WEIGHTOFFSET_STATE)();
            params       = {};

            DECODE_CHK_STATUS(SET_HCP_WEIGHTOFFSET_STATE(sliceIdx));
            DECODE_CHK_STATUS(m_hcpItf->MHW_ADDCMD_F(HCP_WEIGHTOFFSET_STATE)(&cmdBuffer));

            if (weightedBiPred)
            {
                params.ucList = 1;
                DECODE_CHK_STATUS(m_hcpItf->MHW_ADDCMD_F(HCP_WEIGHTOFFSET_STATE)(&cmdBuffer));
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcDecodeSlcPkt::SET_HCP_BSD_OBJECT(uint32_t sliceIdx, uint32_t subTileIdx)
    {
        DECODE_FUNC_CALL();

        auto &params = m_hcpItf->MHW_GETPAR_F(HCP_BSD_OBJECT)();

        const HevcTileCoding::SliceTileInfo *sliceTileInfo = m_hevcBasicFeature->m_tileCoding.GetSliceTileInfo(sliceIdx);
        DECODE_CHK_NULL(sliceTileInfo);
        DECODE_CHK_STATUS(ValidateSubTileIdx(*sliceTileInfo, subTileIdx));

        CODEC_HEVC_SLICE_PARAMS *sliceParams = m_hevcSliceParams + sliceIdx;

        if (sliceTileInfo->numTiles > 1)
        {
            params.bsdDataLength      = sliceTileInfo->tileArrayBuf[subTileIdx].bsdLength;
            params.bsdDataStartOffset = sliceParams->slice_data_offset + sliceTileInfo->tileArrayBuf[subTileIdx].bsdOffset;
        }
        else
        {
            params.bsdDataLength      = sliceParams->slice_data_size;
            params.bsdDataStartOffset = sliceParams->slice_data_offset;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcDecodeSlcPkt::AddCmd_HCP_BSD_OBJECT(
        MOS_COMMAND_BUFFER &cmdBuffer,
        uint32_t            sliceIdx,
        uint32_t            subTileIdx)
    {
        DECODE_FUNC_CALL();

        auto &params = m_hcpItf->MHW_GETPAR_F(HCP_BSD_OBJECT)();
        params       = {};

        DECODE_CHK_STATUS(SET_HCP_BSD_OBJECT(sliceIdx, subTileIdx));
        DECODE_CHK_STATUS(m_hcpItf->MHW_ADDCMD_F(HCP_BSD_OBJECT)(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcDecodeSlcPkt::AddHcpCpState(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t sliceIdx, uint32_t subTileIdx)
    {
        DECODE_FUNC_CALL();

        PMOS_RESOURCE buffer      = &(m_hevcBasicFeature->m_resDataBuffer.OsResource);
        uint32_t      startoffset = m_hevcSliceParams->slice_data_offset;

        const HevcTileCoding::SliceTileInfo *sliceTileInfo = m_hevcBasicFeature->m_tileCoding.GetSliceTileInfo(sliceIdx);
        DECODE_CHK_NULL(sliceTileInfo);
        DECODE_CHK_STATUS(ValidateSubTileIdx(*sliceTileInfo, subTileIdx));

        CODEC_HEVC_SLICE_PARAMS *sliceParams = m_hevcSliceParams + sliceIdx;

        if (sliceTileInfo->numTiles > 1)
        {
            startoffset = sliceParams->slice_data_offset + sliceTileInfo->tileArrayBuf[subTileIdx].bsdOffset;
        }
        else
        {
            startoffset = sliceParams->slice_data_offset;
        }
        if(m_decodecp)
        {
            DECODE_CHK_STATUS(m_decodecp->AddHcpState(&cmdBuffer, buffer, m_hevcSliceParams->slice_data_size, startoffset, sliceIdx));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcDecodeSlcPkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
    {
        DECODE_FUNC_CALL();

        commandBufferSize      = m_sliceStatesSize;
        requestedPatchListSize = m_slicePatchListSize;
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcDecodeSlcPkt::CalculateSliceStateCommandSize()
    {
        DECODE_FUNC_CALL();

        // Slice Level Commands
        DECODE_CHK_STATUS(m_hwInterface->GetHcpPrimitiveCommandSize(m_hevcBasicFeature->m_mode,
                                                                     &m_sliceStatesSize,
                                                                     &m_slicePatchListSize,
                                                                     false));

        return MOS_STATUS_SUCCESS;
    }

}
