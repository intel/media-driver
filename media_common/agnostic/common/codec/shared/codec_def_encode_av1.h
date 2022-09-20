/*
* Copyright (c) 2019-2021, Intel Corporation
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
            uint32_t    Reserved0               : 26;
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
            uint32_t    enable_order_hint       : 1;
            uint32_t    enable_superres         : 1;
            uint32_t    enable_cdef             : 1;
            uint32_t    enable_restoration      : 1;
            uint32_t    enable_warped_motion    : 1;    //[0]
            uint32_t    Reserved3               : 27;
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
    uint8_t         Reserved8b3;
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
    uint8_t     reserved8b[3];
    uint32_t    Reserved10[14];
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

struct EncoderParamsAV1 : EncoderParams
{
    uint32_t segmentMapDataSize = 0;     //!< [AV1] size of data in segment map buffer
    uint8_t  *pSegmentMap = nullptr;     //!< [AV1] pointer to segment map buffer from DDI
};

#endif  // __CODEC_DEF_ENCODE_AV1_H__
