/*
* Copyright (c) 2019-2024, Intel Corporation
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
//! \file     codec_def_encode_av1.h
//! \brief    Defines encode AV1 types and macros shared by CodecHal, MHW, and DDI layer
//! \details  Applies to AV1 encode only. Should not contain any DDI specific code.
//!
#ifndef __CODEC_DEF_ENCODE_AV1_H__
#define __CODEC_DEF_ENCODE_AV1_H__

#include "codec_def_common_av1.h"
#include "codec_def_common_encode.h"
#include "codec_def_encode_avc.h"
#include "codec_def_encode.h"

#define CODECHAL_AV1_FRAME_HEADER_SIZE           8192
#define CODECHAL_ENCODE_AV1_PAK_INSERT_UNCOMPRESSED_HEADER 80
#define CODEC_AV1_NUM_REF_FRAMES 8
#define CODEC_AV1_NUM_REFL0P_FRAMES 3
#define CODEC_AV1_NUM_REFL0B_FRAMES 2
#define CODEC_AV1_NUM_REFL1B_FRAMES 1
#define CODEC_AV1_NUM_UNCOMPRESSED_SURFACE 127  // Maximum number of uncompressed decoded buffers that driver supports
#define ENCODE_VDENC_AV1_MAX_DIRTYRECT     16
#define ENCODE_VDENC_AV1_MAX_MOVERECT      16
#define ENCODE_AV1_MAX_NUM_TEMPORAL_LAYERS 8
#define ENCODE_VDENC_AV1_MAX_TILE_GROUP_NUM      128
#define ENCODE_MIM_SEGID_BLOCK_SIZE 32
#define ENCODE_AV1_MIN_ICQ_QUALITYFACTOR      0
#define ENCODE_AV1_MAX_ICQ_QUALITYFACTOR      255
#define ENCODE_AV1_ORDER_HINT_SIZE            256
// for VAConfigAttribValEncAV1Ext2.tx_mode_support
#define AV1_TX_MODE_ONLY_4X4_SUPPORTED 0x01
#define AV1_TX_MODE_LARGEST_SUPPORTED 0x02
#define AV1_TX_MODE_SELECT_SUPPORTED 0x04
#define AV1_MAX_NUM_OF_BATCH_BUFFER 5
#define AV1_MAX_NUM_OF_SEGMENTS 8
constexpr uint32_t TILE_SIZE_BYTES = 4;
const uint8_t OBU_LEB128_SIZE  = 4;
const uint8_t LEB128_BYTE_MASK = 0x7f;
const uint8_t av1CuReordSize   = 32;
static const uint32_t codechalEncodeModeAv1    = CODECHAL_ENCODE_MODE_AV1;           // codechal encode mode AV1

enum {
    AV1_Y_DC,
    AV1_U_DC,
    AV1_U_AC,
    AV1_V_DC,
    AV1_V_AC,
    AV1_NUM_COEFF_QUANTIZERS
};

typedef enum
{
    AVP_CHROMA_FORMAT_MONOCHROME = 0,
    AVP_CHROMA_FORMAT_YUV420 = 1,
    AVP_CHROMA_FORMAT_YUV422 = 2,
    AVP_CHROMA_FORMAT_YUV444 = 3
} AVP_CHROMA_FORMAT_IDC;

typedef enum
{
    SEQUENCE_HEADER_OBU        = 0,
    TEMPORAL_DELIMITER_OBU     = 1,
    PADDING_OBU                = 2,
    META_DATA_OBU              = 3,
    FRAME_HEADER_OBU           = 4,
    FRAME_OBU                  = 5,
    TILE_GROUP_OBU             = 6,
    TILE_LIST_OBU              = 7,
    REDUNDANT_FRAME_HEADER_OBU = 8,
    MAX_NUM_OBU_TYPES          = 9
} AV1_OBU_TYPES;

typedef enum
{
    AV1_ENCODED_BIT_DEPTH_8 = 0,
    AV1_ENCODED_BIT_DEPTH_10 = 1
} AV1_ENCODED_BIT_DEPTH;

typedef enum  //VDEnc Frame Types
{
    AV1_I_FRAME   = 0,  // I (Intra)
    AV1_P_FRAME   = 1,  // P (Inter/Pred)
    AV1_B_FRAME   = 2,  // B (BiPred/Random Access)
    AV1_GPB_FRAME = 3,  // B (GPB/LowDelay)
} VDEncFrameType;

enum TABLE_A1_COLS_INDEX
{
    MAX_PIC_SIZE_INDEX = 0,
    MAX_H_SIZE_INDEX,
    MAX_V_SIZE_INDEX,
    MAX_DISPLAY_RATE_INDEX,
    MAX_DECODE_RATE_INDEX,
    TABLE_A1_COLS_NUM,
};

enum TABLE_A2_COLS_INDEX
{
    MAX_HEADER_RATE_INDEX = 0,
    MAIN_BPS_INDEX,
    HIGH_BPS_INDEX,
    MAIN_CR_INDEX,
    HIGH_CR_INDEX,
    MAX_TILES_INDEX,
    MAX_TILE_COLS_INDEX,
    TABLE_A2_COLS_NUM,
};
const uint64_t TableA1[][TABLE_A1_COLS_NUM] =
    {
        //  Level   |MaxPicSize | MaxHSize | MaxVSize | MaxDiaplayRate | MaxDecodeRate
        /*  2.0 */ {147456, 2048, 1152, 4423680, 5529600},
        /*  2.1 */ {278784, 2816, 1584, 8363520, 10454400},
        /*  3.0 */ {665856, 4352, 2448, 19975680, 24969600},
        /*  3.1 */ {1065024, 5504, 3096, 31950720, 39938400},
        /*  4.0 */ {2359296, 6144, 3456, 70778880, 77856768},
        /*  4.1 */ {2359296, 6144, 3456, 141557760, 155713536},
        /*  5.0 */ {8912896, 8192, 4352, 267386880, 273705200},
        /*  5.1 */ {8912896, 8192, 4352, 534773760, 547430400},
        /*  5.2 */ {8912896, 8192, 4352, 1069547520, 1094860800},
        /*  5.3 */ {8912896, 8192, 4352, 1069547520, 1176502272},
        /*  6.0 */ {35651584, 16384, 8704, 1069547520, 1176502272},
        /*  6.1 */ {35651584, 16384, 8704, 2139095040, 2189721600},
        /*  6.2 */ {35651584, 16384, 8704, 4278190080, 4379443200},
        /*  6.3 */ {35651584, 16384, 8704, 4278190080, 4706009088},
};
const uint32_t TableA2[][TABLE_A2_COLS_NUM] =
    {
        //  Level   | MaxHeaderRate |    Mainbps    |    Highbps    | MainCR | HighCR | MaxTiles | MaxTileCols
        /*  2.0  */ {150, 1500000, 0, 2, 0, 8, 4},
        /*  2.1  */ {150, 3000000, 0, 2, 0, 8, 4},
        /*  3.0  */ {150, 6000000, 0, 2, 0, 16, 6},
        /*  3.1  */ {150, 10000000, 0, 2, 0, 16, 6},
        /*  4.0  */ {300, 12000000, 30000000, 4, 4, 32, 8},
        /*  4.1  */ {300, 20000000, 50000000, 4, 4, 32, 8},
        /*  5.0  */ {300, 30000000, 100000000, 6, 4, 64, 8},
        /*  5.1  */ {300, 40000000, 160000000, 8, 4, 64, 8},
        /*  5.2  */ {300, 60000000, 240000000, 8, 4, 64, 8},
        /*  5.3  */ {300, 60000000, 240000000, 8, 4, 64, 8},
        /*  6.0  */ {300, 60000000, 240000000, 8, 4, 128, 16},
        /*  6.1  */ {300, 100000000, 480000000, 8, 4, 128, 16},
        /*  6.2  */ {300, 160000000, 800000000, 8, 4, 128, 16},
        /*  6.3  */ {300, 160000000, 800000000, 8, 4, 128, 16},
};
const uint32_t RdMultLUT[2][256] = 
    {
        {58, 234, 234, 297, 366, 443, 528,
        528, 619, 718, 825, 938, 1059, 1188, 1323, 1323, 1466, 1617, 1774, 
        1939, 2112, 2291, 2478, 2478, 2673, 2874, 3083, 3300, 3523, 3754, 
        3754, 3993, 4238, 4491, 4752, 5019, 5294, 5294, 5577, 5866, 6163, 
        6468, 6779, 6779, 7098, 7425, 7758, 8099, 8448, 8448, 8803, 9166, 
        9537, 9914, 10299, 10299, 10692, 11091, 11498, 11913, 11913, 12334, 
        12763, 13200, 13643, 14094, 14094, 14553, 15018, 15491, 15972, 
        15972, 16459, 16954, 17457, 17966, 17966, 18483, 19008, 19539, 
        20078, 20078, 20625, 21178, 21739, 22308, 22308, 22883, 23466, 
        24057, 24057, 24654, 25259, 25872, 26491, 26491, 27753, 28394, 
        29700, 31034, 31713, 33091, 33792, 35214, 35937, 37403, 38148, 
        39658, 40425, 41979, 42768, 44366, 45177, 46819, 47652, 49338, 
        50193, 51054, 52800, 53683, 55473, 57291, 59139, 61017, 62923, 
        65838, 67818, 69828, 71866, 73934, 76032, 78158, 80314, 82500, 
        84714, 86958, 89232, 91534, 95043, 98618, 101038, 104723, 108474, 
        111012, 114873, 118800, 121454, 125491, 128219, 132366, 135168, 
        139425, 145203, 149614, 154091, 158634, 163243, 167918, 172659, 
        177466, 182339, 187278, 193966, 199059, 205953, 211200, 216513, 
        223699, 229166, 234699, 242179, 249777, 257491, 265323, 271274, 
        279312, 287466, 295738, 304128, 312634, 321258, 330000, 338858, 
        350097, 359219, 368459, 380174, 389678, 399300, 411491, 423866, 
        433898, 446603, 459492, 472563, 485818, 499257, 512878, 526683, 
        540672, 554843, 572091, 586666, 604398, 619377, 637593, 656073, 
        674817, 693825, 713097, 732633, 755758, 779243, 799659, 827291, 
        851854, 876777, 905699, 935091, 964953, 999108, 1029966, 1065243, 
        1105137, 1145763, 1187123, 1229217, 1276366, 1328814, 1382318, 
        1436878, 1501866, 1568292, 1636154, 1715472, 1796666, 1884993, 
        1986218, 2090091, 2202291, 2323258, 2459457, 2605713, 2768923, 
        2943658, 3137291, 3344091, 3579194, 3829774, 4104334, 4420548,
        4756843, 5140138, 5565354, 6026254, 6544618},
        {4, 19, 23, 39, 52, 66, 92, 111, 143, 180, 220, 265, 314, 367, 
        424, 506, 573, 644, 745, 825, 939, 1060, 1155, 1289, 1394, 1541,
        1695, 1856, 1982, 2156, 2338, 2527, 2723, 2926, 3084, 3300, 3524,
        3755, 3993, 4239, 4492, 4686, 4952, 5225, 5506, 5794, 6089, 6315,
        6623, 6938, 7261, 7591, 7843, 8186, 8536, 8894, 9167, 9537, 9915,
        10300, 10593, 10991, 11396, 11705, 12123, 12441, 12872, 13310, 13644,
        14095, 14438, 14902, 15373, 15731, 16215, 16583, 17080, 17457, 17967, 
        18354, 18876, 19273, 19674, 20215, 20625, 21179, 21599, 22023, 22595,
        23029, 23614, 24057, 24505, 25108, 25565, 26026, 26961, 28073, 29044,
        30031, 31204, 32227, 33266, 34322, 35575, 36667, 37775, 38900, 40041,
        41199, 42373, 43564, 44771, 45995, 47235, 48492, 49765, 51055, 52361,
        53684, 55023, 57063, 58907, 61017, 63164, 65104, 67321, 69323, 71610,
        73675, 76032, 78159, 80315, 82775, 84994, 87241, 89518, 92115, 95044,
        98318, 101648, 104724, 108160, 111651, 114873, 118141, 121789, 125153,
        128563, 132019, 135873, 140141, 144839, 149245, 153716, 158254, 163244,
        167919, 172660, 177467, 181931, 188108, 193967, 199487, 205519, 211640,
        217852, 223700, 229625, 236093, 243123, 250256, 257978, 265324, 272273,
        279818, 287467, 296260, 304656, 313706, 322345, 331101, 339974, 350097,
        359794, 370205, 380175, 390875, 401117, 412721, 424490, 435793, 447884,
        459492, 472564, 485819, 499257, 512879, 526684, 541376, 556985, 572092,
        587400, 604399, 621640, 639123, 656073, 675604, 694623, 714715, 735094,
        756591, 779244, 802230, 827292, 852739, 878571, 907523, 936018, 966835, 
        999108, 1032884, 1068210, 1106144, 1145764, 1187124, 1232404, 1279614,
        1331023, 1384571, 1441473, 1503040, 1568292, 1639831, 1716726, 1799234,
        1888939, 1986219, 2090092, 2205134, 2329100, 2465467, 2610352, 2772111,
        2946945, 3140684, 3349346, 3581006, 3831649, 4112097, 4424575, 4763110,
        5142310, 5567614, 4289813442, 4290334454}
};

//DDI version 0.20
typedef struct _CODEC_AV1_ENCODE_SEQUENCE_PARAMS
{
    uint8_t     seq_profile;    // [0]
    uint8_t     seq_level_idx;  // [0..23, 31]
    uint16_t    GopPicSize;
    uint8_t     TargetUsage;
    uint8_t     RateControlMethod;
    uint8_t     GopRefDist;                          
    uint8_t     GopOptFlag                      : 2; 
    uint8_t     reserved6b                      : 6; 
    uint32_t    TargetBitRate[8];   // One per temporal layer
    uint32_t    MaxBitRate;
    uint32_t    MinBitRate;
    uint32_t    InitVBVBufferFullnessInBit;
    uint32_t    VBVBufferSizeInBit;
    uint32_t    OptimalVBVBufferLevelInBit;
    uint32_t    UpperVBVBufferLevelThresholdInBit;
    uint32_t    LowerVBVBufferLevelThresholdInBit;

    union
    {
        struct
        {
            uint32_t    ResetBRC                : 1;
            uint32_t    StillPicture            : 1;
            uint32_t    UseRawReconRef          : 1;
            uint32_t    DisplayFormatSwizzle    : 1;    //[0]
            uint32_t    bLookAheadPhase         : 1; 
            uint32_t    HierarchicalFlag        : 1; 
            uint32_t    RGBInputStudioRange     : 1;    // [0, 1]
            uint32_t    ConvertedYUVStudioRange : 1;    // [0, 1]
            uint32_t    Reserved0               : 24;
        } fields;
        uint32_t    value;
    } SeqFlags;

    uint32_t    UserMaxIFrameSize;
    uint32_t    UserMaxPBFrameSize;
    FRAMERATE   FrameRate[8];   // One per temporal layer
    uint8_t NumTemporalLayersMinus1;
    uint8_t ICQQualityFactor;   // [0..255], with 0 being the best quality

    ENCODE_INPUT_COLORSPACE     InputColorSpace;
    ENCODE_SCENARIO             ScenarioInfo;
    ENCODE_CONTENT              ContentInfo;
    ENCODE_FRAMESIZE_TOLERANCE  FrameSizeTolerance;
    uint16_t                    SlidingWindowSize;
    uint32_t                    MaxBitRatePerSlidingWindow;
    uint32_t                    MinBitRatePerSlidingWindow;

    union
    {
        struct
        {
            uint32_t    enable_order_hint           : 1;
            uint32_t    enable_superres             : 1;
            uint32_t    enable_cdef                 : 1;
            uint32_t    enable_restoration          : 1;
            uint32_t    enable_warped_motion        : 1;    //[0]
            uint32_t    enable_filter_intra         : 1;
            uint32_t    enable_intra_edge_filter    : 1;
            uint32_t    enable_interintra_compound  : 1;
            uint32_t    enable_masked_compound      : 1;
            uint32_t    enable_dual_filter          : 1;
            uint32_t    enable_jnt_comp             : 1;
            uint32_t    enable_ref_frame_mvs        : 1;
            uint32_t    Reserved3                   : 20;
        } fields;
        uint32_t    value;
    } CodingToolFlags;

    uint8_t     order_hint_bits_minus_1;    // [0..7]
    union
    {
        uint8_t LookaheadDepth;               // [0..100]
        uint8_t TargetFrameSizeConfidence;    // [0..100]
    };

    uint8_t     Reserved8b2;
    uint8_t     Reserved8b3;
    uint32_t    Reserved32b[16];
} CODEC_AV1_ENCODE_SEQUENCE_PARAMS, *PCODEC_AV1_ENCODE_SEQUENCE_PARAMS;

struct CODEC_Intel_Seg_AV1
{
    union
    {
        struct
        {
            uint8_t   segmentation_enabled    : 1;
            uint8_t   SegmentNumber           : 4;    //[0..8]
            uint8_t   update_map              : 1;
            uint8_t   temporal_update         : 1;
            uint8_t   Reserved0               : 1;
        } fields;
        uint8_t   value;
    } SegmentFlags;

    int16_t     feature_data[8][8];
    uint8_t     feature_mask[8];
    uint32_t    Reserved1[4];
};

struct CODEC_Warped_Motion_Params_AV1
{
    uint32_t    wmtype;
    int32_t     wmmat[8];
    int8_t      invalid;
};

struct CODEC_Ref_Frame_Ctrl_AV1
{
    union
    {
        struct
        {
            uint32_t search_idx0 : 3;
            uint32_t search_idx1 : 3;
            uint32_t search_idx2 : 3;
            uint32_t search_idx3 : 3;
            uint32_t search_idx4 : 3;
            uint32_t search_idx5 : 3;
            uint32_t search_idx6 : 3;
            uint32_t ReservedField : 11;  //[0]
        } fields;
        uint32_t value;
    } RefFrameCtrl;
};

/*! \brief Provides the picture-level parameters of a compressed picture for AV1 decoding.
*/
typedef struct _CODEC_AV1_ENCODE_PICTURE_PARAMS
{
    uint16_t        frame_width_minus1;     // [15..2^16-1]
    uint16_t        frame_height_minus1;    // [15..2^16-1]
    uint8_t         NumTileGroupsMinus1;    // [0..255]
    uint8_t         Reserved8b;              // [0]
    CODEC_PICTURE   CurrOriginalPic;        // [0..127]
    CODEC_PICTURE   CurrReconstructedPic;   // [0..11]
    CODEC_PICTURE   RefFrameList [8];       // [0..11, 0xFF]
    uint8_t         ref_frame_idx[7];       // [0..6]
    uint8_t         HierarchLevelPlus1;
    uint8_t         primary_ref_frame;      // [0..7]
    uint8_t         AdaptiveTUEnabled;
    uint8_t         Reserved8b4;
    uint8_t         order_hint;

    CODEC_Ref_Frame_Ctrl_AV1 ref_frame_ctrl_l0;
    CODEC_Ref_Frame_Ctrl_AV1 ref_frame_ctrl_l1;

    union
    {
        struct
        {
            uint32_t    frame_type                      : 2;    // [0..3]
            uint32_t    error_resilient_mode            : 1;    // [0..1]
            uint32_t    disable_cdf_update              : 1;    // [0..1]
            uint32_t    use_superres                    : 1;    // [0..1]
            uint32_t    allow_high_precision_mv         : 1;    // [0..1]
            uint32_t    use_ref_frame_mvs               : 1;    // [0..1]
            uint32_t    disable_frame_end_update_cdf    : 1;    // [0..1]
            uint32_t    reduced_tx_set_used             : 1;    // [0..1]
            uint32_t    reserved1b                      : 1;    // [0..1]
            uint32_t    SegIdBlockSize                  : 2;    // [0..3]
            uint32_t    EnableFrameOBU                  : 1;
            uint32_t    DisableFrameRecon               : 1;
            uint32_t    LongTermReference               : 1;
            uint32_t    allow_intrabc                   : 1;
            uint32_t    PaletteModeEnable               : 1;
            uint32_t    Reserved2                       : 15;
        } fields;
        uint32_t    value;
    } PicFlags;

    // deblocking filter
    uint8_t   filter_level[2];    // [0..63]
    uint8_t   filter_level_u;     // [0..63]
    uint8_t   filter_level_v;     // [0..63]

    union
    {
        struct
        {
            uint8_t   sharpness_level         : 3;    // [0..7]
            uint8_t   mode_ref_delta_enabled  : 1;
            uint8_t   mode_ref_delta_update   : 1;
            uint8_t   Reserved3               : 3;    // [0]
        } fields;
        uint8_t   value;
    } cLoopFilterInfoFlags;

    uint8_t   superres_scale_denominator; // [9..16]
    uint8_t   interp_filter;              // [0..9]
    uint8_t   Reserved4;                  // [0]
    int8_t    ref_deltas[8];              // [-63..63]
    int8_t    mode_deltas[2];             // [-63..63]

    // quantization
    uint16_t  base_qindex;    // [0..255]
    int8_t    y_dc_delta_q;   // [-15..15]
    int8_t    u_dc_delta_q;   // [-63..63]
    int8_t    u_ac_delta_q;   // [-63..63]
    int8_t    v_dc_delta_q;   // [-63..63]
    int8_t    v_ac_delta_q;   // [-63..63]
    uint8_t   MinBaseQIndex;  // [1..255]
    uint8_t   MaxBaseQIndex;  // [1..255]
    uint8_t   Reserved5;      // [0]

    // quantization_matrix
    union {
        struct {
            uint16_t  using_qmatrix   : 1;   // verify if supported   

            // valid only when using_qmatrix is 1.
            uint16_t  qm_y            : 4;   // [0..15]
            uint16_t  qm_u            : 4;   // [0..15]
            uint16_t  qm_v            : 4;   // [0..15]
            uint16_t  Reserved6       : 3;   // [0]
        } fields;
        uint16_t  value;
    } wQMatrixFlags;

    uint16_t  Reserved7;   // [0]

    union
    {
        struct
        {
            // delta_q parameters
            uint32_t    delta_q_present_flag    : 1;    // [0..1]
            uint32_t    log2_delta_q_res        : 2;    // [0..3]

            // delta_lf parameters
            uint32_t    delta_lf_present_flag   : 1;    // [0..1]
            uint32_t    log2_delta_lf_res       : 2;    // [0..3]
            uint32_t    delta_lf_multi          : 1;    // [0..1]

            // read_tx_mode
            uint32_t    tx_mode                 : 2;    // [0..3]

            // read_frame_reference_mode
            uint32_t    reference_mode          : 2;    // [0..3]
            uint32_t    reduced_tx_set_used     : 1;    // [0..1]

            uint32_t    skip_mode_present       : 1;    // [0..1]
            uint32_t    Reserved8               : 19;   // [0]
        } fields;
        uint32_t    value;
    } dwModeControlFlags;

    CODEC_Intel_Seg_AV1 stAV1Segments;
    uint16_t            tile_cols;  
    uint16_t            width_in_sbs_minus_1[63];
    uint16_t            tile_rows;
    uint16_t            height_in_sbs_minus_1[63];
    uint8_t             context_update_tile_id; // [0..127]
    uint8_t             temporal_id;

    // CDEF
    uint8_t   cdef_damping_minus_3;   // [0..3]
    uint8_t   cdef_bits;              // [0..3]
    uint8_t   cdef_y_strengths[8];    // [0..63]
    uint8_t   cdef_uv_strengths[8];   // [0..63]

    union
    {
        struct
        {
            uint16_t  yframe_restoration_type     : 2;    // [0..3] 
            uint16_t  cbframe_restoration_type    : 2;    // [0..3] 
            uint16_t  crframe_restoration_type    : 2;    // [0..3] 
            uint16_t  lr_unit_shift               : 2;    // [0..2] 
            uint16_t  lr_uv_shift                 : 1;    // [0..1] 
            uint16_t  Reserved9                   : 7;    // [0]        
        } fields;
        uint16_t  value;
    } LoopRestorationFlags;

    // global motion
    CODEC_Warped_Motion_Params_AV1   wm[7];

    uint32_t    QIndexBitOffset;
    uint32_t    SegmentationBitOffset;
    uint32_t    LoopFilterParamsBitOffset;
    uint32_t    CDEFParamsBitOffset;
    uint8_t     CDEFParamsSizeInBits;
    uint8_t     reserved8bits0;
    uint16_t    FrameHdrOBUSizeInBits;
    uint32_t    FrameHdrOBUSizeByteOffset;
    uint32_t    StatusReportFeedbackNumber;

    // Tile Group OBU header
    union
    {
        struct
        {
            uint8_t obu_extension_flag : 1;  // [0..1]
            uint8_t obu_has_size_field : 1;  // [0..1]
            uint8_t temporal_id : 3;         // [0..7]
            uint8_t spatial_id : 2;          // [0..3]
            uint8_t ReservedField : 1;       // [0]
        } fields;
        uint8_t value;
    } TileGroupOBUHdrInfo; //DDI 0.06

    uint8_t reserved8bs1;  // [0]
    uint8_t reserved8bs2;  // [0]
    // Skip Frames
    uint8_t     NumSkipFrames;
    int32_t     FrameSizeReducedInBytes;

    uint16_t    NumDirtyRects;
    ENCODE_RECT *pDirtyRect;
    uint16_t    NumMoveRects;
    MOVE_RECT   *pMoveRect;
    uint32_t    InputType;
    uint32_t    TargetFrameSize;
    uint8_t     QpModulationStrength;

    /*! \brief quality information report enable flags.
    */
    union
    {
        struct
        {
            uint8_t enable_frame : 1;
            uint8_t enable_block : 1;
            uint8_t reserved     : 6;
        } fields;
        uint8_t value;
    } QualityInfoSupportFlags;
    uint8_t     reserved8b[2];

    union
    {
        void    *pBlkQualityInfo;
        uint32_t Reserved9[2];
    };

    uint32_t    Reserved10[12];
} CODEC_AV1_ENCODE_PICTURE_PARAMS, *PCODEC_AV1_ENCODE_PICTURE_PARAMS;

/*! \brief Slice-level parameters of a compressed picture for AV1 encoding.
*/
typedef struct _CODEC_AV1_ENCODE_TILE_GROUP_PARAMS
{
    uint8_t  TileGroupStart;
    uint8_t  TileGroupEnd;
    uint16_t Reserved16b;
    uint32_t Reserved32b[9];
} CODEC_AV1_ENCODE_TILE_GROUP_PARAMS, *PCODEC_AV1_ENCODE_TILE_GROUP_PARAMS;

typedef struct _CODEC_AV1_ENCODE_PACKEDHEADER_DATA
{
    uint8_t *pData;
    uint32_t   BufferSize;
    uint32_t   DataLength;
    uint32_t   DataOffset;
    uint32_t   SkipEmulationByteCount;
    uint32_t   Reserved;
} CODEC_AV1ENCODE_PACKEDHEADER_DATA, *PCODEC_AV1ENCODE_PACKEDHEADER_DATA;

#define MAX_BSTRUCTURE_GOP_SIZE 16
#define MAX_BSTRUCTURE_REF_NUM 8
#define MAX_TLEVEL  16
#define AV1_NUM_OF_REF_LF_DELTAS 8
#define AV1_NUM_OF_MODE_LF_DELTAS 2
#define AV1_NUM_OF_DUAL_CTX 2
struct EncodeAv1Par
{

    /* Configurable parameters */
    uint32_t NumFrames;
    uint32_t GopRefDist;
    uint32_t GopOptFlag;
    uint32_t Width;
    uint32_t Height;
    uint32_t FrameRateNom;
    uint32_t FrameRateDeNom;
    uint32_t EncMode;
    uint32_t NumP;

    uint32_t Profile;
    uint32_t InternalBitDepth;
    uint32_t InputBitDepth;
    uint32_t OutputBitDepth;
    uint32_t ChromaFormatIDC;
    uint32_t InputFileFormat;    
    uint32_t AdaptiveRounding;
    uint32_t ColorSpace;
    uint32_t DisableCdfUpdate;
    uint32_t EnableSeg;
    uint32_t SegMapUpdateCycle;
    uint32_t SegTemporalUpdate;
    uint32_t BaseKeyFrameQP;
    uint32_t BasePFrameQP;
    uint32_t KeyFrameQP[8];      /* Fixed QP for K frame, all segments */
    uint32_t PFrameQP[8];        /* Fixed QP for P frame, all segments */
    int32_t KeyCTQPDelta[AV1_NUM_COEFF_QUANTIZERS]; // Key frame QP deltas for coefficient type [YAC, UDC, UAC]
    int32_t PCTQPDelta[AV1_NUM_COEFF_QUANTIZERS];   //P frame  QP deltas for coefficient type [YAC, UDC, UAC]
    uint32_t BaseLoopFilterLevel[2];    /* Loop filter level, all segments */
    uint32_t SegLoopFilterLevel[2][8];  /* Loop filter level, all segments */
    int32_t RefTypeLFDelta[AV1_NUM_OF_REF_LF_DELTAS];  // Loop filter level delta for ref types
    int32_t MBTypeLFDelta[AV1_NUM_OF_MODE_LF_DELTAS];  // Look filter level delta for MB types
    int32_t SegmentRef[8];     //Fixed reference per segment: -1 = not fixed
    uint32_t SegmentSkip[8];   //Segment skip flag, per segment.
    uint32_t SegmentGlobalMV[8];     //Segment global MV flag, per segment.
    uint32_t NumTileCols;
    uint32_t NumTileRows;
    uint32_t UniformSpacingFlag;
    uint32_t TileWidths[av1MaxTileColumn];
    uint32_t TileHeights[av1MaxTileColumn];
    uint32_t Log2TileCols;
    uint32_t Log2TileRows;
    uint32_t LoopFilterAcrossTiles;
    uint32_t NumTileGroup;
    uint32_t MaxNumTileCols;
    uint32_t MaxNumTileRows;
    uint32_t MinNumTileCols;
    uint32_t MinNumTileRows;
    uint32_t TxMode;
    uint32_t IntraOnly;
    uint32_t McMode;
    uint32_t AllowHpMv;
    uint32_t CompPredMode;
    uint32_t FrameParallel;
    uint32_t ContextUpdateTileId;

    //RDO
    uint32_t RdoEnable;
    //intra prediction control
    uint32_t KeyIntraPrediction;
    uint32_t PIntraPrediction;

    //adaptive deadzone
    uint32_t AdaptiveDeadZoneEnable;

    //VDEnc Mode
    uint32_t VDEncSpeedMode;

    //Super Resolution
    uint32_t    EnableSuperResolution;
    uint32_t    SuperresScaleDenominator;
    uint32_t    SuperresUsePreScaleRef;
    uint32_t    SuperresStartFrame;
    uint32_t    SuperresEndFrame;

    uint32_t    FrameIdNumbersPresentFlag;
    int32_t     GroupTileHeaderNumBytes;

    uint32_t    LoopFilterLevelFormulaType;
    uint32_t    DoSwapInRef1List;
    uint32_t    PrimaryRefFrameSelection;

    uint32_t    UsingQmatrix;
    uint32_t    QMatrixY;
    uint32_t    QMatrixU;
    uint32_t    QMatrixV;

    //Ref ctrl
    uint32_t    RefCtrl;

    bool        AdditionalFWDAlphaSearchEnable;
    bool        AdditionalFWDBetaSearchEnable;

    uint32_t    EnableStatistics;
    uint32_t    RDOQEnable;
    uint32_t    CDEFMode;
    uint32_t    LRMode;
    uint32_t    LRFilterTypeY;
    uint32_t    LRFilterTypeU;
    uint32_t    LRFilterTypeV;
    uint32_t    EnableCDEFSearchForRandEnc;
    uint32_t    EnableLRSearchForRandEnc;

    uint32_t    CDEFYStrength[8];
    uint32_t    CDEFUVStrength[8];
    uint32_t    CDEFBits;
    uint32_t    UseDefaultCDEFStrengths;

    // StreamIn
    uint32_t    StreamInEnable;
    uint32_t    StreamInSegEnable;
    uint32_t    StreamInMaxCuSize;
    uint32_t    StreamInMaxTuSize;
    uint32_t    StreamInNumImePredictors;
    uint32_t    StreamInNumMergeCandidateCu8x8;
    uint32_t    StreamInNumMergeCandidateCu16x16;
    uint32_t    StreamInNumMergeCandidateCu32x32;
    uint32_t    StreamInNumMergeCandidateCu64x64;

    //B Frame Coding Structure
    uint32_t    BGOPSize;
    int32_t     IntraPeriod;
    uint32_t    PerBFramePOC[MAX_BSTRUCTURE_GOP_SIZE];
    uint32_t    PerBFrameQPOffset[MAX_BSTRUCTURE_GOP_SIZE];
    double      PerBFrameLambdaQPOffset[MAX_BSTRUCTURE_GOP_SIZE];
    uint32_t    PerBFrameRoundingInter[MAX_BSTRUCTURE_GOP_SIZE];
    uint32_t    PerBFrameRoundingIntra[MAX_BSTRUCTURE_GOP_SIZE];
    double      PerBFrameQPFactor[MAX_BSTRUCTURE_GOP_SIZE];
    uint32_t    PerBFrameTemporalID[MAX_BSTRUCTURE_GOP_SIZE];
    uint32_t    PerBFrameNumRefPicsActiveL0[MAX_BSTRUCTURE_GOP_SIZE];
    uint32_t    PerBFrameNumRefPicsActiveL1[MAX_BSTRUCTURE_GOP_SIZE];
    uint32_t    PerBFrameNumRefPics[MAX_BSTRUCTURE_GOP_SIZE];
    int32_t     PerBFrameRefPics[MAX_BSTRUCTURE_GOP_SIZE * MAX_BSTRUCTURE_REF_NUM];
    uint32_t    PerBFrameEnableCurrPicInL1[MAX_BSTRUCTURE_GOP_SIZE];

};

struct MetadataAV1PostFeature
{
    struct
    {
        uint64_t RowCount;
        uint64_t ColCount;
        uint64_t RowHeights[64];
        uint64_t ColWidths[64];
        uint64_t ContextUpdateTileId;
    } tilePartition;

    struct
    {
        uint64_t CompoundPredictionType;
        struct
        {
            uint64_t LoopFilterLevel[2];
            uint64_t LoopFilterLevelU;
            uint64_t LoopFilterLevelV;
            uint64_t LoopFilterSharpnessLevel;
            uint64_t LoopFilterDeltaEnabled;
            uint64_t UpdateRefDelta;
            int64_t  RefDeltas[8];
            uint64_t UpdateModeDelta;
            int64_t  ModeDeltas[2];
        } LoopFilter;
        struct
        {
            uint64_t DeltaLFPresent;
            uint64_t DeltaLFMulti;
            uint64_t DeltaLFRes;
        } LoopFilterDelta;
        struct
        {
            uint64_t BaseQIndex;
            int64_t  YDCDeltaQ;
            int64_t  UDCDeltaQ;
            int64_t  UACDeltaQ;
            int64_t  VDCDeltaQ;
            int64_t  VACDeltaQ;
            uint64_t UsingQMatrix;
            uint64_t QMY;
            uint64_t QMU;
            uint64_t QMV;
        } Quantization;
        struct
        {
            uint64_t DeltaQPresent;
            uint64_t DeltaQRes;
        } QuantizationDelta;
        struct
        {
            uint64_t CdefBits;
            uint64_t CdefDampingMinus3;
            uint64_t CdefYPriStrength[8];
            uint64_t CdefUVPriStrength[8];
            uint64_t CdefYSecStrength[8];
            uint64_t CdefUVSecStrength[8];
        } CDEF;
        struct
        {
            uint64_t UpdateMap;
            uint64_t TemporalUpdate;
            uint64_t UpdateData;
            uint64_t NumSegments;
            struct
            {
                uint64_t EnabledFeatures;
                int64_t  FeatureValue[8];
            } SegmentsData[8];
        } SegmentationConfig;
        uint64_t PrimaryRefFrame;
        uint64_t ReferenceIndices[7];
    } postFeature;
};

struct AV1MetaDataOffset
{
    //tile partition
    uint32_t dwRowCount            = 0;
    uint32_t dwColCount            = 0;
    uint32_t dwRowHeights          = 0;
    uint32_t dwColWidths           = 0;
    uint32_t dwContextUpdateTileId = 0;
    //post feature
    uint32_t dwCompoundPredictionType = 0;
    uint32_t dwLoopFilter             = 0;
    uint32_t dwLoopFilterDelta        = 0;
    uint32_t dwQuantization           = 0;
    uint32_t dwQuantizationDelta      = 0;
    uint32_t dwCDEF                   = 0;
    uint32_t dwSegmentationConfig     = 0;
    uint32_t dwPrimaryRefFrame        = 0;
    uint32_t dwReferenceIndices       = 0;
    //loop filter
    uint32_t dwLoopFilterLevel          = 0;
    uint32_t dwLoopFilterLevelU         = 0;
    uint32_t dwLoopFilterLevelV         = 0;
    uint32_t dwLoopFilterSharpnessLevel = 0;
    uint32_t dwLoopFilterDeltaEnabled   = 0;
    uint32_t dwUpdateRefDelta           = 0;
    uint32_t dwRefDeltas                = 0;
    uint32_t dwUpdateModeDelta          = 0;
    uint32_t dwModeDeltas               = 0;
    //loop filter delta
    uint32_t dwDeltaLFPresent = 0;
    uint32_t dwDeltaLFMulti   = 0;
    uint32_t dwDeltaLFRes     = 0;
    //Quantization
    uint32_t dwBaseQIndex   = 0;
    uint32_t dwYDCDeltaQ    = 0;
    uint32_t dwUDCDeltaQ    = 0;
    uint32_t dwUACDeltaQ    = 0;
    uint32_t dwVDCDeltaQ    = 0;
    uint32_t dwVACDeltaQ    = 0;
    uint32_t dwUsingQMatrix = 0;
    uint32_t dwQMY          = 0;
    uint32_t dwQMU          = 0;
    uint32_t dwQMV          = 0;
    //QuantizationDelta
    uint32_t dwDeltaQPresent = 0;
    uint32_t dwDeltaQRes     = 0;
    //CDEF
    uint32_t dwCdefBits          = 0;
    uint32_t dwCdefDampingMinus3 = 0;
    uint32_t dwCdefYPriStrength  = 0;
    uint32_t dwCdefUVPriStrength = 0;
    uint32_t dwCdefYSecStrength  = 0;
    uint32_t dwCdefUVSecStrength = 0;
    //SegmentationConfig
    uint32_t dwUpdateMap      = 0;
    uint32_t dwTemporalUpdate = 0;
    uint32_t dwUpdateData     = 0;
    uint32_t dwNumSegments    = 0;
    uint32_t dwSegmentsData   = 0;
    //dwSegmentsData
    uint32_t dwEnabledFeatures  = 0;
    uint32_t dwFeatureValue     = 0;
    uint32_t dwSegmentsDataSize = 0;
};

struct EncoderParamsAV1 : EncoderParams
{
    uint32_t segmentMapDataSize = 0;     //!< [AV1] size of data in segment map buffer
    uint8_t  *pSegmentMap = nullptr;     //!< [AV1] pointer to segment map buffer from DDI
    AV1MetaDataOffset AV1metaDataOffset  = {};       //!< [AV1] AV1 Specific metadata offset
};

enum RoundingMethod
{
    fixedRounding = 0,
    adaptiveRounding,
    lookUpTableRounding
};
#endif  // __CODEC_DEF_ENCODE_AV1_H__
