/*
* Copyright (c) 2021-2024, Intel Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,HevcEncodeTile
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
//! \file     encode_hevc_aqm.cpp
//! \brief    Defines the common interface for hevc aqm
//!

#include "encode_hevc_aqm.h"
#include "encode_hevc_vdenc_feature_manager.h"
#include "encode_hevc_basic_feature.h"

namespace encode
{
HevcEncodeAqm::HevcEncodeAqm(MediaFeatureManager *featureManager,
    EncodeAllocator *                           allocator,
    CodechalHwInterfaceNext *                       hwInterface,
    void *                                      constSettings) : EncodeAqmFeature(featureManager, allocator, hwInterface, constSettings)
{
    auto encFeatureManager = dynamic_cast<EncodeHevcVdencFeatureManager *>(featureManager);
    ENCODE_CHK_NULL_NO_STATUS_RETURN(encFeatureManager);

    m_basicFeature = dynamic_cast<EncodeBasicFeature *>(encFeatureManager->GetFeature(HevcFeatureIDs::basicFeature));
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);

    if (HCP_CHROMA_FORMAT_YUV422 == m_basicFeature->m_chromaFormat)
    {
        m_enabled = false;
        ENCODE_VERBOSEMESSAGE("422 is not supported for VDAQM feature!");
    }
};

MOS_STATUS HevcEncodeAqm::Update(void *params)
{
    auto basicFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(basicFeature);
    if (basicFeature->m_hevcPicParams->QualityInfoSupportFlags.fields.enable_frame)
    {
        m_enabled = true;
    }
    m_numTiles = (basicFeature->m_hevcPicParams->num_tile_rows_minus1 + 1) * (basicFeature->m_hevcPicParams->num_tile_columns_minus1 + 1);
    m_tileBasedEngine = m_numTiles > 1 ? true : false;
    uint32_t minCodingBlkSize = basicFeature->m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3;
    if (!basicFeature->m_hevcPicParams->tiles_enabled_flag)
    {
        m_tile_width[0]     = (1 << minCodingBlkSize) * (basicFeature->m_hevcSeqParams->wFrameWidthInMinCbMinus1 + 1);
        m_tile_height[0]    = (1 << minCodingBlkSize) * (basicFeature->m_hevcSeqParams->wFrameHeightInMinCbMinus1 + 1);
    }
    else
    {
        for (uint32_t tileIdx = 0; tileIdx < m_numTiles && tileIdx < ENCODE_VDENC_MAX_TILE_NUM; tileIdx++)
        {
            EncodeTileData tileData = {};
            RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetTileByIndex, tileData, tileIdx);
            m_tile_width[tileIdx]       = ((tileData.tileWidthInMinCbMinus1 + 1) << minCodingBlkSize);
            m_tile_height[tileIdx]      = ((tileData.tileHeightInMinCbMinus1 + 1) << minCodingBlkSize);
        }
    }
#if USE_CODECHAL_DEBUG_TOOL
    UpdateFrameDisplayOrder(basicFeature->m_pictureCodingType, basicFeature->m_hevcPicParams->CurrPicOrderCnt, basicFeature->m_hevcSeqParams->GopPicSize);
#endif
    if (basicFeature->m_hevcPicParams->num_tile_columns_minus1 == 1)
    {
        m_numRowStore = 2;
    }
    ENCODE_CHK_STATUS_RETURN(EncodeAqmFeature::Update(params));
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AQM_PIC_STATE, HevcEncodeAqm)
{
    ENCODE_CHK_STATUS_RETURN(EncodeAqmFeature::MHW_SETPAR_F(AQM_PIC_STATE)(params));

    params.lcuSize   = LCU_SIZE_64X64;
    params.codectype = CODECTYPE_HEVC;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_PIC_STATE, HevcEncodeAqm)
{

    params.vdaqmEnable = m_enabled;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AQM_TILE_CODING, HevcEncodeAqm)
{
    auto encFeatureManager = dynamic_cast<EncodeHevcVdencFeatureManager *>(m_featureManager);
    ENCODE_CHK_NULL_RETURN(encFeatureManager);

    auto hevcTile = dynamic_cast<HevcEncodeTile *>(encFeatureManager->GetFeature(HevcFeatureIDs::encodeTile));
    ENCODE_CHK_NULL_RETURN(hevcTile);

    HevcTileInfo hevcTileInfo;

    hevcTile->GetTileInfo(&hevcTileInfo);

    params.tileId               = hevcTileInfo.tileId;
    params.tileColPositionInSb  = hevcTileInfo.tileColPositionInSb;
    params.tileRowPositionInSb  = hevcTileInfo.tileRowPositionInSb;
    params.tileWidthInSbMinus1  = hevcTileInfo.tileWidthInSbMinus1;
    params.tileHeightInSbMinus1 = hevcTileInfo.tileHeightInSbMinus1;
    params.tileNum              = hevcTileInfo.tileNum;
    params.tileGroupId          = hevcTileInfo.tileGroupId;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AQM_SLICE_STATE, HevcEncodeAqm)
{
    auto encFeatureManager = dynamic_cast<EncodeHevcVdencFeatureManager *>(m_featureManager);
    ENCODE_CHK_NULL_RETURN(encFeatureManager);

    auto hevcTile = dynamic_cast<HevcEncodeTile *>(encFeatureManager->GetFeature(HevcFeatureIDs::encodeTile));
    ENCODE_CHK_NULL_RETURN(hevcTile);

    HevcTileInfo hevcTileInfo;

    hevcTile->GetTileInfo(&hevcTileInfo);

    auto hevcBasicFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(hevcBasicFeature);
    auto picParams = hevcBasicFeature->m_hevcPicParams;
    ENCODE_CHK_NULL_RETURN(picParams);
    auto seqParams = hevcBasicFeature->m_hevcSeqParams;
    ENCODE_CHK_NULL_RETURN(seqParams);
    auto t_sliceParams = hevcBasicFeature->m_hevcSliceParams;
    ENCODE_CHK_NULL_RETURN(t_sliceParams);
    CODEC_HEVC_ENCODE_SLICE_PARAMS *sliceParams = (CODEC_HEVC_ENCODE_SLICE_PARAMS *)&t_sliceParams[hevcBasicFeature->m_curNumSlices];

    uint32_t ctbSize     = 1 << (seqParams->log2_max_coding_block_size_minus3 + 3);
    uint32_t widthInPix  = (1 << (seqParams->log2_min_coding_block_size_minus3 + 3)) * (seqParams->wFrameWidthInMinCbMinus1 + 1);
    uint32_t widthInCtb  = (widthInPix / ctbSize) + ((widthInPix % ctbSize) ? 1 : 0);  // round up
    uint32_t heightInPix = (1 << (seqParams->log2_min_coding_block_size_minus3 + 3)) * (seqParams->wFrameHeightInMinCbMinus1 + 1);
    uint32_t heightInCtb = (heightInPix / ctbSize) + ((heightInPix % ctbSize) ? 1 : 0);  // round up
    uint32_t shift       = seqParams->log2_max_coding_block_size_minus3 - seqParams->log2_min_coding_block_size_minus3;

    bool m_enabled = false;
    ENCODE_CHK_STATUS_RETURN(hevcTile->IsEnabled(m_enabled));
    if (!m_enabled)
    {
        params.firstSuperSlice = 0;
        // No tiling support
        params.tileSliceStartLcuMbY     = sliceParams->slice_segment_address / widthInCtb;
        params.nextTileSliceStartLcuMbX = (sliceParams->slice_segment_address + sliceParams->NumLCUsInSlice) / heightInCtb;
        params.nextTileSliceStartLcuMbY = (sliceParams->slice_segment_address + sliceParams->NumLCUsInSlice) / widthInCtb;
    }
    else
    {
        params.tileSliceStartLcuMbX     = hevcTileInfo.tileStartXInLCU;
        params.tileSliceStartLcuMbY     = hevcTileInfo.tileStartYInLCU;
        params.nextTileSliceStartLcuMbX = hevcTileInfo.tileEndXInLCU;
        params.nextTileSliceStartLcuMbY = hevcTileInfo.tileEndYInLCU;
    }

    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS HevcEncodeAqm::UpdateFrameDisplayOrder(const uint16_t pictureCodingType, const uint32_t framePOC, const uint32_t gopPicSize)
{
    auto basicFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(basicFeature);
    if (basicFeature->m_hevcPicParams->CurrPicOrderCnt == 0)
    {
        m_frameNumPrevious += m_gopSizePrevious;
    }
    uint32_t    displayOrderInGOP   = framePOC;
    uint32_t    displayOrderInSeq   = displayOrderInGOP + m_frameNumPrevious;
    m_gopSizePrevious               = gopPicSize;
    m_frameIdxQueue.push(displayOrderInSeq);
    return MOS_STATUS_SUCCESS;
}
#endif

}  // namespace encode
