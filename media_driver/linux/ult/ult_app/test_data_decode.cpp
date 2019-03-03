/*
* Copyright (c) 2018, Intel Corporation
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
#include "test_data_decode.h"

using namespace std;

DecBufHEVC::DecBufHEVC()
{
    m_pps = make_shared<VAPictureParameterBufferHEVC>();
    memset(m_pps.get(), 0, sizeof(VAPictureParameterBufferHEVC));

    for (auto i = 0; i < 15; i++)
    {
        m_pps->ReferenceFrames[i].picture_id = 0xffffffff;
        m_pps->ReferenceFrames[i].flags      = 0x1;
    }
    m_pps->pic_width_in_luma_samples                  = 0x40;
    m_pps->pic_height_in_luma_samples                 = 0x40;
    m_pps->pic_fields.value                           = 0x20441;
    m_pps->sps_max_dec_pic_buffering_minus1           = 0x3;
    m_pps->pcm_sample_bit_depth_luma_minus1           = 0xff;
    m_pps->pcm_sample_bit_depth_chroma_minus1         = 0xff;
    m_pps->log2_diff_max_min_luma_coding_block_size   = 0x2;
    m_pps->log2_diff_max_min_transform_block_size     = 0x3;
    m_pps->log2_min_pcm_luma_coding_block_size_minus3 = 0xfd;
    m_pps->max_transform_hierarchy_depth_intra        = 0x2;
    m_pps->max_transform_hierarchy_depth_inter        = 0x2;
    m_pps->slice_parsing_fields.value                 = 0x3907;
    m_pps->log2_max_pic_order_cnt_lsb_minus4          = 0x2;
    m_pps->num_short_term_ref_pic_sets                = 0x4;
    m_pps->num_ref_idx_l0_default_active_minus1       = 0x2;

    m_slc = make_shared<VASliceParameterBufferHEVC>();
    memset(m_slc.get(), 0, sizeof(VASliceParameterBufferHEVC));

    m_slc->slice_data_size        = 0x10;
    m_slc->slice_data_byte_offset = 0x7;
    for (auto i = 0; i < 15; i++)
    {
        m_slc->RefPicList[0][i] = 0xff;
        m_slc->RefPicList[1][i] = 0xff;
    }
    m_slc->LongSliceFlags.value          = 0x1009;
    m_slc->collocated_ref_idx            = 0xff;
    m_slc->num_ref_idx_l0_active_minus1  = 0xff;
    m_slc->num_ref_idx_l1_active_minus1  = 0xff;
    m_slc->five_minus_max_num_merge_cand = 0x5;

    m_bitstream.assign({
        0x00, 0x00, 0x01, 0x26, 0x01, 0xaf, 0x1d, 0x80, 0xa3, 0xf3, 0xe6, 0x0b, 0x57, 0xd0, 0x9f, 0xf9});
}

DecBufAVC::DecBufAVC()
{
    m_pps = make_shared<VAPictureParameterBufferH264>();
    memset(m_pps.get(), 0, sizeof(VAPictureParameterBufferH264));

    m_pps->CurrPic.flags = 0x8;
    for (auto i = 0; i < 16; i++)
    {
        m_pps->ReferenceFrames[i].picture_id = 0xffffffff;
        m_pps->ReferenceFrames[i].flags      = 0x1;
    }
    m_pps->picture_width_in_mbs_minus1  = 0x27;
    m_pps->picture_height_in_mbs_minus1 = 0x1d;
    m_pps->num_ref_frames               = 0x2;
    m_pps->seq_fields.value             = 0x451;
    m_pps->pic_fields.value             = 0x519;

    m_slc = make_shared<VASliceParameterBufferH264>();
    memset(m_slc.get(), 0, sizeof(VASliceParameterBufferH264));

    m_slc->slice_data_size       = 0x10;
    m_slc->slice_data_bit_offset = 0x24;
    m_slc->slice_type            = 0x2;
    for (auto i = 0; i < 32; i++)
    {
        m_slc->RefPicList0[i].picture_id = 0xffffffff;
        m_slc->RefPicList0[i].flags      = 0x1;
        m_slc->RefPicList1[i].picture_id = 0xffffffff;
        m_slc->RefPicList1[i].flags      = 0x1;
    }

    m_bitstream.assign({
        0x00, 0x00, 0x01, 0x02, 0x01, 0xd0, 0x11, 0xff, 0xd4, 0x43, 0x02, 0x0e, 0x40, 0x92, 0xf9, 0x84});
}

DecTestDataHEVC::DecTestDataHEVC(FeatureID testFeatureID, bool bInShortFormat)
{
    m_bShortFormat = bInShortFormat;
    m_picWidth     = 64;
    m_picHeight    = 64;
    m_surfacesNum  = 32;
    m_featureId    = testFeatureID;

    m_confAttrib.resize(1);
    m_confAttrib[0].type  = VAConfigAttribDecSliceMode;
    m_confAttrib[0].value = (m_bShortFormat) ? VA_DEC_SLICE_MODE_BASE:VA_DEC_SLICE_MODE_NORMAL;

    m_resources.resize(m_surfacesNum);
    InitCompBuffers();

    vector<DecFrameDataHEVC> &currentArray = (m_bShortFormat) ? m_frameArray : m_frameArrayLong;
    m_num_frames = currentArray.size();

    m_compBufs.resize(m_num_frames);
    for (uint32_t i = 0; i < m_num_frames; i++) // Set for each frame
    {
        m_compBufs[i].resize(3);
        // Need one Picture Parameters for all the slices.
        m_compBufs[i][0] = { VAPictureParameterBufferType, (uint32_t)currentArray[i].picParam.size(), &currentArray[i].picParam[0], 0 };
        // Due to driver limiation, we just send all the slice data in one buffer.
        m_compBufs[i][1] = { VASliceParameterBufferType  , (uint32_t)currentArray[i].slcParam.size(), &currentArray[i].slcParam[0], 0 };
        m_compBufs[i][2] = { VASliceDataBufferType       , (uint32_t)currentArray[i].bsData.size()  , &currentArray[i].bsData[0]  , 0 };
    }
}

DecTestDataAVC::DecTestDataAVC(FeatureID testFeatureID, bool bInShortFormat)
{
    m_bShortFormat = bInShortFormat;
    m_picWidth     = 64;
    m_picHeight    = 64;
    m_surfacesNum  = 32;
    m_featureId    = testFeatureID;

    m_confAttrib.resize(1);
    m_confAttrib[0].type  = VAConfigAttribDecSliceMode;
    m_confAttrib[0].value = (m_bShortFormat) ? VA_DEC_SLICE_MODE_BASE:VA_DEC_SLICE_MODE_NORMAL;

    m_resources.resize(m_surfacesNum);
    InitCompBuffers();

    vector<DecFrameDataAVC> &currentArray = m_bShortFormat ? m_frameArray : m_frameArrayLong;
    m_num_frames                          = currentArray.size();

    m_compBufs.resize(m_num_frames);
    for (uint32_t i = 0; i < m_num_frames; i++) // Set for each frame
    {
        m_compBufs[i].resize(3);
        // Need one Picture Parameters for all the slices.
        m_compBufs[i][0] = { VAPictureParameterBufferType, (uint32_t)currentArray[i].picParam.size(), &currentArray[i].picParam[0], 0 };
        // Due to driver limiation, we just send all the slice data in one buffer.
        m_compBufs[i][1] = { VASliceParameterBufferType  , (uint32_t)currentArray[i].slcParam.size(), &currentArray[i].slcParam[0], 0 };
        m_compBufs[i][2] = { VASliceDataBufferType       , (uint32_t)currentArray[i].bsData.size()  , &currentArray[i].bsData[0]  , 0 };
    }
}

void DecTestDataHEVC::InitCompBuffers()
{
    m_frameArrayLong.resize(DEC_FRAME_NUM);
    m_pBufs = make_shared<DecBufHEVC>();
    for (auto i = 0; i < DEC_FRAME_NUM; i++)
    {
        m_frameArrayLong[i].picParam.assign((char*)m_pBufs->GetPpsBuf(), (char*)m_pBufs->GetPpsBuf() + m_pBufs->GetPpsSize());
        m_frameArrayLong[i].slcParam.assign((char*)m_pBufs->GetSlcBuf(), (char*)m_pBufs->GetSlcBuf() + m_pBufs->GetSlcSize());
        m_frameArrayLong[i].bsData.assign((char*)m_pBufs->GetBsBuf()   , (char*)m_pBufs->GetBsBuf()  + m_pBufs->GetBsSize());
    }
}

void DecTestDataAVC::InitCompBuffers()
{
    m_frameArrayLong.resize(DEC_FRAME_NUM);
    m_pBufs = make_shared<DecBufAVC>();
    for (auto i = 0; i < DEC_FRAME_NUM; i++)
    {
        m_frameArrayLong[i].picParam.assign((char*)m_pBufs->GetPpsBuf(), (char*)m_pBufs->GetPpsBuf() + m_pBufs->GetPpsSize());
        m_frameArrayLong[i].slcParam.assign((char*)m_pBufs->GetSlcBuf(), (char*)m_pBufs->GetSlcBuf() + m_pBufs->GetSlcSize());
        m_frameArrayLong[i].bsData.assign((char*)m_pBufs->GetBsBuf()   , (char*)m_pBufs->GetBsBuf()  + m_pBufs->GetBsSize());
    }
}

void DecTestDataHEVC::UpdateCompBuffers(int frameId)
{
    auto &currentArray      = (m_bShortFormat) ? m_frameArray : m_frameArrayLong;
    auto *pps               = (VAPictureParameterBufferHEVC *)&currentArray[frameId].picParam[0];
    auto *slc               = (VASliceParameterBufferHEVC *)&currentArray[frameId].slcParam[0];
    auto &bitstream         = currentArray[frameId].bsData;

    pps->CurrPic.picture_id = m_resources[frameId];
    switch (frameId)
    {
    case 1:
        pps->ReferenceFrames[0].picture_id    = m_resources[0];
        pps->CurrPic.pic_order_cnt            = 0x2;
        pps->ReferenceFrames[0].flags         = 0x10;
        pps->slice_parsing_fields.value       = 0x107;
        pps->st_rps_bits                      = 0x9;
        slc->slice_data_byte_offset           = 0x9;
        slc->RefPicList[0][0]                 = 0;
        slc->RefPicList[1][0]                 = 0;
        slc->LongSliceFlags.value             = 0x1401;
        slc->collocated_ref_idx               = 0;
        slc->num_ref_idx_l0_active_minus1     = 0;
        slc->num_ref_idx_l1_active_minus1     = 0;
        slc->five_minus_max_num_merge_cand    = 0;
        bitstream.assign({
            0x00, 0x00, 0x01, 0x02, 0x01, 0xd0, 0x09, 0x7e, 0x10, 0xc2, 0x0e, 0xc0, 0xfd, 0xb5, 0xce, 0x30});
        break;
    case 2:
        pps->ReferenceFrames[0].picture_id    = m_resources[0];
        pps->ReferenceFrames[1].picture_id    = m_resources[1];
        pps->CurrPic.pic_order_cnt            = 0x1;
        pps->ReferenceFrames[0].flags         = 0x10;
        pps->slice_parsing_fields.value       = 0x107;
        pps->st_rps_bits                      = 0xa;
        pps->ReferenceFrames[1].pic_order_cnt = 0x2;
        pps->ReferenceFrames[1].flags         = 0x20;
        slc->slice_data_byte_offset           = 0xa;
        slc->RefPicList[0][0]                 = 0;
        slc->RefPicList[1][0]                 = 0x1;
        slc->LongSliceFlags.value             = 0x1401;
        slc->collocated_ref_idx               = 0;
        slc->num_ref_idx_l0_active_minus1     = 0;
        slc->num_ref_idx_l1_active_minus1     = 0;
        slc->five_minus_max_num_merge_cand    = 0;
        bitstream.assign({
            0x00, 0x00, 0x01, 0x02, 0x01, 0xd0, 0x11, 0xff, 0xd4, 0x43, 0x02, 0x0e, 0x40, 0x92, 0xf9, 0x84});
        break;
    default:
        break;
    }
}

void DecTestDataAVC::UpdateCompBuffers(int frameId)
{
    auto &currentArray      = m_bShortFormat ? m_frameArray : m_frameArrayLong;
    auto *pps               = (VAPictureParameterBufferH264 *)&currentArray[frameId].picParam[0];
    auto *slc               = (VASliceParameterBufferH264 *)&currentArray[frameId].slcParam[0];

    pps->CurrPic.picture_id = m_resources[frameId];
    switch (frameId)
    {
    case 1:
        pps->ReferenceFrames[0].picture_id          = m_resources[0];
        slc->RefPicList0[0].picture_id              = m_resources[0];
        slc->RefPicList1[0].picture_id              = m_resources[0];
        pps->CurrPic.frame_idx                      = 0x1;
        pps->CurrPic.TopFieldOrderCnt               = 0x6;
        pps->CurrPic.BottomFieldOrderCnt            = 0x6;
        pps->ReferenceFrames[0].flags               = 0x8;
        pps->frame_num                              = 0x1;
        slc->slice_data_bit_offset                  = 0x23;
        slc->slice_type                             = 0;
        slc->RefPicList0[0].flags                   = 0x8;
        break;
    case 2:
        pps->ReferenceFrames[0].picture_id          = m_resources[0];
        pps->ReferenceFrames[1].picture_id          = m_resources[1];
        slc->RefPicList0[0].picture_id              = m_resources[0];
        slc->RefPicList1[0].picture_id              = m_resources[1];
        pps->CurrPic.frame_idx                      = 0x2;
        pps->CurrPic.flags                          = 0;
        pps->CurrPic.TopFieldOrderCnt               = 0x2;
        pps->CurrPic.BottomFieldOrderCnt            = 0x2;
        pps->ReferenceFrames[1].frame_idx           = 0x1;
        pps->ReferenceFrames[1].flags               = 0x8;
        pps->ReferenceFrames[1].TopFieldOrderCnt    = 0x6;
        pps->ReferenceFrames[1].BottomFieldOrderCnt = 0x6;
        pps->pic_fields.value                       = 0x119;
        pps->frame_num                              = 0x2;
        slc->slice_data_bit_offset                  = 0x24;
        slc->slice_type                             = 0x1;
        slc->direct_spatial_mv_pred_flag            = 0x1;
        slc->RefPicList0[0].flags                   = 0x8;
        slc->RefPicList1[0].frame_idx               = 0x1;
        slc->RefPicList1[0].flags                   = 0x8;
        slc->RefPicList1[0].TopFieldOrderCnt        = 0x6;
        slc->RefPicList1[0].BottomFieldOrderCnt     = 0x6;
        break;
    default:
        break;
    }
}
