/*
* Copyright (c) 2016-2018, Intel Corporation
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
//! \file     codec_def_encode_hevc.h
//! \brief    Defines encode HEVC types and macros shared by CodecHal, MHW, and DDI layer
//! \details  Applies to HEVC encode only. Should not contain any DDI specific code.
//!
#ifndef __CODEC_DEF_ENCODE_HEVC_H__
#define __CODEC_DEF_ENCODE_HEVC_H__

#include "codec_def_common_hevc.h"
#include "codec_def_common_encode.h"

// HEVC
#define CODEC_MAX_NUM_REF_FRAME_HEVC        15
#define CODEC_NUM_REF_FRAME_HEVC_WP         3       // number of reference frames used for weighted prediction
#define HEVC_NUM_MAX_TILE_ROW               22
#define HEVC_NUM_MAX_TILE_COLUMN            20
#define CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6  600
#define CODECHAL_HEVC_MAX_NUM_SLICES_LVL_5  200
#define CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC 8
#define CODECHAL_NUM_INTERNAL_NV12_RT_HEVC  16
#define CODECHAL_ENCODE_HEVC_MAX_NUM_ROI    16

// HEVC VDENC
#define ENCODE_HEVC_VDENC_NUM_MAX_SLICES        70
#define ENCODE_VDENC_HEVC_MAX_STREAMINROI_G10   3
#define ENCODE_VDENC_HEVC_MAX_ROI_NUMBER_G10    8
#define ENCODE_VDENC_HEVC_MAX_DIRTYRECT_G10     256
#define ENCODE_VDENC_HEVC_ROI_BLOCKSIZE_G10     2        // 0:8x8, 1:16x16, 2:32x32, 3:64x64
#define ENCODE_VDENC_HEVC_MIN_ROI_DELTA_QP_G10  -8
#define ENCODE_VDENC_HEVC_MAX_ROI_DELTA_QP_G10  7        // Max delta QP for VDEnc ROI
#define ENCODE_VDENC_HEVC_PADDING_DW_SIZE       8

// HEVC DP
#define ENCODE_DP_HEVC_NUM_MAX_VME_L0_REF_G9  3
#define ENCODE_DP_HEVC_NUM_MAX_VME_L1_REF_G9  1
#define ENCODE_DP_HEVC_MAX_NUM_ROI            16
#define ENCODE_DP_HEVC_ROI_BLOCK_SIZE         1     //From DDI, 0:8x8, 1:16x16, 2:32x32, 3:64x64
#define ENCODE_DP_HEVC_ROI_BLOCK_Width        16    
#define ENCODE_DP_HEVC_ROI_BLOCK_HEIGHT       16

typedef enum
{
    ENCODE_HEVC_BIT_DEPTH_8     = 0,
    ENCODE_HEVC_BIT_DEPTH_10    = 1,
    ENCODE_HEVC_BIT_DEPTH_12    = 2,
    ENCODE_HEVC_BIT_DEPTH_16    = 3,
} ENCODE_HEVC_BIT_DEPTH;

//!
//! \enum HEVC_NAL_UNIT_TYPE
//! \brief HEVC NAL unit type
//!
typedef enum
{
    HEVC_NAL_UT_TRAIL_N           = 0x00, //!< Coded slice segment of a non-TSA, non-STSA trailing picture - slice_segment_layer_rbsp, VLC
    HEVC_NAL_UT_TRAIL_R           = 0x01, //!< Coded slice segment of a non-TSA, non-STSA trailing picture - slice_segment_layer_rbsp, VLC
    HEVC_NAL_UT_TSA_N             = 0x02, //!< Coded slice segment of a TSA picture - slice_segment_layer_rbsp, VLC
    HEVC_NAL_UT_TSA_R             = 0x03, //!< Coded slice segment of a TSA picture - slice_segment_layer_rbsp, VLC
    HEVC_NAL_UT_STSA_N            = 0x04, //!< Coded slice of an STSA picture - slice_layer_rbsp, VLC
    HEVC_NAL_UT_STSA_R            = 0x05, //!< Coded slice of an STSA picture - slice_layer_rbsp, VLC
    HEVC_NAL_UT_RADL_N            = 0x06, //!< Coded slice of an RADL picture - slice_layer_rbsp, VLC
    HEVC_NAL_UT_RADL_R            = 0x07, //!< Coded slice of an RADL picture - slice_layer_rbsp, VLC
    HEVC_NAL_UT_RASL_N            = 0x08, //!< Coded slice of an RASL picture - slice_layer_rbsp, VLC
    HEVC_NAL_UT_RASL_R            = 0x09, //!< Coded slice of an RASL picture - slice_layer_rbsp, VLC
    /* 0x0a..0x0f - Reserved */
    HEVC_NAL_UT_BLA_W_LP          = 0x10, //!< Coded slice segment of an BLA picture - slice_segment_layer_rbsp, VLC
    HEVC_NAL_UT_BLA_W_DLP         = 0x11, //!< Coded slice segment of an BLA picture - slice_segment_layer_rbsp, VLC
    HEVC_NAL_UT_BLA_N_LP          = 0x12, //!< Coded slice segment of an BLA picture - slice_segment_layer_rbsp, VLC
    HEVC_NAL_UT_IDR_W_DLP         = 0x13, //!< Coded slice segment of an IDR picture - slice_segment_layer_rbsp, VLC
    HEVC_NAL_UT_IDR_N_LP          = 0x14, //!< Coded slice segment of an IDR picture - slice_segment_layer_rbsp, VLC
    HEVC_NAL_UT_CRA               = 0x15, //!< Coded slice segment of an CRA picture - slice_segment_layer_rbsp, VLC
    HEVC_NAL_UT_RSV_IRAP_VCL23    = 0x17, //!< Reserved IRAP VCL NAL unit type
    /* 0x16..0x1f - Reserved */
    HEVC_NAL_UT_VPS               = 0x20, //!< Video parameter set - video_parameter_set_rbsp, non-VLC
    HEVC_NAL_UT_SPS               = 0x21, //!< Sequence parameter set - seq_parameter_set_rbsp, non-VLC
    HEVC_NAL_UT_PPS               = 0x22, //!< Picture parameter set - pic_parameter_set_rbsp, non-VLC
    HEVC_NAL_UT_AUD               = 0x23, //!< Access unit delimiter - access_unit_delimiter_rbsp, non-VLC
    HEVC_NAL_UT_EOS               = 0x24, //!< End of sequence - end_of_seq_rbsp, non-VLC
    HEVC_NAL_UT_EOB               = 0x25, //!< End of bitsteam - end_of_bitsteam_rbsp, non-VLC
    HEVC_NAL_UT_FD                = 0x26, //!< Filler data - filler_data_rbsp, non-VLC
    HEVC_NAL_UT_PREFIX_SEI        = 0x27, //!< Supplemental enhancement information (SEI) - sei_rbsp, non_VLC
    HEVC_NAL_UT_SUFFIX_SEI        = 0x28, //!< Supplemental enhancement information (SEI) - sei_rbsp, non_VLC

    //this should be the last element of this enum
    //chagne this value if NAL unit type increased
    HEVC_MAX_NAL_UNIT_TYPE        = 0x3f,
} HEVC_NAL_UNIT_TYPE;

typedef struct _CODEC_FRAMERATE
{
    uint32_t    Numerator;
    uint32_t    Denominator;
}CODEC_FRAMERATE;

/*! \brief Provides the picture-level parameters of a compressed picture for HEVC decoding.
*
*   The framework is expected to only send a sequence parameter compressed buffer for the first picture(first Execute() call) in a sequence, associated with a RAP(IDR, CRA, BLA) picture.
*/
typedef struct _CODEC_HEVC_ENCODE_SEQUENCE_PARAMS
{
    /*! \brief Plus 1 specifies the width of each encoded picture in units of minimum coding block size.
    *
    *   The encoded picture width in units of luma samples equals (wFrameWidthInMinCbMinus1 + 1) * (1 << (log2_min_coding_block_size_minus3 + 3))
    *   Programming Note: HW requres surface allocation Y offset of chroma plain to be multiple of 32 pixels. And HEVC spec requires frame resolution to be multiple of minimal CU (could be 8 pixels) horizontally and vertically. Framework needs to pad accordingly. If source resolution is different from what is actually encoded (padding happens), cropping information should be provided in the SPS header accordingly.
    */
    uint16_t      wFrameWidthInMinCbMinus1;
    /*! \brief Plus 1 specifies the height of each encoded picture in units of minimum coding block size.
    *
    *   The encoded picture height in units of luma samples equals (wFrameHeightInMinCbMinus1 + 1) * (1 << (log2_min_coding_block_size_minus3 + 3))
    */
    uint16_t      wFrameHeightInMinCbMinus1;
    uint8_t       general_profile_idc;                //!< Same as HEVC syntax element
    /*! \brief Equal to syntax element general_level_idc / 3
    *
    *   Value range of general_level_idc is 30 times [1, 2, 2.1, 3, 3.1, 4, 4.1, 5, 5.1, 5.2, 6, 6.1, 6.2].
    *   Programming Note: HW requres surface allocation Y offset of chroma plain to be multiple of 32 pixels. And HEVC spec requires frame resolution to be multiple of minimal CU (could be 8 pixels) horizontally and vertically. Framework needs to pad accordingly. If source resolution is different from what is actually encoded (padding happens), cropping information should be provided in the SPS header accordingly.
    */
    uint8_t       Level;
    uint8_t       general_tier_flag;                  //!< Same as HEVC syntax element

    uint16_t      GopPicSize;                         //!< Distance between IRAP pictures.
    /*! \brief Distance between anchor frames (I or P).
    *
    *    Here, P may also means low delay B (GPB) frames.
    *    Programming Note: GopPicSize > GopRefDist should be ensured by application. It is required by BRC.
    */
    uint8_t       GopRefDist;
    uint8_t       GopOptFlag                  : 2;
    uint8_t                                   : 6;

    uint8_t       TargetUsage;
    /*! \brief Specifies rate control method.
    *
    *    \n 1: CBR
    *    \n 2: VBR
    *    \n 3: CQP, if set, internal BRC, multi-pass and panic mode will be disabled
    *    \n 4: AVBR, if set, HRD compliance is not guaranteed. Buffering period SEI and picture timing SEI messages are not necessary for AVBR.
    *    \n 5: QVBR
    *    \n 6: Reserved
    *    \n 7: Reserved
    *    \n 8: VCM, defined for video converencing
    *    \n 9: ICQ
    *    \n Programming note: Define the minimum value as indicated above for AVBR accuracy & convergence, clamp any value that is less than the minimum value to the minimum value.  Define the maximum value for AVBR accuracy as 100 (10%) and for AVBR convergence as 500, clamp any value that is greater than the maximum value to the maximum value. The maximum & minimum value may be adjusted when necessary. If bResetBRC is set to 1 for a non-I picture, driver shall not insert SPS into bitstream.  Driver needs to calculate the maximum allowed frame size per profile/level for all RateControlMethod except CQP, and use the calculated value to program kernel for non AVBR modes; for AVBR mode, driver needs to clamp the upper bound of UserMaxFrameSize to the calculated value and use the clamped UserMaxFrameSize to program kernel.  If IWD_VBR is set, driver programs it the same as VBR except not to enable panic mode.
    */
    uint8_t       RateControlMethod;
    uint32_t      TargetBitRate;                      //!< Target bit rate Kbit per second
    uint32_t      MaxBitRate;                         //!< Maximum bit rate Kbit per second
    /*! \brief Minimun bit rate Kbit per second.
    *
    *   This is used in VBR control. For CBR control, this field is ignored.
    */
    uint32_t          MinBitRate;
    CODEC_FRAMERATE   FrameRate;                          //!< Actual frame rate is the decimal derivative of FrameRate.Numerator / FrameRate.Denominator.
    uint32_t          InitVBVBufferFullnessInBit;         //!< Initial VBV buffer fullness in bits.
    /*! \brief VBV buffer size in bit unit.
    *
    *   The HEVC spec defines a max coded picture buffer size for each level.
    */
    uint32_t       VBVBufferSizeInBit;

    union
    {
        struct
        {
            /*! \brief Indicate if a BRC reset is desired to set a new bit rate or frame rate.
            *
            *   This setting is only valid if RateControlMethod is AVBR or VBR and the current picture is an I picture. If the frame resolution is changed, it should be set with IDR picture. It should not be set when RateControlMethod is CBR or CQP. The following table indicates which BRC parameters can be changed via a BRC reset.
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
            *  \n AVBRAccuracy                  No
            *  \n AVBRConvergence               No
            *  \n Note: when resolution (FrameWidth and/or FrameHeight) changes, framework should re-start a new bit stream and not using BRC reset.
            */
            uint32_t        bResetBRC           : 1;
            /*! \brief Indicates the global search options.
            *
            *   It is only valid if EnhancedEncInput is reported as a capability.
            *       \n - 0 : Default - option internally selected based on target usage
            *       \n - 1 : Long - SHME enabled
            *       \n - 2 : Medium - HME only enabled, SHME disabled
            *       \n - 3 : Short - SHME/HME disabled
            */
            uint32_t        GlobalSearch        : 2;

            /*! \brief Indicates the local search options.
            *
            *    It is only valid if EnhancedEncInput is reported as a capability.
            *        \n - 0 : Default - option internally selected based on target usage
            *        \n - 1 : Tiny – MaxLenSP = 4, Reference Window = 24x24 SP = Spiral
            *        \n - 2 : Small – MaxLenSP = 9, Reference Window = 28x28 SP = Spiral
            *        \n - 3 : Square – MaxLenSP = 16, Reference Window = 32x32 SP = Spiral
            *        \n - 4 : Diamond – MaxLenSP = 16, Reference Window = 48x40 SP = Diamond
            *        \n - 5 : Large Diamond – MaxLenSP = 30, Reference Window = 48x40 SP = Diamond
            *        \n - 6 : Exhaustive – MaxLenSP = 57, Reference Window = 48x40 SP = Spiral
            *        \n - 7 : Heavy Horizontal – MaxLenSP = 57, Reference Window = 64x32 SP = Spiral
            *        \n - 8 : Heavy Vertical – MaxLenSP = 57, Reference Window = 32x64 SP = Spiral
            */
            uint32_t        LocalSearch         : 4;
            /*! \brief Indicates the EarlySkip control.
            *
            *    It is only valid if EnhancedEncInput is reported as a capability.
            *        \n - 0 : Default, decided internally
            *        \n - 1 : EarlySkip enabled
            *        \n - 2 : EarlySkip disabled
            */
            uint32_t        EarlySkip           : 2;
            /*! \brief Indicates that MB BRC is enabled.
            *
            *    It is only valid if MBBRCSupport is reported as a capability. Currently MB BRC can be applied to all bit rate control methods except CQP.
            *        \n - 0 : Default, decided internally based on target usage.
            *        \n - 1 : MB BRC enabled.
            *        \n - 2 : MB BRC disabled.
            *        \n - Other values are reserved.
            */
            uint32_t        MBBRC               : 4;
            /*! \brief Indicates that Parallel BRC is enabled.
            *
            *    It is only valid if ParallelBRC is reported as a capability.
            *        \n - 0 : no parallel BRC required.
            *        \n - 1 : request parallel BRC.
            */
            uint32_t        ParallelBRC         : 1;
            /*! \brief Indicates that Dynamic Slice Size Control is enabled.
            *
            *    It is only valid if SliceByteSizeControl is reported as a capability. When enabled, the slice information that app provides will be applied as "supper slice". It means that GPU may further split the slice region that slice control data specifies into finer slice segments based on slice size upper limit (MaxSliceSize). GPU will duplicate the slice header bit stream for each splitted slice segments while updating the necessary fields such as slice_segment_address.
            */
            uint32_t        SliceSizeControl    : 1;
            /*! \brief Specifies input source format
            *
            *    \n 0:    YUV420
            *    \n 1:    YUV422
            *    \n 2:    YUV444
            *    \n 3:    RGB
            *    \n Note1: Encoder cannot do up-sampling. For example, if source format is YUV422, the encoder can generates bit stream of 420 or 422 only. It cannot generate YUV444 format. But it may generates RGB format.
            *    Note2: For RGB, the actual input source format is also determined by flag DisplayFormatSwizzle (formats below is in MSB->LSB order).
            *        \n - If DisplayFromatSwizzle is 0, and 8b intut is specified the format is A8B8G8R8, if 10 bit is specified the format is A2B10G10R10
            *        \n - If DisplayFromatSwizzle is 0, and 8b intut is specified the format is A8R8G8B8, if 10 bit is specified the format is A2R10G10B10
            */
            uint32_t        SourceFormat        : 2;
            /*! \brief Specifies input source bit depth.
            *
            *    \n 0:    8b
            *    \n 1:    10b
            *    \n 2:    12b
            *    \n 3:    16b
            *    \n Note: Encoder cannot do up-sampling. For example, if source bit depth is 10b, the encoder can generates bit stream of 8b or 10b only, and that it cannot generate 12b format. It is subjected to the limit set by MaxEncodedBitDepth.
            */
            uint32_t        SourceBitDepth      : 2;
            /*! \brief Enable QP Adjustment at CQP mode.
            *
            *        \n - 0 : no QP adjustment.
            *        \n - 1 : enable QP adjustment.
            *        \n Note: Basically ENC may define fine-tune QP adjustment based on motion search result, which is similar as MBBRC for BRC mode. With BRC modes, this flag should be set to 0.
            */
            uint32_t        QpAdjustment        : 1;
            /*! \brief Indicates ROI[] value is in delta QP.
            *
            *    It is valid only when parameter NumROI is greater than 0 and either ROIDeltaQPSupport or ROIBRCPriorityLevelSupport equals to 1.
            *        \n - 0 : ROI[] value is in priority.
            *        \n - 1 : ROI[] value is in delta QP.
            *        \n Note: ROIValueInDeltaQP must be set to 1 for CQP. Currently only ROIValueInDeltaQP equal 1 is validated.
            */
            uint32_t        ROIValueInDeltaQP        : 1;
            /*! \brief Indicates block level absolute QP value is provided.
            *
            *        \n - 0 : block level absolute QP value is not provided.
            *        \n - 1 : block level absolute QP value is provided.
            */
            uint32_t        BlockQPforNonRectROI     : 1;
            /*! \brief Enables tile based encoding.
            *
            *        \n - 0 : tile based encoding disabled.
            *        \n - 1 : tile based encoding enabled.
            */
            uint32_t        EnableTileBasedEncode    : 1;
            /*! \brief Indicates if BRC can use larger P/B frame size than UserMaxPBFrameSize 
            *
            *        \n - 0 : BRC can not use larger P/B frame size  than UserMaxPBFrameSize.
            *        \n - 1 : BRC can use larger P/B frame size  than UserMaxPBFrameSize.
            */
            uint32_t        bAutoMaxPBFrameSizeForSceneChange : 1;
            /*! \brief Enables streaming buffer in LLC
            *
            *        \n - 0 : streaming buffer by LLC is disabled.
            *        \n - 1 : streaming buffer by LLC is enabled.
            */
            uint32_t        EnableStreamingBufferLLC : 1;
            /*! \brief Enables streaming buffer in DDR
            *
            *        \n - 0 : streaming buffer by DDR is disabled.
            *        \n - 1 : streaming buffer by DDR is enabled.
            */
            uint32_t        EnableStreamingBufferDDR : 1;

            /*! \brief Low Delay Mode
            *
            *        \n - 0 : Random Access B.
            *        \n - 1 : Low delay encoding with P or LDB.
            */
            uint32_t        LowDelayMode            : 1;

            /*! \brief Disable HRD conformance
            *
            *        \n - 0 : HRD conformance is enabled.
            *        \n - 1 : HRD conformance is disabled (aka no panic mode).
            */
            uint32_t        DisableHRDConformance    : 1;

            /*! \brief Hierarchical Mini GOP
            *
            *        \n - 0 : Flat GOP (No Hierarchical Mini GOP).
            *        \n - 1 : Hierarchical Mini GOP.
            */
            uint32_t        HierarchicalFlag         : 1;

            uint32_t        ReservedBits             : 3;
        };
        uint32_t    SeqFlags;
    };

    /*! \brief Framework defined maximum frame size in bytes for I frames.
    *
    *    Applicable for all RateControlMethod values except CQP; guarantees that the compressed frame size will be less than this value. If UserMaxPBFrameSize equals 0, UserMaxIFrameSize will be used for all frame types. Maximum allowed frame size per profile/level will be calculated in driver and be used when UserMaxIFrameSize and UserMaxPBFrameSize are both set to 0.
    */
    uint32_t            UserMaxIFrameSize;
    /*! \brief Framework defined maximum frame size in bytes for P & B frames.
    *
    *    Applicable for all RateControlMethod values except CQP; guarantees that the compressed frame size will be less than this value. If UserMaxPBFrameSize equals 0, UserMaxIFrameSize will be used for all frame types. Maximum allowed frame size per profile/level will be calculated in driver and be used when UserMaxIFrameSize and UserMaxPBFrameSize are both set to 0.
    */
    uint32_t            UserMaxPBFrameSize;
    /*! \brief For Constant Rate Factor BRC method, it indicates the measure of quality.
    *
    *    The range is from 1 – 51, with 1 being the best quality.
    */
    uint8_t   ICQQualityFactor;

        /*! \brief Specigy session that IPU and GPU communicate on.
    *
    *    It is for streaming buffer.
    */
    uint8_t StreamBufferSessionID;

    uint8_t Reserved16b;

    /*! \brief Number of B frames per level in BGOP (between each two consecutive anchor frames).
    *
    *   \n NumOfBInGop[0] – regular B, or no reference to other B frames.
    *   \n NumOfBInGop[1] – B1, reference to only I, P or regular B frames.
    *   \n NumOfBInGop[2] – B2, references include B1.
    *   \n Invalid when ParallelBRC is disabled (value 0).
    */
    uint32_t NumOfBInGop[3];  // depricated

    union
    {
        struct
        {
            /*! \brief Same as syntax element.
            *
            *    When the scaling_list_enable_flag is set to disable, the scaling matrix is still sent to the PAK, and with all entries programmed to the same value of 16.
            */
            uint32_t    scaling_list_enable_flag            : 1;
            uint32_t    sps_temporal_mvp_enable_flag        : 1;    //!< Same as HEVC syntax element
            uint32_t    strong_intra_smoothing_enable_flag  : 1;    //!< Same as HEVC syntax element
            uint32_t    amp_enabled_flag                    : 1;    //!< Same as HEVC syntax element
            /*! \brief Same as syntax element sample_adaptive_offset_enabled_flag.
            *
            *    Programming notes: must be 0 on SKL
            */
            uint32_t    SAO_enabled_flag : 1;
            /*! \brief Same as syntax element.
            *
            *    Programming note: must be 0 on SKL.
            */
            uint32_t    pcm_enabled_flag                    : 1;
            uint32_t    pcm_loop_filter_disable_flag        : 1;    //!< Same as HEVC syntax element
            uint32_t    reserved                            : 1;
            uint32_t    chroma_format_idc                   : 2;    //!< Same as HEVC syntax element
            uint32_t    separate_colour_plane_flag          : 1;    //!< Same as HEVC syntax element
            uint32_t    palette_mode_enabled_flag           : 1;
            uint32_t    RGBEncodingEnable                   : 1;
            uint32_t    PrimaryChannelForRGBEncoding        : 2;
            uint32_t    SecondaryChannelForRGBEncoding      : 2;
            uint32_t                                        : 15;  // [0]
        };
        uint32_t    EncodeTools;
    };

    /*! \brief Specifies the maximum size of a coding block
    *
    *   Corresponds to HEVC spec variable Log2CtbSize - 3. The full value range is [0..3], inclusive.
    *   Programming note: On SKL, must be set at 2.
    */
    uint8_t    log2_max_coding_block_size_minus3;
    /*! \brief Specifies the minimum size of a coding block.
    *
    *   Corresponds to HEVC spec variable Log2MinCbSize - 3. The full value range is [0..3], inclusive.
    *   Programming note: On SKL, must be set at 0.
    */
    uint8_t    log2_min_coding_block_size_minus3;
    /*! \brief Specifies the maximum size of a transform unit.
    *
    *   Corresponds to HEVC spec variable Log2MaxTrafoSize - 2. The value range is [0..3], inclusive.
    *   Programming note: On SKL, must be set to 3 to indicate 32x32.
    */
    uint8_t    log2_max_transform_block_size_minus2;
    /*! \brief Specifies the minimum size of a transform unit.
    *
    *   Corresponds to HEVC spec variable Log2MinTrafoSize - 2. The value range is [0..3], inclusive.
    *   Programming note: On SKL, must be set to 0 to indicate 4x4.
    */
    uint8_t    log2_min_transform_block_size_minus2;
    /*! \brief Same as HEVC syntax element.
    *
    *    Programming note: On SKL, must be set to 2.
    */
    uint8_t    max_transform_hierarchy_depth_intra;
    /*! \brief Same as HEVC syntax element.
    *
    *    Programming note: On SKL, must be set to 2.
    */
    uint8_t    max_transform_hierarchy_depth_inter;
    /*! \brief Specifies the minimum size of I_PCM coding blocks.
    *
    *    Corresponds to HEVC spec variable Log2MinIpcmCbSizeY.
    */
    uint8_t    log2_min_PCM_cb_size_minus3;
    /*! \brief Specifies the maximum size of I_PCM coding blocks.
    *
    *   Corresponds to HEVC spec variable Log2MaxIpcmCbSizeY.
    */
    uint8_t    log2_max_PCM_cb_size_minus3;
    uint8_t    bit_depth_luma_minus8;                 //!< Same as HEVC syntax element
    uint8_t    bit_depth_chroma_minus8;               //!< Same as HEVC syntax element
    uint8_t    pcm_sample_bit_depth_luma_minus1;      //!< Same as HEVC syntax element
    uint8_t    pcm_sample_bit_depth_chroma_minus1;    //!< Same as HEVC syntax element

    uint8_t    bVideoSurveillance;

    /*! \brief This flag is used for compatibility between various DDIs.
    *
    *    Do NOT proram a kernel or otherwise make decisions based on this value.  Instead use the flag inside CODEC_HEVC_ENCODE_PICTURE_PARAMS.
    */
    uint8_t    bScreenContent;

    /*! \brief Indicates the uncompressed input color space
    *
    *    Valid only when input is ARGB format.
    */
    ENCODE_INPUT_COLORSPACE     InputColorSpace;

    /*! \brief Provides a hint to encoder about the scenario for the encoding session.
    *
    *   BRC algorithm may tune differently based on this info.
    */
    ENCODE_SCENARIO             ScenarioInfo;

    ENCODE_CONTENT              contentInfo;

    /*! \brief Indicates the tolerance the application has to variations in the frame size.
    *
    *   It affects the BRC algorithm used, but may or may not have an effect based on the combination of other BRC parameters.  Only valid when the driver reports support for FrameSizeToleranceSupport.
    */
    ENCODE_FRAMESIZE_TOLERANCE  FrameSizeTolerance;


    uint16_t                   SlidingWindowSize;
    uint32_t                   MaxBitRatePerSlidingWindow;
    uint32_t                   MinBitRatePerSlidingWindow;

    /*! \brief Indicates number of frames to lookahead.
    *
    *    Range is [0~127]. Default is 0 which means lookahead disabled. Valid only when LookaheadBRCSupport is 1. When not 0, application should send LOOKAHEADDATA to driver.
    */
    uint8_t                   LookaheadDepth;

    uint32_t motion_vector_resolution_control_idc;
    uint32_t intra_boundary_filtering_disabled_flag;
    uint8_t     palette_max_size;
    uint8_t     delta_palette_max_predictor_size;
} CODEC_HEVC_ENCODE_SEQUENCE_PARAMS, *PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS;

/*! \brief Provides the picture-level parameters of a compressed picture for HEVC decoding.
*/
typedef struct _CODEC_HEVC_ENCODE_PICTURE_PARAMS
{
    /*! \brief Specifies the uncompressed source surface of the frame for the current picture to be encode.
    *
    *    The PicFlags regarding reference usage are expected to be valid at this time.
    */
    CODEC_PICTURE           CurrOriginalPic;
    /*! \brief Specifies the uncompressed surface of the reconstructed frame for the current encoded picture.
    *
    *    The PicFlags regarding reference usage are expected to be valid at this time.
    *    The recon surface may be of different format and different bit depth from that of source.
    *    The framework needs to specify it through chroma_format_idc and bit_depth_luma_minus8
    *    and bit_depth_chroma_minus8 in SPS data structure.
    */
    CODEC_PICTURE           CurrReconstructedPic;
    /*! \brief Specifies the collocated reference picture’s index into the RefFrameList[].
    *
    *    Application should generate collocated_ref_idx from collocated_from_l0_flag value per slice. When  the HEVC syntax element slice_temporal_mvp_enable_flag takes value 0, CollocatedRefPicIndex should take value 0xFF.
    */
    uint8_t                 CollocatedRefPicIndex;
    /*! \brief Each entry of the list specifies the frame index of the reference pictures.
    *
    *   The value of FrameIdx specifies the index of RefFrameList structure. And valid value range is [0..14, 0x7F]. Invalid entries are indicated by setting PicFlags to PICTURE_INVALID.
    *   RefFrameList[] should include all the reference pictures in DPB, which means either the picture is referred by current picture or future pictures, it should have a valid entry in it.
    */
    CODEC_PICTURE           RefFrameList[CODEC_MAX_NUM_REF_FRAME_HEVC];
    int32_t                 CurrPicOrderCnt;                            //!< Picture Order Count value of current picture.
    /*! \brief Picture Order Count values of reference pictures corresponding to the entries of RefFrameList[].
    *
    *    For invalid entries of RefFrameList[], its RefFramePOCList value can be ignored.
    */
    int32_t                 RefFramePOCList[CODEC_MAX_NUM_REF_FRAME_HEVC];

    /*! \brief Specifies picture coding type.
    *
    *    \n 1: I picture
    *    \n 2: P picture
    *    \n 3: B picture
    *    \n 4: B1 picutre
    *    \n 5: B2 picture
    *    \n For B1 and B2 explanation refer to NumOfBInGop[]
    */
    uint8_t                 CodingType;
    uint8_t                 HierarchLevelPlus1;
    uint16_t                NumSlices;

    union
    {
        struct
        {
            uint32_t            tiles_enabled_flag                  : 1;    //!< Same as HEVC syntax element
            uint32_t            entropy_coding_sync_enabled_flag    : 1;    //!< Same as HEVC syntax element
            /*! \brief Same as HEVC syntax element.
            *
            *    Programming note: On SKL, must be set to 0.
            */
            uint32_t            sign_data_hiding_flag               : 1;
            /*! \brief Same as HEVC syntax element.
            *
            *    Programming note: On SKL, must be set to 0.
            */
            uint32_t            constrained_intra_pred_flag         : 1;
            /*! \brief Same as HEVC syntax element.
            *
            *    Programming note: On SKL, must be set to 0.
            */
            uint32_t            transform_skip_enabled_flag         : 1;
            uint32_t            transquant_bypass_enabled_flag      : 1;    //!< Same as HEVC syntax element
            uint32_t            cu_qp_delta_enabled_flag            : 1;    //!< Same as HEVC syntax element
            uint32_t            weighted_pred_flag                  : 1;    //!< Same as HEVC syntax element
            uint32_t            weighted_bipred_flag                : 1;    //!< Same as HEVC syntax element
            uint32_t            loop_filter_across_slices_flag      : 1;    //!< Same as HEVC syntax element of seq_loop_filter_across_slices_enabled_flag
            uint32_t            loop_filter_across_tiles_flag       : 1;    //!< Same as HEVC syntax element
            /*! \brief A combination of HEVC syntax element of sps_scaling_list_data_present_flag and pps_scaling_list_data_present_flag.
            *
            *    If the value is 1, application will send a quantization matrix (scaling list) buffer to DDI. Otherwise not. If scaling_list_enable_flag is 0. This flag should also be 0.
            */
            uint32_t            scaling_list_data_present_flag      : 1;
            uint32_t            dependent_slice_segments_enabled_flag : 1;  //!< Same as HEVC syntax element
            uint32_t            bLastPicInSeq                       : 1;
            uint32_t            bLastPicInStream                    : 1;
            uint32_t            bUseRawPicForRef                    : 1;
            uint32_t            bEmulationByteInsertion             : 1;
            uint32_t            BRCPrecision                        : 2;
            /*! \brief Specifies that slice level reporting may be requested for this frame.
            *
            *    If this flag is set, then slice level parameter reporting will be set up for this frame. Only valid if SliceLevelReportSupport is reported as a capability, else this flag is ignored.
            */
            uint32_t            bEnableSliceLevelReport             : 1;
            /*! \brief Specifies whether intra refresh is enabled in colum/row, or disabled.
            *
            *    It applies to P picture only (not valid with IBP) and is effective only when RollingIntraRefresh capability is set..
            *        \n - 0: disabled
            *        \n - 1: enabled in colum
            *        \n - 2: enabled in row
            *    \n Programming Note: When RollingIntraRefresh is used, there are some restrictions the application must obey.
            *        \n - No B frames.
            *        \n - No multiple references.
            *    \n Additionally the driver will disable Multiple Prediction (MultiPred).  This is to simplify the kernel when ensuring inter serach doesn’t refer to illegal regions.
            */
            uint32_t            bEnableRollingIntraRefresh          : 2;
            /*! \brief Same as HEVC syntax element
            *
            */
            uint32_t            no_output_of_prior_pics_flag        : 1;
            /*! \brief Requests GPU to determine weighted prediction factors.
            *
            *    Is valid when either weighted_pred_flag or weighted_bipred_flag is turned on.
            *    In this case, the following parameters in slice control data structure shall be ignored: luma_log2_weight_denom, delta_chroma_log2_weight_denom, luma_offset[2][15], delta_luma_weight[2][15], chroma_offset[2][15][2], and delta_chroma_weight[2][15][2].
            */
            uint32_t            bEnableGPUWeightedPrediction        : 1;
            /*! \brief Indicates that the allocated source buffer format is a swizzle format from display.
            *
            *    Framework still allocates the buffer as a standard FOURCC format. The swizzled format will be reported as supported for the encoder configuration during capability reporting.
            *    \n Source/Raw Allocated Buffer Format, DisplayFormatSwizzle, Layout Format in Buffer
            *        \n - YUY2, 0, YUY2
            *        \n - YUY2, 1, 2YUY (Y0U0Y1V0)
            *        \n - AYUV, 0, AYUV
            *        \n - AYUV, 1, YUXV
            */
            uint32_t            bDisplayFormatSwizzle                   : 1;
            uint32_t            deblocking_filter_override_enabled_flag : 1;
            uint32_t            pps_deblocking_filter_disabled_flag     : 1;
            uint32_t            bEnableCTULevelReport                   : 1;  // [0..1]
            uint32_t            bEnablePartialFrameUpdate               : 1;
            uint32_t            reservedbits                            : 3;        
        };
        uint32_t                PicFlags;
    };

    char                    QpY;                                    //!< QpY = pic_init_qp_minus26 + 26
    uint8_t                 diff_cu_qp_delta_depth;                 //!< Same as HEVC syntax element
    char                    pps_cb_qp_offset;                       //!< Same as HEVC syntax element
    char                    pps_cr_qp_offset;                       //!< Same as HEVC syntax element
    uint8_t                 num_tile_columns_minus1;                //!< Same as HEVC syntax element
    uint8_t                 num_tile_rows_minus1;                   //!< Same as HEVC syntax element
    /*! \brief Same as HEVC syntax elements of column_width_minus1 + 1 in PPS of HEVC bitstreams.
    *
    *    Driver expected to calculate the size of last column from frame resolution.
    */
    uint16_t                tile_column_width[20];
    /*! \brief Same as HEVC syntax elements of row_height_minus1 + 1 in PPS of HEVC bitstreams.
    *
    *    Driver expected to calculate the size of last row from frame resolution.
    */
    uint16_t                tile_row_height[22];
    /*! \brief Same as HEVC syntax element.
    *
    *    Programming note: On SKL, must be set to 0.
    */
    uint8_t                 log2_parallel_merge_level_minus2;
    uint8_t                 num_ref_idx_l0_default_active_minus1;   //!< Same as HEVC syntax element
    uint8_t                 num_ref_idx_l1_default_active_minus1;   //!< Same as HEVC syntax element
    /*! \brief Specifies the CTU bit size limit based on spec requirement, or other value for special purpose.
    *
    *    If the value is set 0, no bit size limit is checked.
    */
    uint32_t                LcuMaxBitsizeAllowed;
    /*! \brief Indicates the column or row location in block unit which is dictated by IntraRefreshBlockUnitSize from encoding capability.
    *
    *    Ignored if bEnableRollingIntraRefresh is 0.
    *    If IntraInsertionSize is equal or larger than the LCU size (CTB), IntraInsertionLocation has to be aligned with LCU boundaries. Otherwise, IntraInsertionLocation could have an offset of integer multiple of the block unit size from LCU boundary and must make sure that IntraInsertionLocation + IntraInsertionSize -1 would reside in the same LCU region.
    */
    uint16_t                IntraInsertionLocation;
    /*! \brief Indicates the number of columns or rows in block unit which is dictated by IntraRefreshBlockUnitSize from encoding capability.
    *
    *    Ignored if bEnableRollingIntraRefresh is 0.
    *    If IntraInsertionSize is equal or larger than the LCU size (CTB), it has to be multiple of LCU size. Otherwise, it can only be integer multiple of the block unit size (equal or smaller than LCU).
    *    When RateControlMode takes VCM mode. IntraInsertionSize defines the number of LCUs to be turn into Intra LCUs when bEnableRollingIntraRefresh is 1.
    */
    uint16_t                IntraInsertionSize;
    /*! \brief Indicates the Qp difference for inserted intra columns or rows.
    *
    *    Framework can use this to adjust intra Qp based on bitrate & max frame size. It is ignored if bEnableRollingIntraRefresh is 0. Value range [-8..7]. Driver will clamp it if out of range.
    */
    char                    QpDeltaForInsertedIntra;
    /*! \brief Arbitrary number set by the host decoder to use as a tag in the status report feedback data.
    *
    *   The value should not equal 0, and should be different in each call to Execute.
    */
    uint32_t                StatusReportFeedbackNumber;

    /*! \brief Same as HEVC syntax element
    *
    */
    uint8_t                 slice_pic_parameter_set_id;
    /*! \brief Same as HEVC syntax element
    *
    */
    uint8_t                 nal_unit_type;
    bool                    bUsedAsRef;

    /*! \brief Slice byte size upper limit.
    *
    *    Used when SliceSizeControl is enabled. Currently only valid for VDENC.
    */
    uint32_t                MaxSliceSizeInBytes;

    /*! \brief Number of Region Of Interest (ROI).
    *
    *    Value range is 0 to 16 inclusively. If no ROI to be set, this value shall be set to 0.
    */
    uint8_t                 NumROI;
    /*! \brief Defines ROI settings.
    *
    *    Value entries are ROI[0] up to ROI[NumROI – 1], inclusively, if NumROI > 0. And it can be ignored otherwise.
    */
    CODEC_ROI               ROI[16];
    /*! \brief Distinct delta QP values assigned to the ROI
    *
    *    Value entries are distinct and within [MinDeltaQp..MaxDeltaQp].
    */
    int8_t                  ROIDistinctDeltaQp[8];
    uint32_t                RollingIntraReferenceLocation[16];
    /*! \brief Dictates the value of delta QP for any ROI should be within [MinDeltaQp..MaxDeltaQp]
    *
    *    Applies only to BRC case.
    */
    char                    MaxDeltaQp;
    /*! \brief Dictates the value of delta QP for any ROI should be within [MinDeltaQp..MaxDeltaQp]
    *
    *    Applies only to BRC case.
    */
    char                    MinDeltaQp;
    
    union
    {
        struct
        {
            uint32_t        EnableCustomRoudingIntra : 1;
            uint32_t        RoundingOffsetIntra : 7;
            uint32_t        EnableCustomRoudingInter : 1;
            uint32_t        RoundingOffsetInter : 7;
            uint32_t        reservedbits : 16;
        } fields;
        
        uint32_t            value;
    } CustomRoundingOffsetsParams;

    /*! \brief Specifies skip frames.
    *
    *    0: Encode as normal, no skip.
    *    1: One or more frames were skipped prior to the current frame, encode the current frame as normal.  The driver will pass the NumSkipFrames and SizeSkipFrames to BRC for adjustment.
    */
    uint8_t                 SkipFrameFlag;
    /*! \brief The number of frames skipped prior to the current frame.
    *
    *    Valid when SkipFlag = 1.
    */
    uint8_t                 NumSkipFrames;
    /*! \brief Differs in meaning based on value of SkipFrameFlag
    *
    *    SkipFrameFlag = 1, the size of the skipped frames in bits.
    *    Note: Currently kernel only supports 16 bits for SizeSkipFrames.
    */
    uint32_t                SizeSkipFrames;

    uint8_t                 BRCMaxQp; //!< Specifies the maximum Qp to be used for BRC.
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
    uint8_t                 BRCMinQp;

    /*! \brief Indicates that an HMEOffset will be sent by the application in HMEOffset for each reference.
    *
    *    This offset will be added to the co-located (0, 0) location before HME search is performed.  This is only valid if HMEOffsetSupport is reported as supported as a capability, else this flag is ignored.
    */
    uint8_t                 bEnableHMEOffset;
    /*! \brief Specifies the HME offsets.
    *
    *    Curently the supported range is -128 to +127, programmed in 4x downscaled HME precision, not the original size. HMEOffset[i][j] where:
    *        \n - i – RefFrameList Index
    *        \n - j – 0 = x, 1 = y Coordinates
    *    \n So for example, HMEOffset[i] specifies the (x, y) offset corresponding to the ith entry in RefFrameList.
    *    Programming Note: The HME offset must be a multiple of 4x4 to align to the 4x4 HME block, so the driver must align the application supplied value.
    */
    int16_t                 HMEOffset[15][2];

    /*! \brief Number of dirty rectangles.
    *
    *    If no dirty rectangle to be set, this value shall be set to 0. Encoder does not have number of dirty rectangle limitation.
    */
    uint8_t                 NumDirtyRects;
    /*! \brief Buffer containing an array of NumDirtyRects number of dirty rectangle elements
    *
    *    It’s framework's responsibility to ensure that the content in non-dirty rectangle region is the same as previous frame.
    */
    PCODEC_ROI              pDirtyRect;
    /*! \brief Number of moving rectangles.
    *
    *    If no moving rectangle to be set, this value shall be set to 0.
    */
    uint8_t                 NumMoveRects;
    //MOVE_RECT             *pMoveRect;     \\!< Buffer containing an array of NumMoveRects number of moving rectangle elements.

    /*! \brief Provides a hint to encoder about the content for the encoding session.
    *
    *   Hint is of the format CODEC_CONTENT.
    */
    uint32_t                bScreenContent;

    uint16_t                LcuMaxBitsizeAllowedHigh16b;

    /*! \brief Picture parameter, Same as syntax element.
    *
    */
    uint32_t                pps_curr_pic_ref_enabled_flag;
    uint32_t                residual_adaptive_colour_transform_enabled_flag;
    uint32_t                pps_slice_act_qp_offsets_present_flag;
    uint8_t                 PredictorPaletteSize;
    uint16_t                PredictorPaletteEntries[3][128];
    char                    pps_act_y_qp_offset_plus5;
    char                    pps_act_cb_qp_offset_plus5;
    char                    pps_act_cr_qp_offset_plus3;
} CODEC_HEVC_ENCODE_PICTURE_PARAMS, *PCODEC_HEVC_ENCODE_PICTURE_PARAMS;

/*! \brief Slice-level parameters of a compressed picture for HEVC encoding.
*/
typedef struct _CODEC_HEVC_ENCODE_SLICE_PARAMS
{
    /*! \brief Same as HEVC syntax element.
    *
    *    For first slice in the picture, slice_segment_address shall be set to 0.
    */
    uint32_t            slice_segment_address;
    uint32_t            NumLCUsInSlice;                                 //!< Specifies the number of LCUs in the current slice.
    /*! \brief Specifies the surfaces of reference pictures.
    *
    *    The value of FrameIdx specifies the index of RefFrameList structure, PicFlags has no meaning.
    *    RefPicIdx[0][15] corresponds to reference list 0 and RefPicIdx[1][15] corresponds to reference list 1.
    *    Each list may contain duplicated reference picture indexes. Same as RefFrameList[] from picture parameter data structure, the bPicEntry of invalid entries should take value 0xFF.
    *    Must be same across all slices.
    */
    CODEC_PICTURE       RefPicList[2][CODEC_MAX_NUM_REF_FRAME_HEVC];
    /*! \brief Same as HEVC syntax element.
    *
    *    If num_ref_idx_active_override_flag == 0, host encoder shall set their values with num_ref_idx_l0_default_minus1.
    *    Must be same across all slices.
    */
    uint8_t             num_ref_idx_l0_active_minus1;
    /*! \brief Same as HEVC syntax element.
    *
    *    If num_ref_idx_active_override_flag == 0, host encoder shall set their values with num_ref_idx_l1_default_minus1.
    *    Must be same across all slices.
    */
    uint8_t             num_ref_idx_l1_active_minus1;

   union
    {
        struct
        {
            uint32_t        bLastSliceOfPic                         : 1;    //!< Specifies if current slice is the last slice of picture.
            uint32_t        dependent_slice_segment_flag            : 1;    //!< Same as HEVC syntax element
            uint32_t        slice_temporal_mvp_enable_flag          : 1;    //!< Same as HEVC syntax element
            /*! \brief Same as HEVC syntax element.
            *
            *    HEVC has three slice types, B, P, and I slices, and their corresponding values are 0, 1, and 2.
            */
            uint32_t        slice_type                              : 2;
            uint32_t        slice_sao_luma_flag                     : 1;    //!< Same as HEVC syntax element
            uint32_t        slice_sao_chroma_flag                   : 1;    //!< Same as HEVC syntax element
            /*! \brief Same as HEVC syntax element.
            *
            *    Programming note: On SKL, must be 0.
            */
            uint32_t        mvd_l1_zero_flag                        : 1;
            uint32_t        cabac_init_flag                         : 1;    //!< Same as HEVC syntax element
            /*! \brief Same as HEVC syntax element.
            *
            *    Affects the decoding process accordingly if it appears in slice header of the bitstream. Otherwise, host encode should set it to be the value of pic_disable_deblocking_filter_flag in picture parameter set bit stream.
            */
            uint32_t        slice_deblocking_filter_disable_flag    : 1;
            uint32_t        collocated_from_l0_flag                 : 1;    //!< Same as HEVC syntax element
            uint32_t                                                : 21;
        };
        uint32_t        SliceFlags;
    };

    /*! \brief Same as HEVC syntax element.
    *
    *    Must be the same across all slices.
    */
    char                slice_qp_delta;
    char                slice_cb_qp_offset;             //!< Same as HEVC syntax element, [-12..12]
    char                slice_cr_qp_offset;             //!< Same as HEVC syntax element, [-12..12]
    char                beta_offset_div2;               //!< Same as HEVC syntax element
    char                tc_offset_div2;                 //!< Same as HEVC syntax element
    /*! \brief Same as HEVC syntax element.
    *
    *    Specifies the base 2 logarithm of the denominator for all luma weighting factors. Value range: 0 to 7, inclusive.
    */
    uint8_t             luma_log2_weight_denom;
    /*! \brief Same as HEVC syntax element.
    *
    *    Framework is expected to ensure that the spec variable ChromaLog2WeightDenom, which is luma_log2_weight_denom plus delta_chroma_log2_weight_denom, specifies the base 2 logarithm of the denominator for all chroma weighting factors, with value range: 0 to 7, inclusive.
    */
    char                delta_chroma_log2_weight_denom;
    /*! \brief Specifies the additive offsets applied to the luma prediction values for list 0 and list 1.
    *
    *    Its values correspond to HEVC syntax elements luma_offset_l0, and luma_offset_l1. In luma_offset[i][j]:
    *        \n - i equal to 0 is for reference picture list 0, equal to 1 is for reference picture list 1.
    *        \n - j is for reference list index [0..14].
    *    \n Value range: [-128..127], inclusive. When luma_weight_l0_flag or luma_weight_l1_flag take value 0, corresponding luma_offset should be set 0.
    */
    char                luma_offset[2][CODEC_MAX_NUM_REF_FRAME_HEVC];
    /*! \brief Specifies the difference of the weighting factor applied to the luma prediction value for list 0/1 prediction using RefPicList.
    *
    *    Corresponds to syntax element delta_luma_weight_l0, and delta_luma_weight_l1. In delta_luma_weight[i][j]:
    *        \n - i equal to 0 is for reference picture list 0, equal to 1 is for reference picture list 1.
    *        \n - j is for reference list index [0..14].
    *    \n Value range: [-127..128], inclusive. When luma_weight_l0_flag or luma_weight_l1_flag take value 0, corresponding delta_luma_weight should be set 0.
    */
    char                delta_luma_weight[2][CODEC_MAX_NUM_REF_FRAME_HEVC];
    /*! \brief Corresponds to HEVC spec variable ChromaOffsetL0 and ChromaOffsetL1.
    *
    *    In chroma_offset[i][j][k]:
    *        \n - i equal to 0 is for reference picture list 0, equal to 1 is for reference picture list 1.
    *        \n - j is for reference list index [0..14].
    *        \n - k  equal to 0 is for Cb, and equal to 1 is for Cr.
    *    \n Value range: [-128..127], inclusive. When chroma_weight_l0_flag or chroma_weight_l1_flag take value 0, corresponding chroma_offset should be set 0.
    */
    char                chroma_offset[2][CODEC_MAX_NUM_REF_FRAME_HEVC][2];
    /*! \brief Specifies the difference of the weighting factor applied to the chroma prediction values for list 0/1 prediction using RefPicList.
    *
    *    Corresponds to HEVC syntax elements delta_chroma_weight_l0 and delta_chroma_weight_l1. In delta_chroma_weight[i][j][k]:
    *       \n - i equal to 0 is for reference picture list 0, equal to 1 is for reference picture list 1.
    *       \n - j is for reference list index [0..14].
    *       \n - k  equal to 0 is for Cb, and equal to 1 is for Cr.
    *    \n Value range: [-127..128], inclusive. When chroma_weight_l0_flag or chroma_weight_l1_flag take value 0, corresponding delta_chroma_weight should be set 0.
    */
    char                delta_chroma_weight[2][CODEC_MAX_NUM_REF_FRAME_HEVC][2];

    /*! \brief Specifies the maximum number of merging MVP candidates supported in the slice.
    *
    *    Corresponds to HEVC spec variable MaxNumMergeCand.
    */
    uint8_t             MaxNumMergeCand;
    /*! \brief ????
    *
    *    ????
    */
    uint16_t            slice_id;
    /*! \brief ????
    *
    *    ????
    */
    uint16_t            BitLengthSliceHeaderStartingPortion;
    /*! \brief ????
    *
    *    ????
    */
    uint32_t            SliceHeaderByteOffset;
    /*! \brief ????
    *
    *    ????
    */
    uint32_t            SliceQpDeltaBitOffset;
    /*! \brief Bit offset of syntax element pred_weight_table() in slice segment header.
    *
    *    It aligns with the starting position of the current packed slice header. It is used when encoder prefers to override the weighted prediction parameters passed in from DDI.
    */
    uint32_t            PredWeightTableBitOffset;
    /*! \brief Bit length of syntax element pred_weight_table() in slice segment header.
    *
    *    It is used when encoder prefers to override the weighted prediction parameters passed in from DDI.
    */
    uint32_t            PredWeightTableBitLength;
    /*! \brief Bit offset of syntax element slice_sao_luma_flag in slice segment header.
    *
    *    It aligns with the starting position of the current packed slice header. BRC kernel may use it to update the slice_sao_luma_flag and slice_sao_chroma_flag values. The value is invalid if SAO_enabled_flag is false.
    */
    uint32_t            SliceSAOFlagBitOffset;
} CODEC_HEVC_ENCODE_SLICE_PARAMS, *PCODEC_HEVC_ENCODE_SLICE_PARAMS;

//!
//! \struct    CodecEncodeHevcFeiPicParams
//! \brief     Codec encode HEVC FEI pic params
//!
struct CodecEncodeHevcFeiPicParams
{
    MOS_RESOURCE                resCTBCtrl;              // input CTB control buffer
    MOS_RESOURCE                resCTBCmd;               // ENC CTB cmd output buffer or PAK CTB cmd input buffer
    MOS_RESOURCE                resCURecord;             // ENC CU record output buffer or PAK CU record input buffer
    MOS_RESOURCE                resMVPredictor;          // input external MV predictor surface
    MOS_RESOURCE                resCTBQp;                // input QP per CTB surface
    MOS_RESOURCE                resDistortion;           // ENC or ENC_PAK Distortion output surface

    uint32_t                    NumMVPredictorsL0;
    uint32_t                    NumMVPredictorsL1;

    bool                        bCTBCmdCuRecordEnable;
    bool                        bDistortionEnable;

    /** \brief control parameters */
    uint32_t                    SearchPath;
    uint32_t                    LenSP;
    uint32_t                    MultiPredL0;
    uint32_t                    MultiPredL1;
    uint32_t                    SubPelMode;
    uint32_t                    MVPredictorInput;

    bool                        AdaptiveSearch;
    bool                        bPerBlockQP;
    bool                        bPerCTBInput;
    bool                        bColocatedCTBDistortion;
    bool                        bForceLCUSplit;
    bool                        bEnableCU64Check;
    bool                        bEnableCU64AmpCheck;
    bool                        bCU64SkipCheckOnly;

    uint32_t                    RefWidth;
    uint32_t                    RefHeight;
    uint32_t                    SearchWindow;
    uint32_t                    MaxNumIMESearchCenter;
    uint32_t                    FastIntraMode;
    uint32_t                    NumConcurrentEncFramePartition;

    /** \brief add for mutlple pass pak */
    uint32_t                    dwMaxFrameSize;
    uint32_t                    dwNumPasses;    //number of QPs
    uint8_t                    *pDeltaQp;       //list of detla QPs
};
#endif  // __CODEC_DEF_ENCODE_HEVC_H__
