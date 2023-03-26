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
//! \file     codec_def_decode_avc.h
//! \brief    Defines decode AVC types and macros shared by CodecHal, MHW, and DDI layer
//! \details  Applies to AVC decode only. Should not contain any DDI specific code.
//!
#ifndef __CODEC_DEF_DECODE_AVC_H__
#define __CODEC_DEF_DECODE_AVC_H__

#include "codec_def_common_avc.h"

//Check whether interview prediction is used through POC
#define CodecHal_IsInterviewPred(currPic, currPoc, avcRefListIdx) ( ((avcRefListIdx)!=(currPic).FrameIdx) &&              \
    (!CodecHal_PictureIsTopField(currPic) && (ppAvcRefList[avcRefListIdx]->iFieldOrderCnt[1] == (currPoc)[1]) || \
    !CodecHal_PictureIsBottomField(currPic) && (ppAvcRefList[avcRefListIdx]->iFieldOrderCnt[0] == (currPoc)[0])) && \
    ((currPic).FrameIdx != 0x7f))

typedef struct _CODEC_AVC_DMV_LIST
{
    uint8_t             ucFrameId;
    bool                bInUse;
    bool                bReUse;
} CODEC_AVC_DMV_LIST, *PCODEC_AVC_DMV_LIST;

//!
//! \enum     AvcChromaFormatIdc
//! \brief    AVC chroma format Idc
//!
enum AvcChromaFormatIdc
{
    avcChromaFormatMono = 0,
    avcChromaFormat420  = 1,
    avcChromaFormat422  = 2,
    avcChromaFormat444  = 3,
};

// H.264 Picture Parameters Buffer
typedef struct _CODEC_AVC_PIC_PARAMS
{
    /*! \brief Uncompressed destination surface of the frame for the current decoded picture.
    *
    *   The long_term_ref_flag has no meaning. The valid value range for Curr.FrameIdx is [0..126]. Value 127 or 0x7F can be treated as an invalid surface index.
    */
    CODEC_PICTURE           CurrPic;
    /*! \brief FrameIdx for each entry specifies the surface index for all pictures that are or will be referred to by the current or future pictures.
    *
    *   The valid entries are indexed from 0 to 126, inclusive. The PicFlags of non-valid entries (including the picture of the entry which is not referred by current picture or future pictures) should take value PICTURE_INVALID. A PicFlags setting of PICTURE_LONG_TERM_REFERENCE indicates if the picture is a long term reference or not.
    *   NOTE: for interlace (field) pictures, the FrameIdx field of two RefFrameList entries may have same value and point to same reference surface. And in this case, application should allocate buffer size with double picture height to hold the whole picture.
    */
    CODEC_PICTURE           RefFrameList[CODEC_AVC_MAX_NUM_REF_FRAME];

    uint16_t                pic_width_in_mbs_minus1;    //!< Same as AVC syntax element.
    /*! \brief The height of the frame in MBs minus 1.
    *
    *    Derived from pic_height_in_map_units_minus1: pic_height_in_map_units_minus1 << uint16_t(frame_mbs_only_flag == 0)
    */
    uint16_t                pic_height_in_mbs_minus1;
    uint8_t                 bit_depth_luma_minus8;      //!< Same as AVC syntax element.
    uint8_t                 bit_depth_chroma_minus8;    //!< Same as AVC syntax element.
    uint8_t                 num_ref_frames;             //!< Same as AVC syntax element.
    /*! \brief Contains the picture order counts (POC) for the current frame
    *
    *   If field_pic_flag is 0:
    *        \n - CurrFieldOrderCnt[0] contains the top field order count for the current picture
    *        \n - CurrFieldOrderCnt[1] contains the bottom field order count for the current picture
    *   \n If field_pic_flag is 1 and CurrPic.PicFlags indicates that this is a top field:
    *        \n - CurrFieldOrderCnt[0] contains the top field order count for the current picture
    *        \n - CurrFieldOrderCnt[1]
    *   \n If field_pic_flag is 1 and CurrPic.PicFlags indicates that this is a bottom field:
    *        \n - CurrFieldOrderCnt[0] should be 0 or ignored
    *        \n - CurrFieldOrderCnt[1] contains the bottom field order count for the current picture
    */
    int32_t                 CurrFieldOrderCnt[2];
    /*! \brief Contains the POCs for the reference frames in RefFrameList.
    *
    *   For each entry FieldOrderCntList[i][j]:
    *        \n - i: the picture index
    *        \n - j: 0 specifies the top field order count and 1 specifies the bottom field order count
    *   \n If a entry i in RefFrameList is not relevant (it is not used for reference) or valid, the entry FieldOrderCount[i][0 and 1] should be 0.
    */
    int32_t                 FieldOrderCntList[16][2];

    union
    {
        struct
        {
            uint32_t            chroma_format_idc                      : 2; //!< Same as AVC syntax element.
            uint32_t            residual_colour_transform_flag         : 1; //!< Same as AVC syntax element.
            uint32_t            frame_mbs_only_flag                    : 1; //!< Same as AVC syntax element.
            uint32_t            mb_adaptive_frame_field_flag           : 1; //!< Same as AVC syntax element.
            uint32_t            direct_8x8_inference_flag              : 1; //!< Same as AVC syntax element.
            uint32_t                                                   : 1;
            uint32_t            log2_max_frame_num_minus4              : 4; //!< Same as AVC syntax element.
            uint32_t            pic_order_cnt_type                     : 2; //!< Same as AVC syntax element.
            uint32_t            log2_max_pic_order_cnt_lsb_minus4      : 4; //!< Same as AVC syntax element.
            uint32_t            delta_pic_order_always_zero_flag       : 1; //!< Same as AVC syntax element.
        };
        uint32_t         value;
    } seq_fields;

    uint8_t                 num_slice_groups_minus1;        //!< Same as AVC syntax element.
    uint8_t                 slice_group_map_type;           //!< Same as AVC syntax element.
    uint16_t                slice_group_change_rate_minus1; //!< Same as AVC syntax element.
    char                    pic_init_qp_minus26;            //!< Same as AVC syntax element.
    char                    chroma_qp_index_offset;         //!< Same as AVC syntax element.
    char                    second_chroma_qp_index_offset;  //!< Same as AVC syntax element.

    union
    {
        struct
        {
            uint32_t            entropy_coding_mode_flag               : 1; //!< Same as AVC syntax element.
            uint32_t            weighted_pred_flag                     : 1; //!< Same as AVC syntax element.
            uint32_t            weighted_bipred_idc                    : 2; //!< Same as AVC syntax element.
            uint32_t            transform_8x8_mode_flag                : 1; //!< Same as AVC syntax element.
            uint32_t            field_pic_flag                         : 1; //!< Same as AVC syntax element.
            uint32_t            constrained_intra_pred_flag            : 1; //!< Same as AVC syntax element.
            uint32_t            pic_order_present_flag                 : 1; //!< Same as AVC syntax element.
            uint32_t            deblocking_filter_control_present_flag : 1; //!< Same as AVC syntax element.
            uint32_t            redundant_pic_cnt_present_flag         : 1; //!< Same as AVC syntax element.
            uint32_t            reference_pic_flag                     : 1; //!< Same as AVC syntax element.
            uint32_t            IntraPicFlag                           : 1; //!< All MBs in frame use intra prediction mode.
        };
        uint32_t         value;
    } pic_fields;

    // Short format specific
    uint8_t                   num_ref_idx_l0_active_minus1;   //!< Same as AVC syntax element.
    uint8_t                   num_ref_idx_l1_active_minus1;   //!< Same as AVC syntax element.
    /*! \brief Contains the value of FrameNum or LongTermRefIdx depending on the PicFlags for the frame.
    *
    *   Each entry in FrameNumList has a corresponding entry in RefFrameList, if an entry in RefFrameList is not relevant (it is not used for reference) or valid, the entry in FrameNumList should be 0.
    */
    uint16_t                  FrameNumList[16];
    /*! \brief Denotes "non-existing" frames as defined in the AVC specification.
    *
    *   The flag is accessed by: Flag(i) = (NonExistingFrameFlags >> i) & 1. If Flag(i) is 1, frame i is marked as "non-existing", otherwise the frame is existing.
    */
    uint16_t                  NonExistingFrameFlags;
    /*! \brief Denotes "used for reference" frames as defined in the AVC specification.
    *
    *   The flag is accessed by:
    *        \n - FlagTop(i) = (UsedForReferenceFlags >> (2 * i)) & 1
    *        \n - FlagBottom(i) = (UsedForReferenceFlags >> (2 * i + 1)) & 1
    *   \n If FlagTop(i) is 1, the top field or frame numger i is marked as "used for reference"; if FlagBottom(i) is 1 then then bottom field of frame i is marked as "used for reference". If either is 0 then the frame is not marked as "used for reference".
    */
    uint32_t                  UsedForReferenceFlags;
    uint16_t                  frame_num;                      //!< Same as AVC syntax element.

    /*! \brief Arbitrary number set by the host decoder to use as a tag in the status report feedback data.
    *
    *   The value should not equal 0, and should be different in each call to Execute.
    */
    uint32_t                  StatusReportFeedbackNumber;
} CODEC_AVC_PIC_PARAMS, *PCODEC_AVC_PIC_PARAMS;

// H.264 Decode Slice Parameter Buffer (Long/Short format)
typedef struct _CODEC_AVC_SLICE_PARAMS
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
    /*! \brief Specifies the reference picture lists 0 and 1
    *
    *    Contains field/frame information concerning the reference in PicFlags. RefPicList[i][j]:
    *        \n - i: the reference picture list (0 or 1)
    *        \n - j: if the PicFlags are not PICTURE_INVALID, the index variable j is a reference to entry j in teh reference picture list.
    */
    CODEC_PICTURE               RefPicList[2][32];
    uint8_t                     luma_log2_weight_denom;         //!< Same as AVC syntax element.
    uint8_t                     chroma_log2_weight_denom;       //!< Same as AVC syntax element.
    /*! \brief Specifies the weights and offsets used for explicit mode weighted prediction.
    *
    *    Weigths[i][j][k][m]:
    *        \n - i: the reference picture list (0 or 1)
    *        \n - j: reference to entry j in RefPicList (has range [0...31])
    *        \n - k: the YUV component (0 = luma, 1 = Cb chroma, 2 = Cr chroma)
    *        \n - m: the weight or offset used in the weighted prediction process (0 = weight, 1 = offset)
    */
    int16_t                     Weights[2][32][3][2];
    uint16_t                    slice_id;                       //!< Same as AVC syntax element.
    uint16_t                    first_mb_in_next_slice;         //!< If there is a subsequent slice, specifies first_mb_in_slice for the next slice, otherwise is 0.
} CODEC_AVC_SLICE_PARAMS, *PCODEC_AVC_SLICE_PARAMS;

typedef struct _CODEC_AVC_SF_SLICE_PARAMS
{
    uint32_t slice_data_size;    //!< Number of bytes in the bitstream buffer for this slice.
    uint32_t slice_data_offset;  //!< The offset to the NAL start code for this slice.
} CODEC_AVC_SF_SLICE_PARAMS, *PCODEC_AVC_SF_SLICE_PARAMS;

// AVC MVC Extension Picture Parameter Set
// (sent along with regular _CODEC_AVC_PIC_PARAMS)
typedef struct _CODEC_MVC_EXT_PIC_PARAMS
{
    uint16_t                    CurrViewID;
    uint8_t                     anchor_pic_flag;
    uint8_t                     inter_view_flag;
    uint8_t                     NumInterViewRefsL0;
    uint8_t                     NumInterViewRefsL1;
    union
    {
        uint8_t                 bPicFlags;
        struct
        {
            uint8_t             SwitchToAVC     : 1;
            uint8_t             Reserved7Bits   : 7;
        };
    };
    uint8_t                     Reserved8Bits;
    uint16_t                    ViewIDList[16];
    uint16_t                    InterViewRefList[2][16];
} CODEC_MVC_EXT_PIC_PARAMS, *PCODEC_MVC_EXT_PIC_PARAMS;

#endif  // __CODEC_DEF_DECODE_AVC_H__
