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
//! \file     codechal_oca_debug.h
//! \brief    Defines the oca debug interface shared by codec only.
//! \details  The debug interface dumps output from Media based on in input config file.
//!
#ifndef __CODEC_OCA_DEBUG_H__
#define __CODEC_OCA_DEBUG_H__

#include "codec_def_decode_avc.h"
#include "codec_def_decode_hevc.h"

#define CODECHAL_OCA_DECODE_MAX_SLICE_NUM 100

struct CODECHAL_OCA_LOG_HEADER
{
    uint32_t size;        //!< Size of valid data occupied, which is used when filling OCA buffer
    uint32_t allocSize;
};

struct CODECHAL_OCA_DECODE_HEADER
{
    CODECHAL_OCA_LOG_HEADER Header;
    MOS_COMPONENT           Component;         //!< DDI component
    uint32_t                numSlices;
    bool                    shortFormatInUse;  //HEVC only
};

struct CODECHAL_OCA_DECODE_AVC_PIC_PARAM
{
    struct
    {
        bool                    bValid;
        CODEC_AVC_PIC_PARAMS    params;
    } picParams;
};

struct CODECHAL_OCA_DECODE_AVC_SLICE_PARAM
{
    bool                        bValid;
    struct
    {
        uint32_t                    slice_data_size;                //!< Number of bytes in the bitstream buffer for this slice.
        uint32_t                    slice_data_offset;              //!< The offset to the NAL start code for this slice.
        // Long format specific
        uint16_t                    slice_data_bit_offset;          //!< Bit offset from NAL start code to the beginning of slice data.
        uint16_t                    first_mb_in_slice;              //!< Same as AVC syntax element.
        uint16_t                    NumMbsForSlice;                 //!< Number of MBs in the bitstream associated with this slice.
        uint8_t                     slice_type;                     //!< Same as AVC syntax element.
        uint8_t                     direct_spatial_mv_pred_flag;    //!< Same as AVC syntax element.
        uint8_t                     num_ref_idx_l0_active_minus1;   //!< Same as AVC syntax element.
        uint8_t                     num_ref_idx_l1_active_minus1;   //!< Same as AVC syntax element.
        uint8_t                     cabac_init_idc;                 //!< Same as AVC syntax element.
        char                        slice_qp_delta;                 //!< Same as AVC syntax element.
        uint8_t                     disable_deblocking_filter_idc;  //!< Same as AVC syntax element.
        char                        slice_alpha_c0_offset_div2;     //!< Same as AVC syntax element.
        char                        slice_beta_offset_div2;         //!< Same as AVC syntax element.
        uint16_t                    slice_id;                       //!< Same as AVC syntax element.
        uint16_t                    first_mb_in_next_slice;
    } sliceParams;
};

struct CODECHAL_OCA_DECODE_HEVC_PIC_PARAM
{
    struct
    {
        bool                       bValid;
        CODEC_HEVC_PIC_PARAMS      params;
    } picParams;

    struct
    {
        bool                       bValid;
        CODEC_HEVC_EXT_PIC_PARAMS  params;
    } extPicParams;

    struct
    {
        bool                       bValid;
        CODEC_HEVC_SCC_PIC_PARAMS  params;
    } sccPicParams;
};

struct CODECHAL_OCA_DECODE_HEVC_SLICE_PARAM
{
    bool                        bValid;
    struct
    {
        uint32_t                slice_data_size;
        uint32_t                slice_data_offset;
        uint16_t                NumEmuPrevnBytesInSliceHdr;
        uint32_t                ByteOffsetToSliceData;
        struct
        {
            uint32_t        LastSliceOfPic                               : 1;   //!< Specifies if current slice is the last slice of picture.
            uint32_t        dependent_slice_segment_flag                 : 1;   //!< Same as HEVC syntax element
            uint32_t        slice_type                                   : 2;   //!< Same as HEVC syntax element
            uint32_t        color_plane_id                               : 2;   //!< Same as HEVC syntax element
            uint32_t        slice_sao_luma_flag                          : 1;   //!< Same as HEVC syntax element
            uint32_t        slice_sao_chroma_flag                        : 1;   //!< Same as HEVC syntax element
            uint32_t        mvd_l1_zero_flag                             : 1;   //!< Same as HEVC syntax element
            uint32_t        cabac_init_flag                              : 1;   //!< Same as HEVC syntax element
            uint32_t        slice_temporal_mvp_enabled_flag              : 1;   //!< Same as HEVC syntax element
            uint32_t        slice_deblocking_filter_disabled_flag        : 1;   //!< Same as HEVC syntax element
            uint32_t        collocated_from_l0_flag                      : 1;   //!< Same as HEVC syntax element
            uint32_t        slice_loop_filter_across_slices_enabled_flag : 1;   //!< Same as HEVC syntax element
            uint32_t        reserved                                     : 18;  //!< Value is used for alignemnt and has no meaning, set to 0.
        }LongSliceFlags;
        uint8_t             collocated_ref_idx;
        uint8_t             num_ref_idx_l0_active_minus1;
        uint8_t             num_ref_idx_l1_active_minus1;
    } sliceParams;
};

class CodechalOcaDumper
{
public:
    CodechalOcaDumper();
    virtual ~CodechalOcaDumper();

    void AllocateBufferSize(uint32_t allocSize);

    void SetAvcDecodeParam(
        PCODEC_AVC_PIC_PARAMS   picParams,
        PCODEC_AVC_SLICE_PARAMS sliceParams,
        uint32_t                numSlices);

    void SetHevcDecodeParam(
        PCODEC_HEVC_PIC_PARAMS       picParams,
        PCODEC_HEVC_EXT_PIC_PARAMS   extPicParams,
        PCODEC_HEVC_SCC_PIC_PARAMS   sccPicParams,
        PCODEC_HEVC_SLICE_PARAMS     sliceParams,
        PCODEC_HEVC_EXT_SLICE_PARAMS extSliceParams,
        uint32_t                     numSlices,
        bool                         shortFormatInUse);

    CODECHAL_OCA_DECODE_HEADER *GetDecodeParam()
    {
        return m_pOcaDecodeParam;
    }

    static void Delete(void *&p);

public:
    CODECHAL_OCA_DECODE_HEADER *m_pOcaDecodeParam = nullptr;
MEDIA_CLASS_DEFINE_END(CodechalOcaDumper)
};

#endif  /* __MEDIA_OCA_DEBUG_H__ */
