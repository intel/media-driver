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
#include "test_data_encode.h"

using namespace std;

HevcEncBufs::HevcEncBufs()
{
    m_sps = make_shared<VAEncSequenceParameterBufferHEVC>();
    memset(m_sps.get(), 0, sizeof(VAEncSequenceParameterBufferHEVC));

    m_sps->general_profile_idc                      = 0x1;
    m_sps->general_level_idc                        = 0x5a;
    m_sps->intra_period                             = 0x5;
    m_sps->intra_idr_period                         = 0xffffffff;
    m_sps->ip_period                                = 0x2;
    m_sps->bits_per_second                          = 0x190;
    m_sps->pic_width_in_luma_samples                = 0x140;
    m_sps->pic_height_in_luma_samples               = 0xf0;
    m_sps->seq_fields.value                         = 0xc801;
    m_sps->log2_diff_max_min_luma_coding_block_size = 0x2;
    m_sps->log2_diff_max_min_transform_block_size   = 0x3;
    m_sps->max_transform_hierarchy_depth_inter      = 0x2;
    m_sps->max_transform_hierarchy_depth_intra      = 0x2;
    m_sps->pcm_sample_bit_depth_luma_minus1         = 0x7;
    m_sps->pcm_sample_bit_depth_chroma_minus1       = 0x7;

    m_pps = make_shared<VAEncPictureParameterBufferHEVC>();
    memset(m_pps.get(), 0, sizeof(VAEncPictureParameterBufferHEVC));

    m_slc = make_shared<VAEncSliceParameterBufferHEVC>();
    memset(m_slc.get(), 0, sizeof(VAEncSliceParameterBufferHEVC));

    for (auto i = 0; i < 15; i++)
    {
        m_pps->reference_frames[i].picture_id = 0xffffffff;
        m_pps->reference_frames[i].flags      = 0x1;
    }

    for (auto i = 0; i < 15; i++)
    {
        m_slc->ref_pic_list0[i].picture_id = 0xffffffff;
        m_slc->ref_pic_list0[i].flags      = 0x1;
        m_slc->ref_pic_list1[i].picture_id = 0xffffffff;
        m_slc->ref_pic_list1[i].flags      = 0x1;
    }

    m_pps->column_width_minus1[0] = 0x9;
    m_pps->row_height_minus1[0]   = 0x7;
    m_pps->pic_fields.value       = 0x215;
    m_pps->diff_cu_qp_delta_depth = 0x2;
    m_slc->num_ctu_in_slice       = 0x50;
    m_slc->max_num_merge_cand     = 0x5;
    m_slc->slice_beta_offset_div2 = 0x2;
    m_slc->slice_fields.value     = 0x2010;
}

AvcEncBufs::AvcEncBufs()
{
    m_sps = make_shared<VAEncSequenceParameterBufferH264>();
    memset(m_sps.get(), 0, sizeof(VAEncSequenceParameterBufferH264));

    m_sps->level_idc             = 0x33;
    m_sps->intra_period          = 0x1f;
    m_sps->intra_idr_period      = 0x3d;
    m_sps->ip_period             = 0x2;
    m_sps->bits_per_second       = 0x4c4b40;
    m_sps->max_num_ref_frames    = 0x10;
    m_sps->picture_width_in_mbs  = 0x14;
    m_sps->picture_height_in_mbs = 0xf;
    m_sps->seq_fields.value      = 0x3105;

    m_pps = make_shared<VAEncPictureParameterBufferH264>();
    memset(m_pps.get(), 0, sizeof(VAEncPictureParameterBufferH264));

    m_slc = make_shared<VAEncSliceParameterBufferH264>();
    memset(m_slc.get(), 0, sizeof(VAEncSliceParameterBufferH264));

    for (auto i = 0; i < 16; i++)
    {
        m_pps->ReferenceFrames[i].picture_id = 0xffffffff;
        m_pps->ReferenceFrames[i].flags      = 0x1;
    }

    for (auto i = 0; i < 32; i++)
    {
        m_slc->RefPicList0[i].picture_id    = 0xffffffff;
        m_slc->RefPicList0[i].flags         = 0x1;
        m_slc->RefPicList1[i].picture_id    = 0xffffffff;
        m_slc->RefPicList1[i].flags         = 0x1;
    }
    m_slc->direct_spatial_mv_pred_flag      = 0x1;
    m_slc->num_ref_idx_active_override_flag = 0x1;
    m_slc->cabac_init_idc                   = 0x1;
    m_slc->slice_alpha_c0_offset_div2       = 0x2;
    m_slc->slice_beta_offset_div2           = 0x2;
    m_slc->num_macroblocks                  = 0x12c;
}

EncTestDataHEVC::EncTestDataHEVC(FeatureID testFeatureID)
{
    m_featureId   = testFeatureID;
    m_picWidth    = 320;
    m_picHeight   = 240;
    m_surfacesNum = 1+15 ; // 1 raw data, 8 references.

    m_confAttrib.resize(1);
    m_confAttrib[0].type  = VAConfigAttribRateControl;
    m_confAttrib[0].value = VA_RC_CQP;

    m_surfAttrib.resize(1);
    m_surfAttrib[0].type          = VASurfaceAttribPixelFormat;
    m_surfAttrib[0].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
    m_surfAttrib[0].value.type    = VAGenericValueTypePointer;
    m_surfAttrib[0].value.value.i = VA_FOURCC_NV12;

    m_resources.resize(m_surfacesNum);
    InitCompBuffers();
    vector<EncFrameDataHEVC> &currentArray = m_frameArray;
    m_num_frames                           = currentArray.size();
    m_compBufs.resize(m_num_frames);

    m_packedsps.type                = VAEncPackedHeaderSequence;
    m_packedsps.bit_length          = 4 * 8;
    m_packedsps.has_emulation_bytes = 0;
    m_packedpps.type                = VAEncPackedHeaderPicture;
    m_packedpps.bit_length          = 4 * 8;
    m_packedpps.has_emulation_bytes = 0;
    m_packedsh.type                 = VAEncPackedHeaderSlice;
    m_packedsh.bit_length           = 4 * 8;
    m_packedsh.has_emulation_bytes  = 0;

    for (uint32_t i = 0; i < m_num_frames; i++)
    {
        m_compBufs[i].resize(12);
        // BS buffer
        m_compBufs[i][0]  = { VAEncCodedBufferType                 , (m_picWidth * m_picHeight * 3) >> 1       , nullptr                      , 0 };
        // Parameters buffer SPS, PPS, SliceHeader
        m_compBufs[i][1]  = { VAEncSequenceParameterBufferType     , (uint32_t)currentArray[i].spsData.size()  , &currentArray[i].spsData[0]  , 0 };
        m_compBufs[i][2]  = { VAEncPictureParameterBufferType      , (uint32_t)currentArray[i].ppsData.size()  , &currentArray[i].ppsData[0]  , 0 };
        m_compBufs[i][3]  = { VAEncSliceParameterBufferType        , (uint32_t)currentArray[i].sliceData.size(), &currentArray[i].sliceData[0], 0 };

        // Packed VPS, SPS, PPS, SliceHeader, we fix all the data length as 4bytes.
        m_compBufs[i][4]  = { VAEncPackedHeaderParameterBufferType , (uint32_t)sizeof(m_packedsps)             , (void *)&m_packedsps         , 0 };
        m_compBufs[i][5]  = { VAEncPackedHeaderDataBufferType      , 4                                         , (void *)&m_headerData[0]     , 0 };
        m_compBufs[i][6]  = { VAEncPackedHeaderParameterBufferType , (uint32_t)sizeof(m_packedsps)             , (void *)&m_packedsps         , 0 };
        m_compBufs[i][7]  = { VAEncPackedHeaderDataBufferType      , 4                                         , (void *)&m_headerData[0]     , 0 };
        m_compBufs[i][8]  = { VAEncPackedHeaderParameterBufferType , (uint32_t)sizeof(m_packedpps)             , (void *)&m_packedpps         , 0 };
        m_compBufs[i][9]  = { VAEncPackedHeaderDataBufferType      , 4                                         , (void *)&m_headerData[0]     , 0 };
        m_compBufs[i][10] = { VAEncPackedHeaderParameterBufferType , (uint32_t)sizeof(m_packedsh)              , (void *)&m_packedsh          , 0 };
        m_compBufs[i][11] = { VAEncPackedHeaderDataBufferType      , 4                                         , (void *)&m_headerData[0]     , 0 };

    }
}

void EncTestDataHEVC::InitCompBuffers()
{
    m_frameArray.resize(ENC_FRAME_NUM);
    m_pBufs = make_shared<HevcEncBufs>();

    for (auto i = 0; i < ENC_FRAME_NUM; i++)
    {
        m_frameArray[i].spsData.assign((char*)m_pBufs->GetSpsBuf()  , (char*)m_pBufs->GetSpsBuf() + m_pBufs->GetSpsSize());
        m_frameArray[i].ppsData.assign((char*)m_pBufs->GetPpsBuf()  , (char*)m_pBufs->GetPpsBuf() + m_pBufs->GetPpsSize());
        m_frameArray[i].sliceData.assign((char*)m_pBufs->GetSlcBuf(), (char*)m_pBufs->GetSlcBuf() + m_pBufs->GetSlcSize());
    }
}
void EncTestDataHEVC::UpdateCompBuffers(int frameId)
{
    VAEncPictureParameterBufferHEVC *pps = (VAEncPictureParameterBufferHEVC*)&m_frameArray[frameId].ppsData[0];
    VAEncSliceParameterBufferHEVC   *slc = (VAEncSliceParameterBufferHEVC*)&m_frameArray[frameId].sliceData[0];
    pps->coded_buf                       = m_compBufs[frameId][0].bufID;
    pps->decoded_curr_pic.picture_id     = m_resources[frameId];

    if (frameId == 0) // I frame
    {
        pps->decoded_curr_pic.pic_order_cnt    = 0x0;
        pps->pic_init_qp                       = 0xf;
        slc->slice_type                        = 0x2;
    }
    else if (frameId == 1) // NoDelay B frame
    {
        pps->decoded_curr_pic.pic_order_cnt    = 0x2;
        pps->reference_frames[0].flags         = 0x0;
        pps->reference_frames[0].picture_id    = m_resources[0];
        pps->reference_frames[0].pic_order_cnt = 0x0;
        pps->pic_init_qp                       = 0x19;
        pps->pic_fields.value                  = 0x210;

        slc->slice_type                        = 0x0;
        slc->ref_pic_list0[0].picture_id       = m_resources[0];
        slc->ref_pic_list0[0].flags            = 0x0;
        slc->ref_pic_list0[0].pic_order_cnt    = 0x0;
        slc->ref_pic_list1[0].picture_id       = m_resources[0];
        slc->ref_pic_list1[0].flags            = 0x0;
        slc->ref_pic_list1[0].pic_order_cnt    = 0x0;
    }
    else if (frameId == 2) // B frame
    {
        pps->decoded_curr_pic.pic_order_cnt    = 0x1;
        pps->reference_frames[0].flags         = 0x0;
        pps->reference_frames[0].picture_id    = m_resources[0];
        pps->reference_frames[0].pic_order_cnt = 0x0;
        pps->reference_frames[1].flags         = 0x0;
        pps->reference_frames[1].picture_id    = m_resources[1];
        pps->reference_frames[1].pic_order_cnt = 0x2;
        pps->pic_init_qp                       = 0x19;
        pps->pic_fields.value                  = 0x200;
        slc->slice_type                        = 0x0;
        slc->ref_pic_list0[0].picture_id       = m_resources[0];
        slc->ref_pic_list0[0].flags            = 0x0;
        slc->ref_pic_list1[0].pic_order_cnt    = 0x0;
        slc->ref_pic_list1[0].picture_id       = m_resources[1];
        slc->ref_pic_list1[0].flags            = 0x0;
        slc->ref_pic_list1[0].pic_order_cnt    = 0x2;
    }
}

EncTestDataAVC::EncTestDataAVC(FeatureID testFeatureID)
{
    m_featureId   = testFeatureID;
    m_picWidth    = 320;
    m_picHeight   = 240;
    m_surfacesNum = 1 + 15; // 1 raw data, 8 references.

    m_confAttrib.resize(1);
    m_confAttrib[0].type  = VAConfigAttribRateControl;
    m_confAttrib[0].value = VA_RC_CQP;

    m_surfAttrib.resize(1);
    m_surfAttrib[0].type          = VASurfaceAttribPixelFormat;
    m_surfAttrib[0].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
    m_surfAttrib[0].value.type    = VAGenericValueTypePointer;
    m_surfAttrib[0].value.value.i = VA_FOURCC_NV12;

    m_resources.resize(m_surfacesNum);
    InitCompBuffers();
    vector<EncFrameDataAVC> &currentArray = m_frameArray;
    m_num_frames                           = currentArray.size();
    m_compBufs.resize(m_num_frames);

    m_packedsps.type                = VAEncPackedHeaderSequence;
    m_packedsps.bit_length          = 4 * 8;
    m_packedsps.has_emulation_bytes = 0;
    m_packedpps.type                = VAEncPackedHeaderPicture;
    m_packedpps.bit_length          = 4 * 8;
    m_packedpps.has_emulation_bytes = 0;
    m_packedsh.type                 = VAEncPackedHeaderSlice;
    m_packedsh.bit_length           = 4 * 8;
    m_packedsh.has_emulation_bytes  = 0;

    for (uint32_t i = 0; i < m_num_frames; i++)
    {
        m_compBufs[i].resize(12);
        // BS buffer
        m_compBufs[i][0] =  { VAEncCodedBufferType                , (m_picWidth * m_picHeight * 3) >> 1       , nullptr                      , 0 };
        // Parameters buffer SPS, PPS, SliceHeader
        m_compBufs[i][1] =  { VAEncSequenceParameterBufferType    , (uint32_t)currentArray[i].spsData.size()  , &currentArray[i].spsData[0]  , 0 };
        m_compBufs[i][2] =  { VAEncPictureParameterBufferType     , (uint32_t)currentArray[i].ppsData.size()  , &currentArray[i].ppsData[0]  , 0 };
        m_compBufs[i][3] =  { VAEncSliceParameterBufferType       , (uint32_t)currentArray[i].sliceData.size(), &currentArray[i].sliceData[0], 0 };
        // In fact, AVC doesn't need VPS, just add here for future improvement if we can merge it with HEVC.
        // Packed VPS, SPS, PPS, SliceHeader, we fix all the data length as 4bytes.
        m_compBufs[i][4]  = { VAEncPackedHeaderParameterBufferType, (uint32_t)sizeof(m_packedsps)             , (void *)&m_packedsps         , 0 };
        m_compBufs[i][5]  = { VAEncPackedHeaderDataBufferType     , 4                                         , (void *)&m_headerData[0]     , 0 };
        m_compBufs[i][6]  = { VAEncPackedHeaderParameterBufferType, (uint32_t)sizeof(m_packedsps)             , (void *)&m_packedsps         , 0 };
        m_compBufs[i][7]  = { VAEncPackedHeaderDataBufferType     , 4                                         , (void *)&m_headerData[0]     , 0 };
        m_compBufs[i][8]  = { VAEncPackedHeaderParameterBufferType, (uint32_t)sizeof(m_packedpps)             , (void *)&m_packedpps         , 0 };
        m_compBufs[i][9]  = { VAEncPackedHeaderDataBufferType     , 4                                         , (void *)&m_headerData[0]     , 0 };
        m_compBufs[i][10] = { VAEncPackedHeaderParameterBufferType, (uint32_t)sizeof(m_packedsh)              , (void *)&m_packedsh          , 0 };
        m_compBufs[i][11] = { VAEncPackedHeaderDataBufferType     , 4                                         , (void *)&m_headerData[0]     , 0 };
    }
}

void EncTestDataAVC::InitCompBuffers()
{
    m_frameArray.resize(3);
    m_pBufs = make_shared<AvcEncBufs>();

    for (auto i = 0; i < ENC_FRAME_NUM; i++)
    {
        m_frameArray[i].spsData.assign((char*)m_pBufs->GetSpsBuf()  , (char*)m_pBufs->GetSpsBuf() + m_pBufs->GetSpsSize());
        m_frameArray[i].ppsData.assign((char*)m_pBufs->GetPpsBuf()  , (char*)m_pBufs->GetPpsBuf() + m_pBufs->GetPpsSize());
        m_frameArray[i].sliceData.assign((char*)m_pBufs->GetSlcBuf(), (char*)m_pBufs->GetSlcBuf() + m_pBufs->GetSlcSize());
    }
}

void EncTestDataAVC::UpdateCompBuffers(int frameId)
{
    VAEncPictureParameterBufferH264 *pps = (VAEncPictureParameterBufferH264*)&m_frameArray[frameId].ppsData[0];
    VAEncSliceParameterBufferH264   *slc = (VAEncSliceParameterBufferH264*)&m_frameArray[frameId].sliceData[0];
    pps->coded_buf                       = m_compBufs[frameId][0].bufID;
    pps->CurrPic.picture_id              = m_resources[frameId];

    if (frameId==0)
    {
        pps->CurrPic.BottomFieldOrderCnt            = 0x1;
        pps->pic_init_qp                            = 0x1c;
        pps->pic_fields.value                       = 0x30b;
        slc->slice_type                             = 0x7;
    }
    else if (frameId == 1)
    {
        pps->CurrPic.frame_idx                      = 0x1;
        pps->CurrPic.TopFieldOrderCnt               = 0x4;
        pps->CurrPic.BottomFieldOrderCnt            = 0x5;
        pps->ReferenceFrames[0].picture_id          = m_resources[0];
        pps->ReferenceFrames[0].flags               = 0x8;
        pps->ReferenceFrames[0].TopFieldOrderCnt    = 0x0;
        pps->ReferenceFrames[0].BottomFieldOrderCnt = 0x1;
        pps->frame_num                              = 0x1;
        pps->pic_fields.value                       = 0x30a;
        slc->idr_pic_id                             = 0x1;
        slc->slice_type                             = 0x0;
        slc->pic_order_cnt_lsb                      = 0x4;
        slc->RefPicList0[0].picture_id              = m_resources[0];
        slc->RefPicList0[0].flags                   = 0x8;
        slc->RefPicList0[0].TopFieldOrderCnt        = 0x0;
        slc->RefPicList0[0].BottomFieldOrderCnt     = 0x1;
    }
    else if (frameId == 2)
    {
        pps->CurrPic.frame_idx                      = 0x1;
        pps->CurrPic.TopFieldOrderCnt               = 0x2;
        pps->CurrPic.BottomFieldOrderCnt            = 0x3;
        pps->ReferenceFrames[0].picture_id          = m_resources[0];
        pps->ReferenceFrames[0].flags               = 0x8;
        pps->ReferenceFrames[0].TopFieldOrderCnt    = 0x0;
        pps->ReferenceFrames[0].BottomFieldOrderCnt = 0x1;
        pps->ReferenceFrames[1].picture_id          = m_resources[1];
        pps->ReferenceFrames[1].frame_idx           = 0x1;
        pps->ReferenceFrames[1].flags               = 0x8;
        pps->ReferenceFrames[1].TopFieldOrderCnt    = 0x4;
        pps->ReferenceFrames[1].BottomFieldOrderCnt = 0x5;
        pps->frame_num                              = 0x1;
        pps->pic_fields.value                       = 0x308;
        slc->slice_type                             = 0x1;
        slc->idr_pic_id                             = 0x1;
        slc->pic_order_cnt_lsb                      = 0x2;
        slc->RefPicList0[0].picture_id              = m_resources[0];
        slc->RefPicList0[0].flags                   = 0x8;
        slc->RefPicList0[0].TopFieldOrderCnt        = 0x0;
        slc->RefPicList0[0].BottomFieldOrderCnt     = 0x1;
        slc->RefPicList1[0].picture_id              = m_resources[1];
        slc->RefPicList1[0].frame_idx               = 0x1;
        slc->RefPicList1[0].flags                   = 0x8;
        slc->RefPicList1[0].TopFieldOrderCnt        = 0x4;
        slc->RefPicList1[0].BottomFieldOrderCnt     = 0x5;
    }
}
