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
//! \file     codec_def_decode_vc1.h
//! \brief    Defines decode VC1 types and macros shared by CodecHal, MHW, and DDI layer
//! \details  Applies to JPEG decode only. Should not contain any DDI specific code.
//!
#ifndef __CODEC_DEF_DECODE_VC1_H__
#define __CODEC_DEF_DECODE_VC1_H__

// VC1 reference flags
#define CODECHAL_WMV9_RANGE_ADJUSTMENT   0x00100000
#define CODECHAL_VC1_PROGRESSIVE         0x00200000
#define CODECHAL_VC1_TOP_FIELD_COMP      0x00010000
#define CODECHAL_VC1_TOP_FIELD_COMP_2    0x00020000
#define CODECHAL_VC1_BOT_FIELD_COMP      0x00040000
#define CODECHAL_VC1_BOT_FIELD_COMP_2    0x00080000
#define CODECHAL_VC1_FRAME_COMP          (CODECHAL_VC1_TOP_FIELD_COMP | CODECHAL_VC1_BOT_FIELD_COMP)
#define CODECHAL_VC1_ALL_COMP            0x000F0000

//!
//! \enum  Vc1FramPictureType
//! \brief VC1 fram picture Types
//!
enum Vc1FramPictureType
{
    vc1IFrame = 0,
    vc1PFrame,
    vc1BFrame,
    vc1BIFrame,
    vc1SkippedFrame
};

//!
//! \enum  Vc1FieldPictureType
//! \brief VC1 field picture Types
//!
enum Vc1FieldPictureType
{
    vc1IIField = 0,
    vc1IPField,
    vc1PIField,
    vc1PPField,
    vc1BBField,
    vc1BBIField,
    vc1BIBField,
    vc1BIBIField
};

//!
//! \enum  Vc1QuantizerType
//! \brief VC-1 Spec Table 259: Quantizer Specification
//!
enum Vc1QuantizerType
{
    vc1QuantizerImplicit = 0, // specified at frame level
    vc1QuantizerExplicit,     // specified at frame level
    vc1QuantizerNonuniform,   // used for all frames
    vc1QuantizerUniform       // used for all frames
};

typedef struct _CODEC_VC1_PIC_PARAMS
{
    CODEC_PICTURE    CurrPic;
    uint16_t         DeblockedPicIdx;
    uint16_t         ForwardRefIdx;
    uint16_t         BackwardRefIdx;

    /* sequence layer for AP or meta data for SP and MP */
    union
    {
        struct
        {
            uint32_t         pulldown                       : 1;    /* SEQUENCE_LAYER::PULLDOWN */
            uint32_t         interlace                      : 1;    /* SEQUENCE_LAYER::INTERLACE */
            uint32_t         tfcntrflag                     : 1;    /* SEQUENCE_LAYER::TFCNTRFLAG */
            uint32_t         finterpflag                    : 1;    /* SEQUENCE_LAYER::FINTERPFLAG */
            uint32_t         psf                            : 1;    /* SEQUENCE_LAYER::PSF */
            uint32_t         multires                       : 1;    /* METADATA::MULTIRES */
            uint32_t         overlap                        : 1;    /* METADATA::OVERLAP */
            uint32_t         syncmarker                     : 1;    /* METADATA::SYNCMARKER */
            uint32_t         rangered                       : 1;    /* METADATA::RANGERED */
            uint32_t         max_b_frames                   : 3;    /* METADATA::MAXBFRAMES */
            uint32_t         AdvancedProfileFlag            : 1;
        };
        uint32_t         value;
    } sequence_fields;

    uint16_t         coded_width;                                 /* ENTRY_POINT_LAYER::CODED_WIDTH */
    uint16_t         coded_height;                                /* ENTRY_POINT_LAYER::CODED_HEIGHT */
    union
    {
        struct
        {
            uint32_t         broken_link                    : 1;    /* ENTRY_POINT_LAYER::BROKEN_LINK */
            uint32_t         closed_entry                   : 1;    /* ENTRY_POINT_LAYER::CLOSED_ENTRY */
            uint32_t         panscan_flag                   : 1;    /* ENTRY_POINT_LAYER::PANSCAN_FLAG */
            uint32_t         loopfilter                     : 1;    /* ENTRY_POINT_LAYER::LOOPFILTER */
        };
        uint32_t         value;
    } entrypoint_fields;
    uint8_t         conditional_overlap_flag;                     /* ENTRY_POINT_LAYER::CONDOVER */
    uint8_t         fast_uvmc_flag;                               /* ENTRY_POINT_LAYER::FASTUVMC */
    union
    {
        struct
        {
            uint32_t         luma_flag                      : 1;    /* ENTRY_POINT_LAYER::RANGE_MAPY_FLAG */
            uint32_t         luma                           : 3;    /* ENTRY_POINT_LAYER::RANGE_MAPY */
            uint32_t         chroma_flag                    : 1;    /* ENTRY_POINT_LAYER::RANGE_MAPUV_FLAG */
            uint32_t         chroma                         : 3;    /* ENTRY_POINT_LAYER::RANGE_MAPUV */
        };
        uint32_t         range_mapping_enabled;
    } range_mapping_fields;

    uint8_t         UpsamplingFlag;
    uint8_t         ScaleFactor;                                  /* derived from BFRACTION*/
    uint8_t         b_picture_fraction;                           /* PICTURE_LAYER::BFRACTION */
    uint8_t         cbp_table;                                    /* PICTURE_LAYER::CBPTAB/ICBPTAB */
    uint8_t         mb_mode_table;                                /* PICTURE_LAYER::MBMODETAB */
    uint8_t         range_reduction_frame;                        /* PICTURE_LAYER::RANGEREDFRM */
    uint8_t         rounding_control;                             /* PICTURE_LAYER::RNDCTRL */
    uint8_t         post_processing;                              /* PICTURE_LAYER::POSTPROC */
    uint8_t         picture_resolution_index;                     /* PICTURE_LAYER::RESPIC */
    uint16_t        luma_scale;                                   /* PICTURE_LAYER::LUMSCALE */
    uint16_t        luma_shift;                                   /* PICTURE_LAYER::LUMSHIFT */
    union
    {
        struct
        {
            uint32_t         picture_type                   : 3;    /* PICTURE_LAYER::PTYPE */
            uint32_t         frame_coding_mode              : 3;    /* PICTURE_LAYER::FCM */
            uint32_t         top_field_first                : 1;    /* PICTURE_LAYER::TFF */
            uint32_t         is_first_field                 : 1;    /* set to 1 if it is the first field */
            uint32_t         intensity_compensation         : 1;    /* PICTURE_LAYER::INTCOMP */
        };
        uint32_t         value;
    } picture_fields;
    union
    {
        struct
        {
            uint32_t         bitplane_present               : 1;
            uint32_t         mv_type_mb                     : 1;    /* PICTURE::MVTYPEMB */
            uint32_t         direct_mb                      : 1;    /* PICTURE::DIRECTMB */
            uint32_t         skip_mb                        : 1;    /* PICTURE::SKIPMB */
            uint32_t         field_tx                       : 1;    /* PICTURE::FIELDTX */
            uint32_t         forward_mb                     : 1;    /* PICTURE::FORWARDMB */
            uint32_t         ac_pred                        : 1;    /* PICTURE::ACPRED */
            uint32_t         overflags                      : 1;    /* PICTURE::OVERFLAGS */
        };
        uint32_t         value;
    } raw_coding;

    union
    {
        struct
        {
            uint32_t         reference_distance_flag        : 1;    /* PICTURE_LAYER::REFDIST_FLAG */
            uint32_t         reference_distance             : 5;    /* PICTURE_LAYER::REFDIST */
            uint32_t         BwdReferenceDistance           : 5;
            uint32_t         num_reference_pictures         : 1;    /* PICTURE_LAYER::NUMREF */
            uint32_t         reference_field_pic_indicator  : 1;    /* PICTURE_LAYER::REFFIELD */
            uint32_t         reference_picture_flag         : 1;    /* set to 1 if it will be used as a reference picture */
        };
        uint32_t         value;
    } reference_fields;
    union
    {
        struct
        {
            uint32_t         MvMode                         : 4;
            uint32_t         UnifiedMvMode                  : 3;    /* Combination of MVMODE and MVMODE1 */
            uint32_t         mv_table                       : 3;    /* PICTURE_LAYER::MVTAB/IMVTAB */
            uint32_t         two_mv_block_pattern_table     : 2;    /* PICTURE_LAYER::2MVBPTAB */
            uint32_t         four_mv_switch                 : 1;    /* PICTURE_LAYER::4MVSWITCH */
            uint32_t         four_mv_block_pattern_table    : 2;    /* PICTURE_LAYER::4MVBPTAB */
            uint32_t         extended_mv_flag               : 1;    /* ENTRY_POINT_LAYER::EXTENDED_MV */
            uint32_t         extended_mv_range              : 2;    /* PICTURE_LAYER::MVRANGE */
            uint32_t         extended_dmv_flag              : 1;    /* ENTRY_POINT_LAYER::EXTENDED_DMV */
            uint32_t         extended_dmv_range             : 2;    /* PICTURE_LAYER::DMVRANGE */
            uint32_t         four_mv_allowed                : 1;    /* PICTURE_LAYER::4MVSWITCH */
        };
        uint32_t         value;
    } mv_fields;
    union
    {
        struct
        {
            uint32_t         dquant                         : 2;    /* ENTRY_POINT_LAYER::DQUANT */
            uint32_t         quantizer                      : 2;    /* ENTRY_POINT_LAYER::QUANTIZER */
            uint32_t         half_qp                        : 1;    /* PICTURE_LAYER::HALFQP */
            uint32_t         AltPQuantEdgeMask              : 4;    /* Derived from DQUANT, DQUANTTFRM, DQPROFILE, DDQSBEDGE, DQDBEDGE, DQBILEVEL*/
            uint32_t         AltPQuantConfig                : 2;    /* Derived from DQUANT, DQUANTTFRM, DQPROFILE, DDQSBEDGE, DQDBEDGE, DQBILEVEL*/
            uint32_t         pic_quantizer_scale            : 5;    /* PICTURE_LAYER::PQUANT */
            uint32_t         pic_quantizer_type             : 1;    /* PICTURE_LAYER::PQUANTIZER */
            uint32_t         alt_pic_quantizer              : 5;    /* VOPDQUANT::ALTPQUANT */
        };
        uint32_t         value;
    } pic_quantizer_fields;
    union
    {
        struct
        {
            uint32_t         variable_sized_transform_flag  : 1;    /* ENTRY_POINT_LAYER::VSTRANSFORM */
            uint32_t         mb_level_transform_type_flag   : 1;    /* PICTURE_LAYER::TTMBF */
            uint32_t         frame_level_transform_type     : 2;    /* PICTURE_LAYER::TTFRM */
            uint32_t         transform_ac_codingset_idx1    : 2;    /* PICTURE_LAYER::TRANSACFRM */
            uint32_t         transform_ac_codingset_idx2    : 2;    /* PICTURE_LAYER::TRANSACFRM2 */
            uint32_t         intra_transform_dc_table       : 1;    /* PICTURE_LAYER::TRANSDCTAB */
        };
        uint32_t         value;
    } transform_fields;

    uint32_t         StatusReportFeedbackNumber;
} CODEC_VC1_PIC_PARAMS, *PCODEC_VC1_PIC_PARAMS;

typedef struct _CODEC_VC1_SLICE_PARAMS
{
    uint32_t                        slice_data_size;    /* number of bytes in the slice data buffer for this slice */
    uint32_t                        slice_data_offset;  /* the offset to the first byte of slice data */
    uint32_t                        macroblock_offset;  /* the offset to the first bit of MB from the first byte of slice data */
    uint32_t                        slice_vertical_position;
    uint32_t                        b_picture_fraction; /* BFRACTION */
    uint32_t                        number_macroblocks; /* number of macroblocks in the slice */
} CODEC_VC1_SLICE_PARAMS, *PCODEC_VC1_SLICE_PARAMS;

typedef struct _CODEC_VC1_MB_PARAMS
{
    uint16_t                      mb_address;
    uint8_t                       mb_skips_following; /* the number of skipped macroblocks to be generated following the current macroblock */
    uint8_t                       num_coef[CODEC_NUM_BLOCK_PER_MB];    /* the number of coefficients in the residual difference data buffer for each block i of the macroblock */
    uint32_t                      data_offset;        /* data offset in the residual data buffer, byte offset (32-bit multiple index) */
    uint32_t                      data_length;        /* length of the residual data for the macroblock */
    union
    {
        struct
        {
            uint16_t         intra_mb                     : 1;
            uint16_t         motion_forward               : 1;
            uint16_t         motion_backward              : 1;
            uint16_t         motion_4mv                   : 1;
            uint16_t         h261_loopfilter              : 1;
            uint16_t         field_residual               : 1;
            uint16_t         mb_scan_method               : 2;
            uint16_t         motion_type                  : 2;
            uint16_t         host_resid_diff              : 1;
            uint16_t         reserved                     : 1;
            uint16_t         mvert_field_sel_0            : 1;
            uint16_t         mvert_field_sel_1            : 1;
            uint16_t         mvert_field_sel_2            : 1;
            uint16_t         mvert_field_sel_3            : 1;
        };
        uint16_t         value;
    } mb_type;
    union
    {
        struct
        {
            uint16_t         block_coded_pattern          : 6;
            uint16_t         block_luma_intra             : 4;
            uint16_t         block_chroma_intra           : 1;
            uint16_t                                      : 5;
        };
        uint16_t         value;
    } pattern_code;
    union
    {
        struct
        {
            uint16_t         mv_x         : 16;
            uint16_t         mv_y         : 16;
        };
        uint32_t         value;
    } motion_vector[4];
} CODEC_VC1_MB_PARAMS, *PCODEC_VC1_MB_PARAMS;

#endif  // __CODEC_DEF_DECODE_VC1_H__

