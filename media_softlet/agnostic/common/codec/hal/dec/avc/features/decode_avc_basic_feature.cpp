/*
* Copyright (c) 2020-2023, Intel Corporation
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
//! \file     decode_avc_basic_features.cpp
//! \brief    Defines the common interface for decode avc basic feature
//!
#include "decode_avc_basic_feature.h"
#include "decode_utils.h"
#include "decode_allocator.h"

namespace decode {

    AvcBasicFeature::~AvcBasicFeature()
    {
        if (m_allocator != nullptr && m_resMonoPicChromaBuffer != nullptr)
        {
            m_allocator->Destroy(m_resMonoPicChromaBuffer);
        }
    }

    MOS_STATUS AvcBasicFeature::Init(void *setting)
    {
        DECODE_FUNC_CALL();

        PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

        DECODE_CHK_NULL(setting);

        DECODE_CHK_STATUS(DecodeBasicFeature::Init(setting));
        CodechalSetting *codecSettings = (CodechalSetting*)setting;
        DECODE_CHK_NULL(codecSettings);
        m_shortFormatInUse = codecSettings->shortFormatInUse;

        DECODE_CHK_STATUS(m_refFrames.Init(this, *m_allocator));
        DECODE_CHK_STATUS(m_mvBuffers.Init(m_hwInterface, *m_allocator, *this, CODEC_AVC_NUM_INIT_DMV_BUFFERS));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcBasicFeature::Update(void *params)
    {
        DECODE_FUNC_CALL();

        PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

        DECODE_CHK_NULL(params);

        DECODE_CHK_STATUS(DecodeBasicFeature::Update(params));

        CodechalDecodeParams* decodeParams = (CodechalDecodeParams*)params;
        DECODE_CHK_NULL(decodeParams->m_picParams);
        DECODE_CHK_NULL(decodeParams->m_sliceParams);
        m_avcPicParams            = (PCODEC_AVC_PIC_PARAMS)decodeParams->m_picParams;
        m_avcSliceParams          = (PCODEC_AVC_SLICE_PARAMS)decodeParams->m_sliceParams;
        m_mvcExtPicParams         = (PCODEC_MVC_EXT_PIC_PARAMS)decodeParams->m_extPicParams;
        m_avcIqMatrixParams       = (PCODEC_AVC_IQ_MATRIX_PARAMS)decodeParams->m_iqMatrixBuffer;
        m_picIdRemappingInUse     = decodeParams->m_picIdRemappingInUse;
        m_fullFrameData           = decodeParams->m_bFullFrameData;
        m_streamOutEnabled        = decodeParams->m_streamOutEnabled;
        m_externalStreamOutBuffer = decodeParams->m_externalStreamOutBuffer;
        m_cencBuf                 = decodeParams->m_cencBuf;

        DECODE_CHK_NULL(m_avcPicParams);
        DECODE_CHK_NULL(m_avcSliceParams);

        // Do error detection and concealment
        DECODE_CHK_STATUS(ErrorDetectAndConceal());


        MEDIA_FEATURE_TABLE* skuTable = m_osInterface->pfnGetSkuTable(m_osInterface);
        m_usingVeRing = (skuTable != nullptr) ? MEDIA_IS_SKU(skuTable, FtrVERing) : false;

        if (m_avcPicParams->seq_fields.chroma_format_idc == avcChromaFormatMono &&
            (m_resMonoPicChromaBuffer == nullptr))
        {
            uint32_t pitch  = m_destSurface.dwPitch;
            uint32_t chromaHeight = m_destSurface.dwHeight >> 1;
            uint32_t alignedChromaHeight = MOS_ALIGN_CEIL(chromaHeight, MOS_YTILE_H_ALIGNMENT);
            uint32_t chromaBufSize = MOS_ALIGN_CEIL(pitch * alignedChromaHeight, MHW_PAGE_SIZE);
            m_resMonoPicChromaBuffer = m_allocator->AllocateBuffer(
                                            chromaBufSize,
                                            "MonoPictureChromaBuffer",
                                            resourceInternalReadWriteCache,
                                            lockableVideoMem,
                                            true,
                                            DECODE_AVC_MONOPIC_CHROMA_DEFAULT);
        }

        DECODE_CHK_STATUS(SetPictureStructs());
        DECODE_CHK_STATUS(SetSliceStructs());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcBasicFeature::ErrorDetectAndConceal()
    {
        /*
         *Only check the invalid syntax instead of return error to skip decoding since this invalid syntax will not cause critical issue
         * */
        DECODE_FUNC_CALL();
        DECODE_CHK_NULL(m_avcPicParams);
        DECODE_CHK_NULL(m_avcSliceParams);

        DECODE_CHK_STATUS(CheckBitDepthAndChromaSampling());

        if(m_avcPicParams->seq_fields.chroma_format_idc != 3
            && m_avcPicParams->seq_fields.residual_colour_transform_flag)
        {
            DECODE_ASSERTMESSAGE("Conflict with H264 Spec! residual_colour_transform_flag can be set only when chroma_format_idc != 3");
        }

        if(m_avcPicParams->seq_fields.frame_mbs_only_flag && m_avcPicParams->seq_fields.mb_adaptive_frame_field_flag != 0)
        {
            if(m_avcPicParams->seq_fields.mb_adaptive_frame_field_flag != 0)
            {
                DECODE_ASSERTMESSAGE("Conflict with H264 Spec! mb_adaptive_frame_field_flag should be 0 when frame_mbs_only_flag is set");
                m_avcPicParams->seq_fields.mb_adaptive_frame_field_flag = 0;
            }

            if(m_avcPicParams->pic_fields.field_pic_flag)
            {
                DECODE_ASSERTMESSAGE("Conflict with H264 Spec! field_pic_flag should be 0 when frame_mbs_only_flag is set");
            }
        }

        if(m_avcPicParams->seq_fields.log2_max_frame_num_minus4 > 12)
        {
            DECODE_ASSERTMESSAGE("Conflict with H264 Spec! log2_max_frame_num_minus4 is out of range");
        }

        if(m_avcPicParams->seq_fields.pic_order_cnt_type > 2)
        {
            DECODE_ASSERTMESSAGE("Conflict with H264 Spec! pic_order_cnt_type is out of range");
        }

        if(m_avcPicParams->seq_fields.pic_order_cnt_type == 1 && m_avcPicParams->seq_fields.log2_max_pic_order_cnt_lsb_minus4 != 0)
        {
            DECODE_ASSERTMESSAGE("Conflict with H264 Spec!  log2_max_pic_order_cnt_lsb_minus4 should be 0 when pic_order_cnt_type is set");
            m_avcPicParams->seq_fields.log2_max_pic_order_cnt_lsb_minus4 = 0;
        }

        if(m_avcPicParams->seq_fields.pic_order_cnt_type == 0)
        {
            if(m_avcPicParams->seq_fields.delta_pic_order_always_zero_flag != 0)
            {
                DECODE_ASSERTMESSAGE("Conflict with H264 Spec! delta_pic_order_always_zero_flag should be 0 when pic_order_cnt_type is not set");
                m_avcPicParams->seq_fields.delta_pic_order_always_zero_flag = 0;
            }

            if(m_avcPicParams->seq_fields.log2_max_pic_order_cnt_lsb_minus4 > 12)
            {
                DECODE_ASSERTMESSAGE("Conflict with H264 Spec! log2_max_pic_order_cnt_lsb_minus4 is out of range");
            }
        }

        //currently num_slice_groups_minus1, slice_group_map_type and slice_group_change_rate_minus1 are all set to 0 in driver
        if(m_avcPicParams->num_slice_groups_minus1 > 7)
        {
            DECODE_ASSERTMESSAGE("Conflict with H264 Spec! num_slice_groups_minus1 is out of range");
        }
        else
        {
            if(m_avcPicParams->slice_group_map_type > 6)
            {
                DECODE_ASSERTMESSAGE("Conflict with H264 Spec! slice_group_map_type is out of range");
            }
            else if(m_avcPicParams->slice_group_map_type == 3
                    || m_avcPicParams->slice_group_map_type == 4
                    || m_avcPicParams->slice_group_map_type == 5)
            {

                if(m_avcPicParams->slice_group_change_rate_minus1 > (m_avcPicParams->pic_width_in_mbs_minus1 + 1) * (m_avcPicParams->pic_height_in_mbs_minus1 + 1))
                {
                    DECODE_ASSERTMESSAGE("Conflict with H264 Spec! slice_group_change_rate_minus1 is out of range");
                }
            }
            else if(m_avcPicParams->slice_group_change_rate_minus1 != 0)
            {
                DECODE_ASSERTMESSAGE("Conflict with H264 Spec!");
            }
        }

        if(m_avcPicParams->pic_init_qp_minus26 < -(26 + 6 * m_avcPicParams->bit_depth_luma_minus8)
            || m_avcPicParams->pic_init_qp_minus26 > 25)
        {
            DECODE_ASSERTMESSAGE("Conflict with H264 Spec! pic_init_qp_minus26 is out of range");
        }

        if(m_avcPicParams->chroma_qp_index_offset < -12
            || m_avcPicParams->chroma_qp_index_offset > 12)
        {
            DECODE_ASSERTMESSAGE("Conflict with H264 Spec! chroma_qp_index_offset is out of range");
        }

        if(m_avcPicParams->second_chroma_qp_index_offset < -12
            || m_avcPicParams->second_chroma_qp_index_offset > 12)
        {
            DECODE_ASSERTMESSAGE("Conflict with H264 Spec! second_chroma_qp_index_offset is out of range");
        }

        if(m_avcPicParams->pic_fields.weighted_bipred_idc > 2)
        {
            DECODE_ASSERTMESSAGE("Conflict with H264 Spec! weighted_bipred_idc is out of range");
        }

        if (!m_shortFormatInUse)
        {
            for (uint32_t slcIdx = 0; slcIdx < m_numSlices; slcIdx++)
            {
                PCODEC_AVC_SLICE_PARAMS slc = m_avcSliceParams + slcIdx;
                if(m_avcPicParams->pic_fields.field_pic_flag == 0)
                {
                    if (slc->num_ref_idx_l0_active_minus1 > 15)
                    {
                        slc->num_ref_idx_l0_active_minus1 = 0;
                        DECODE_ASSERTMESSAGE("Conflict with H264 Spec! num_ref_idx_l0_active_minus1 is out of range");
                    }
                    if (slc->num_ref_idx_l1_active_minus1 > 15)
                    {
                        slc->num_ref_idx_l1_active_minus1 = 0;
                        DECODE_ASSERTMESSAGE("Conflict with H264 Spec! num_ref_idx_l1_active_minus1 is out of range");
                    }
                }
                else if(m_avcPicParams->pic_fields.field_pic_flag == 1)
                {
                    if (slc->num_ref_idx_l0_active_minus1 > 31)
                    {
                        slc->num_ref_idx_l0_active_minus1 = 0;
                        DECODE_ASSERTMESSAGE("Conflict with H264 Spec! num_ref_idx_l0_active_minus1 is out of range");
                    }
                    if (slc->num_ref_idx_l1_active_minus1 > 31)
                    {
                        slc->num_ref_idx_l1_active_minus1 = 0;
                        DECODE_ASSERTMESSAGE("Conflict with H264 Spec! num_ref_idx_l1_active_minus1 is out of range");
                    }
                }
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcBasicFeature::SetPictureStructs()
    {
        DECODE_FUNC_CALL();

        m_width  = (m_avcPicParams->pic_width_in_mbs_minus1 + 1) * CODECHAL_MACROBLOCK_WIDTH;
        m_height = (m_avcPicParams->pic_height_in_mbs_minus1 + 1) * CODECHAL_MACROBLOCK_HEIGHT;
        m_pictureCodingType = m_avcPicParams->pic_fields.IntraPicFlag ? I_TYPE : MIXED_TYPE;
        m_curRenderPic = m_avcPicParams->CurrPic;
        DECODE_CHK_COND(m_curRenderPic.FrameIdx >= CODEC_AVC_NUM_UNCOMPRESSED_SURFACE,
                        "currPic.FrameIdx is out of range!");

        //reset to default value
        m_secondField = false;
        if (m_fullFeildsReceived == (PICTURE_BOTTOM_FIELD | PICTURE_TOP_FIELD))
        {
            m_fullFeildsReceived = 0;
        }

        if (CodecHal_PictureIsField(m_avcPicParams->CurrPic))
        {
           if (CodecHal_PictureIsTopField(m_avcPicParams->CurrPic))
           {
               m_fullFeildsReceived |= PICTURE_TOP_FIELD;
           }
           if (CodecHal_PictureIsBottomField(m_avcPicParams->CurrPic))
           {
               m_fullFeildsReceived |= PICTURE_BOTTOM_FIELD;
           }
        }
        else
        {
            m_fullFeildsReceived = 0;
        }

        if (m_fullFeildsReceived == (PICTURE_BOTTOM_FIELD | PICTURE_TOP_FIELD))
        {
            m_secondField = true;
        }

        if (m_shortFormatInUse)
        {
            // When HW parses the slice_header, disable_deblocking_filter_idc is not yet known,
            // so always enable ILDB for this case.
            m_deblockingEnabled = true;
        }
        else
        {
            for (uint32_t i = 0; i < m_numSlices; i++)
            {
                if (m_avcSliceParams[i].disable_deblocking_filter_idc != 1)
                {
                    m_deblockingEnabled = true;
                    break;
                }
            }
        }

        m_fixedFrameIdx = 0xff;
        m_refFrameIndexList.clear();
        for (uint32_t i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
        {
            if (m_avcPicParams->RefFrameList[i].FrameIdx < m_maxFrameIndex &&
                !CodecHal_PictureIsInvalid(m_avcPicParams->RefFrameList[i]))
            {
                m_refFrameIndexList.push_back(m_avcPicParams->RefFrameList[i].FrameIdx);
                // For interlaced case, if second field refers to first field in the same frame, need to
                // keep the current FrameIdx into the refFrameList, two fields will use the same MV buffer.
                if (m_secondField && (m_curRenderPic.FrameIdx == m_avcPicParams->RefFrameList[i].FrameIdx))
                {
                    m_fixedFrameIdx = m_curRenderPic.FrameIdx;
                }
            }
        }

        DECODE_CHK_STATUS(m_refFrames.UpdatePicture(*m_avcPicParams));

        if (!m_isSecondField)
        {
            if (m_osInterface->pfnIsMismatchOrderProgrammingSupported())
            {
                for (auto &refFrameIdx : m_refFrameIndexList)
                {
                    DECODE_CHK_STATUS(m_mvBuffers.ActiveCurBuffer(refFrameIdx));
                }
                DECODE_CHK_STATUS(m_mvBuffers.ActiveCurBuffer(m_avcPicParams->CurrPic.FrameIdx));
            }
            else
            {
                DECODE_CHK_STATUS(m_mvBuffers.UpdatePicture(m_avcPicParams->CurrPic.FrameIdx, m_refFrameIndexList, m_fixedFrameIdx));
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcBasicFeature::SetSliceStructs()
    {
        DECODE_FUNC_CALL();

        PCODEC_AVC_SLICE_PARAMS slc = m_avcSliceParams;
        uint16_t frameInMbs = (m_avcPicParams->pic_height_in_mbs_minus1 + 1) * (m_avcPicParams->pic_width_in_mbs_minus1 + 1);

        //reset to default value for incoming new frame
        bool invalidSlicePresent = false;
        m_lastValidSlice = false;
        m_slcOffset = 0;
        m_slcLength = 0;

        SliceRecord initialValue = {0};
        std::fill(m_sliceRecord.begin(), m_sliceRecord.end(), initialValue);

        if (m_sliceRecord.size() < m_numSlices)
        {
            m_sliceRecord.resize(m_numSlices, {0, 0, 0});
        }

        for (uint32_t slcCount = 0; slcCount < m_numSlices; slcCount++)
        {
            //For DECE clear bytes calculation: Total bytes in the bit-stream consumed so far
            m_sliceRecord[slcCount].totalBytesConsumed = slc->slice_data_offset + slc->slice_data_size;

            if (invalidSlicePresent == true)
            {
                break;
            }

            if (m_sliceRecord[slcCount].skip)
            {
                continue;
            }

            m_slcLength = slc->slice_data_size;

            // error handling for garbage data
            if (((uint64_t)(slc->slice_data_offset) + m_slcLength) > m_dataSize)
            {
                slc++;
                m_sliceRecord[slcCount].skip = true;
                continue;
            }

            if (!m_shortFormatInUse)
            {
                if (slcCount < m_numSlices - 1)
                {
                    // Skip remaining slices if the number of MBs already reaches the total before the last slice or slice overlap occurs.
                    if (((slc->first_mb_in_slice + slc->NumMbsForSlice >= frameInMbs) || ((slc + 1)->first_mb_in_slice <= slc->first_mb_in_slice)))
                    {
                        uint32_t count = slcCount + 1;
                        slc->first_mb_in_next_slice = 0;
                        invalidSlicePresent = true;

                        while (count < m_numSlices)
                        {
                            m_sliceRecord[count++].skip = true;
                        }
                    }
                    else
                    {
                        slc->first_mb_in_next_slice = (slc + 1)->first_mb_in_slice;
                    }
                }
                else
                {
                    //For last slice, set it to 0
                    slc->first_mb_in_next_slice = 0;
                }

                m_slcOffset = (slc->slice_data_bit_offset >> 3) + m_osInterface->dwNumNalUnitBytesIncluded;

                // For first slice, first_mb_in_slice must be 0, otherwise it's corrupted, skip the slice when first_mb_in_slice corrupted.
                if ((m_slcOffset > m_slcLength) || (0 == slcCount && slc->first_mb_in_slice) || (slc->first_mb_in_slice >= frameInMbs)
                    || (m_avcPicParams->seq_fields.mb_adaptive_frame_field_flag && !m_avcPicParams->pic_fields.field_pic_flag &&
                    (slc->first_mb_in_slice >= frameInMbs / 2)))
                {
                    slc++;
                    m_sliceRecord[slcCount].skip = true;
                    continue;
                }
                m_slcLength -= m_slcOffset;
            }

            m_sliceRecord[slcCount].length = m_slcLength;
            m_sliceRecord[slcCount].offset = m_slcOffset;
            m_lastValidSlice = slcCount;
            slc++;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcBasicFeature::CheckBitDepthAndChromaSampling()
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_NULL(m_avcPicParams);

        if (m_avcPicParams->seq_fields.chroma_format_idc > avcChromaFormat420
            || m_avcPicParams->bit_depth_luma_minus8 > 0
            || m_avcPicParams->bit_depth_chroma_minus8 > 0)
        {
            DECODE_ASSERTMESSAGE("Only 4:2:0 8bit is supported!");
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcBasicFeature::SetRequiredBitstreamSize(uint32_t requiredSize)
    {
        DECODE_FUNC_CALL();
        m_dataSize = requiredSize;
        DECODE_NORMALMESSAGE("Estimate bitstream size in this Frame: %u", requiredSize);
        return MOS_STATUS_SUCCESS;
    }

}
