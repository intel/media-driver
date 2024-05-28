/*
* Copyright (c) 2019-2022, Intel Corporation
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
//! \file     decode_hevc_basic_feature.cpp
//! \brief    Defines the common interface for decode hevc parameter
//!

#include <set>
#include "decode_hevc_basic_feature.h"
#include "decode_utils.h"
#include "decode_allocator.h"

namespace decode
{

MOS_STATUS HevcBasicFeature::Init(void *setting)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(setting);
    DECODE_CHK_NULL(m_hwInterface);

    m_shortFormatInUse = ((CodechalSetting*)setting)->shortFormatInUse;

    DECODE_CHK_STATUS(DecodeBasicFeature::Init(setting));

    DECODE_CHK_STATUS(m_refFrames.Init(this, *m_allocator));
    DECODE_CHK_STATUS(m_mvBuffers.Init(m_hwInterface, *m_allocator, *this,
                                       CODEC_NUM_HEVC_INITIAL_MV_BUFFERS));
    DECODE_CHK_STATUS(m_tileCoding.Init(this, (CodechalSetting*)setting));

    return MOS_STATUS_SUCCESS;
}

HevcBasicFeature::~HevcBasicFeature()
{
    if (m_allocator != nullptr)
    {
        m_allocator->Destroy(m_referenceBeforeLoopFilter);
    }
}

MOS_STATUS HevcBasicFeature::Update(void *params)
{
    DECODE_FUNC_CALL();

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    DECODE_CHK_NULL(params);

    DECODE_CHK_STATUS(DecodeBasicFeature::Update(params));

    CodechalDecodeParams* decodeParams = (CodechalDecodeParams*)params;
    DECODE_CHK_NULL(decodeParams->m_picParams);
    DECODE_CHK_NULL(decodeParams->m_sliceParams);
    DECODE_CHK_NULL(decodeParams->m_iqMatrixBuffer);

    m_hevcPicParams      = static_cast<PCODEC_HEVC_PIC_PARAMS>(decodeParams->m_picParams);
    m_hevcSliceParams    = static_cast<PCODEC_HEVC_SLICE_PARAMS>(decodeParams->m_sliceParams);
    m_hevcIqMatrixParams = static_cast<PCODECHAL_HEVC_IQ_MATRIX_PARAMS>(decodeParams->m_iqMatrixBuffer);

    m_hevcRextPicParams  = static_cast<PCODEC_HEVC_EXT_PIC_PARAMS>(decodeParams->m_extPicParams);
    m_hevcRextSliceParams= static_cast<PCODEC_HEVC_EXT_SLICE_PARAMS>(decodeParams->m_extSliceParams);
    m_hevcSccPicParams   = static_cast<PCODEC_HEVC_SCC_PIC_PARAMS>(decodeParams->m_advPicParams);
    m_hevcSubsetParams   = static_cast<PCODEC_HEVC_SUBSET_PARAMS>(decodeParams->m_subsetParams);

    DECODE_CHK_STATUS(SetPictureStructs());
    DECODE_CHK_STATUS(SetSliceStructs());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcBasicFeature::ErrorDetectAndConceal()
{

    DECODE_FUNC_CALL();

    DECODE_CHK_COND(m_curRenderPic.FrameIdx >= CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC,
        "currPic.FrameIdx is out of range!");

    DECODE_CHK_COND(m_hevcPicParams->PicHeightInMinCbsY == 0 || m_hevcPicParams->PicWidthInMinCbsY == 0,
        "picture Weight/Height equals to 0!");

    // check LCU size
    // LCU is restricted based on the picture size.
    // LCU 16x16 can only be used with picture width and height both fewer than or equal to 4222 pixels
    if (m_width > CODECHAL_HEVC_MAX_DIM_FOR_MIN_LCU ||
        m_height > CODECHAL_HEVC_MAX_DIM_FOR_MIN_LCU)
    {
        DECODE_CHK_COND(m_ctbSize == CODECHAL_HEVC_MIN_LCU, "Invalid LCU size.");
    }

    // Todo: The maximum support picture size is 16384 pixels for both encoder and decoder.
    // check if slice number is valid
    DECODE_CHK_COND(m_numSlices > CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6 || m_numSlices == 0,
        "slice number is out of range!");

    // check if min CtbSize is valid
    if (!(m_minCtbSize == 8 || m_minCtbSize == 16 || m_minCtbSize == 32 || m_minCtbSize == 64))
    {
        DECODE_ASSERTMESSAGE("MinCtbSize is invalid\n");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // check if CtbSize is valid
    if (!(m_ctbSize == 16 || m_ctbSize == 32 || m_ctbSize == 64))
    {
        DECODE_ASSERTMESSAGE("CtbSize is invalid\n");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (m_hevcPicParams->tiles_enabled_flag)
    {
        // check if tile_columns and tile_rows is valid
        DECODE_CHK_COND((m_hevcPicParams->num_tile_columns_minus1 + 1 > HEVC_NUM_MAX_TILE_COLUMN) ||
                            (m_hevcPicParams->num_tile_rows_minus1 + 1 > HEVC_NUM_MAX_TILE_ROW),
            "num_tile_columns_minus1 or num_tile_rows_minus1 is out of range!");

        // valid range is [0, m_widthInCtb - 1]
        if (m_hevcPicParams->num_tile_columns_minus1 > m_widthInCtb - 1)
        {
            DECODE_ASSERTMESSAGE("num_tile_columns_minus1 out of range: %d\n",
                m_hevcPicParams->num_tile_columns_minus1);

            return MOS_STATUS_INVALID_PARAMETER;
        }

        // valid range is [0, m_heightInCtb - 1]
        if (m_hevcPicParams->num_tile_rows_minus1 > m_heightInCtb - 1)
        {
            DECODE_ASSERTMESSAGE("num_tile_rows_minus1 out of range: %d\n",
                m_hevcPicParams->num_tile_rows_minus1);

            return MOS_STATUS_INVALID_PARAMETER;
        }
    }

    if (m_hevcPicParams->tiles_enabled_flag && !m_hevcPicParams->uniform_spacing_flag)
    {
        uint8_t  num_tile_columns_minus1                = m_hevcPicParams->num_tile_columns_minus1;
        uint8_t  num_tile_rows_minus1                   = m_hevcPicParams->num_tile_rows_minus1;
        uint16_t tileColWidth[HEVC_NUM_MAX_TILE_COLUMN] = {0};  //!< Table of tile column width
        uint16_t tileRowHeight[HEVC_NUM_MAX_TILE_ROW]   = {0};  //!< Table of tile row height

        tileColWidth[num_tile_columns_minus1]           = m_widthInCtb & 0xffff;
        for (auto i = 0; i < num_tile_columns_minus1; i++)
        {
            tileColWidth[i]                       = m_hevcPicParams->column_width_minus1[i] + 1;
            DECODE_CHK_COND(tileColWidth[i] == 0,
                "column_width_minus1 is invalid");
            DECODE_CHK_COND(tileColWidth[i] > tileColWidth[num_tile_columns_minus1],
                "column_width_minus1 is out of range");
            tileColWidth[num_tile_columns_minus1] -= tileColWidth[i];
        }

        tileRowHeight[num_tile_rows_minus1] = m_heightInCtb & 0xffff;
        for (auto i = 0; i < num_tile_rows_minus1; i++)
        {
            tileRowHeight[i]                    = m_hevcPicParams->row_height_minus1[i] + 1;
            DECODE_CHK_COND(tileRowHeight[i] == 0,
                "row_height_minus1 is invalid");
            DECODE_CHK_COND(tileRowHeight[i] > tileRowHeight[num_tile_rows_minus1],
                "row_height_minus1 is out of range");
            tileRowHeight[num_tile_rows_minus1] -= tileRowHeight[i];
        }
    }

    // diff_cu_qp_delta_depth range is [0, log2_diff_max_min_luma_coding_block_size]
    if (m_hevcPicParams->diff_cu_qp_delta_depth > m_hevcPicParams->log2_diff_max_min_luma_coding_block_size)
    {
        DECODE_ASSERTMESSAGE("diff_cu_qp_delta_depth %d is invalid\n", m_hevcPicParams->diff_cu_qp_delta_depth);
        m_hevcPicParams->diff_cu_qp_delta_depth = 0;
    }

    // cb_qp_offset range is [-12, 12]
    if (m_hevcPicParams->pps_cb_qp_offset > 12 || m_hevcPicParams->pps_cb_qp_offset < -12)
    {
        DECODE_ASSERTMESSAGE("cb_qp_offset is invalid\n", m_hevcPicParams->pps_cb_qp_offset);
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // cr_qp_offset range is [-12, 12]
    if (m_hevcPicParams->pps_cr_qp_offset > 12 || m_hevcPicParams->pps_cr_qp_offset < -12)
    {
        DECODE_ASSERTMESSAGE("cr_qp_offset is invalid\n", m_hevcPicParams->pps_cr_qp_offset);
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // log2_parallel_merge_level_minus2 is in range [0, 4]
    if (m_hevcPicParams->log2_parallel_merge_level_minus2 > 4)
    {
        DECODE_ASSERTMESSAGE("log2_parallel_merge_level_minus2 is out of range\n");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // The value of log2_parallel_merge_level_minus2 shall be in the range of 0 to CtbLog2SizeY - 2
    if (m_hevcPicParams->log2_parallel_merge_level_minus2 >
        (m_hevcPicParams->log2_min_luma_coding_block_size_minus3 + 1 + m_hevcPicParams->log2_diff_max_min_luma_coding_block_size))
    {
        DECODE_ASSERTMESSAGE("log2_parallel_merge_level_minus2 is out of range\n");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (m_hevcPicParams->entropy_coding_sync_enabled_flag && m_hevcPicParams->tiles_enabled_flag)
    {
        // check for non-scc
        if (m_hevcSccPicParams == nullptr)
        {
            DECODE_ASSERTMESSAGE("Only SCC 4:4:4 allows both tiles_enabled_flag and entropy_coding_sync_enabled_flag to be on at the same time\n");
        }
    }

    // Todo: check error detect for Rext
    if (m_hevcRextPicParams != nullptr)
    {
        if (m_hevcPicParams->transform_skip_enabled_flag == 0)
        {
            m_hevcRextPicParams->log2_max_transform_skip_block_size_minus2 = 0;
            DECODE_ASSERTMESSAGE("log2_max_transform_skip_block_size_minus2 should equal to 0 when transform_skip_enabled_flag not present\n");
        }

        if (m_hevcRextPicParams->diff_cu_chroma_qp_offset_depth > m_hevcPicParams->log2_diff_max_min_luma_coding_block_size)
        {
            DECODE_ASSERTMESSAGE("diff_cu_chroma_qp_offset_depth is out of range\n");
        }

        // only for 4:4:4 it can be program to non-zero
        if (m_hevcPicParams->chroma_format_idc != 3)
        {
            DECODE_ASSERTMESSAGE("chroma_qp_offset_list_enabled_flag is only supported in 4:4:4\n");
        }

        // chroma_qp_offset_list_len_minus1 range is [0, 5]
        if (m_hevcRextPicParams->chroma_qp_offset_list_len_minus1 > 5)
        {
            DECODE_ASSERTMESSAGE("chroma_qp_offset_list_len_minus1 is out of range [0, 5]\n");
        }

        // check if TU size is valid
        auto maxTUSize = m_hevcPicParams->log2_diff_max_min_transform_block_size + m_hevcPicParams->log2_min_transform_block_size_minus2;
        if (m_hevcRextPicParams->log2_max_transform_skip_block_size_minus2 > maxTUSize)
        {
            DECODE_ASSERTMESSAGE("log2_max_transform_skip_block_size_minus2 is out of range\n");
        }
    }

    // Todo: check error detect for SCC
    if (m_hevcSccPicParams != nullptr)
    {
        if (m_hevcPicParams->chroma_format_idc != 3)
        {
            if (m_hevcPicParams->entropy_coding_sync_enabled_flag && m_hevcPicParams->tiles_enabled_flag)
            {
                DECODE_ASSERTMESSAGE("Only SCC 4:4:4 allows both tiles_enabled_flag and entropy_coding_sync_enabled_flag to be on at the same time\n");
            }
        }
    }

    if (!m_shortFormatInUse)
    {
        for (uint32_t slcCount = 0; slcCount < m_numSlices; slcCount++)
        {
            for (uint32_t i = 0; i < 2; i++)
            {
                for (uint32_t j = 0; j < CODEC_MAX_NUM_REF_FRAME_HEVC; j++)
                {
                    if (m_hevcSliceParams->RefPicList[i][j].FrameIdx > 0x7f)
                    {
                        DECODE_ASSERTMESSAGE("Reference frame index is out of range\n");
                        return MOS_STATUS_INVALID_PARAMETER;
                    }
                }
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcBasicFeature::SliceDataSizeCheck(uint32_t sliceIdx)
{
    DECODE_FUNC_CALL();

    if (m_hevcSliceParams[sliceIdx].slice_data_offset + m_hevcSliceParams[sliceIdx].slice_data_size > m_dataSize)
    {
        DECODE_ASSERTMESSAGE("invalid slice %d, data size overflow\n", sliceIdx);
        return MOS_STATUS_INVALID_PARAMETER;
    }

    /* Check slice missing data size should not be dropped to 0 */
    if (m_hevcSliceParams[sliceIdx].slice_data_size <= m_hevcSliceParams[sliceIdx].ByteOffsetToSliceData)
    {
        DECODE_ASSERTMESSAGE("slice data size %u should be greater than slice header length %u\n", m_hevcSliceParams[sliceIdx].slice_data_size, m_hevcSliceParams[sliceIdx].ByteOffsetToSliceData);
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcBasicFeature::SliceSegmentAddressCheck(uint32_t sliceIdx, std::vector<int> &sliceSegmentAddressVector)
{
    DECODE_FUNC_CALL();

    /* When sliceIdx = 0, it should be an independent slice, and its slice_segment_address should be 0 */
    if (sliceIdx == 0)
    {
        if (!IsIndependentSlice(sliceIdx) || m_hevcSliceParams[sliceIdx].slice_segment_address != 0)
        {
            DECODE_ASSERTMESSAGE("Invalid slice %d, independent slice = %s, slice_segment_address = %d\n",
                                 sliceIdx, IsIndependentSlice(sliceIdx) ? "True" : "False", m_hevcSliceParams[sliceIdx].slice_segment_address);
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }

    /* Check slice_segment_address should be in range [0, picSizeInCtbsY - 1] and
     * shall not be equal to the value of slice_segment_address of any other coded slice segment NAL unit of the same coded picture */
    uint32_t picSizeInCtbsY = m_widthInCtb * m_heightInCtb;
    if (sliceIdx != 0) // Previous check has conformed when sliceIdx = 0, slice_segment_address = 0
    {
        if ((m_hevcSliceParams[sliceIdx].slice_segment_address >= picSizeInCtbsY))
        {
            DECODE_ASSERTMESSAGE("slice segment address %d is out of range [0, %d]\n",
                                 m_hevcSliceParams[sliceIdx].slice_segment_address, picSizeInCtbsY - 1);
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }
    sliceSegmentAddressVector.push_back(m_hevcSliceParams[sliceIdx].slice_segment_address);
    if (std::set<int>(sliceSegmentAddressVector.begin(), sliceSegmentAddressVector.end()).size() != sliceSegmentAddressVector.size()) // Has duplicate slice_segment_address
    {
        DECODE_ASSERTMESSAGE("%dth slice_segment_address %d has duplicated.\n", sliceIdx, m_hevcSliceParams[sliceIdx].slice_segment_address);
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcBasicFeature::NumEntryPointOffsetsCheck(uint32_t sliceIdx)
{
    DECODE_FUNC_CALL();

    /* Check num_entry_point_offsets */
    if (m_hevcPicParams->tiles_enabled_flag || m_hevcPicParams->entropy_coding_sync_enabled_flag)
    {
        if (m_hevcPicParams->tiles_enabled_flag == 0 && m_hevcPicParams->entropy_coding_sync_enabled_flag == 1)
        {
            if (m_hevcSliceParams[sliceIdx].num_entry_point_offsets > m_hevcPicParams->PicHeightInMinCbsY - 1)
            {
                DECODE_ASSERTMESSAGE("num_entry_point_offsets %d is out of range [0, %d]\n",
                                     m_hevcSliceParams[sliceIdx].num_entry_point_offsets, m_hevcPicParams->PicHeightInMinCbsY - 1);
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }
        else if (m_hevcPicParams->tiles_enabled_flag == 1 && m_hevcPicParams->entropy_coding_sync_enabled_flag == 0)
        {
            if (m_hevcSliceParams[sliceIdx].num_entry_point_offsets > ((m_hevcPicParams->num_tile_columns_minus1 + 1) * (m_hevcPicParams->num_tile_rows_minus1 + 1) - 1))
            {
                DECODE_ASSERTMESSAGE("num_entry_point_offsets %d is out of range [0, %d]\n",
                                     m_hevcSliceParams[sliceIdx].num_entry_point_offsets, (m_hevcPicParams->num_tile_columns_minus1 + 1) * (m_hevcPicParams->num_tile_rows_minus1 + 1) - 1);
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }
        else
        {
            if (m_hevcSliceParams[sliceIdx].num_entry_point_offsets > ((m_hevcPicParams->num_tile_columns_minus1 + 1) * m_hevcPicParams->PicHeightInMinCbsY - 1))
            {
                DECODE_ASSERTMESSAGE("num_entry_point_offsets %d is out of range [0, %d]\n",
                                     m_hevcSliceParams[sliceIdx].num_entry_point_offsets, (m_hevcPicParams->num_tile_columns_minus1 + 1) * m_hevcPicParams->PicHeightInMinCbsY - 1);
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcBasicFeature::ReferenceParamCheck(uint32_t sliceIdx)
{
    DECODE_FUNC_CALL();

    /* If it's P or B slice, the num_ref_idx_lx_active_minus1 should be in range [0, 14] */
    if (decodeHevcISlice != m_hevcSliceParams[sliceIdx].LongSliceFlags.fields.slice_type)
    {
        if (m_hevcSliceParams[sliceIdx].num_ref_idx_l0_active_minus1 >= CODEC_MAX_NUM_REF_FRAME_HEVC )
        {
            DECODE_ASSERTMESSAGE("num_ref_idx_l0_active_minus1 %d is out of range [0, 14]\n", m_hevcSliceParams[sliceIdx].num_ref_idx_l0_active_minus1);
            return MOS_STATUS_INVALID_PARAMETER;
        }
        if (decodeHevcBSlice == m_hevcSliceParams[sliceIdx].LongSliceFlags.fields.slice_type)
        {
            if (m_hevcSliceParams[sliceIdx].num_ref_idx_l1_active_minus1 >= CODEC_MAX_NUM_REF_FRAME_HEVC)
            {
                DECODE_ASSERTMESSAGE("num_ref_idx_l1_active_minus1 %d is out of range [0, 14]\n", m_hevcSliceParams[sliceIdx].num_ref_idx_l1_active_minus1);
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }
    }

    /* Check RefPicList[listIdx][refIdx].FrameIdx */
    uint32_t num_ref_idx_active_minus1[2] = {0};
    num_ref_idx_active_minus1[0] = m_hevcSliceParams[sliceIdx].num_ref_idx_l0_active_minus1;
    num_ref_idx_active_minus1[1] = m_hevcSliceParams[sliceIdx].num_ref_idx_l1_active_minus1;
    bool hasRefPic[2] = {false};
    uint8_t reflistNum = 0;

    switch (m_hevcSliceParams[sliceIdx].LongSliceFlags.fields.slice_type)
    {
    case decodeHevcISlice:
        reflistNum = 0;
        break;
    case decodeHevcPSlice:
        reflistNum = 1;
        break;
    case decodeHevcBSlice:
        reflistNum = 2;
        break;
    default:
        break;
    }

    for (uint8_t listIdx = 0; listIdx < reflistNum; listIdx++)
    {
        /* If num_ref_idx_lx_active_minus1 is in [0, 14], it should have at least 1 reference picture. */
        for (uint8_t refListIdx = 0; refListIdx <= num_ref_idx_active_minus1[listIdx]; refListIdx++)
        {
            if (m_hevcSliceParams[sliceIdx].RefPicList[listIdx][refListIdx].FrameIdx < CODEC_MAX_NUM_REF_FRAME_HEVC)
            {
                hasRefPic[listIdx] = true;
                break;
            }
        }

        if (false == hasRefPic[listIdx])
        {
            DECODE_ASSERTMESSAGE("num_ref_idx_active_minus1[%d] = %d, RefPicList[%d].FrameIdx is all 127\n",
                                 listIdx, num_ref_idx_active_minus1[listIdx], listIdx);
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcBasicFeature::CollocatedRefIdxCheck(uint32_t sliceIdx)
{
    DECODE_FUNC_CALL();

    if (m_hevcSliceParams[sliceIdx].LongSliceFlags.fields.slice_temporal_mvp_enabled_flag)
    {
        /* Check collocated_ref_idx */
        if (((m_hevcSliceParams[sliceIdx].LongSliceFlags.fields.slice_type == decodeHevcPSlice) || (m_hevcSliceParams[sliceIdx].LongSliceFlags.fields.slice_type == decodeHevcBSlice))
            && m_hevcSliceParams[sliceIdx].LongSliceFlags.fields.collocated_from_l0_flag)
        {
            if (m_hevcSliceParams[sliceIdx].collocated_ref_idx > m_hevcSliceParams[sliceIdx].num_ref_idx_l0_active_minus1)
            {
                DECODE_ASSERTMESSAGE("collocated_ref_idx %d is out of range [%d, %d]\n", m_hevcSliceParams[sliceIdx].collocated_ref_idx, 0, m_hevcSliceParams[sliceIdx].num_ref_idx_l0_active_minus1);
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }
        if ((m_hevcSliceParams[sliceIdx].LongSliceFlags.fields.slice_type == decodeHevcBSlice) && (m_hevcSliceParams[sliceIdx].LongSliceFlags.fields.collocated_from_l0_flag == 0))
        {
            if (m_hevcSliceParams[sliceIdx].collocated_ref_idx > m_hevcSliceParams[sliceIdx].num_ref_idx_l1_active_minus1)
            {
                DECODE_ASSERTMESSAGE("collocated_ref_idx %d is out of range [%d, %d]\n", m_hevcSliceParams[sliceIdx].collocated_ref_idx, 0, m_hevcSliceParams[sliceIdx].num_ref_idx_l1_active_minus1);
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcBasicFeature::ErrorDetectAndConcealForLongFormat()
{
    DECODE_FUNC_CALL();

    std::vector<int> sliceSegmentAddressVector;

    for (uint32_t sliceIdx = 0; sliceIdx < m_numSlices; sliceIdx++)
    {
        DECODE_CHK_STATUS(SliceDataSizeCheck(sliceIdx));

        DECODE_CHK_STATUS(SliceSegmentAddressCheck(sliceIdx, sliceSegmentAddressVector));

        DECODE_CHK_STATUS(NumEntryPointOffsetsCheck(sliceIdx));

        DECODE_CHK_STATUS(ReferenceParamCheck(sliceIdx));

        DECODE_CHK_STATUS(CollocatedRefIdxCheck(sliceIdx));
    }

    return MOS_STATUS_SUCCESS;
}

bool HevcBasicFeature::IsLastSlice(uint32_t sliceIdx)
{
    return (sliceIdx == (m_numSlices-1)) ||
           m_hevcSliceParams[sliceIdx].LongSliceFlags.fields.LastSliceOfPic;
}

bool HevcBasicFeature::IsIndependentSlice(uint32_t sliceIdx)
{
    return (sliceIdx == 0) ||
           (! m_hevcSliceParams[sliceIdx].LongSliceFlags.fields.dependent_slice_segment_flag);
}

MOS_STATUS HevcBasicFeature::SetRequiredBitstreamSize(uint32_t requiredSize)
{
    DECODE_FUNC_CALL();
    if (requiredSize > m_dataSize)
    {
        m_dataOffset = 0;
        m_dataSize   = MOS_ALIGN_CEIL(requiredSize, MHW_CACHELINE_SIZE);
    }
    DECODE_NORMALMESSAGE("Estimate bitstream size in this Frame: %u", requiredSize);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcBasicFeature::SetPictureStructs()
{
    DECODE_FUNC_CALL();

    m_minCtbSize = 1 << (m_hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3);
    m_width      = m_hevcPicParams->PicWidthInMinCbsY * m_minCtbSize;
    m_height     = m_hevcPicParams->PicHeightInMinCbsY * m_minCtbSize;

    m_ctbSize     = 1 << (m_hevcPicParams->log2_diff_max_min_luma_coding_block_size +
                      m_hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3);
    m_widthInCtb  = MOS_ROUNDUP_DIVIDE(m_width, m_ctbSize);
    m_heightInCtb = MOS_ROUNDUP_DIVIDE(m_height, m_ctbSize);

    m_curRenderPic = m_hevcPicParams->CurrPic;

    // Do error detection and concealment
    DECODE_CHK_STATUS(ErrorDetectAndConceal());

    m_secondField = CodecHal_PictureIsBottomField(m_curRenderPic);

    m_isWPPMode = m_hevcPicParams->entropy_coding_sync_enabled_flag;

    if (m_hevcSccPicParams == nullptr)
    {
        m_isSCCIBCMode = false;
        m_isSCCPLTMode = false;
        m_isSCCACTMode = false;
    }
    else
    {
        m_isSCCIBCMode = m_hevcSccPicParams->PicSCCExtensionFlags.fields.pps_curr_pic_ref_enabled_flag;
        m_isSCCPLTMode = m_hevcSccPicParams->PicSCCExtensionFlags.fields.palette_mode_enabled_flag;
        m_isSCCACTMode = m_hevcSccPicParams->PicSCCExtensionFlags.fields.residual_adaptive_colour_transform_enabled_flag;
    }

    if (m_isSCCIBCMode)
    {
        DECODE_CHK_STATUS(CreateReferenceBeforeLoopFilter());
    }

    m_refFrameIndexList.clear();
    for(uint32_t i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        if (m_hevcPicParams->RefFrameList[i].FrameIdx < m_maxFrameIndex)
        {
            m_refFrameIndexList.push_back(m_hevcPicParams->RefFrameList[i].FrameIdx);
        }
    }

    m_reportFrameCrc = m_hevcPicParams->RequestCRC;

    DECODE_CHK_STATUS(m_refFrames.UpdatePicture(*m_hevcPicParams, m_isSCCIBCMode));
    if (m_osInterface->pfnIsMismatchOrderProgrammingSupported())
    {
        for (auto &refFrameIdx : m_refFrameIndexList)
        {
            DECODE_CHK_STATUS(m_mvBuffers.ActiveCurBuffer(refFrameIdx));
        }
        DECODE_CHK_STATUS(m_mvBuffers.ActiveCurBuffer(m_curRenderPic.FrameIdx));
    }
    else
    {
        DECODE_CHK_STATUS(m_mvBuffers.UpdatePicture(m_curRenderPic.FrameIdx, m_refFrameIndexList));
    }
    DECODE_CHK_STATUS(m_tileCoding.UpdatePicture(*m_hevcPicParams));

    // Not possible to determine whether P or B is used for short format.
    // For long format iterating through all of the slices to determine P vs
    // B, so in order to avoid this, declare all other pictures MIXED_TYPE.
    m_pictureCodingType = m_refFrames.m_curIsIntra ? I_TYPE : MIXED_TYPE;

    // Clean dummy reference slot, will be upated by Hevc picture packet accrodingly
    MOS_ZeroMemory(m_dummyReferenceSlot, sizeof(m_dummyReferenceSlot));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcBasicFeature::SetSliceStructs()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_STATUS(m_tileCoding.UpdateSlice(*m_hevcPicParams, m_hevcSliceParams));

    if (m_numSlices > 0)
    {
        PCODEC_HEVC_SLICE_PARAMS lastSlice = m_hevcSliceParams + (m_numSlices - 1);
        DECODE_CHK_STATUS(SetRequiredBitstreamSize(lastSlice->slice_data_offset + lastSlice->slice_data_size));
    }

    if (!m_shortFormatInUse)
    {
        DECODE_CHK_STATUS(ErrorDetectAndConcealForLongFormat());
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcBasicFeature::CreateReferenceBeforeLoopFilter()
{
    if (m_destSurface.dwWidth  == 0 ||
        m_destSurface.dwHeight == 0)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (m_referenceBeforeLoopFilter == nullptr)
    {
        m_referenceBeforeLoopFilter = m_allocator->AllocateSurface(
            m_destSurface.dwWidth, m_destSurface.dwHeight, "Reference before loop filter",
            m_destSurface.Format, m_destSurface.bCompressible, resourceOutputPicture, notLockableVideoMem);
        DECODE_CHK_NULL(m_referenceBeforeLoopFilter);
    }
    else
    {
        DECODE_CHK_STATUS(m_allocator->Resize(m_referenceBeforeLoopFilter, m_destSurface.dwWidth, m_destSurface.dwHeight,
            notLockableVideoMem, false, "Reference before loop filter"));
    }
    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
