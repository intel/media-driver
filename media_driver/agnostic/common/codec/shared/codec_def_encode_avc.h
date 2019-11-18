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
//! \file     codec_def_encode_avc.h
//! \brief    Defines encode AVC types and macros shared by CodecHal, MHW, and DDI layer
//! \details  Applies to AVC encode only. Should not contain any DDI specific code.
//!
#ifndef __CODEC_DEF_ENCODE_AVC_H__
#define __CODEC_DEF_ENCODE_AVC_H__

#include "codec_def_common_avc.h"
#include "codec_def_common_encode.h"
#include "codec_def_common.h"

#define CODEC_AVC_NUM_MAX_DIRTY_RECT        4
#define CODEC_AVC_NUM_QP                    52

#define CODEC_AVC_NUM_WP_FRAME              8
#define CODEC_AVC_MAX_FORWARD_WP_FRAME      6
#define CODEC_AVC_MAX_BACKWARD_WP_FRAME     2
#define CODEC_AVC_WP_OUTPUT_L0_START        0
#define CODEC_AVC_WP_OUTPUT_L1_START        6

#define ENCODE_VDENC_AVC_MAX_ROI_NUMBER_G9            3        // Max 4 regions including non-ROI - used from DDI
#define ENCODE_VDENC_AVC_MAX_ROI_NUMBER_ADV          16        // Max 16 regions including non-ROI - used from DDI
#define ENCODE_VDENC_AVC_MIN_ROI_DELTA_QP_G9         -8        // Min delta QP for VDEnc ROI
#define ENCODE_VDENC_AVC_MAX_ROI_DELTA_QP_G9          7        // Max delta QP for VDEnc ROI

#define ENCODE_DP_AVC_MAX_ROI_NUMBER               4
#define ENCODE_DP_AVC_MAX_ROI_NUM_BRC              8

#define ENCODE_VDENC_AVC_MAX_ROI_NUMBER            3

#define ENCODE_AVC_MAX_SLICES_SUPPORTED      256 // Limted to 256 due to memory constraints.

//AVC
#define CODECHAL_ENCODE_AVC_ROI_WIDTH_SCALE_FACTOR          16
#define CODECHAL_ENCODE_AVC_ROI_FRAME_HEIGHT_SCALE_FACTOR   16
#define CODECHAL_ENCODE_AVC_ROI_FIELD_HEIGHT_SCALE_FACTOR   32
#define CODECHAL_ENCODE_AVC_MAX_ROI_NUMBER                  4

typedef struct _CODECHAL_ENCODE_AVC_ROUNDING_PARAMS
{
    bool       bEnableCustomRoudingIntra;
    bool       bEnableCustomRoudingInter;
    uint32_t   dwRoundingIntra;
    uint32_t   dwRoundingInter;
} CODECHAL_ENCODE_AVC_ROUNDING_PARAMS, *PCODECHAL_ENCODE_AVC_ROUNDING_PARAMS;

// video quality control parameters
typedef struct _CODECHAL_ENCODE_AVC_QUALITY_CTRL_PARAMS
{
    union
    {
        struct
        {
            // Disables skip check for ENC.
            unsigned int skipCheckDisable : 1;
            // Indicates app will override default driver FTQ settings using FTQEnable.
            unsigned int FTQOverride : 1;
            // Enables/disables FTQ.
            unsigned int FTQEnable : 1;
            // Indicates the app will provide the Skip Threshold LUT to use when FTQ is
            // enabled (FTQSkipThresholdLUT), else default driver thresholds will be used.
            unsigned int FTQSkipThresholdLUTInput : 1;
            // Indicates the app will provide the Skip Threshold LUT to use when FTQ is
            // disabled (NonFTQSkipThresholdLUT), else default driver thresholds will be used.
            unsigned int NonFTQSkipThresholdLUTInput : 1;
            // Control to enable the ENC mode decision algorithm to bias to fewer B Direct/Skip types.
            // Applies only to B frames, all other frames will ignore this setting.
            unsigned int directBiasAdjustmentEnable : 1;
            // Enables global motion bias.
            unsigned int globalMotionBiasAdjustmentEnable : 1;
            // MV cost scaling ratio for HME predictors.  It is used when
            // globalMotionBiasAdjustmentEnable == 1, else it is ignored.  Values are:
            //      0: set MV cost to be 0 for HME predictor.
            //      1: scale MV cost to be ? of the default value for HME predictor.
            //      2: scale MV cost to be ? of the default value for HME predictor.
            //      3: scale MV cost to be 1/8 of the default value for HME predictor.
            unsigned int HMEMVCostScalingFactor : 2;
            //disable HME
            unsigned int HMEDisable                         : 1;
            //disable Super HME
            unsigned int SuperHMEDisable                    : 1;
            //disable Ultra HME
            unsigned int UltraHMEDisable                    : 1;
            // Force RepartitionCheck
            unsigned int ForceRepartitionCheck              : 2;

        };
        unsigned int encControls;
    };

    // Maps QP to skip thresholds when FTQ is enabled.  Valid range is 0-255.
    unsigned char FTQSkipThresholdLUT[CODEC_AVC_NUM_QP];
    // Maps QP to skip thresholds when FTQ is disabled.  Valid range is 0-65535.
    unsigned short NonFTQSkipThresholdLUT[CODEC_AVC_NUM_QP];
    // Reserved for future use.
    unsigned int reserved[8];
} CODECHAL_ENCODE_AVC_QUALITY_CTRL_PARAMS, *PCODECHAL_ENCODE_AVC_QUALITY_CTRL_PARAMS;

// AVC VUI Parameters
typedef struct _CODECHAL_ENCODE_AVC_VUI_PARAMS
{
    uint32_t          aspect_ratio_info_present_flag          : 1;
    uint32_t          overscan_info_present_flag              : 1;
    uint32_t          overscan_appropriate_flag               : 1;
    uint32_t          video_signal_type_present_flag          : 1;
    uint32_t          video_full_range_flag                   : 1;
    uint32_t          colour_description_present_flag         : 1;
    uint32_t          chroma_loc_info_present_flag            : 1;
    uint32_t          timing_info_present_flag                : 1;
    uint32_t          fixed_frame_rate_flag                   : 1;
    uint32_t          nal_hrd_parameters_present_flag         : 1;
    uint32_t          vcl_hrd_parameters_present_flag         : 1;
    uint32_t          low_delay_hrd_flag                      : 1;
    uint32_t          pic_struct_present_flag                 : 1;
    uint32_t          bitstream_restriction_flag              : 1;
    uint32_t          motion_vectors_over_pic_boundaries_flag : 1;
    uint32_t                                                    : 17;
    uint16_t          sar_width;
    uint16_t          sar_height;
    uint8_t           aspect_ratio_idc;
    uint8_t           video_format;
    uint8_t           colour_primaries;
    uint8_t           transfer_characteristics;
    uint8_t           matrix_coefficients;
    uint8_t           chroma_sample_loc_type_top_field;
    uint8_t           chroma_sample_loc_type_bottom_field;
    uint8_t           max_bytes_per_pic_denom;
    uint8_t           max_bits_per_mb_denom;
    uint8_t           log2_max_mv_length_horizontal;
    uint8_t           log2_max_mv_length_vertical;
    uint8_t           num_reorder_frames;
    uint32_t          num_units_in_tick;
    uint32_t          time_scale;
    uint8_t           max_dec_frame_buffering;

    //HRD parameters
    uint8_t           cpb_cnt_minus1;
    uint8_t           bit_rate_scale;
    uint8_t           cpb_size_scale;
    uint32_t          bit_rate_value_minus1[32];
    uint32_t          cpb_size_value_minus1[32];
    uint32_t          cbr_flag; // bit 0 represent SchedSelIdx 0 and so on
    uint8_t           initial_cpb_removal_delay_length_minus1;
    uint8_t           cpb_removal_delay_length_minus1;
    uint8_t           dpb_output_delay_length_minus1;
    uint8_t           time_offset_length;
} CODECHAL_ENCODE_AVC_VUI_PARAMS, *PCODECHAL_ENCODE_AVC_VUI_PARAMS;

typedef enum
{
    CODECHAL_ENCODE_AVC_NAL_UT_RESERVED  = 0x00, // Unspecified
    CODECHAL_ENCODE_AVC_NAL_UT_SLICE     = 0x01, // Coded Slice - slice_layer_no_partioning_rbsp
    CODECHAL_ENCODE_AVC_NAL_UT_DPA       = 0x02, // Coded Data partition A - dpa_layer_rbsp
    CODECHAL_ENCODE_AVC_NAL_UT_DPB       = 0x03, // Coded Data partition B - dpa_layer_rbsp
    CODECHAL_ENCODE_AVC_NAL_UT_DPC       = 0x04, // Coded Data partition C - dpa_layer_rbsp
    CODECHAL_ENCODE_AVC_NAL_UT_IDR_SLICE = 0x05, // Coded Slice of a IDR Picture - slice_layer_no_partioning_rbsp
    CODECHAL_ENCODE_AVC_NAL_UT_SEI       = 0x06, // Supplemental Enhancement Information - sei_rbsp
    CODECHAL_ENCODE_AVC_NAL_UT_SPS       = 0x07, // Sequence Parameter Set - seq_parameter_set_rbsp
    CODECHAL_ENCODE_AVC_NAL_UT_PPS       = 0x08, // Picture Parameter Set - pic_parameter_set_rbsp
    CODECHAL_ENCODE_AVC_NAL_UT_AUD       = 0x09, // Access Unit Delimiter - access_unit_delimiter_rbsp
    CODECHAL_ENCODE_AVC_NAL_UT_EOSEQ     = 0x0a, // End of sequence - end_of_seq_rbsp
    CODECHAL_ENCODE_AVC_NAL_UT_EOSTREAM  = 0x0b, // End of stream - end_of_stream_rbsp
    CODECHAL_ENCODE_AVC_NAL_UT_FILL      = 0x0c, // Filler data - filler_data_rbsp
    CODECHAL_ENCODE_AVC_NAL_UT_SEQEXT    = 0x0d, // Sequence parameter set extension - seq_parameter_set_extension_rbsp
    CODECHAL_ENCODE_AVC_NAL_UT_PREFIX    = 0x0e, // Prefix NAL unit in scalable extension - prefix_nal_unit_rbsp
    CODECHAL_ENCODE_AVC_NAL_UT_SUBSEQ    = 0x0f, // Subset sequence parameter set - subset_seq_parameter_set_rbsp
    /* 0x10..0x12 - Reserved */
    CODECHAL_ENCODE_AVC_NAL_UT_LAYERNOPART = 0x13, // Coded slice of an auxiliary coded picture without paritioning - slice_layer_without_partitioning_rbsp
    CODECHAL_ENCODE_AVC_NAL_UT_LAYERSCALEEXT = 0x14, // Coded slice in scalable extension - slice_layer_in_scalable_extension_rbsp
    /* 0x15..0x17 - Reserved */
    /* 0x18..0x1f - Unspcified */

    //this should be the last element of this enum
    //chagne this value if NAL unit type increased
    CODECHAL_ENCODE_AVC_MAX_NAL_TYPE = 0x1f,
} CODECHAL_ENCODE_AVC_NAL_UNIT_TYPE;

enum
{
    SLICE_P = 0,
    SLICE_B = 1,
    SLICE_I = 2,
    SLICE_SP = 3,
    SLICE_SI = 4
};

const uint8_t Slice_Type[10] = { SLICE_P, SLICE_B, SLICE_I, SLICE_SP, SLICE_SI, SLICE_P, SLICE_B, SLICE_I, SLICE_SP, SLICE_SI };

typedef struct _CODEC_ROI_MAP
{
    char                PriorityLevelOrDQp; // [-3..3] or [-51..51]
    uint8_t             NumRect;
    CODEC_ROI           Rect[16];   // disconnected areas which have same PriorityLevelOrDQp
} CODEC_ROI_MAP, *PCODEC_ROI_MAP;

typedef struct _CODEC_ENCODE_MB_CONTROL
{
    union
    {
        struct
        {
            uint32_t        bForceIntra : 1;
            uint32_t        Reserved : 31;
        };
        uint32_t            value;
    } MBParams;
} CODEC_ENCODE_MB_CONTROL, *PCODEC_ENCODE_MB_CONTROL;

typedef struct _CODEC_PIC_REORDER
{
    uint32_t           PicNum;
    uint32_t           POC;
    uint8_t            ReorderPicNumIDC;
    uint8_t            DiffPicNumMinus1;
    CODEC_PICTURE      Picture;
} CODEC_PIC_REORDER, *PCODEC_PIC_REORDER;

/*! \brief Provides the sequence-level parameters of a compressed picture for AVC encoding.
*
*   The framework is expected to only send a sequence parameter compressed buffer for the first picture(first Execute() call) in a sequence, associated with a RAP(IDR, CRA, BLA) picture.
*/
typedef struct _CODEC_AVC_ENCODE_SEQUENCE_PARAMS
{
    uint16_t          FrameWidth;         //!< Width of the frame in pixels.
    uint16_t          FrameHeight;        //!< Height of the frame in pixels.
    uint8_t           Profile;            //!< Same as AVC syntax parameter profile_idc.
    uint8_t           Level;              //!< Same as AVC syntax parameter level_idc.

    uint16_t          GopPicSize;         //!< Distance between IRAP pictures.
    /*! \brief Distance between anchor frames (I or P).
    *
    *    Here, P may also means low delay B (GPB) frames.
    *    Programming Note: GopPicSize > GopRefDist should be ensured by application. It is required by BRC.
    */
    uint16_t          GopRefDist;
    uint16_t          GopOptFlag  : 2;
    uint16_t                      : 6;

    uint8_t           TargetUsage;
    /*! \brief Specifies rate control method.
    *
    *    \n 0: CQP, if set, internal BRC, multi-pass and panic mode will be disabled
    *    \n 1: CBR
    *    \n 2: VBR
    *    \n 3: CQP, see above
    *    \n 4: AVBR, if set, HRD compliance is not guaranteed. Buffering period SEI and picture timing SEI messages are not necessary for AVBR.
    *    \n 5: Reserved
    *    \n 6: Reserved
    *    \n 7: Reserved
    *    \n 8: LA, look ahead
    *    \n 9: ICQ (Intelligent Constant Quality)
    *    \n 10: VCM, defined for video converencing
    *    \n 11: LA_ICQ
    *    \n 12: LA_EXT, defined for server transcoding usage, 1 input video sequence generates several sequences with different bitrate
    *    \n 13: LA_HRD, defined for server and client usage, lookahead with HRD and strict handling of maximum bitrate window
    *    \n 14: QVBR (Quality Regulated VBR)
    *    \n Programming note: Define the minimum value as indicated above for AVBR accuracy & convergence, clamp any value that is less than the minimum value to the minimum value.  Define the maximum value for AVBR accuracy as 100 (10%) and for AVBR convergence as 500, clamp any value that is greater than the maximum value to the maximum value. The maximum & minimum value may be adjusted when necessary. If bResetBRC is set to 1 for a non-I picture, driver shall not insert SPS into bitstream.  Driver needs to calculate the maximum allowed frame size per profile/level for all RateControlMethod except CQP, and use the calculated value to program kernel for non AVBR modes; for AVBR mode, driver needs to clamp the upper bound of UserMaxFrameSize to the calculated value and use the clamped UserMaxFrameSize to program kernel.  If IWD_VBR is set, driver programs it the same as VBR except not to enable panic mode.
    */
    uint8_t           RateControlMethod;
    uint32_t          TargetBitRate;      //!< Target bit rate Kbit per second
    uint32_t          MaxBitRate;         //!< Maximum bit rate Kbit per second
    /*! \brief Minimun bit rate Kbit per second.
    *
    *   This is used in VBR control. For CBR control, this field is ignored.
    */
    uint32_t          MinBitRate;
    uint16_t          FramesPer100Sec;                        //!< Total frames per 100 second (frame rate fps * 100).
    uint32_t          InitVBVBufferFullnessInBit;             //!< Initial VBV buffer fullness in bits.
    /*! \brief VBV buffer size in bit unit.
    *
    *   The AVC spec defines a max coded picture buffer size for each level.
    */
    uint32_t          VBVBufferSizeInBit;
    /*! \brief Specifies number of reference frames.
    *
    *    Should not be greater than driver reported max number of references.
    */
    uint8_t           NumRefFrames;

    /*! \brief Same as AVC syntax element.
    *
    *   Should not be greater than max SPS set reported by driver.
    */
    uint8_t           seq_parameter_set_id;
    uint8_t           chroma_format_idc;                      //!< Same as AVC syntax element.
    uint8_t           bit_depth_luma_minus8;                  //!< Same as AVC syntax element.
    uint8_t           bit_depth_chroma_minus8;                //!< Same as AVC syntax element.
    uint8_t           log2_max_frame_num_minus4;              //!< Same as AVC syntax element.
    uint8_t           pic_order_cnt_type;                     //!< Same as AVC syntax element.
    uint8_t           log2_max_pic_order_cnt_lsb_minus4;      //!< Same as AVC syntax element.
    uint8_t           num_ref_frames_in_pic_order_cnt_cycle;  //!< Same as AVC syntax element.
    int32_t           offset_for_non_ref_pic;                 //!< Same as AVC syntax element.
    int32_t           offset_for_top_to_bottom_field;         //!< Same as AVC syntax element.
    int32_t           offset_for_ref_frame[256];              //!< Same as AVC syntax element.
    uint16_t          frame_crop_left_offset;                 //!< Same as AVC syntax element.
    uint16_t          frame_crop_right_offset;                //!< Same as AVC syntax element.
    uint16_t          frame_crop_top_offset;                  //!< Same as AVC syntax element.
    uint16_t          frame_crop_bottom_offset;               //!< Same as AVC syntax element.

    uint16_t          seq_scaling_list_present_flag[12];          //!< Same as AVC syntax element.
    uint16_t          seq_scaling_matrix_present_flag     : 1;    //!< Same as AVC syntax element.
    uint16_t          delta_pic_order_always_zero_flag    : 1;    //!< Same as AVC syntax element.
    uint16_t          frame_mbs_only_flag                 : 1;    //!< Same as AVC syntax element.
    uint16_t          direct_8x8_inference_flag           : 1;    //!< Same as AVC syntax element.
    uint16_t          vui_parameters_present_flag         : 1;    //!< Same as AVC syntax element.
    uint16_t          frame_cropping_flag                 : 1;    //!< Same as AVC syntax element.
    /*! \brief Specifies that encoded slices returned fit within the slice size specified in the picture parameter set for AVC.
    *
    *    When enabled, this overrides the slice structures specified by the application using slice level parameters.
    */
    uint16_t          EnableSliceLevelRateCtrl            : 1;
    uint16_t                                              : 8;
    union
    {
        struct
        {
            uint32_t           bInitBRC                     : 1;
            /*! \brief Indicate if a BRC reset is desired to set a new bit rate or frame rate.
            *
            *   This setting is only valid if RateControlMethod is AVBR, VBR, CBR, VCM, ICQ, CQL or QVBR and the current picture is an I picture. If the frame resolution is changed, it should be set with IDR picture. It should not be set when RateControlMethod is CBR or CQP. The following table indicates which BRC parameters can be changed via a BRC reset.
            *
            *  \n BRC Parameters       Changes allowed via reset
            *  \n Profile & Level               Yes
            *  \n UserMaxFrameSize              Yes
            *  \n InitVBVBufferFullnessInBit    No
            *  \n TargetBitRate                 Yes
            *  \n VBVBufferSizeInBit            No
            *  \n MaxBitRate                    Yes
            *  \n FramesPer100Sec *             No
            *  \n RateControlMethod             No
            *  \n GopPicSize                    No
            *  \n GopRefDist                    No
            *  \n GopOptFlag                    Yes
            *  \n FrameWidth                    No
            *  \n FrameHeight                   No
            *  \n Note: when resolution (FrameWidth and/or FrameHeight) changes, framework should re-start a new bit stream and not using BRC reset.
            */
            uint32_t           bResetBRC                    : 1;
            /*! \brief Indicates that current SPS is just a BRC parameter update, not a SPS change to be inserted into the bitstream.
            *
            *    When set to 1, current SPS will not be packed and inserted into bitstream by the driver.
            */
            uint32_t           bNoAcceleratorSPSInsertion   : 1;
            /*! \brief Indicates the global search options.
            *
            *    It is only valid if EnhancedEncInput is reported during capability checking:
            *        \n - 0: Default - option internally selected based on target usage
            *        \n - 1: Long - SHME enabled
            *        \n - 2: Medium - HME only enabled, SHME disabled
            *        \n - 3: Short - SHME/HME disabled
            */
            uint32_t           GlobalSearch                 : 2;
            /*! \brief Indicates the local search options.
            *
            *    It is only valid if EnhancedEncInput is reported during capability checking:
            *        \n - 0: Default - option internally selected based on target usage
            *        \n - 1: Tiny – MaxLenSP = 4, Reference Window = 24x24 SP = Spiral
            *        \n - 2: Small – MaxLenSP = 9, Reference Window = 28x28 SP = Spiral
            *        \n - 3: Square – MaxLenSP = 16, Reference Window = 32x32 SP = Spiral
            *        \n - 4: Diamond – MaxLenSP = 16, Reference Window = 48x40 SP = Diamond
            *        \n - 5: Large Diamond – MaxLenSP = 30, Reference Window = 48x40 SP = Diamond
            *        \n - 6: Exhaustive – MaxLenSP = 57, Reference Window = 48x40 SP = Spiral
            *        \n - 7: Heavy Horizontal – MaxLenSP = 57, Reference Window = 64x32 SP = Spiral
            *        \n - 8: Heavy Vertical – MaxLenSP = 57, Reference Window = 32x64 SP = Spiral
            */
            uint32_t           LocalSearch                  : 4;
            /*! \brief Indicates early skip control.
            *
            *    It is only valid if EnhancedEncInput is reported during capability checking:
            *        \n - 0: Default, decided internally
            *        \n - 1: EarlySkip enabled
            *        \n - 2: EarlySkip disabled
            */
            uint32_t           EarlySkip                    : 2;
            uint32_t           Reserved0                    : 1;
            /*! \brief Indicates that MB BRC is enabled.
            *
            *    It is only valid if MBBRCSupport is reported during capability checking:
            *        \n - 0: Default, decided internally based on target usage.
            *        \n - 1: MB BRC enabled.
            *        \n - 2: MB BRC disabled.
            *        \n - Other values are Reserved.
            *    \n Currently MB BRC can be applied to all bit rate control methods except CQP.
            */
            uint32_t           MBBRC                        : 4;
            /*! \brief Indicates trellis control.
            *
            *    The Trellis_I, Trellis_P and Trellis_B settings may be combined using bitwise OR like "Trellis_I | Trellis_P" to enable Trellis for I & P.  If Trellis_Disabled is set with any combination, Trellis will be disabled.
            *        \n - 0: Trellis_Default – Trellis decided internally.
            *        \n - 1: Trellis_Disabled – Trellis disabled for all frames/fields.
            *        \n - 2: Trellis_I – Trellis enabled for I frames/fields.
            *        \n - 4: Trellis_P – Trellis enabled for P frames/fields.
            *        \n - 8: Trellis_B – Trellis enabled for B frames/fields.
            */
            uint32_t           Trellis                      : 4;
            /*! \brief Indicates current sequence is encoded for Temporal Scalability.
            *    The driver may or may not use this flag.  For example, for VME+PAK AVC encoder MSDK handles all header insertion to indicate a temporal id in the SVC ext slice header and this flag is not used.  However, for VDEnc AVC encoder in certain cases BRC is required to know if the current frames are being encoded for temporal scalability and therefore will have extra SVC ext added in the slice header.
            *        \n - 0: Default, current sequence is not encoded for Temporal Scalability.
            *        \n - 1: current sequence is encoded is encoded for Temporal Scalability.
            */
            uint32_t           bTemporalScalability         : 1;
            /*! \brief Indicates ROI[] value is in delta QP or priority.
            *
            *    It is valid only when parameter NumROI is greater than 0 and either ROIDeltaQPSupport or ROIBRCPriorityLevelSupport equals to 1.
            *        \n - 0: Default, ROI[] value is in priority.
            *        \n - 1: ROI[] value is in delta QP.
            *    \n Currently only ROIValueInDeltaQP equal 1 is validated for CQP
            */
            uint32_t           ROIValueInDeltaQP            : 1;
            /*! \brief Indicates larger P/B frame size than UserMaxPBFrameSize may be used.
            *
            *    if enabled, BRC may decide a larger P or B frame size than what UserMaxPBFrameSize dictates when the scene change is detected.
            *        \n - 0: Default, normal BRC.
            *        \n - 1: BRC may decide larger P/B frame size.
            */
            uint32_t           bAutoMaxPBFrameSizeForSceneChange : 1;
            /* Control the force panic mode through DDI other than user feature key */
            uint32_t           bForcePanicModeControl       : 1;
            uint32_t           bPanicModeDisable            : 1;

            /*! \brief Enables streaming buffer in LLC
            *
            *        \n - 0 : streaming buffer by LLC is disabled.
            *        \n - 1 : streaming buffer by LLC is enabled.
            */
            uint32_t           EnableStreamingBufferLLC     : 1;
            /*! \brief Enables streaming buffer in DDR
            *
            *        \n - 0 : streaming buffer by DDR is disabled.
            *        \n - 1 : streaming buffer by DDR is enabled.
            */
            uint32_t           EnableStreamingBufferDDR     : 1;
            uint32_t           Reserved1                    : 5;
        };
        uint32_t            sFlags;
    };
    /*! \brief Framework defined maximum frame size in bytes for I frames.
    *
    *    Applicable for all RateControlMethod values except CQP; guarantees that the compressed frame size will be less than this value. If UserMaxPBFrameSize equals 0, UserMaxIFrameSize will be used for all frame types. Maximum allowed frame size per profile/level will be calculated in driver and be used when UserMaxIFrameSize and UserMaxPBFrameSize are both set to 0.
    */
    uint32_t            UserMaxFrameSize;
    /*! \brief Framework defined maximum frame size in bytes for P & B frames.
    *
    *    Applicable for all RateControlMethod values except CQP; guarantees that the compressed frame size will be less than this value. If UserMaxPBFrameSize equals 0, UserMaxIFrameSize will be used for all frame types. Maximum allowed frame size per profile/level will be calculated in driver and be used when UserMaxIFrameSize and UserMaxPBFrameSize are both set to 0.
    */
    uint32_t            UserMaxPBFrameSize;
    /*! \brief Indicates the measure of quality for ICQ and QVBR
    *
    *    The range is from 1 – 51, with 1 being the best quality.
    */
    uint16_t           ICQQualityFactor;
    /*! \brief Indicates the bitrate accuracy for AVBR
    *
    *    The range is [1, 100], 1 means one percent, and so on.
    */
    uint32_t           AVBRAccuracy;
    /*! \brief Indicates the bitrate convergence period for AVBR
    *
    *    The unit is frame.
    */
    uint32_t          AVBRConvergence;

    /*! \brief Indicates the uncompressed input color space
    *
    *   Valid only when input is ARGB format.
    */
    ENCODE_INPUT_COLORSPACE     InputColorSpace;
    /*! \brief Provides a hint to encoder about the scenario for the encoding session.
    *
    *   BRC algorithm may tune differently based on this info.
    */
    ENCODE_SCENARIO             ScenarioInfo;
    ENCODE_CONTENT              ContentInfo;                    //!< Provides a hint to encoder about the content for the encoding session.

    /*! \brief Indicates the tolerance the application has to variations in the frame size.
    *
    *   It affects the BRC algorithm used, but may or may not have an effect based on the combination of other BRC parameters.  Only valid when the driver reports support for FrameSizeToleranceSupport.
    */
    ENCODE_FRAMESIZE_TOLERANCE  FrameSizeTolerance;

    /*! \brief Indicates BRC Sliding window size in terms of number of frames.
    *
    *   Defined for CBR and VBR. For other BRC modes or CQP, values are ignored. 
    */
    uint16_t  SlidingWindowSize;

    /*! \brief Indicates maximun bit rate Kbit per second within the sliding window during. 
    *
    *  Defined for CBR and VBR. For other BRC modes or CQP, values are ignored. 
    */
    uint32_t  MaxBitRatePerSlidingWindow;

    /*! \brief Indicates minimun bit rate Kbit per second within the sliding window during. 
    *
    *  Defined for CBR and VBR. For other BRC modes or CQP, values are ignored. 
    */
    uint32_t  MinBitRatePerSlidingWindow;
    
    /*! \brief Indicates number of frames to lookahead.
    *
    *    Range is [0~127]. Default is 0 which means lookahead disabled. Valid only when LookaheadBRCSupport is 1. When not 0, application should send LOOKAHEADDATA buffer to driver.
    */
    uint8_t   LookaheadDepth;

    uint8_t            constraint_set0_flag               : 1;    //!< Same as AVC syntax element.
    uint8_t            constraint_set1_flag               : 1;    //!< Same as AVC syntax element.
    uint8_t            constraint_set2_flag               : 1;    //!< Same as AVC syntax element.
    uint8_t            constraint_set3_flag               : 1;    //!< Same as AVC syntax element.
    uint8_t                                               : 4;
    uint8_t           separate_colour_plane_flag;                 //!< Same as AVC syntax element.
    bool              qpprime_y_zero_transform_bypass_flag;       //!< Same as AVC syntax element.
    bool              gaps_in_frame_num_value_allowed_flag;       //!< Same as AVC syntax element.
    uint16_t          pic_width_in_mbs_minus1;                    //!< Same as AVC syntax element.
    uint16_t          pic_height_in_map_units_minus1;             //!< Same as AVC syntax element.
    bool              mb_adaptive_frame_field_flag;               //!< Same as AVC syntax element.
} CODEC_AVC_ENCODE_SEQUENCE_PARAMS, *PCODEC_AVC_ENCODE_SEQUENCE_PARAMS;

typedef struct _CODEC_AVC_ENCODE_USER_FLAGS
{
    union
    {
        struct
        {
            /*! \brief Indicates that raw pictures should be used as references instead of recon pictures.
            *
            *   Setting to 1 may improve performance at the cost of image quality.  The accelerator may or may not support toggling this value on a per frame basis.
            */
            uint32_t    bUseRawPicForRef                        : 1;
            /*! \brief Indicates whether or not the driver will pack non-slice headers.
            *
            *   Applies to ENC + PAK mode only. This flag is only valid only when AcceleratorHeaderPacking = 1, and driver does the header packing.
            *        \n - 0: Accelerator will pack AU delimiter, SPS (including VUI if present), PPS, SEI messages if present, end of sequence if indicated, and end of stream if indicated, along with coded slice to form a complete bitstream.
            *        \n - 1: Accelerator will just pack coded slice (slice header + data), like in PAK only mode, and the application will pack the rest of the headers.
            */
            uint32_t    bDisableAcceleratorHeaderPacking        : 1;
            uint32_t                                            : 5;
            uint32_t    bDisableSubMBPartition                  : 1;    //!< Indicates that sub MB partitioning should be disabled.
            /*! \brief Inidicates whether or not emulation byte are inserted.
            *
            *   If 1, accelerator will perform start code prefix (0x 00 00 01/02/03/00) search and emulation byte (0x 03) insertion on packed header data. This doesn’t apply to packed slice header data. Packed slice header data must not have emulation byte inserted, accelerator will always perform start code prefix search and emulation byte (0x 03) insertion on packed slice header data.
            *   Note: If cabac_zero_word insertion compliance is required, this value should be set to 0.  This means the application must perform emulation prevention byte insertion in the frame header.  This is due to the restriction in MFX_PAK_INSERT_OBJECT HeaderLengthExcludeFrmSize cannot allow EmulationFlag to be true.
            */
            uint32_t    bEmulationByteInsertion                 : 1;
            /*! \brief Specifies the type of intra refresh used.
            *
            *    Effective only when RollingINtraRefresh capability in use. Applies to P pictures only (not valid with IBP). When used field encoding, B frames, and multiple references are not allowed.
            *        \n - 0 : disabled
            *        \n - 1 : enabled in colum
            *        \n - 2 : enabled in row
            *        \n - 3 : enabled in region
            */
            uint32_t    bEnableRollingIntraRefresh              : 2;

            /*! \brief Specifies if Slice Level Reporitng may be requested for this frame
            *
            *    If this flag is set, then slice level parameter reporting will be set up for this frame.  Only valid if SliceLevelReportSupport is reported in ENCODE_CAPS, else this flag is ignored.  
            *
            */
            uint32_t    bEnableSliceLevelReport                 : 1;

            /*! \brief Specifies if integer mode searching is performed
            *
            *    when set to 1, integer mode searching is performed
            *
            */
            uint32_t    bDisableSubpixel                        : 1;

            /*! \brief Specifies if the overlapped operation of intra refresh is disabled
            *
            *    It is valid only when bEnableRollingIntraRefresh is on.
            *    \n - 0 : default, overlapped Intra refresh is applied
            *    \n - 1 : intra refresh without overlap operation
            *
            */
            uint32_t    bDisableRollingIntraRefreshOverlap      : 1;

            /*! \brief Specifies whether extra partition decision refinement is done after the initial partition decision candidate is determined.  
            *
            *    It has performance tradeoff for better quality.  
            *    \n - 0 : DEFAULT - Follow driver default settings.
            *    \n - 1 : FORCE_ENABLE - Enable this feature totally for all cases.
            *    \n - 2 : FORCE_DISABLE - Disable this feature totally for all cases.
            */
            uint32_t    ForceRepartitionCheck                   : 2;
            uint32_t    bReserved                               : 16;
        };
        uint32_t        Value;
    };
} CODEC_AVC_ENCODE_USER_FLAGS, *PCODEC_AVC_ENCODE_USER_FLAGS;

/*! \brief Provides the picture-level parameters of a compressed picture for AVC encoding.
*/
typedef struct _CODEC_AVC_ENCODE_PIC_PARAMS
{
    /*! \brief Specifies the uncompressed source surface of the frame for the current picture to be encode.
    *
    *    The PicFlags regarding reference usage are expected to be valid at this time.
    */
    CODEC_PICTURE   CurrOriginalPic;
    /*! \brief Specifies the uncompressed surface of the reconstructed frame for the current encoded picture.
    *
    *    The PicFlags regarding reference usage are expected to be valid at this time.
    *    The recon surface may be of different format and different bit depth from that of source.
    *    The framework needs to specify it through chroma_format_idc and bit_depth_luma_minus8 and
    *    bit_depth_chroma_minus8 in SPS data structure.
    */
    CODEC_PICTURE   CurrReconstructedPic;
    /*! \brief Specifies picture coding type.
    *
    *    \n 1: I picture
    *    \n 2: P picture
    *    \n 3: B picture
    */
    uint8_t         CodingType;
    /*! \brief Specifies that field mode coding is in use.
    *
    *    Top or bottom field indicated by CurrOriginalPic.PicFlags.
    */
    uint8_t         FieldCodingFlag         : 1;
    /*! \brief Specifies that MBAFF coding mode is in  use.
    *
    *    It shall not be set if NoFieldFrame flag is reported in CodingLimit during capability checking.
    */
    uint8_t         FieldFrameCodingFlag    : 1;
    uint8_t                                 : 6;
    /*! \brief Specifies the number of slices per frame or per field in field coding.
    *
    *    Note the restriction on slice based on the SliceStructure reported during capability checking.
    */
    uint32_t        NumSlice;

    /*! \brief Quantization parameter for Y.
    *
    *    Valid range is 0 - 51. If QpY is set to -1, driver will use an internal default value when CQP is not set, otherwise, driver will return error. Please note that, QpY is a frame level QP. QP for each slice is determined by QpY + slice_qp_delta. And QpY + slice_qp_delta should be also in the range of 0 – 51, inclusive.
    */
    char            QpY;
    /*! \brief Each entry of the list specifies the frame index of the reference pictures.
    *
    *   The value of FrameIdx specifies the index of RefFrameList structure. And valid value range is [0..14, 0x7F]. Invalid entries are indicated by setting PicFlags to PICTURE_INVALID.
    *   RefFrameList[] should include all the reference pictures in DPB, which means either the picture is referred by current picture or future pictures, it should have a valid entry in it.
    */
    CODEC_PICTURE   RefFrameList[CODEC_AVC_MAX_NUM_REF_FRAME];
    /*! \brief Denotes "used for reference" frames as defined in the AVC specification.
    *
    *   The flag is accessed by:
    *        \n - FlagTop(i) = (UsedForReferenceFlags >> (2 * i)) & 1
    *        \n - FlagBottom(i) = (UsedForReferenceFlags >> (2 * i + 1)) & 1
    *   \n If FlagTop(i) is 1, the top field or frame numger i is marked as "used for reference"; if FlagBottom(i) is 1 then then bottom field of frame i is marked as "used for reference". If either is 0 then the frame is not marked as "used for reference".
    */
    uint32_t        UsedForReferenceFlags;
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
    int32_t         CurrFieldOrderCnt[2];
    /*! \brief Contains the POCs for the reference frames in RefFrameList.
    *
    *   For each entry FieldOrderCntList[i][j]:
    *        \n - i: the picture index
    *        \n - j: 0 specifies the top field order count and 1 specifies the bottom field order count
    *   \n If a entry i in RefFrameList is not relevant (it is not used for reference) or valid, the entry FieldOrderCount[i][0 and 1] should be 0.
    */
    int32_t         FieldOrderCntList[16][2];
    uint16_t        frame_num;                      //!< Same as AVC syntax element.
    bool            bLastPicInSeq;                  //!< Indicate whether to insert sequence closing NAL unit.
    bool            bLastPicInStream;               //!< Indicate whether to insert stream closing NAL unit.

    CODEC_AVC_ENCODE_USER_FLAGS     UserFlags;

    /*! \brief Arbitrary number set by the host decoder to use as a tag in the status report feedback data.
    *
    *   The value should not equal 0, and should be different in each call to Execute.
    */
    uint32_t        StatusReportFeedbackNumber;

    uint8_t         bIdrPic;                        //!< Indicates that the current picture is IDR.
    uint8_t         pic_parameter_set_id;           //!< Same as AVC syntax element.
    uint8_t         seq_parameter_set_id;           //!< Same as AVC syntax element.
    uint8_t         num_ref_idx_l0_active_minus1;   //!< Same as AVC syntax element.
    uint8_t         num_ref_idx_l1_active_minus1;   //!< Same as AVC syntax element.
    char            chroma_qp_index_offset;         //!< Same as AVC syntax element.
    char            second_chroma_qp_index_offset;  //!< Same as AVC syntax element.

    uint16_t        pic_scaling_list_present_flag[12];      //!< Same as AVC syntax element.
    uint16_t        entropy_coding_mode_flag        : 1;    //!< Same as AVC syntax element.
    uint16_t        pic_order_present_flag          : 1;    //!< Same as AVC syntax element.
    uint16_t        weighted_pred_flag              : 1;    //!< Same as AVC syntax element.
    uint16_t        weighted_bipred_idc             : 2;    //!< Same as AVC syntax element.
    uint16_t        constrained_intra_pred_flag     : 1;    //!< Same as AVC syntax element.
    uint16_t        transform_8x8_mode_flag         : 1;    //!< Same as AVC syntax element.
    uint16_t        pic_scaling_matrix_present_flag : 1;    //!< Same as AVC syntax element.
    uint16_t        RefPicFlag                      : 1;    //!< Indicates that the current picture (raw or recon) may be used as a reference for encoding other pictures.
    /*! \brief Indicates how precise the framework would like BRC to be to reach the ideal requested framesize.
    *
    *    The driver will internally make programming decisions based on this parameter, it will be a tradeoff between performance and precision.  This flag is ignored if BRC is not enabled (CQP mode).
    *        \n - 0: default precision (normal)
    *        \n - 1: lowest precision
    *        \n - 2: normal precision
    *        \n - 3: highest precision
    */
    uint16_t        BRCPrecision                    : 2;
    /*! \brief Indicates that the allocated source buffer format is a swizzle format from display.
    *
    *    Framework still allocates the buffer as a standard FOURCC format. The swizzled format will be reported as supported for the encoder configuration during capability reporting.
    *    \n Source/Raw Allocated Buffer Format, DisplayFormatSwizzle, Layout Format in Buffer
    *        \n - YUY2, 0, YUY2
    *        \n - YUY2, 1, 2YUY (Y0U0Y1V0)
    *        \n - AYUV, 0, AYUV
    *        \n - AYUV, 1, YUXV
    */
    uint16_t        bDisplayFormatSwizzle           : 1;
    uint16_t                                        : 3;

    uint8_t         num_slice_groups_minus1;                //!< Same as AVC syntax element.
    char            pic_init_qp_minus26;                    //!< Same as AVC syntax element.
    char            pic_init_qs_minus26;                    //!< Same as AVC syntax element.
    bool            deblocking_filter_control_present_flag; //!< Same as AVC syntax element.
    bool            redundant_pic_cnt_present_flag;         //!< Same as AVC syntax element.

    // Parameters for RollingI feature
    bool            bDisableRollingIntraRefreshOverlap;
    uint8_t         EnableRollingIntraRefresh;
    uint8_t         IntraRefreshMBNum;
    uint8_t         IntraRefreshMBx;
    uint8_t         IntraRefreshMBy;
    uint8_t         IntraRefreshUnitinMB;
    char            IntraRefreshQPDelta;
    uint32_t        FirstPFrameIntraInsertionSize;

    /*! \brief Indicates the maximum size of each slice in Bytes.
    *
    *    This is valid only when EnableSliceLevelRateCtrl is enabled in the sequence level.
    */
    uint32_t        SliceSizeInBytes;

    /*! \brief Number of Region Of Interest (ROI).
    *
    *    Value range is 0 to 16 inclusively. If no ROI to be set, this value shall be set to 0.
    */
    uint8_t         NumROI;
    uint8_t         NumDirtyROI;                        //!< Number of dirty ROIs [0...4]
    /*! \brief Dictates the value of delta QP for any ROI should be within [MinDeltaQp..MaxDeltaQp]
    *
    *    Applies only to BRC case.
    */
    char            MaxDeltaQp;
    /*! \brief Dictates the value of delta QP for any ROI should be within [MinDeltaQp..MaxDeltaQp]
    *
    *    Applies only to BRC case.
    */
    char            MinDeltaQp;
    /*! \brief Defines ROI settings.
    *
    *    Value entries are ROI[0] up to ROI[NumROI – 1], inclusively, if NumROI > 0. And it can be ignored otherwise.
    */
    CODEC_ROI       ROI[16];
    /*! \brief Distinct delta QP values assigned to the ROI
    *
    *    Value entries are distinct and within [MinDeltaQp..MaxDeltaQp].
    */
    int8_t ROIDistinctDeltaQp[16];
    /*! \brief indicate native ROI / force QP ROI to be used.
    */
    bool        bNativeROI;
    /*! \brief Defines dirty ROI settings.
    *
    *    Value entries are DirtyROI[0] up DirtyROI ROI[NumROI – 1], inclusively, if NumDirtyROI > 0. And it can be ignored otherwise.
    */
    CODEC_ROI       DirtyROI[CODEC_AVC_NUM_MAX_DIRTY_RECT];

    CODEC_ROI_MAP   ROIMapArray[16];

    /*! \brief Specifies skip frames.
    *
    *    0: Encode as normal, no skip.
    *    1: One or more frames were skipped prior to the current frame, encode the current frame as normal.  The driver will pass the NumSkipFrames and SizeSkipFrames to BRC for adjustment.
    */
    uint8_t         SkipFrameFlag;
    /*! \brief The number of frames skipped prior to the current frame.
    *
    *    Valid when SkipFlag = 1.
    */
    uint8_t         NumSkipFrames;
    /*! \brief Differs in meaning based on value of SkipFrameFlag
    *
    *    SkipFrameFlag = 1, the size of the skipped frames in bits.
    *    Note: Currently kernel only supports 16 bits for SizeSkipFrames.
    */
    uint32_t        SizeSkipFrames;

    /*! \brief Specifies the minimum Qp to be used for BRC.
    *
    *    BRCMaxQp and BRCMinQp may be set at a per frame type (I, P, B) granularity.
    *    BRCMaxQp and BRCMinQp should be set to zero if Qp control is not desired.
    *    If non-zero min/max QP is passed for I-frame, it will be used for I, P and B frames.
    *    If non-zero min/max QP is passed for P-frame also, then P and B frame will be updated to this. I-frame remains unchanged.
    *    If non-zero min/max QP is passed for B-frame also, then B-frame will be updated to this. I and P frame remains unchanged.
    *    If new QP values are passed in between the sequence, then it will be updated for that frame-type and any other frame types that are not explicitly set. Eg. if min/max QP for P-frame was passed separately, and an update for I-frame is received, then only I-frame values are updated.  P and B will continue to use old values.  But, if P-frame and B-frame were never explicitly set then updating I-frame will also update P and B.
    *    If the application wants to keep the current non-zero min/max QP settings, then min/max QP should be set to zero, so the driver will not change previously set values and continue to use them.
    *    Min QP is expected to be less than or equal to Max QP.  Driver clamps max QP to [1, 51] and min QP to [1, max QP].  Zero QP is not used.
    *    Only single PAK pass is supported plus the IPCM pass.  Panic mode is disabled.  This is because min/maxQP requirement conflicts with the HRD compliancy requirement, so the HRD compliancy restriction is relaxed.
    */
    uint8_t         ucMinimumQP;
    uint8_t         ucMaximumQP;    //!< Specifies the maximum Qp to be used for BRC.

    uint32_t        dwZMvThreshold; //!< Used for static frame detection.

    /*! \brief Indicates that an HMEOffset will be sent by the application in HMEOffset for each reference.
    *
    *    This offset will be added to the co-located (0, 0) location before HME search is performed.  This is only valid if HMEOffsetSupport is reported as supported as a capability, else this flag is ignored.
    */
    bool            bEnableHMEOffset;
    /*! \brief Specifies the HME offsets.
    *
    *    Curently the supported range is -128 to +127, programmed in 4x downscaled HME precision, not the original size. HMEOffset[i][j] where:
    *        \n - i – RefFrameList Index
    *        \n - j – 0 = x, 1 = y Coordinates
    *    \n So for example, HMEOffset[i] specifies the (x, y) offset corresponding to the ith entry in RefFrameList.
    *    Programming Note: The HME offset must be a multiple of 4x4 to align to the 4x4 HME block, so the driver must align the application supplied value.
    */
    int16_t         HMEOffset[16][2][2];

    /*! \brief Specifies Inter MB partition modes that will be disabled.
    *
    *    SubMbPartMask is only valid when bEnableSubMbPartMask is true. Bit0~6 indicate inter 16x16, 16x8, 8x16, 8x8, 8x4, 4x8, 4x4.
    */
    bool            bEnableSubMbPartMask;
    uint8_t         SubMbPartMask;

    /*! \brief Specifies motion search modes that will be used.
    *
    *    SubPelMode is only valid when bEnableSubPelMode is true. Following are valid values of SubPelMode:
    *    0:Integer mode searching
    *    1:Half-pel mode searching
    *    2:Reserved
    *    3:Quarter-pel mode searching
    */
    bool            bEnableSubPelMode;
    uint8_t         SubPelMode;

    /*! \brief Specifies whether extra partition decision refinement is done after the initial partition decision candidate is determined.
    *
    *    It has performance tradeoff for better quality.
    *    \n - 0 : DEFAULT - Follow driver default settings.
    *    \n - 1 : FORCE_ENABLE - Enable this feature totally for all cases.
    *    \n - 2 : FORCE_DISABLE - Disable this feature totally for all cases.
    */
    uint32_t        ForceRepartitionCheck;

    /*! \brief Specifies force-to-skip for HRD compliance in BRC kernel that will be disabled.
    *
    *    bDisableFrameSkip is only valid for P/B frames
    *    0: force-to-skip will be enabled as required in BRC kernel. Default value.
    *    1: force-to-skip will be disabled in BRC kernel.
    */
    bool            bDisableFrameSkip;

    /*! \brief Maximum frame size for all frame types in bytes.
    *
    *    Applicable for CQP and multi PAK. If dwMaxFrameSize > 0, driver will do multiple PAK and adjust QP 
    *    (frame level QP + slice_qp_delta) to make the compressed frame size to be less than this value. 
    *    If dwMaxFrameSize equals 0, driver will not do multiple PAK and do not adjust QP.
    */
    uint32_t        dwMaxFrameSize;

    /*! \brief Total pass number for multiple PAK.
    *
    *    Valid range is 0 - 4. If dwNumPasses is set to 0, driver will not do multiple PAK and do not adjust 
    *    QP (frame level QP + slice_qp_delta), otherwise, driver will do multiple times PAK and in each time 
    *    the QP will be adjust according deltaQp parameters.
    */
    uint32_t        dwNumPasses;

    /*! \brief Delta QP array for each PAK pass.
    *
    *    This pointer points to an array of deltaQp, the max array size for AVC encoder is 4. The valid range 
    *    for each deltaQp is 0 - 51. If the value is out of this valid range, driver will return error. 
    *    Otherwise, driver will adjust QP (frame level QP + slice_qp_delta) by adding this value in each PAK pass.
    */
    uint8_t        *pDeltaQp;

    /*! \brief Specifies target frame size in TCBRC mode.
    *
    *    If TCBRCSupport == 1, this parameter enables "Transport Controlled BRC mode" and indicates the desired frame size in bytes.
    *      - If the value equals 0, the BRC mode defined in RateControlMethod will take control for that certain frame.
    *    If TCBRCSupport == 0, this parameter will be ignored and should be set to 0. The BRC mode defined in RateControlMethod will be applied.
    */
    uint32_t        TargetFrameSize;

    /*! \brief Indicates if GPU polling based sync is enabled. 
    *
    *  Applicaiton sets to 1 to enable GPU polling based sync in driver. 
    */
    bool            bEnableSync;

    /*! \brief Indicates if the current frame is repeat frame. 
    *
    *  Applicaiton sets to 1 if current frame is repeat frame. 
    */
    bool            bRepeatFrame;

    /*! \brief Indicates if enable QP adjustment for current frame. 
    *
    *  Applicaiton sets to 1 to enable QP adjustment for current frame in CQP mode. 
    *  When QP adjustment is enabled, driver calls MBBRC kernel to adjust per MB QP for perceptual quality in CQP mode.
    */
    bool            bEnableQpAdjustment;

    /*! \brief Indicates marker coordinates in raw surface for GPU polling based sync. 
    *
    *  In unite of bytes. Valid for encoders which report SyncSupport capability as true.
    */
    uint16_t        SyncMarkerX;
    uint16_t        SyncMarkerY;

    /*! \brief Point to marker value for GPU polling based sync. 
    *
    *  Valid for encoders which report SyncSupport capability as true.
    */
    uint8_t         *pSyncMarkerValue;
    
    /*! \brief Indicates marker value for GPU polling based sync. 
    *
    *  In unit of bytes. Should be larger than or equal to 4. Valid for encoders which report SyncSupport capability as true.
    */
    uint32_t        SyncMarkerSize;

} CODEC_AVC_ENCODE_PIC_PARAMS, *PCODEC_AVC_ENCODE_PIC_PARAMS;

/*! \brief Slice-level parameters of a compressed picture for AVC encoding.
*/
typedef struct _CODEC_AVC_ENCODE_SLICE_PARAMS
{
    /*! \brief Specifies the number of macroblocks for this slice.
    *
    *    Note the slice height restriction in picture parameter structure.
    */
    uint32_t        NumMbsForSlice;
    /*! \brief Specifies the reference picture lists 0 and 1
    *
    *    Contains field/frame information concerning the reference in PicFlags. RefPicList[i][j]:
    *        \n - i: the reference picture list (0 or 1)
    *        \n - j: if the PicFlags are not PICTURE_INVALID, the index variable j is a reference to entry j in teh reference picture list.
    */
    CODEC_PICTURE   RefPicList[CODEC_AVC_NUM_REF_LISTS][CODEC_MAX_NUM_REF_FIELD];
    /*! \brief Specifies the weights and offsets used for explicit mode weighted prediction.
    *
    *    Weigths[i][j][k][m]:
    *        \n - i: the reference picture list (0 or 1)
    *        \n - j: reference to entry j in RefPicList (has range [0...31])
    *        \n - k: the YUV component (0 = luma, 1 = Cb chroma, 2 = Cr chroma)
    *        \n - m: the weight or offset used in the weighted prediction process (0 = weight, 1 = offset)
    */
    int16_t         Weights[2][32][3][2];

    uint32_t        first_mb_in_slice;                          //!< Same as AVC syntax element.
    uint8_t         slice_type;                                 //!< Same as AVC syntax element.
    uint8_t         pic_parameter_set_id;                       //!< Same as AVC syntax element.
    uint16_t        direct_spatial_mv_pred_flag         : 1;    //!< Same as AVC syntax element.
    uint16_t        num_ref_idx_active_override_flag    : 1;    //!< Same as AVC syntax element.
    uint16_t        long_term_reference_flag            : 1;    //!< Same as AVC syntax element.
    uint16_t                                            : 13;
    uint16_t        idr_pic_id;                                 //!< Same as AVC syntax element.
    uint16_t        pic_order_cnt_lsb;                          //!< Same as AVC syntax element.
    int32_t         delta_pic_order_cnt_bottom;                 //!< Same as AVC syntax element.
    int32_t         delta_pic_order_cnt[2];                     //!< Same as AVC syntax element.
    uint8_t         num_ref_idx_l0_active_minus1;               //!< Same as AVC syntax element.
    uint8_t         num_ref_idx_l1_active_minus1;               //!< Same as AVC syntax element.
    uint8_t         num_ref_idx_l0_active_minus1_from_DDI;
    uint8_t         num_ref_idx_l1_active_minus1_from_DDI;
    uint8_t         luma_log2_weight_denom;                     //!< Same as AVC syntax element.
    uint8_t         chroma_log2_weight_denom;                   //!< Same as AVC syntax element.
    uint8_t         cabac_init_idc;                             //!< Same as AVC syntax element.
    char            slice_qp_delta;                             //!< Same as AVC syntax element.
    uint8_t         disable_deblocking_filter_idc;              //!< Same as AVC syntax element.
    char            slice_alpha_c0_offset_div2;                 //!< Same as AVC syntax element.
    char            slice_beta_offset_div2;                     //!< Same as AVC syntax element.
    uint32_t        slice_id;                                   //!< Same as AVC syntax element.
    /*! \brief Indicates that the weighting factors for the luma component are present.
    *
    *    luma_weight_flag[i] is interpreted as corresponding to L0 when i=0 and L1 when i=1.  Each bit n of luma_weight_flag[i] corresponds to the nth entry in reference list i.  The framework must obey the caps the driver reported in MaxNum_WeightedPredL0/L1.
    */
    uint32_t        luma_weight_flag[2];
    /*! \brief Indicates that the weighting factors for the chroma component are present.
    *
    *    chroma_weight_flag[i] is interpreted as corresponding to L0 when i=0 and L1 when i=1.  Each bit n of chroma_weight_flag[i] corresponds to the nth entry in reference list i. The framework must obey the caps the driver reported in MaxNum_WeightedPredL0/L1.
    */
    uint32_t        chroma_weight_flag[2];

    CODEC_PIC_REORDER PicOrder[2][32];                       //!< Set by the driver

    uint8_t         colour_plane_id;                         //!< Same as AVC syntax element.
    uint32_t        frame_num;                               //!< Same as AVC syntax element.
    bool            field_pic_flag;                          //!< Same as AVC syntax element.
    bool            bottom_field_flag;                       //!< Same as AVC syntax element.
    uint8_t         redundant_pic_cnt;                       //!< Same as AVC syntax element.
    char            sp_for_switch_flag;                      //!< Same as AVC syntax element.
    char            slice_qs_delta;                          //!< Same as AVC syntax element.
    uint8_t         ref_pic_list_reordering_flag_l0     : 1; //!< Same as AVC syntax element.
    uint8_t         ref_pic_list_reordering_flag_l1     : 1; //!< Same as AVC syntax element.
    uint8_t         no_output_of_prior_pics_flag        : 1; //!< Same as AVC syntax element.
    uint8_t         adaptive_ref_pic_marking_mode_flag  : 1; //!< Same as AVC syntax element.
    uint8_t                                             : 3;
    uint32_t        MaxFrameNum; //!< Set by the driver: 1 << (pSeqParams[pPicParams->seq_parameter_set_id].log2_max_frame_num_minus4 + 4);
    uint8_t         NumReorder;  //!< Set by the driver
} CODEC_AVC_ENCODE_SLICE_PARAMS, *PCODEC_AVC_ENCODE_SLICE_PARAMS;

// H.264 Inverse Quantization Weight Scale
typedef struct _CODEC_AVC_ENCODE_IQ_WEIGTHSCALE_LISTS
{
    uint8_t         WeightScale4x4[6][16];
    uint8_t         WeightScale8x8[2][64];
} CODEC_AVC_ENCODE_IQ_WEIGTHSCALE_LISTS, *PCODEC_AVC_ENCODE_IQ_WEIGTHSCALE_LISTS;

// used for PAFF case, 0: frame, 1: tff field, 2: invalid, 3: bff field
typedef enum _CODEC_AVC_PIC_CODING_TYPE_VALUE
{
    CODEC_AVC_PIC_CODING_TYPE_FRAME     = 0x0,
    CODEC_AVC_PIC_CODING_TYPE_TFF_FIELD = 0x1,
    CODEC_AVC_PIC_CODING_TYPE_INVALID   = 0x2,
    CODEC_AVC_PIC_CODING_TYPE_BFF_FIELD = 0x3
} CODEC_AVC_PIC_CODING_TYPE_VALUE;

//!
//! \struct    CodecEncodeAvcFeiPicParams
//! \brief     Codec encode AVC FEI pic params
//!
struct CodecEncodeAvcFeiPicParams
{
    MOS_RESOURCE                resMBCtrl;              // input MB control buffer
    MOS_RESOURCE                resMVData;              // ENC MV output buffer or PAK MV input buffer
    MOS_RESOURCE                resMBCode;              // ENC MBCode output buffer or PAK MBCode input buffer
    MOS_RESOURCE                resMVPredictor;         // input MV predictor surface
    MOS_RESOURCE                resMBQp;                // input QP per MB surface
    MOS_RESOURCE                resDistortion;          // ENC or ENC_PAK Distortion output surface
    uint32_t                    NumMVPredictorsL0;
    uint32_t                    NumMVPredictorsL1;

    bool                        MbCodeMvEnable;
    bool                        DistortionEnable;

    /** \brief control parameters */
    uint32_t                    SearchPath;
    uint32_t                    LenSP;

    uint32_t                    SubMBPartMask;
    uint32_t                    IntraPartMask;
    bool                        MultiPredL0;
    bool                        MultiPredL1;
    uint32_t                    SubPelMode;
    uint32_t                    InterSAD;
    uint32_t                    IntraSAD;
    uint32_t                    DistortionType;
    bool                        RepartitionCheckEnable;
    bool                        AdaptiveSearch;
    bool                        MVPredictorEnable;
    bool                        bMBQp;
    bool                        bPerMBInput;
    bool                        bMBSizeCtrl;
    uint32_t                    RefWidth;
    uint32_t                    RefHeight;
    uint32_t                    SearchWindow;

    //add for mutlple pass pak
    uint32_t                    dwMaxFrameSize;
    uint32_t                    dwNumPasses;     //number of QPs
    uint8_t                    *pDeltaQp;        //list of detla QPs
};
#endif  // __CODEC_DEF_ENCODE_AVC_H__
