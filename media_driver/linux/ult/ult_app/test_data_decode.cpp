/*
* Copyright (c) 2017, Intel Corporation
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

DecBufHEVC::DecBufHEVC()
{
    pps = make_shared<VAPictureParameterBufferHEVC>();
    memset(pps.get(), 0, sizeof(VAPictureParameterBufferHEVC));

    for (auto i = 0; i < 15; i++)
    {
        pps->ReferenceFrames[i].picture_id = 0xffffffff;
        pps->ReferenceFrames[i].flags = 0x1;
    }
    pps->pic_width_in_luma_samples = 0x40;
    pps->pic_height_in_luma_samples = 0x40;
    pps->pic_fields.value = 0x20441;
    pps->sps_max_dec_pic_buffering_minus1 = 0x3;
    pps->pcm_sample_bit_depth_luma_minus1 = 0xff;
    pps->pcm_sample_bit_depth_chroma_minus1 = 0xff;
    pps->log2_diff_max_min_luma_coding_block_size = 0x2;
    pps->log2_diff_max_min_transform_block_size = 0x3;
    pps->log2_min_pcm_luma_coding_block_size_minus3 = 0xfd;
    pps->max_transform_hierarchy_depth_intra = 0x2;
    pps->max_transform_hierarchy_depth_inter = 0x2;
    pps->slice_parsing_fields.value = 0x3907;
    pps->log2_max_pic_order_cnt_lsb_minus4 = 0x2;
    pps->num_short_term_ref_pic_sets = 0x4;
    pps->num_ref_idx_l0_default_active_minus1 = 0x2;

    slc = make_shared<VASliceParameterBufferHEVC>();
    memset(slc.get(), 0, sizeof(VASliceParameterBufferHEVC));

    slc->slice_data_size = 0x10;
    slc->slice_data_byte_offset = 0x7;
    for (auto i = 0; i < 15; i++)
    {
        slc->RefPicList[0][i] = 0xff;
        slc->RefPicList[1][i] = 0xff;
    }
    slc->LongSliceFlags.value = 0x1009;
    slc->collocated_ref_idx = 0xff;
    slc->num_ref_idx_l0_active_minus1 = 0xff;
    slc->num_ref_idx_l1_active_minus1 = 0xff;
    slc->five_minus_max_num_merge_cand = 0x5;

    bitstream.assign({
        0x00, 0x00, 0x01, 0x26, 0x01, 0xaf, 0x1d, 0x80, 0xa3, 0xf3, 0xe6, 0x0b, 0x57, 0xd0, 0x9f, 0xf9
        });
}

DecBufAVC::DecBufAVC()
{
    pps = make_shared<VAPictureParameterBufferH264>();
    memset(pps.get(), 0, sizeof(VAPictureParameterBufferH264));

    pps->CurrPic.flags = 0x8;
    for (auto i = 0; i < 16; i++)
    {
        pps->ReferenceFrames[i].picture_id = 0xffffffff;
        pps->ReferenceFrames[i].flags = 0x1;
    }
    pps->picture_width_in_mbs_minus1 = 0x27;
    pps->picture_height_in_mbs_minus1 = 0x1d;
    pps->num_ref_frames = 0x2;
    pps->seq_fields.value = 0x451;
    pps->pic_fields.value = 0x519;

    slc = make_shared<VASliceParameterBufferH264>();
    memset(slc.get(), 0, sizeof(VASliceParameterBufferH264));

    slc->slice_data_size = 0x10;
    slc->slice_data_bit_offset = 0x24;
    slc->slice_type = 0x2;
    for (auto i = 0; i < 32; i++)
    {
        slc->RefPicList0[i].picture_id = 0xffffffff;
        slc->RefPicList0[i].flags = 0x1;
        slc->RefPicList1[i].picture_id = 0xffffffff;
        slc->RefPicList1[i].flags = 0x1;
    }

    bitstream.assign({
        0x00, 0x00, 0x01, 0x02, 0x01, 0xd0, 0x11, 0xff, 0xd4, 0x43, 0x02, 0x0e, 0x40, 0x92, 0xf9, 0x84
        });
}

DecTestDataHEVC::DecTestDataHEVC(FeatureID testFeatureID, bool bInShortFormat)
{
    bShortFormat = bInShortFormat;
    picWidth = 64;
    picHeight = 64;
    SurfacesNum = 32;
    featureId = testFeatureID;

    ConfAttrib.resize(1);
    ConfAttrib[0].type = VAConfigAttribDecSliceMode;
    ConfAttrib[0].value = (bShortFormat) ? VA_DEC_SLICE_MODE_BASE:VA_DEC_SLICE_MODE_NORMAL;
    //Infact, Lucas tool keep below attrib as default in driver.
    //config_attrib.type = VAConfigAttribEncryption;
    //config_attrib.type = VAConfigAttribDecProcessing;

    resources.resize(SurfacesNum);
    InitCompBuffers();

    vector<DecFrameDataHEVC> &currentArray = (bShortFormat) ? frameArray : frameArrayLong;
    num_frames =currentArray.size();

    compBufs.resize(num_frames);
    for (uint32_t i = 0; i < num_frames; i++) //Set for each frame
    {
        compBufs[i].resize(3);
        //we need one Picture Parameters for all the slices.
        compBufs[i][0] = { VAPictureParameterBufferType, (uint32_t)currentArray[i].picParam.size(), &currentArray[i].picParam[0], 0 };
        //Due to driver limiation, we just send all the slice data in one buffer.
        compBufs[i][1] = { VASliceParameterBufferType, (uint32_t)currentArray[i].slcParam.size(), &currentArray[i].slcParam[0], 0 };
        compBufs[i][2] = { VASliceDataBufferType, (uint32_t)currentArray[i].bsData.size(), &currentArray[i].bsData[0], 0 };
    }
}

DecTestDataAVC::DecTestDataAVC(FeatureID testFeatureID, bool bInShortFormat)
{
    bShortFormat = bInShortFormat;
    picWidth = 64;
    picHeight = 64;
    SurfacesNum = 32;
    featureId = testFeatureID;

    ConfAttrib.resize(1);
    ConfAttrib[0].type = VAConfigAttribDecSliceMode;
    ConfAttrib[0].value = (bShortFormat) ? VA_DEC_SLICE_MODE_BASE:VA_DEC_SLICE_MODE_NORMAL;
    //Infact, Lucas tool keep below attrib as default in driver.
    //config_attrib.type = VAConfigAttribEncryption;
    //config_attrib.type = VAConfigAttribDecProcessing;

    resources.resize(SurfacesNum);
    InitCompBuffers();

    vector<DecFrameDataAVC> &currentArray = (bShortFormat) ? frameArray : frameArrayLong;
    num_frames =currentArray.size();

    compBufs.resize(num_frames);
    for (uint32_t i = 0; i < num_frames; i++) //Set for each frame
    {
        compBufs[i].resize(3);
        //we need one Picture Parameters for all the slices.
        compBufs[i][0] = { VAPictureParameterBufferType, (uint32_t)currentArray[i].picParam.size(), &currentArray[i].picParam[0], 0 };
        //Due to driver limiation, we just send all the slice data in one buffer.
        compBufs[i][1] = { VASliceParameterBufferType, (uint32_t)currentArray[i].slcParam.size(), &currentArray[i].slcParam[0], 0 };
        compBufs[i][2] = { VASliceDataBufferType, (uint32_t)currentArray[i].bsData.size(), &currentArray[i].bsData[0], 0 };
    }
}

void DecTestDataHEVC::InitCompBuffers()
{
    frameArrayLong.resize(DEC_FRAME_NUM);
    pBufs = make_shared<DecBufHEVC>();
    for (auto i = 0; i < DEC_FRAME_NUM; i++)
    {
        frameArrayLong[i].picParam.assign((char*)pBufs->GetPpsBuf(), (char*)pBufs->GetPpsBuf() + pBufs->GetPpsSize());
        frameArrayLong[i].slcParam.assign((char*)pBufs->GetSlcBuf(), (char*)pBufs->GetSlcBuf() + pBufs->GetSlcSize());
        frameArrayLong[i].bsData.assign((char*)pBufs->GetBsBuf(), (char*)pBufs->GetBsBuf() + pBufs->GetBsSize());
    }
}
void DecTestDataAVC::InitCompBuffers()
{
    frameArrayLong.resize(DEC_FRAME_NUM);
    pBufs = make_shared<DecBufAVC>();
    for (auto i = 0; i < DEC_FRAME_NUM; i++)
    {
        frameArrayLong[i].picParam.assign((char*)pBufs->GetPpsBuf(), (char*)pBufs->GetPpsBuf() + pBufs->GetPpsSize());
        frameArrayLong[i].slcParam.assign((char*)pBufs->GetSlcBuf(), (char*)pBufs->GetSlcBuf() + pBufs->GetSlcSize());
        frameArrayLong[i].bsData.assign((char*)pBufs->GetBsBuf(), (char*)pBufs->GetBsBuf() + pBufs->GetBsSize());
    }
}

void DecTestDataHEVC::UpdateCompBuffers(int frameId)
{
    auto &currentArray = (bShortFormat) ? frameArray : frameArrayLong;
    auto *pps = (VAPictureParameterBufferHEVC*)&currentArray[frameId].picParam[0];
    auto *slc = (VASliceParameterBufferHEVC*)&currentArray[frameId].slcParam[0];
    auto &bitstream = currentArray[frameId].bsData;

    pps->CurrPic.picture_id = resources[frameId];
    switch (frameId)
    {
    case 1:
        pps->ReferenceFrames[0].picture_id = resources[0];
        pps->CurrPic.pic_order_cnt = 0x2;
        pps->ReferenceFrames[0].flags = 0x10;
        pps->slice_parsing_fields.value = 0x107;
        pps->st_rps_bits = 0x9;
        slc->slice_data_byte_offset = 0x9;
        slc->RefPicList[0][0] = 0;
        slc->RefPicList[1][0] = 0;
        slc->LongSliceFlags.value = 0x1401;
        slc->collocated_ref_idx = 0;
        slc->num_ref_idx_l0_active_minus1 = 0;
        slc->num_ref_idx_l1_active_minus1 = 0;
        slc->five_minus_max_num_merge_cand = 0;
        bitstream.assign({
        0x00, 0x00, 0x01, 0x02, 0x01, 0xd0, 0x09, 0x7e, 0x10, 0xc2, 0x0e, 0xc0, 0xfd, 0xb5, 0xce, 0x30
        });
        break;
    case 2:
        pps->ReferenceFrames[0].picture_id = resources[0];
        pps->ReferenceFrames[1].picture_id = resources[1];
        pps->CurrPic.pic_order_cnt = 0x1;
        pps->ReferenceFrames[0].flags = 0x10;
        pps->slice_parsing_fields.value = 0x107;
        pps->st_rps_bits = 0xa;
        pps->ReferenceFrames[1].pic_order_cnt = 0x2;
        pps->ReferenceFrames[1].flags = 0x20;
        slc->slice_data_byte_offset = 0xa;
        slc->RefPicList[0][0] = 0;
        slc->RefPicList[1][0] = 0x1;
        slc->LongSliceFlags.value = 0x1401;
        slc->collocated_ref_idx = 0;
        slc->num_ref_idx_l0_active_minus1 = 0;
        slc->num_ref_idx_l1_active_minus1 = 0;
        slc->five_minus_max_num_merge_cand = 0;
        bitstream.assign({
        0x00, 0x00, 0x01, 0x02, 0x01, 0xd0, 0x11, 0xff, 0xd4, 0x43, 0x02, 0x0e, 0x40, 0x92, 0xf9, 0x84
        });
        break;
    default:
        break;
    }
}

void DecTestDataAVC::UpdateCompBuffers(int frameId)
{
    auto &currentArray = (bShortFormat) ? frameArray : frameArrayLong;
    auto *pps = (VAPictureParameterBufferH264*)&currentArray[frameId].picParam[0];
    auto *slc = (VASliceParameterBufferH264*)&currentArray[frameId].slcParam[0];

    pps->CurrPic.picture_id = resources[frameId];
    switch (frameId)
    {
    case 1:
        pps->ReferenceFrames[0].picture_id = resources[0];
        slc->RefPicList0[0].picture_id = resources[0];
        slc->RefPicList1[0].picture_id = resources[0];
        pps->CurrPic.frame_idx = 0x1;
        pps->CurrPic.TopFieldOrderCnt = 0x6;
        pps->CurrPic.BottomFieldOrderCnt = 0x6;
        pps->ReferenceFrames[0].flags = 0x8;
        pps->frame_num = 0x1;
        slc->slice_data_bit_offset = 0x23;
        slc->slice_type = 0;
        slc->RefPicList0[0].flags = 0x8;
        break;
    case 2:
        pps->ReferenceFrames[0].picture_id = resources[0];
        pps->ReferenceFrames[1].picture_id = resources[1];
        slc->RefPicList0[0].picture_id = resources[0];
        slc->RefPicList1[0].picture_id = resources[1];
        pps->CurrPic.frame_idx = 0x2;
        pps->CurrPic.flags = 0;
        pps->CurrPic.TopFieldOrderCnt = 0x2;
        pps->CurrPic.BottomFieldOrderCnt = 0x2;
        pps->ReferenceFrames[1].frame_idx = 0x1;
        pps->ReferenceFrames[1].flags = 0x8;
        pps->ReferenceFrames[1].TopFieldOrderCnt = 0x6;
        pps->ReferenceFrames[1].BottomFieldOrderCnt = 0x6;
        pps->pic_fields.value = 0x119;
        pps->frame_num = 0x2;
        slc->slice_data_bit_offset = 0x24;
        slc->slice_type = 0x1;
        slc->direct_spatial_mv_pred_flag = 0x1;
        slc->RefPicList0[0].flags = 0x8;
        slc->RefPicList1[0].frame_idx = 0x1;
        slc->RefPicList1[0].flags = 0x8;
        slc->RefPicList1[0].TopFieldOrderCnt = 0x6;
        slc->RefPicList1[0].BottomFieldOrderCnt = 0x6;
        break;
    default:
        break;
    }
}