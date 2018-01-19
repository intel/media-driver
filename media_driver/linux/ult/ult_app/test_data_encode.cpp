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
#include "test_data_encode.h"

HevcEncBufs::HevcEncBufs()
{
    sps = make_shared<VAEncSequenceParameterBufferHEVC>();
    memset(sps.get(), 0, sizeof(VAEncSequenceParameterBufferHEVC));

    sps->general_profile_idc = 0x1;
    sps->general_level_idc = 0x5a;
    sps->intra_period = 0x5;
    sps->intra_idr_period = 0xffffffff;
    sps->ip_period = 0x2;
    sps->bits_per_second = 0x190;
    sps->pic_width_in_luma_samples = 0x140;
    sps->pic_height_in_luma_samples = 0xf0;
    sps->seq_fields.value = 0xc801;
    sps->log2_diff_max_min_luma_coding_block_size = 0x2;
    sps->log2_diff_max_min_transform_block_size = 0x3;
    sps->max_transform_hierarchy_depth_inter = 0x2;
    sps->max_transform_hierarchy_depth_intra = 0x2;
    sps->pcm_sample_bit_depth_luma_minus1 = 0x7;
    sps->pcm_sample_bit_depth_chroma_minus1 = 0x7;

    pps = make_shared<VAEncPictureParameterBufferHEVC>();
    memset(pps.get(), 0, sizeof(VAEncPictureParameterBufferHEVC));

    slc = make_shared<VAEncSliceParameterBufferHEVC>();
    memset(slc.get(), 0, sizeof(VAEncSliceParameterBufferHEVC));

    for (auto i = 0; i < 15; i++)
    {
        pps->reference_frames[i].picture_id = 0xffffffff;
        pps->reference_frames[i].flags = 0x1;
    }

    for (auto i = 0; i < 15; i++)
    {
        slc->ref_pic_list0[i].picture_id = 0xffffffff;
        slc->ref_pic_list0[i].flags = 0x1;
        slc->ref_pic_list1[i].picture_id = 0xffffffff;
        slc->ref_pic_list1[i].flags = 0x1;
    }

    pps->column_width_minus1[0] = 0x9;
    pps->row_height_minus1[0] = 0x7;
    pps->pic_fields.value = 0x215;
    pps->diff_cu_qp_delta_depth = 0x2;
    slc->num_ctu_in_slice = 0x50;
    slc->max_num_merge_cand = 0x5;
    slc->slice_beta_offset_div2 = 0x2;
    slc->slice_fields.value = 0x2010;

}

AvcEncBufs::AvcEncBufs()
{
    sps = make_shared<VAEncSequenceParameterBufferH264>();
    memset(sps.get(), 0, sizeof(VAEncSequenceParameterBufferH264));

    sps->level_idc = 0x33;
    sps->intra_period = 0x1f;
    sps->intra_idr_period = 0x3d;
    sps->ip_period = 0x2;
    sps->bits_per_second = 0x4c4b40;
    sps->max_num_ref_frames = 0x10;
    sps->picture_width_in_mbs = 0x14;
    sps->picture_height_in_mbs = 0xf;
    sps->seq_fields.value = 0x3105;

    pps = make_shared<VAEncPictureParameterBufferH264>();
    memset(pps.get(), 0, sizeof(VAEncPictureParameterBufferH264));

    slc = make_shared<VAEncSliceParameterBufferH264>();
    memset(slc.get(), 0, sizeof(VAEncSliceParameterBufferH264));

    for (auto i = 0; i < 16; i++)
    {
        pps->ReferenceFrames[i].picture_id = 0xffffffff;
        pps->ReferenceFrames[i].flags = 0x1;
    }

    for (auto i = 0; i < 32; i++)
    {
        slc->RefPicList0[i].picture_id = 0xffffffff;
        slc->RefPicList0[i].flags = 0x1;
        slc->RefPicList1[i].picture_id = 0xffffffff;
        slc->RefPicList1[i].flags = 0x1;
    }
    slc->direct_spatial_mv_pred_flag = 0x1;
    slc->num_ref_idx_active_override_flag = 0x1;

    slc->cabac_init_idc = 0x1;
    slc->slice_alpha_c0_offset_div2 = 0x2;
    slc->slice_beta_offset_div2 = 0x2;
    slc->num_macroblocks = 0x12c;

}

EncTestDataHEVC::EncTestDataHEVC(FeatureID testFeatureID)
{
    featureId = testFeatureID;
    picWidth = 320;
    picHeight = 240;
    SurfacesNum= 1+15 ;// 1 raw data, 8 references.

    ConfAttrib.resize(1);
    ConfAttrib[0].type = VAConfigAttribRateControl;
    ConfAttrib[0].value = VA_RC_CQP;

    SurfAttrib.resize(1);
    SurfAttrib[0].type          = VASurfaceAttribPixelFormat;
    SurfAttrib[0].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
    SurfAttrib[0].value.type    = VAGenericValueTypePointer;
    SurfAttrib[0].value.value.i = VA_FOURCC_NV12;

    resources.resize(SurfacesNum);

    InitCompBuffers();
    vector<EncFrameDataHEVC> &currentArray = frameArray;
    num_frames =currentArray.size();
    compBufs.resize(num_frames);

    packedsps.type = VAEncPackedHeaderSequence;
    packedsps.bit_length = 4*8;
    packedsps.has_emulation_bytes = 0;
    packedpps.type = VAEncPackedHeaderPicture;
    packedpps.bit_length = 4*8;
    packedpps.has_emulation_bytes = 0;
    packedsh.type = VAEncPackedHeaderSlice;
    packedsh.bit_length = 4*8;
    packedsh.has_emulation_bytes = 0;

    for (uint32_t i = 0; i < num_frames; i++) //Set for each frame
    {
        compBufs[i].resize(12);
        //BS buffer
        compBufs[i][0] = { VAEncCodedBufferType, (picWidth*picHeight*3)>>1, nullptr, 0 };
        //Parameters buffer SPS, PPS, SliceHeader
        compBufs[i][1] = { VAEncSequenceParameterBufferType, (uint32_t)currentArray[i].spsData.size(), &currentArray[i].spsData[0], 0 };
        compBufs[i][2] = { VAEncPictureParameterBufferType, (uint32_t)currentArray[i].ppsData.size(), &currentArray[i].ppsData[0], 0 };
        compBufs[i][3] = { VAEncSliceParameterBufferType, (uint32_t)currentArray[i].sliceData.size(), &currentArray[i].sliceData[0], 0 };

        //Packed VPS, SPS, PPS, SliceHeader, we fix all the data length as 4bytes.
        compBufs[i][4] = { VAEncPackedHeaderParameterBufferType, (uint32_t)sizeof(packedsps), (void*)&packedsps, 0 };
        compBufs[i][5] = { VAEncPackedHeaderDataBufferType, 4, (void*)&headerData[0], 0 };
        compBufs[i][6] = { VAEncPackedHeaderParameterBufferType, (uint32_t)sizeof(packedsps), (void*)&packedsps, 0 };
        compBufs[i][7] = { VAEncPackedHeaderDataBufferType, 4, (void*)&headerData[0], 0 };
        compBufs[i][8] = { VAEncPackedHeaderParameterBufferType, (uint32_t)sizeof(packedpps), (void*)&packedpps, 0 };
        compBufs[i][9] = { VAEncPackedHeaderDataBufferType, 4, (void*)&headerData[0], 0 };
        compBufs[i][10] = { VAEncPackedHeaderParameterBufferType, (uint32_t)sizeof(packedsh), (void*)&packedsh, 0 };
        compBufs[i][11] = { VAEncPackedHeaderDataBufferType, 4, (void*)&headerData[0], 0 };

    }
}

void EncTestDataHEVC::InitCompBuffers()
{
    frameArray.resize(ENC_FRAME_NUM);
    pBufs = make_shared<HevcEncBufs>();

    for (auto i = 0; i < ENC_FRAME_NUM; i++)
    {
        frameArray[i].spsData.assign((char*)pBufs->GetSpsBuf(), (char*)pBufs->GetSpsBuf() + pBufs->GetSpsSize());
        frameArray[i].ppsData.assign((char*)pBufs->GetPpsBuf(), (char*)pBufs->GetPpsBuf() + pBufs->GetPpsSize());
        frameArray[i].sliceData.assign((char*)pBufs->GetSlcBuf(), (char*)pBufs->GetSlcBuf() + pBufs->GetSlcSize());
    }
}
void EncTestDataHEVC::UpdateCompBuffers(int frameId)
{
    VAEncPictureParameterBufferHEVC* pps = (VAEncPictureParameterBufferHEVC*)&frameArray[frameId].ppsData[0];
    VAEncSliceParameterBufferHEVC* slc = (VAEncSliceParameterBufferHEVC*)&frameArray[frameId].sliceData[0];

    pps->coded_buf = compBufs[frameId][0].BufID;
    pps->decoded_curr_pic.picture_id = resources[frameId];

    if (frameId == 0)//I frame
    {
        pps->decoded_curr_pic.pic_order_cnt = 0x0;
        pps->pic_init_qp = 0xf;
        slc->slice_type = 0x2;
    }
    else if (frameId == 1)//NoDelay B frame
    {
        pps->decoded_curr_pic.pic_order_cnt = 0x2;
        pps->reference_frames[0].flags = 0;
        pps->reference_frames[0].picture_id = resources[0];
        pps->reference_frames[0].pic_order_cnt = 0x0;
        pps->pic_init_qp = 0x19;
        pps->pic_fields.value = 0x210;

        slc->slice_type = 0;
        slc->ref_pic_list0[0].picture_id = resources[0];
        slc->ref_pic_list0[0].flags = 0;
        slc->ref_pic_list0[0].pic_order_cnt = 0x0;
        slc->ref_pic_list1[0].picture_id = resources[0];
        slc->ref_pic_list1[0].flags = 0;
        slc->ref_pic_list1[0].pic_order_cnt = 0x0;
    }
    else if (frameId == 2)//B frame
    {
        pps->decoded_curr_pic.pic_order_cnt = 0x1;
        pps->reference_frames[0].flags = 0;
        pps->reference_frames[0].picture_id = resources[0];
        pps->reference_frames[0].pic_order_cnt = 0x0;
        pps->reference_frames[1].flags = 0;
        pps->reference_frames[1].picture_id = resources[1];
        pps->reference_frames[1].pic_order_cnt = 0x2;
        pps->pic_init_qp = 0x19;
        pps->pic_fields.value = 0x200;
        slc->slice_type = 0;
        slc->ref_pic_list0[0].picture_id = resources[0];
        slc->ref_pic_list0[0].flags = 0;
        slc->ref_pic_list1[0].pic_order_cnt = 0x0;
        slc->ref_pic_list1[0].picture_id = resources[1];
        slc->ref_pic_list1[0].flags = 0;
        slc->ref_pic_list1[0].pic_order_cnt = 0x2;
    }
}

EncTestDataAVC::EncTestDataAVC(FeatureID testFeatureID)
{
    featureId = testFeatureID;
    picWidth = 320;
    picHeight = 240;
    SurfacesNum= 1+15 ;// 1 raw data, 8 references.

    ConfAttrib.resize(1);
    ConfAttrib[0].type = VAConfigAttribRateControl;
    ConfAttrib[0].value = VA_RC_CQP;

    SurfAttrib.resize(1);
    SurfAttrib[0].type          = VASurfaceAttribPixelFormat;
    SurfAttrib[0].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
    SurfAttrib[0].value.type    = VAGenericValueTypePointer;
    SurfAttrib[0].value.value.i = VA_FOURCC_NV12;

    resources.resize(SurfacesNum);

    InitCompBuffers();
    vector<EncFrameDataHEVC> &currentArray = frameArray;
    num_frames =currentArray.size();
    compBufs.resize(num_frames);

    packedsps.type = VAEncPackedHeaderSequence;
    packedsps.bit_length = 4*8;
    packedsps.has_emulation_bytes = 0;
    packedpps.type = VAEncPackedHeaderPicture;
    packedpps.bit_length = 4*8;
    packedpps.has_emulation_bytes = 0;
    packedsh.type = VAEncPackedHeaderSlice;
    packedsh.bit_length = 4*8;
    packedsh.has_emulation_bytes = 0;

    for (uint32_t i = 0; i < num_frames; i++) //Set for each frame
    {
        compBufs[i].resize(12);
        //BS buffer
        compBufs[i][0] = { VAEncCodedBufferType, (picWidth*picHeight*3)>>1, nullptr, 0 };
        //Parameters buffer SPS, PPS, SliceHeader
        compBufs[i][1] = { VAEncSequenceParameterBufferType, (uint32_t)currentArray[i].spsData.size(), &currentArray[i].spsData[0], 0 };
        compBufs[i][2] = { VAEncPictureParameterBufferType, (uint32_t)currentArray[i].ppsData.size(), &currentArray[i].ppsData[0], 0 };
        compBufs[i][3] = { VAEncSliceParameterBufferType, (uint32_t)currentArray[i].sliceData.size(), &currentArray[i].sliceData[0], 0 };

        //Infact, AVC doesn't need VPS, just add here for future improvement if we can merge it with HEVC.
        //Packed VPS, SPS, PPS, SliceHeader, we fix all the data length as 4bytes.
        compBufs[i][4] = { VAEncPackedHeaderParameterBufferType, (uint32_t)sizeof(packedsps), (void*)&packedsps, 0 };
        compBufs[i][5] = { VAEncPackedHeaderDataBufferType, 4, (void*)&headerData[0], 0 };
        compBufs[i][6] = { VAEncPackedHeaderParameterBufferType, (uint32_t)sizeof(packedsps), (void*)&packedsps, 0 };
        compBufs[i][7] = { VAEncPackedHeaderDataBufferType, 4, (void*)&headerData[0], 0 };
        compBufs[i][8] = { VAEncPackedHeaderParameterBufferType, (uint32_t)sizeof(packedpps), (void*)&packedpps, 0 };
        compBufs[i][9] = { VAEncPackedHeaderDataBufferType, 4, (void*)&headerData[0], 0 };
        compBufs[i][10] = { VAEncPackedHeaderParameterBufferType, (uint32_t)sizeof(packedsh), (void*)&packedsh, 0 };
        compBufs[i][11] = { VAEncPackedHeaderDataBufferType, 4, (void*)&headerData[0], 0 };

    }
}

void EncTestDataAVC::InitCompBuffers()
{
    frameArray.resize(3);
    pBufs = make_shared<AvcEncBufs>();

    for (auto i = 0; i < ENC_FRAME_NUM; i++)
    {
        frameArray[i].spsData.assign((char*)pBufs->GetSpsBuf(), (char*)pBufs->GetSpsBuf() + pBufs->GetSpsSize());
        frameArray[i].ppsData.assign((char*)pBufs->GetPpsBuf(), (char*)pBufs->GetPpsBuf() + pBufs->GetPpsSize());
        frameArray[i].sliceData.assign((char*)pBufs->GetSlcBuf(), (char*)pBufs->GetSlcBuf() + pBufs->GetSlcSize());
    }
}
void EncTestDataAVC::UpdateCompBuffers(int frameId)
{
    VAEncPictureParameterBufferH264* pps = (VAEncPictureParameterBufferH264*)&frameArray[frameId].ppsData[0];
    VAEncSliceParameterBufferH264* slc = (VAEncSliceParameterBufferH264*)&frameArray[frameId].sliceData[0];

    pps->coded_buf = compBufs[frameId][0].BufID;
    pps->CurrPic.picture_id = resources[frameId];

    if (frameId==0)
    {
        pps->CurrPic.BottomFieldOrderCnt = 0x1;
        pps->pic_init_qp = 0x1c;
        pps->pic_fields.value = 0x30b;
        slc->slice_type = 0x7;
    }
    else if (frameId == 1)
    {
        pps->CurrPic.frame_idx = 0x1;
        pps->CurrPic.TopFieldOrderCnt = 0x4;
        pps->CurrPic.BottomFieldOrderCnt = 0x5;
        pps->ReferenceFrames[0].picture_id = resources[0];
        pps->ReferenceFrames[0].flags = 0x8;
        pps->ReferenceFrames[0].TopFieldOrderCnt = 0x0;
        pps->ReferenceFrames[0].BottomFieldOrderCnt = 0x1;
        pps->frame_num = 0x1;
        pps->pic_fields.value = 0x30a;
        slc->idr_pic_id = 0x1;
        slc->slice_type = 0;
        slc->pic_order_cnt_lsb = 0x4;
        slc->RefPicList0[0].picture_id = resources[0];
        slc->RefPicList0[0].flags = 0x8;
        slc->RefPicList0[0].TopFieldOrderCnt = 0x0;
        slc->RefPicList0[0].BottomFieldOrderCnt = 0x1;
    }
    else if (frameId == 2)
    {
        pps->CurrPic.frame_idx = 0x1;
        pps->CurrPic.TopFieldOrderCnt = 0x2;
        pps->CurrPic.BottomFieldOrderCnt = 0x3;
        pps->ReferenceFrames[0].picture_id = resources[0];
        pps->ReferenceFrames[0].flags = 0x8;
        pps->ReferenceFrames[0].TopFieldOrderCnt = 0x0;
        pps->ReferenceFrames[0].BottomFieldOrderCnt = 0x1;
        pps->ReferenceFrames[1].picture_id = resources[1];
        pps->ReferenceFrames[1].frame_idx = 0x1;
        pps->ReferenceFrames[1].flags = 0x8;
        pps->ReferenceFrames[1].TopFieldOrderCnt = 0x4;
        pps->ReferenceFrames[1].BottomFieldOrderCnt = 0x5;
        pps->frame_num = 0x1;
        pps->pic_fields.value = 0x308;
        slc->slice_type = 0x1;
        slc->idr_pic_id = 0x1;
        slc->pic_order_cnt_lsb = 0x2;
        slc->RefPicList0[0].picture_id = resources[0];
        slc->RefPicList0[0].flags = 0x8;
        slc->RefPicList0[0].TopFieldOrderCnt = 0x0;
        slc->RefPicList0[0].BottomFieldOrderCnt = 0x1;
        slc->RefPicList1[0].picture_id = resources[1];
        slc->RefPicList1[0].frame_idx = 0x1;
        slc->RefPicList1[0].flags = 0x8;
        slc->RefPicList1[0].TopFieldOrderCnt = 0x4;
        slc->RefPicList1[0].BottomFieldOrderCnt = 0x5;
    }

}

