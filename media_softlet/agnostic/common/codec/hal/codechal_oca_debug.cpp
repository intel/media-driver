/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     codechal_oca_debug.cpp
//! \brief    Defines the oca debug interface shared by codec only.
//! \details  The debug interface dumps output from Media based on in input config file.
//!
#include "codechal_oca_debug.h"

CodechalOcaDumper::CodechalOcaDumper()
{
}

CodechalOcaDumper::~CodechalOcaDumper()
{
    MOS_DeleteArray(m_pOcaDecodeParam);
}

void CodechalOcaDumper::Delete(void *&p)
{
    CodechalOcaDumper *pOcaDumper =  (CodechalOcaDumper *)p;
    MOS_Delete(pOcaDumper);
    p = nullptr;
}

void CodechalOcaDumper::AllocateBufferSize(uint32_t allocSize)
{
    if (m_pOcaDecodeParam)
    {
        if (allocSize > ((CODECHAL_OCA_DECODE_HEADER*)m_pOcaDecodeParam)->Header.allocSize)
        {
            MOS_DeleteArray(m_pOcaDecodeParam);
        }
        else
        {
            // Reuse previous buffer.
            allocSize = ((CODECHAL_OCA_DECODE_HEADER*)m_pOcaDecodeParam)->Header.allocSize;
        }
    }

    if (nullptr == m_pOcaDecodeParam)
    {
        m_pOcaDecodeParam = (CODECHAL_OCA_DECODE_HEADER *)MOS_NewArray(char, allocSize);
    }
}

void CodechalOcaDumper::SetAvcDecodeParam(
    PCODEC_AVC_PIC_PARAMS   picParams,
    PCODEC_AVC_SLICE_PARAMS sliceParams,
    uint32_t                numSlices)
{
    uint32_t sliceNum = (numSlices > CODECHAL_OCA_DECODE_MAX_SLICE_NUM) ?  CODECHAL_OCA_DECODE_MAX_SLICE_NUM : numSlices;

    // OCA Buffer dumps header + pic parameter + slice parameter
    uint32_t size = sizeof(CODECHAL_OCA_DECODE_HEADER) +
        sizeof(CODECHAL_OCA_DECODE_AVC_PIC_PARAM) +
        sliceNum * sizeof(CODECHAL_OCA_DECODE_AVC_SLICE_PARAM);
    uint32_t allocSize = size;

    AllocateBufferSize(allocSize);
    if (nullptr == m_pOcaDecodeParam)
    {
        return;
    }
    memset(m_pOcaDecodeParam, 0, size);

    m_pOcaDecodeParam->Header.size      = size;
    m_pOcaDecodeParam->Header.allocSize = allocSize;
    m_pOcaDecodeParam->Component        = COMPONENT_Decode;
    m_pOcaDecodeParam->numSlices        = sliceNum;

    uint32_t offset = sizeof(CODECHAL_OCA_DECODE_HEADER);
    CODECHAL_OCA_DECODE_AVC_PIC_PARAM* pPicParam = (CODECHAL_OCA_DECODE_AVC_PIC_PARAM *)((char*)m_pOcaDecodeParam + offset);

    offset += sizeof(CODECHAL_OCA_DECODE_AVC_PIC_PARAM);
    CODECHAL_OCA_DECODE_AVC_SLICE_PARAM* pSliceParam = (CODECHAL_OCA_DECODE_AVC_SLICE_PARAM *)((char*)m_pOcaDecodeParam + offset);

    if (picParams)
    {
        pPicParam->picParams.params = *picParams;
        pPicParam->picParams.bValid = true;
    }

    for(uint16_t i = 0; i < sliceNum; i++)
    {
        if (sliceParams)
        {
            pSliceParam[i].bValid                                    = true;
            pSliceParam[i].sliceParams.slice_data_size               = sliceParams[i].slice_data_size;
            pSliceParam[i].sliceParams.slice_data_offset             = sliceParams[i].slice_data_offset;
            pSliceParam[i].sliceParams.slice_data_bit_offset         = sliceParams[i].slice_data_bit_offset;
            pSliceParam[i].sliceParams.first_mb_in_slice             = sliceParams[i].first_mb_in_slice;
            pSliceParam[i].sliceParams.NumMbsForSlice                = sliceParams[i].NumMbsForSlice;
            pSliceParam[i].sliceParams.slice_type                    = sliceParams[i].slice_type;
            pSliceParam[i].sliceParams.direct_spatial_mv_pred_flag   = sliceParams[i].direct_spatial_mv_pred_flag;
            pSliceParam[i].sliceParams.num_ref_idx_l0_active_minus1  = sliceParams[i].num_ref_idx_l0_active_minus1;
            pSliceParam[i].sliceParams.num_ref_idx_l1_active_minus1  = sliceParams[i].num_ref_idx_l1_active_minus1;
            pSliceParam[i].sliceParams.cabac_init_idc                = sliceParams[i].cabac_init_idc;
            pSliceParam[i].sliceParams.slice_qp_delta                = sliceParams[i].slice_qp_delta;
            pSliceParam[i].sliceParams.disable_deblocking_filter_idc = sliceParams[i].disable_deblocking_filter_idc;
            pSliceParam[i].sliceParams.slice_alpha_c0_offset_div2    = sliceParams[i].slice_alpha_c0_offset_div2;
            pSliceParam[i].sliceParams.slice_beta_offset_div2        = sliceParams[i].slice_beta_offset_div2;
            pSliceParam[i].sliceParams.slice_id                      = sliceParams[i].slice_id;
            pSliceParam[i].sliceParams.first_mb_in_next_slice        = sliceParams[i].first_mb_in_next_slice;      
        }
    }
}

void CodechalOcaDumper::SetHevcDecodeParam(
    PCODEC_HEVC_PIC_PARAMS       picParams,
    PCODEC_HEVC_EXT_PIC_PARAMS   extPicParams,
    PCODEC_HEVC_SCC_PIC_PARAMS   sccPicParams,
    PCODEC_HEVC_SLICE_PARAMS     sliceParams,
    PCODEC_HEVC_EXT_SLICE_PARAMS extSliceParams,
    uint32_t                     numSlices,
    bool                         shortFormatInUse)
{
    uint32_t sliceNum = (numSlices > CODECHAL_OCA_DECODE_MAX_SLICE_NUM) ?  CODECHAL_OCA_DECODE_MAX_SLICE_NUM : numSlices;

    uint32_t size = sizeof(CODECHAL_OCA_DECODE_HEADER) +
        sizeof(CODECHAL_OCA_DECODE_HEVC_PIC_PARAM) +
        sliceNum * sizeof(CODECHAL_OCA_DECODE_HEVC_SLICE_PARAM);
    uint32_t allocSize = size;

    AllocateBufferSize(allocSize);
    if (nullptr == m_pOcaDecodeParam)
    {
        return;
    }
    memset(m_pOcaDecodeParam, 0, size);

    m_pOcaDecodeParam->Header.size      = size;
    m_pOcaDecodeParam->Header.allocSize = allocSize;
    m_pOcaDecodeParam->Component        = COMPONENT_Decode;
    m_pOcaDecodeParam->numSlices        = sliceNum;
    m_pOcaDecodeParam->shortFormatInUse = shortFormatInUse;

    uint32_t offset = sizeof(CODECHAL_OCA_DECODE_HEADER);
    CODECHAL_OCA_DECODE_HEVC_PIC_PARAM* pPicParam = (CODECHAL_OCA_DECODE_HEVC_PIC_PARAM *)((char*)m_pOcaDecodeParam + offset);

    offset += sizeof(CODECHAL_OCA_DECODE_HEVC_PIC_PARAM);
    CODECHAL_OCA_DECODE_HEVC_SLICE_PARAM* pSliceParam = (CODECHAL_OCA_DECODE_HEVC_SLICE_PARAM *)((char*)m_pOcaDecodeParam + offset);

    if (picParams)
    {
        pPicParam->picParams.params = *picParams;
        pPicParam->picParams.bValid = true;
    }

    if (extPicParams)
    {
        pPicParam->extPicParams.params = *extPicParams;
        pPicParam->extPicParams.bValid = true;
    }

    if (sccPicParams)
    {
        pPicParam->sccPicParams.params = *sccPicParams;
        pPicParam->sccPicParams.bValid = true;
    }

    for(uint16_t i = 0; i < sliceNum; i++)
    {
        if (sliceParams)
        { 
            pSliceParam[i].bValid                                                                  = true;
            pSliceParam[i].sliceParams.slice_data_size                                             = sliceParams[i].slice_data_size;
            pSliceParam[i].sliceParams.slice_data_offset                                           = sliceParams[i].slice_data_offset;
            pSliceParam[i].sliceParams.NumEmuPrevnBytesInSliceHdr                                  = sliceParams[i].NumEmuPrevnBytesInSliceHdr;
            pSliceParam[i].sliceParams.ByteOffsetToSliceData                                       = sliceParams[i].ByteOffsetToSliceData;
            pSliceParam[i].sliceParams.collocated_ref_idx                                          = sliceParams[i].collocated_ref_idx;
            pSliceParam[i].sliceParams.num_ref_idx_l0_active_minus1                                = sliceParams[i].num_ref_idx_l0_active_minus1;
            pSliceParam[i].sliceParams.num_ref_idx_l1_active_minus1                                = sliceParams[i].num_ref_idx_l1_active_minus1; 
            pSliceParam[i].sliceParams.LongSliceFlags.LastSliceOfPic                               = sliceParams[i].LongSliceFlags.fields.LastSliceOfPic;
            pSliceParam[i].sliceParams.LongSliceFlags.dependent_slice_segment_flag                 = sliceParams[i].LongSliceFlags.fields.dependent_slice_segment_flag;
            pSliceParam[i].sliceParams.LongSliceFlags.slice_type                                   = sliceParams[i].LongSliceFlags.fields.slice_type;
            pSliceParam[i].sliceParams.LongSliceFlags.color_plane_id                               = sliceParams[i].LongSliceFlags.fields.color_plane_id;
            pSliceParam[i].sliceParams.LongSliceFlags.slice_sao_luma_flag                          = sliceParams[i].LongSliceFlags.fields.slice_sao_luma_flag;
            pSliceParam[i].sliceParams.LongSliceFlags.slice_sao_chroma_flag                        = sliceParams[i].LongSliceFlags.fields.slice_sao_chroma_flag;
            pSliceParam[i].sliceParams.LongSliceFlags.mvd_l1_zero_flag                             = sliceParams[i].LongSliceFlags.fields.mvd_l1_zero_flag;
            pSliceParam[i].sliceParams.LongSliceFlags.cabac_init_flag                              = sliceParams[i].LongSliceFlags.fields.cabac_init_flag;
            pSliceParam[i].sliceParams.LongSliceFlags.slice_temporal_mvp_enabled_flag              = sliceParams[i].LongSliceFlags.fields.slice_temporal_mvp_enabled_flag;
            pSliceParam[i].sliceParams.LongSliceFlags.slice_deblocking_filter_disabled_flag        = sliceParams[i].LongSliceFlags.fields.slice_deblocking_filter_disabled_flag;
            pSliceParam[i].sliceParams.LongSliceFlags.collocated_from_l0_flag                      = sliceParams[i].LongSliceFlags.fields.collocated_from_l0_flag;
            pSliceParam[i].sliceParams.LongSliceFlags.slice_loop_filter_across_slices_enabled_flag = sliceParams[i].LongSliceFlags.fields.slice_loop_filter_across_slices_enabled_flag;
        }
    }
}
