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
//!
//! \file     codec_def_decode_hevc.h
//! \brief    Defines decode HEVC types and macros shared by CodecHal, MHW, and DDI layer
//! \details  Applies to HEVC decode only. Should not contain any DDI specific code.
//!
#ifndef __CODEC_DEF_DECODE_HEVC_H__
#define __CODEC_DEF_DECODE_HEVC_H__

#include "codec_def_common_hevc.h"

#define CODEC_NUM_REF_HEVC_MV_BUFFERS       CODEC_MAX_NUM_REF_FRAME_HEVC
#define CODEC_NUM_HEVC_MV_BUFFERS           (CODEC_NUM_REF_HEVC_MV_BUFFERS + 1)
#define CODEC_NUM_HEVC_INITIAL_MV_BUFFERS   6
#define HEVC_NUM_MAX_TILE_ROW               22
#define HEVC_NUM_MAX_TILE_COLUMN            20
#define CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6  600
#define CODECHAL_HEVC_MAX_NUM_SLICES_LVL_5  200
#define CODECHAL_HEVC_NUM_DMEM_BUFFERS      32

#define CODEC_HEVC_NUM_SECOND_BB            32

#define CODECHAL_HEVC_MIN_LCU               16
#define CODECHAL_HEVC_MAX_DIM_FOR_MIN_LCU   4222

const uint8_t CODECHAL_DECODE_HEVC_Qmatrix_Scan_4x4[16] = { 0, 4, 1, 8, 5, 2, 12, 9, 6, 3, 13, 10, 7, 14, 11, 15 };
const uint8_t CODECHAL_DECODE_HEVC_Qmatrix_Scan_8x8[64] =
{ 0, 8, 1, 16, 9, 2, 24, 17, 10, 3, 32, 25, 18, 11, 4, 40,
33, 26, 19, 12, 5, 48, 41, 34, 27, 20, 13, 6, 56, 49, 42, 35,
28, 21, 14, 7, 57, 50, 43, 36, 29, 22, 15, 58, 51, 44, 37, 30,
23, 59, 52, 45, 38, 31, 60, 53, 46, 39, 61, 54, 47, 62, 55, 63 };
const uint8_t CODECHAL_DECODE_HEVC_Default_4x4[16] = { 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16 };
const uint8_t CODECHAL_DECODE_HEVC_Default_8x8_Intra[64] =
{ 16, 16, 16, 16, 17, 18, 21, 24, 16, 16, 16, 16, 17, 19, 22, 25,
16, 16, 17, 18, 20, 22, 25, 29, 16, 16, 18, 21, 24, 27, 31, 36,
17, 17, 20, 24, 30, 35, 41, 47, 18, 19, 22, 27, 35, 44, 54, 65,
21, 22, 25, 31, 41, 54, 70, 88, 24, 25, 29, 36, 47, 65, 88, 115 };
const uint8_t CODECHAL_DECODE_HEVC_Default_8x8_Inter[64] =
{ 16, 16, 16, 16, 17, 18, 20, 24, 16, 16, 16, 17, 18, 20, 24, 25,
16, 16, 17, 18, 20, 24, 25, 28, 16, 17, 18, 20, 24, 25, 28, 33,
17, 18, 20, 24, 25, 28, 33, 41, 18, 20, 24, 25, 28, 33, 41, 54,
20, 24, 25, 28, 33, 41, 54, 71, 24, 25, 28, 33, 41, 54, 71, 91 };

enum
{
    decodeHevcBSlice        = 0,
    decodeHevcPSlice        = 1,
    decodeHevcISlice        = 2,
    decodeHevcNumSliceTypes = 3
};

/*! \brief Picture-level parameters of a compressed picture for HEVC decoding.
 *
 *   Note 1: Application only pass in the first num_tile_columns_minus1 tile column widths and first num_tile_rows_minus1 tile row heights. The last width and height need to be calculated by driver from the picture dimension. Values used for data type alignement. Their values should be set to 0, and can be ignored by decoder.
 *   Note 2: HEVC host decoder should discard any NAL units with nal_unit_type in the range of [10 – 15, 22 – 63].
 *   Note 3: When tiles_enabled_flag equals 1 and uniform_spacing_flag takes value 1, driver may ignore the values passed in column_width_minus1[] and raw_height_minus1[]. Instead driver should generate and populate these tile dimension values based on picture resolution and num_tile_columns_minus1, num_tile_rows_minus1. It can be referred to formula (6-3) and (6-4) in HEVC spec.
 */
typedef struct _CODEC_HEVC_PIC_PARAMS
{
    /*! \brief Width of decoded pictures in units of minimum luma coding block size.
    *
    *   The decoded picture width in units of luma samples equals (PicWidthInMinCbsY) * (1 << (log2_min_coding_block_size_minus3 + 3)).
    */
    uint16_t                      PicWidthInMinCbsY;
    /*! \brief Height of decoded pictures in units of minimum luma coding block size.
    *
    *   The decoded picture height in units of luma samples equals (PicHeightInMinCbsY) * (1 << (log2_min_coding_block_size_minus3 + 3)).
    */
    uint16_t                      PicHeightInMinCbsY;

    union
    {
        struct
        {
            uint16_t              chroma_format_idc                   : 2;    //!< Same as HEVC syntax element
            uint16_t              separate_colour_plane_flag          : 1;    //!< Same as HEVC syntax element
            uint16_t              bit_depth_luma_minus8               : 3;    //!< Same as HEVC syntax element
            uint16_t              bit_depth_chroma_minus8             : 3;    //!< Same as HEVC syntax element
            uint16_t              log2_max_pic_order_cnt_lsb_minus4   : 4;    //!< Same as HEVC syntax element
            /*! \brief Indicates that no picture reordering is used in the coded video sequence.
            *
            *   If equal to 1, the maximum allowed number of pictures preceding any picture in decoding order and succeeding that picture in output order is equal to 0. When NoPicReorderingFlag equal to 0, picture reordering may be used in the coded video sequence. This flag does not affect the decoding process.
            *   Note: NoPicReorderingFlag may be set to 1 by the host software decoder when sps_max_num_reorder_pics is equal to 0. However, there is no requirement that NoPicReorderingFlag must be derived from sps_max_num_reorder_pics.
            */
            uint16_t              NoPicReorderingFlag                 : 1;
            /*! \brief Indicates that B slices are not used in the coded video sequence.
            *
            *   This flag does not affect the decoding process.
            *   Note: This flag does not correspond to any indication provided in the HEVC bitstream itself. Thus, a host software decoder would need some external information (e.g. as determined at the application level) to be able to set this flag to 1. In the absence of any such available indication, the host software decoder must set this flag to 0.
            */
            uint16_t              NoBiPredFlag                        : 1;
            uint16_t              ReservedBits1                       : 1;    //!< Value is used for alignemnt and has no meaning, set to 0.
        };

        uint16_t                  wFormatAndSequenceInfoFlags;
    };

    /*! \brief Uncompressed destination surface of the frame for the current decoded picture.
    *
    *   The long_term_ref_flag has no meaning. The valid value range for Curr.FrameIdx is [0..126]. Value 127 or 0x7F can be treated as an invalid surface index.
    */
    CODEC_PICTURE               CurrPic;

    /*! \brief Number of reference frames in the DPB buffer.
    *
    *   Host decoder should set this value to be sps_max_dec_pic_buffering_minus1 of the temporal layer where the current decoding frame is of. The value should be between 0 and 15, inclusive.
    */
    uint8_t                       sps_max_dec_pic_buffering_minus1;
    uint8_t                       log2_min_luma_coding_block_size_minus3;         //!< Same as HEVC syntax element
    uint8_t                       log2_diff_max_min_luma_coding_block_size;       //!< Same as HEVC syntax element
    uint8_t                       log2_min_transform_block_size_minus2;           //!< Same as HEVC syntax element
    uint8_t                       log2_diff_max_min_transform_block_size;         //!< Same as HEVC syntax element
    uint8_t                       max_transform_hierarchy_depth_inter;            //!< Same as HEVC syntax element
    uint8_t                       max_transform_hierarchy_depth_intra;            //!< Same as HEVC syntax element
    uint8_t                       num_short_term_ref_pic_sets;                    //!< Same as HEVC syntax element
    uint8_t                       num_long_term_ref_pic_sps;                      //!< Same as HEVC syntax element
    /*! \brief Same as HEVC syntax element.
    *
    *   When long slice control data format is taken, hardware decoder should take values from num_ref_idx_l0_active_minus1 and num_ref_idx_l1_active_minus1 from slice control data structure.
    */
    uint8_t                       num_ref_idx_l0_default_active_minus1;
    /*! \brief Same as HEVC syntax element.
    *
    *   When long slice control data format is taken, hardware decoder should take values from num_ref_idx_l0_active_minus1 and num_ref_idx_l1_active_minus1 from slice control data structure.
    */
    uint8_t                       num_ref_idx_l1_default_active_minus1;
    char                          init_qp_minus26;                                //!< Same as HEVC syntax element
    /*! \brief Decoder may ignore this value.
    *
    *   This is an redundant parameter which serves as same purpose as wNumBitsForShortTermRPSInSlice.
    */
    uint8_t                       ucNumDeltaPocsOfRefRpsIdx;
    /*! \brief Bit count in the bit stream for parsing short_term_ref_pic_set in slice segment header.
    *
    *   If short_term_ref_pic_set_sps_flag takes value 1, wNumBitsForShortTermRPSInSlice should be  0. The bit count value is calculated when emulation prevention bytes are removed from raw elementary bit stream.
    */
    uint16_t                      wNumBitsForShortTermRPSInSlice;
    uint16_t                      ReservedBits2;                                  //!< Value is used for alignemnt and has no meaning, set to 0.

    union
    {
        struct
        {
            uint32_t              scaling_list_enabled_flag                    : 1;   //!< Same as HEVC syntax element
            uint32_t              amp_enabled_flag                             : 1;   //!< Same as HEVC syntax element
            uint32_t              sample_adaptive_offset_enabled_flag          : 1;   //!< Same as HEVC syntax element
            uint32_t              pcm_enabled_flag                             : 1;   //!< Same as HEVC syntax element
            uint32_t              pcm_sample_bit_depth_luma_minus1             : 4;   //!< Same as HEVC syntax element
            uint32_t              pcm_sample_bit_depth_chroma_minus1           : 4;   //!< Same as HEVC syntax element
            uint32_t              log2_min_pcm_luma_coding_block_size_minus3   : 2;   //!< Same as HEVC syntax element
            uint32_t              log2_diff_max_min_pcm_luma_coding_block_size : 2;   //!< Same as HEVC syntax element
            uint32_t              pcm_loop_filter_disabled_flag                : 1;   //!< Same as HEVC syntax element
            uint32_t              long_term_ref_pics_present_flag              : 1;   //!< Same as HEVC syntax element
            uint32_t              sps_temporal_mvp_enabled_flag                : 1;   //!< Same as HEVC syntax element
            uint32_t              strong_intra_smoothing_enabled_flag          : 1;   //!< Same as HEVC syntax element
            uint32_t              dependent_slice_segments_enabled_flag        : 1;   //!< Same as HEVC syntax element
            uint32_t              output_flag_present_flag                     : 1;   //!< Same as HEVC syntax element
            uint32_t              num_extra_slice_header_bits                  : 3;   //!< Same as HEVC syntax element
            uint32_t              sign_data_hiding_enabled_flag                : 1;   //!< Same as HEVC syntax element
            uint32_t              cabac_init_present_flag                      : 1;   //!< Same as HEVC syntax element
            uint32_t              ReservedBits3                                : 5;   //!< Value is used for alignemnt and has no meaning, set to 0.
       };

        uint32_t                  dwCodingParamToolFlags;
    };

    union
    {
        struct
        {
            uint32_t              constrained_intra_pred_flag                 : 1;    //!< Same as HEVC syntax element
            uint32_t              transform_skip_enabled_flag                 : 1;    //!< Same as HEVC syntax element
            uint32_t              cu_qp_delta_enabled_flag                    : 1;    //!< Same as HEVC syntax element
            uint32_t              pps_slice_chroma_qp_offsets_present_flag    : 1;    //!< Same as HEVC syntax element
            uint32_t              weighted_pred_flag                          : 1;    //!< Same as HEVC syntax element
            uint32_t              weighted_bipred_flag                        : 1;    //!< Same as HEVC syntax element
            uint32_t              transquant_bypass_enabled_flag              : 1;    //!< Same as HEVC syntax element
            uint32_t              tiles_enabled_flag                          : 1;    //!< Same as HEVC syntax element
            uint32_t              entropy_coding_sync_enabled_flag            : 1;    //!< Same as HEVC syntax element
            uint32_t              uniform_spacing_flag                        : 1;    //!< Same as HEVC syntax element
            uint32_t              loop_filter_across_tiles_enabled_flag       : 1;    //!< Same as HEVC syntax element
            uint32_t              pps_loop_filter_across_slices_enabled_flag  : 1;    //!< Same as HEVC syntax element
            uint32_t              deblocking_filter_override_enabled_flag     : 1;    //!< Same as HEVC syntax element
            uint32_t              pps_deblocking_filter_disabled_flag         : 1;    //!< Same as HEVC syntax element
            /*! \brief Same as HEVC syntax element.
            *
            *   Host decoder should set the value properly based on syntax element restricted_ref_pic_lists_flag. If restricted_ref_pic_lists_flag equals 0, lists_modification_present_flag should be set to 1.
            */
            uint32_t              lists_modification_present_flag             : 1;
            uint32_t              slice_segment_header_extension_present_flag : 1;    //!< Same as HEVC syntax element
            /*! \brief Indicates whether the current picture is an IRAP picture.
            *
            *   This flag shall be equal to 1 when the current picture is an IRAP picture and shall be equal to 0 when the current picture is not an IRAP picture.
            */
            uint32_t              IrapPicFlag : 1;
            /*! \brief Indicates whether the current picture is an IDR picture.
            *
            *   This flag shall be equal to 1 when the current picture is an IDR picture and shall be equal to 0 when the current picture is not an IDR picture.
            */
            uint32_t              IdrPicFlag : 1;
            /*! \brief Takes value 1 when all the slices are intra slices, 0 otherwise.
            */
            uint32_t              IntraPicFlag                                : 1;
            /*! \brief CRC values are requested if set to 1.
            */
            uint32_t              RequestCRC                                  : 1;
            /*! \brief Histogram array is requested if set to 1.
            *
            *   If set, SFC should be enabled to generate the histogram array per channel. If other operations by SFC are required such as scaling, the histogram is generated against the final pixel buffer after the operation is performed.
            */
            uint32_t              RequestHistogram                            : 1;
            uint32_t              ReservedBits4                               : 11;   //!< Value is used for alignemnt and has no meaning, set to 0.
        };

        uint32_t                  dwCodingSettingPicturePropertyFlags;
    };

    char                          pps_cb_qp_offset;                       //!< Same as HEVC syntax element
    char                          pps_cr_qp_offset;                       //!< Same as HEVC syntax element
    uint8_t                       num_tile_columns_minus1;                //!< Same as HEVC syntax element
    uint8_t                       num_tile_rows_minus1;                   //!< Same as HEVC syntax element
    uint16_t                      column_width_minus1[19];                //!< Same as HEVC syntax element
    uint16_t                      row_height_minus1[21];                  //!< Same as HEVC syntax element
    uint8_t                       diff_cu_qp_delta_depth;                 //!< Same as HEVC syntax element
    char                          pps_beta_offset_div2;                   //!< Same as HEVC syntax element
    char                          pps_tc_offset_div2;                     //!< Same as HEVC syntax element
    uint8_t                       log2_parallel_merge_level_minus2;       //!< Same as HEVC syntax element

    /*! \brief Picture order count value for the current picture.
    *
    *   Value range is -2^31 to 2^31-1, inclusive.
    */
    int32_t                       CurrPicOrderCntVal;
    /*! \brief FrameIdx for each entry specifies the surface index for all pictures that are or will be referred to by the current or future pictures.
    *
    *   The valid entries are indexed from 0 to 126, inclusive. The PicFlags of non-valid entries (including the picture of the entry which is not referred by current picture or future pictures) should take value PICTURE_INVALID. A PicFlags setting of PICTURE_LONG_TERM_REFERENCE indicates if the picture is a long term reference or not.
    *   NOTE: for interlace (field) pictures, the FrameIdx field of two RefFrameList entries may have same value and point to same reference surface. And in this case, application should allocate buffer size with double picture height to hold the whole picture.
    */
    CODEC_PICTURE                 RefFrameList[15];
    /*! \brief Picture order count value for each of the reference pictures in the DPB buffer surface, corresponding to the entries of RefFrameList[15].
    */
    int32_t                       PicOrderCntValList[15];
    /*! \brief Contain the indices to the RefFrameList[] used in inter predection.
    *
    *   The indices to the RefFrameList[] indicate all the reference pictures that may be used in inter prediction of the current picture and that may be used in inter prediction of one or more of the pictures following the current picture in decoding order.
    *   When an entry in RefPicSetStCurrBefore[], RefPicSetStCurrAfter[] and RefPicSetLtCurr[] is not valid, it shall be set to 0xff. Invalid entries shall not be present between valid entries in RefPicSetStCurrBefore[], RefPicSetStCurrAfter[] and RefPicSetLtCurr[]. Valid entries in RefPicSetStCurrBefore[], RefPicSetStCurrAfter[] and RefPicSetLtCurr[] shall have values in the range of 0 to 7, inclusive, and each corresponding entry in RefFrameList[] referred to by a valid entry in RefPicSetStCurrBefore[], RefPicSetStCurrAfter[] and RefPicSetLtCurr[] shall not have PicFlags equal to PICTURE_INVALID. Any entry in RefPicSetStCurrBefore[], RefPicSetStCurrAfter[] and RefPicSetLtCurr[] that is not equal to 0xFF shall not be equal to the value of any other entry in RefPicSetStCurrBefore[], RefPicSetStCurrAfter[] or RefPicSetLtCurr[].
    */
    uint8_t                       RefPicSetStCurrBefore[8];
    /*! \brief Contain the indices to the RefFrameList[] used in inter predection.
    *
    *   The indices to the RefFrameList[] indicate all the reference pictures that may be used in inter prediction of the current picture and that may be used in inter prediction of one or more of the pictures following the current picture in decoding order.
    *   When an entry in RefPicSetStCurrBefore[], RefPicSetStCurrAfter[] and RefPicSetLtCurr[] is not valid, it shall be set to 0xff. Invalid entries shall not be present between valid entries in RefPicSetStCurrBefore[], RefPicSetStCurrAfter[] and RefPicSetLtCurr[]. Valid entries in RefPicSetStCurrBefore[], RefPicSetStCurrAfter[] and RefPicSetLtCurr[] shall have values in the range of 0 to 7, inclusive, and each corresponding entry in RefFrameList[] referred to by a valid entry in RefPicSetStCurrBefore[], RefPicSetStCurrAfter[] and RefPicSetLtCurr[] shall not have PicFlags equal to PICTURE_INVALID. Any entry in RefPicSetStCurrBefore[], RefPicSetStCurrAfter[] and RefPicSetLtCurr[] that is not equal to 0xFF shall not be equal to the value of any other entry in RefPicSetStCurrBefore[], RefPicSetStCurrAfter[] or RefPicSetLtCurr[].
    */
    uint8_t                       RefPicSetStCurrAfter[8];
    /*! \brief Contain the indices to the RefFrameList[] used in inter predection.
    *
    *   The indices to the RefFrameList[] indicate all the reference pictures that may be used in inter prediction of the current picture and that may be used in inter prediction of one or more of the pictures following the current picture in decoding order.
    *   When an entry in RefPicSetStCurrBefore[], RefPicSetStCurrAfter[] and RefPicSetLtCurr[] is not valid, it shall be set to 0xff. Invalid entries shall not be present between valid entries in RefPicSetStCurrBefore[], RefPicSetStCurrAfter[] and RefPicSetLtCurr[]. Valid entries in RefPicSetStCurrBefore[], RefPicSetStCurrAfter[] and RefPicSetLtCurr[] shall have values in the range of 0 to 7, inclusive, and each corresponding entry in RefFrameList[] referred to by a valid entry in RefPicSetStCurrBefore[], RefPicSetStCurrAfter[] and RefPicSetLtCurr[] shall not have PicFlags equal to PICTURE_INVALID. Any entry in RefPicSetStCurrBefore[], RefPicSetStCurrAfter[] and RefPicSetLtCurr[] that is not equal to 0xFF shall not be equal to the value of any other entry in RefPicSetStCurrBefore[], RefPicSetStCurrAfter[] or RefPicSetLtCurr[].
    */
    uint8_t                       RefPicSetLtCurr[8];
    /*! \brief Is a 16 entry array indicating whether or not a picture is a field picture.
    *
    *   Each bit of the low 15 bits indicats if the associated picture in DPB is a field picture or not. Specifically, if ((RefFieldPicFlag >> i) & 0x01) > 0, then the referencepicture specified by RefFrameList[i] is a field picture. Otherwise, it is frame picture. For field picture, coresponding bit of RefBottomFieldFlag indicates the field polarity. The MSB, (RefFieldPicFlag >> 15) & 0x01, indicates the field or frame status of current decoded picture, CurrPic.
    */
    uint16_t                      RefFieldPicFlag;
    /*! \brief Is a 16 entry array indicating the polarity of a picture.
    *
    *   Each bit of the low 15 bits indicats the polarity of the associated reference field picture. If ((RefBottomFieldFlag >> i) & 0x01) > 0, then the reference picture  takes odd lines in the surface specified by RefFrameList[i]. And ((RefBottomFieldFlag >> i) & 0x01) = 0 indicates the reference picture takes even lines. The MSB, ((RefBottomFieldFlag >> i) & 0x01), indicates the polarity of the current decoded picture, CurrPic.
    */
    uint16_t                      RefBottomFieldFlag;
    /*! \brief Arbitrary number set by the host decoder to use as a tag in the status report feedback data.
    *
    *   The value should not equal 0, and should be different in each call to Execute.
    */
    uint32_t                      StatusReportFeedbackNumber;
    uint32_t                      dwLastSliceEndPos;

    uint16_t                      TotalNumEntryPointOffsets;  //!< Total entrypoint offset in subset buffer
} CODEC_HEVC_PIC_PARAMS, *PCODEC_HEVC_PIC_PARAMS;

/*! \brief Slice-level parameters of a compressed picture for HEVC decoding.
*
*   The slice control buffer is accompanied by a raw bitstream data buffer. The total quantity of data in the bitstream buffer (and the amount of data reported by the host decoder) shall be an integer multiple of 128 bytes.
*/
typedef struct _CODEC_HEVC_SLICE_PARAMS
{
    /*! \brief Number of bytes in the bitstream data buffer that are associated with this slice control data structure.
    *
    *   Starting with the byte at the offset given in slice_data_offset. The bitstream data buffer shall not contain additional byte stream NAL units in the bytes following BSNALunitDataLocation up to the location slice_data_offset + slice_data_size. If slice_data_offset + slice_data_size exceeds the boundary of current bitstream data buffer, the excess slice bytes should continue from the first byte of next bitstream data buffer.
    */
    uint32_t                slice_data_size;
    /*! \brief This member locates the NAL unit with nal_unit_type equal to 1 .. 8 for the current slice.
    *
    *   At least one bit stream data buffer should be present which is associated with the slice control data buffer. If necessary, multiple bit stream data buffers are allowed, but not suggested. The size of the data in the bitstream data buffer (and the amount of data reported by the host decoder) shall be an integer multiple of 128 bytes. When  the end of the slice data is not an even multiple of 128 bytes, the decoder should pad the end of the buffer with zeroes.  When more than one bitstream data buffers are present, these data buffers should be in sequential order. They should be treated as if concatenated linearly with no space in between.  The value of slice_data_offset is the byte offset, from the start of the first bitstream data buffer, of the first byte of the start code prefix in the byte stream NAL unit that contains the NAL unit with nal_unit_type equal to 1 .. 8. The current slice is the slice associated with this slice control data structure. The bitstream data buffer shall not contain NAL units with values of nal_unit_type outside the range [1 .. 8]. However, the accelerator shall allow any such NAL units to be present and should ignore their content if present.
    *   Note: The bitstream data buffer shall contain the full NAL unit byte stream, either encrpted or clear. This means that the buffer will contain emulation_prevention_three_byte syntax elements where those elements are required to be present in a NAL unit, as defined in the HEVC specification. The bitstream data buffer may or may not contrain leading_zero_8bits, zero_byte, and trailing_zero_8bits syntax elements. If present, the accelerator shall ignore these elements.
    */
    uint32_t                slice_data_offset;
    /*! \brief This member indicated if current slice is complete.
    *
    *   0 - Complete; 1 - Partial slice data with the start of slice; 2 - Partial slice data with the end of slice; 3 - Partial slice data with the middle of slice;
    */
    uint16_t                slice_chopping;

    // Long format specific
    uint16_t                NumEmuPrevnBytesInSliceHdr;           //!< Number of emulation prevention bytes in slice head; ByteOffsetToSliceData doesn't include these bytes.
    /*! \brief Byte offset to the location of the first byte of slice_data() data structure for the current slice in the bitstream data buffer.
    *
    *   This byte offset is the offset within the RBSP date for the slice, relative to the starting position of the slice_header() in the RBSP. That is, it represents a byte offset after the removal of any emulation_prevention_three_byte syntax elements that precedes the start of the slice_data() in the NAL unit.
    */
    uint32_t                ByteOffsetToSliceData;
    /*! \brief Same as HEVC syntax element.
    *
    *   For first slice in the picture, slice_segment_address shall be set to 0.
    */
    uint32_t                slice_segment_address;
    /*! \brief Specifies the surfaces of reference pictures
    *
    *   The value of FrameIdx specifies the index of RefFrameList structure. And valid value range is [0..14, 0x7F]. Invalid entries are indicated by setting PicFlags to PICTURE_INVALID and the PicFlags value of PICTURE_LONG_TERM_REFERENCE has no meaning.
    *   RefPicIdx[0][] corresponds to reference list 0.
    *   RefPicIdx[1][] corresponds to reference list 1.
    *   Each list may contain duplicated reference picture indexes.
    */
    CODEC_PICTURE       RefPicList[2][15];
    union
    {
        uint32_t            value;
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
        }fields;
    }LongSliceFlags;

    /*! \brief Index to the RefPicList[0][] or RefPicList[1][].
    *
    *   It should be derived from HEVC syntax element collocated_ref_idx. When the HEVC syntax element slice_temporal_mvp_enabled_flag takes value 0,  collocated_ref_idx should take value 0xFF. Valid value range is [0.. num_ref_idx_l0_active_minus1] or [0..num_ref_idx_l1_active_minus1] depending on collocated_from_l0_flag. If collocated_ref_idx takes a valid value, the corresponding entry of RefFrameList[] must contain a valid surface index.
    */
    uint8_t             collocated_ref_idx;
    /*! \brief Same as HEVC syntax element.
    *
    *   If num_ref_idx_active_override_flag == 0, host decoder shall set their values with num_ref_idx_l0_default_minus1, and num_ref_idx_l1_default_minus1.
    */
    uint8_t             num_ref_idx_l0_active_minus1;
    /*! \brief Same as HEVC syntax element.
    *
    *   If num_ref_idx_active_override_flag == 0, host decoder shall set their values with num_ref_idx_l0_default_minus1, and num_ref_idx_l1_default_minus1.
    */
    uint8_t             num_ref_idx_l1_active_minus1;
    char                slice_qp_delta;                 //!< Same as HEVC syntax element
    char                slice_cb_qp_offset;             //!< Same as HEVC syntax element
    char                slice_cr_qp_offset;             //!< Same as HEVC syntax element
    char                slice_beta_offset_div2;         //!< Same as HEVC syntax element
    char                slice_tc_offset_div2;           //!< Same as HEVC syntax element
    /*! \brief Same as HEVC syntax element.
    *
    *   Specifies the base 2 logarithm of the denominator for all luma weighting factors. Value range: 0 to 7, inclusive.
    */
    uint8_t             luma_log2_weight_denom;
    /*! \brief Same as HEVC syntax element.
    *
    *   Specifies the base 2 logarithm of the denominator for all chroma weighting factors. Value range of luma_log2_weight_denom + delta_chroma_log2_weight_denom: 0 to 7, inclusive.
    */
    uint8_t             delta_chroma_log2_weight_denom;

    /*! \brief Same as HEVC syntax element.
    *
    *   If the corresponding luma or chroma weight flags are 0, the value should also be set to default value according to HEVC specification.
    */
    char                delta_luma_weight_l0[15];
    /*! \brief Same as HEVC syntax element.
    *
    *   If the corresponding luma or chroma weight flags are 0, the value should also be set to default value according to HEVC specification.
    */
    char                luma_offset_l0[15];
    /*! \brief Same as HEVC syntax element.
    *
    *   If the corresponding luma or chroma weight flags are 0, the value should also be set to default value according to HEVC specification.
    */
    char                delta_chroma_weight_l0[15][2];
    /*! \brief Same as HEVC syntax element.
    *
    *   If the corresponding chroma weight flags are 0, the value should also be set to 0. Please note that for range extension profiles other than main, main10, and their related intra or still image profiles, the data types are defined differrently.
    */
    char                ChromaOffsetL0[15][2];

    /*! \brief Same as HEVC syntax element.
    *
    *   If the corresponding luma or chroma weight flags are 0, the value should also be set to default value according to HEVC specification.
    */
    char                delta_luma_weight_l1[15];
    /*! \brief Same as HEVC syntax element.
    *
    *   If the corresponding luma or chroma weight flags are 0, the value should also be set to default value according to HEVC specification.
    */
    char                luma_offset_l1[15];
    /*! \brief Same as HEVC syntax element.
    *
    *   If the corresponding luma or chroma weight flags are 0, the value should also be set to default value according to HEVC specification.
    */
    char                delta_chroma_weight_l1[15][2];
    /*! \brief Same as HEVC syntax element.
    *
    *   If the corresponding chroma weight flags are 0, the value should also be set to 0. Please note that for range extension profiles other than main, main10, and their related intra or still image profiles, the data types are defined differrently.
    */
    char                ChromaOffsetL1[15][2];

    /*! \brief Same as HEVC syntax element.
    *
    *   HEVC spec variable MaxNumMergeCand can be derived by 5 - five_minus_max_num_merge_cand, and specifies the maximum number of merging MVP candidates supported in the slice. Value range: 0 to 4 inclusive.
    */
    uint8_t             five_minus_max_num_merge_cand;
    uint16_t            num_entry_point_offsets;                // [0..540]
    uint16_t            EntryOffsetToSubsetArray;               // [0..540]
} CODEC_HEVC_SLICE_PARAMS, *PCODEC_HEVC_SLICE_PARAMS;

/*! \brief Short Format Slice-level parameters of a compressed picture for HEVC decoding.
*
*   The slice control buffer is accompanied by a raw bitstream data buffer. The total quantity of data in the bitstream buffer (and the amount of data reported by the host decoder) shall be an integer multiple of 128 bytes.
*/
typedef struct _CODEC_HEVC_SF_SLICE_PARAMS
{
    /*! \brief Number of bytes in the bitstream data buffer that are associated with this slice control data structure.
    *
    *   Starting with the byte at the offset given in slice_data_offset. The bitstream data buffer shall not contain additional byte stream NAL units in the bytes following BSNALunitDataLocation up to the location slice_data_offset + slice_data_size. If slice_data_offset + slice_data_size exceeds the boundary of current bitstream data buffer, the excess slice bytes should continue from the first byte of next bitstream data buffer.
    */
    uint32_t slice_data_size;
    /*! \brief This member locates the NAL unit with nal_unit_type equal to 1 .. 8 for the current slice.
    *
    *   At least one bit stream data buffer should be present which is associated with the slice control data buffer. If necessary, multiple bit stream data buffers are allowed, but not suggested. The size of the data in the bitstream data buffer (and the amount of data reported by the host decoder) shall be an integer multiple of 128 bytes. When  the end of the slice data is not an even multiple of 128 bytes, the decoder should pad the end of the buffer with zeroes.  When more than one bitstream data buffers are present, these data buffers should be in sequential order. They should be treated as if concatenated linearly with no space in between.  The value of slice_data_offset is the byte offset, from the start of the first bitstream data buffer, of the first byte of the start code prefix in the byte stream NAL unit that contains the NAL unit with nal_unit_type equal to 1 .. 8. The current slice is the slice associated with this slice control data structure. The bitstream data buffer shall not contain NAL units with values of nal_unit_type outside the range [1 .. 8]. However, the accelerator shall allow any such NAL units to be present and should ignore their content if present.
    *   Note: The bitstream data buffer shall contain the full NAL unit byte stream, either encrpted or clear. This means that the buffer will contain emulation_prevention_three_byte syntax elements where those elements are required to be present in a NAL unit, as defined in the HEVC specification. The bitstream data buffer may or may not contrain leading_zero_8bits, zero_byte, and trailing_zero_8bits syntax elements. If present, the accelerator shall ignore these elements.
    */
    uint32_t slice_data_offset;
    /*! \brief This member indicated if current slice is complete.
    *
    *   0 - Complete; 1 - Partial slice data with the start of slice; 2 - Partial slice data with the end of slice; 3 - Partial slice data with the middle of slice;
    */
    uint16_t slice_chopping;
} CODEC_HEVC_SF_SLICE_PARAMS, *PCODEC_HEVC_SF_SLICE_PARAMS;

/*! \brief Additional picture-level parameters of a compressed picture for HEVC decoding.
*
*   Defined for profiles main12, main4:2:2 10, main4:2:2 12, main4:4:4, main4:4:4 10, main4:4:4 12 and their related intra and still picture profiles.
*/
typedef struct _CODEC_HEVC_EXT_PIC_PARAMS
{
    union
    {
        struct
        {
            uint32_t        transform_skip_rotation_enabled_flag : 1;    //!< Same as HEVC syntax element
            uint32_t        transform_skip_context_enabled_flag : 1;    //!< Same as HEVC syntax element
            uint32_t        implicit_rdpcm_enabled_flag : 1;    //!< Same as HEVC syntax element
            uint32_t        explicit_rdpcm_enabled_flag : 1;    //!< Same as HEVC syntax element
            uint32_t        extended_precision_processing_flag : 1;    //!< Same as HEVC syntax element
            uint32_t        intra_smoothing_disabled_flag : 1;    //!< Same as HEVC syntax element
            uint32_t        high_precision_offsets_enabled_flag : 1;    //!< Same as HEVC syntax element
            uint32_t        persistent_rice_adaptation_enabled_flag : 1;    //!< Same as HEVC syntax element
            uint32_t        cabac_bypass_alignment_enabled_flag : 1;    //!< Same as HEVC syntax element
            uint32_t        cross_component_prediction_enabled_flag : 1;    //!< Same as HEVC syntax element
            uint32_t        chroma_qp_offset_list_enabled_flag : 1;    //!< Same as HEVC syntax element
            uint32_t        BitDepthLuma16 : 1;    //!< Same as HEVC syntax element
            uint32_t        BitDepthChroma16 : 1;    //!< Same as HEVC syntax element
            uint32_t        ReservedBits5 : 19;   //!< Value is used for alignemnt and has no meaning, set to 0.
        } fields;
        uint32_t            dwRangeExtensionPropertyFlags;
    } PicRangeExtensionFlags;

    uint8_t            diff_cu_chroma_qp_offset_depth;             //!< Same as HEVC syntax element, [0..3]
    uint8_t            chroma_qp_offset_list_len_minus1;           //!< Same as HEVC syntax element, [0..5]
    uint8_t            log2_sao_offset_scale_luma;                 //!< Same as HEVC syntax element, [0..6]
    uint8_t            log2_sao_offset_scale_chroma;               //!< Same as HEVC syntax element, [0..6]
    uint8_t         log2_max_transform_skip_block_size_minus2;  //!< Same as HEVC syntax element
    char            cb_qp_offset_list[6];                       //!< Same as HEVC syntax element, [-12..12]
    char            cr_qp_offset_list[6];                       //!< Same as HEVC syntax element, [-12..12]
} CODEC_HEVC_EXT_PIC_PARAMS, *PCODEC_HEVC_EXT_PIC_PARAMS;


/*! \brief Additional range extention slice-level parameters of a compressed picture for HEVC decoding.
*
*   HEVC range extension profiles extend the luma and chroma offset values from 8 bits to 16 bits.
*/
typedef struct _CODEC_HEVC_EXT_SLICE_PARAMS
{
    /*! \brief Same as HEVC syntax element.
    *
    *   These set of values are the most significant 8-bit part of the corresponding luma_offset_l0[]. Combining with the luma_offset_l0[] will give the final values respectively.  The sign for each parameter is determined by the sign of corresponding luma_offset_l0[].
    */
    int16_t                luma_offset_l0[15];
    /*! \brief Same as HEVC syntax element.
    *
    *   These set of values are the most significant 8-bit part of the corresponding chroma_offset_l0[]. Combining with the chroma_offset_l0[] will give the final values respectively.  The sign for each parameter is determined by the sign of corresponding chroma_offset_l0[].
    */
    int16_t                ChromaOffsetL0[15][2];
    /*! \brief Same as HEVC syntax element.
    *
    *   These set of values are the most significant 8-bit part of the corresponding luma_offset_l1[]. Combining with the luma_offset_l1[] will give the final values respectively.  The sign for each parameter is determined by the sign of corresponding luma_offset_l1[].
    */
    int16_t                luma_offset_l1[15];
    /*! \brief Same as HEVC syntax element.
    *
    *   These set of values are the most significant 8-bit part of the corresponding chroma_offset_l1[]. Combining with the chroma_offset_l1[] will give the final values respectively.  The sign for each parameter is determined by the sign of corresponding chroma_offset_l1[].
    */
    int16_t                ChromaOffsetL1[15][2];

    bool                   cu_chroma_qp_offset_enabled_flag;  //!< Same as HEVC syntax element

                                                              // For Screen Content Extension
    char                   slice_act_y_qp_offset;      // [-12..12]
    char                   slice_act_cb_qp_offset;     // [-12..12]
    char                   slice_act_cr_qp_offset;     // [-12..12]
    unsigned char          use_integer_mv_flag;
} CODEC_HEVC_EXT_SLICE_PARAMS, *PCODEC_HEVC_EXT_SLICE_PARAMS;

typedef struct _CODEC_HEVC_SUBSET_PARAMS
{
    uint32_t                 entry_point_offset_minus1[440];
} CODEC_HEVC_SUBSET_PARAMS, *PCODEC_HEVC_SUBSET_PARAMS;
#endif  // __CODEC_DEF_DECODE_HEVC_H__
