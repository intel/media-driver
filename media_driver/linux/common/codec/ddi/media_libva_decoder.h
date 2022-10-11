/*
* Copyright (c) 2009-2020, Intel Corporation
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
//! \file      media_libva_decoder.h
//! \brief     libva(and its extension) decoder head file
//!

#ifndef __MEDIA_LIBVA_DECODER_H__
#define __MEDIA_LIBVA_DECODER_H__

#include "media_libva.h"
#include "media_libva_cp_interface.h"
#include "media_ddi_decode_base.h"

#define DDI_DECODE_SFC_MAX_WIDTH                    4096
#define DDI_DECODE_SFC_MAX_HEIGHT                   4096
#define DDI_DECODE_SFC_MIN_WIDTH                    128
#define DDI_DECODE_SFC_MIN_HEIGHT                   128
#define DDI_DECODE_HCP_SFC_MAX_WIDTH                (16*1024)
#define DDI_DECODE_HCP_SFC_MAX_HEIGHT               (16*1024)

#if MOS_EVENT_TRACE_DUMP_SUPPORTED

#define MACROBLOCK_HEIGHT   16
#define MACROBLOCK_WIDTH    16

typedef struct _DECODE_EVENTDATA_BITSTREAM
{
    uint8_t Data[32];
} DECODE_EVENTDATA_BITSTREAM;

typedef struct _DECODE_EVENTDATA_FRAME
{
    uint32_t FrameIdx;  
    uint32_t PicFlags;  
    uint32_t PicEntry;  
} DECODE_EVENTDATA_FRAME;

typedef struct _DECODE_EVENTDATA_INFO_PICTURE
{
    uint32_t CodecFormat;
    uint32_t FrameType;
    uint32_t PicStruct;
    uint32_t Width;
    uint32_t Height;
    uint32_t Bitdepth;
    uint32_t ChromaFormat;
    bool     EnabledSCC;
    bool     EnabledSegment;
    bool     EnabledFilmGrain;
} DECODE_EVENTDATA_INFO_PICTURE;

typedef struct _DECODE_EVENTDATA_INFO_PICTUREVA
{
    uint32_t CodecFormat;
    uint32_t FrameType;
    uint32_t PicStruct;
    uint32_t Width;
    uint32_t Height;
    uint32_t Bitdepth;
    uint32_t ChromaFormat;
    bool     EnabledSCC;
    bool     EnabledSegment;
    bool     EnabledFilmGrain;
} DECODE_EVENTDATA_INFO_PICTUREVA;

typedef struct _DECODE_EVENTDATA_VA_DISPLAYINFO
{
    uint32_t uiDisplayWidth;
    uint32_t uiDisplayHeight;
} DECODE_EVENTDATA_VA_DISPLAYINFO;

typedef struct _DECODE_EVENTDATA_VA_CREATEBUFFER
{
    VABufferType type;
    uint32_t size;
    uint32_t numElements;
    VABufferID *bufId;
} DECODE_EVENTDATA_VA_CREATEBUFFER;

typedef struct _DECODE_EVENTDATA_VA_BEGINPICTURE_START
{
    uint32_t FrameIndex;
} DECODE_EVENTDATA_VA_BEGINPICTURE_START;

typedef struct _DECODE_EVENTDATA_VA_BEGINPICTURE
{
    uint32_t FrameIndex;
    uint32_t hRes;
} DECODE_EVENTDATA_VA_BEGINPICTURE;

typedef struct _DECODE_EVENTDATA_VA_ENDPICTURE_START
{
    uint32_t FrameIndex;
} DECODE_EVENTDATA_VA_ENDPICTURE_START;

typedef struct _DECODE_VA_EVENTDATA_ENDPICTURE
{
    uint32_t FrameIndex;
    uint32_t hRes;
} DECODE_EVENTDATA_VA_ENDPICTURE;


typedef struct _DECODE_EVENTDATA_VA_RENDERPICTURE_START
{
    VABufferID *buffers;
} DECODE_EVENTDATA_VA_RENDERPICTURE_START;

typedef struct _DECODE_EVENTDATA_VA_RENDERPICTURE
{
    VABufferID *buffers;
    uint32_t hRes;
    uint32_t numBuffers;
} DECODE_EVENTDATA_VA_RENDERPICTURE;

typedef struct _DECODE_EVENTDATA_VA_CREATECONTEXT_START
{
    VABufferID configId;
} DECODE_EVENTDATA_VA_CREATECONTEXT_START;

typedef struct _DECODE_EVENTDATA_VA_CREATECONTEXT
{
    VABufferID configId;
    uint32_t hRes;
} DECODE_EVENTDATA_VA_CREATECONTEXT;

typedef struct _DECODE_EVENTDATA_VA_DESTROYCONTEXT_START
{
    VABufferID context;
} DECODE_EVENTDATA_VA_DESTROYCONTEXT_START;

typedef struct _DECODE_EVENTDATA_VA_DESTROYCONTEXT
{
    VABufferID context;
} DECODE_EVENTDATA_VA_DESTROYCONTEXT;

typedef struct _DECODE_EVENTDATA_VA_GETDECCTX
{
    uint32_t bufferID;
} DECODE_EVENTDATA_VA_GETDECCTX;

typedef struct _DECODE_EVENTDATA_VA_FREEBUFFERHEAPELEMENTS
{
    uint32_t bufNums;
} DECODE_EVENTDATA_VA_FREEBUFFERHEAPELEMENTS;

typedef struct _DECODE_EVENTDATA_VA_FEATURE_REPORTMODE
{
    uint32_t wMode;
    uint32_t ValueID;
} DECODE_EVENTDATA_VA_FEATURE_REPORTMODE;

typedef struct _DECODE_EVENTDATA_PICPARAM_AVC
{
    uint32_t CurrFrameIdx;
    uint32_t CurrPicFlags;
    uint32_t CurrPicEntry;
    uint32_t pic_width_in_mbs_minus1;
    uint32_t pic_height_in_mbs_minus1;
    uint32_t bit_depth_luma_minus8;
    uint32_t bit_depth_chroma_minus8;
    uint32_t num_ref_frames;
    uint32_t CurrFieldOrderCnt[2];
    uint32_t chroma_format_idc;
    uint32_t residual_colour_transform_flag;
    uint32_t frame_mbs_only_flag;
    uint32_t mb_adaptive_frame_field_flag;
    uint32_t direct_8x8_inference_flag;
    uint32_t log2_max_frame_num_minus4;
    uint32_t pic_order_cnt_type;
    uint32_t log2_max_pic_order_cnt_lsb_minus4;
    uint32_t delta_pic_order_always_zero_flag;
    uint32_t num_slice_groups_minus1;
    uint32_t slice_group_map_type;
    uint32_t slice_group_change_rate_minus1;
    uint32_t pic_init_qp_minus26;
    uint32_t chroma_qp_index_offset;
    uint32_t second_chroma_qp_index_offset;
    uint32_t entropy_coding_mode_flag;
    uint32_t weighted_pred_flag;
    uint32_t weighted_bipred_idc;
    uint32_t transform_8x8_mode_flag;
    uint32_t field_pic_flag;
    uint32_t constrained_intra_pred_flag;
    uint32_t pic_order_present_flag;
    uint32_t deblocking_filter_control_present_flag;
    uint32_t redundant_pic_cnt_present_flag;
    uint32_t reference_pic_flag;
    uint32_t IntraPicFlag;
    uint32_t num_ref_idx_l0_active_minus1;
    uint32_t num_ref_idx_l1_active_minus1;
    DECODE_EVENTDATA_FRAME RefFrameList[16];
    uint32_t FrameNumList[16];
    uint32_t NonExistingFrameFlags;
    uint32_t UsedForReferenceFlags;
    uint32_t frame_num;
    uint32_t FieldOrderCntList[16][2];
    uint32_t StatusReportFeedbackNumber;
} DECODE_EVENTDATA_PICPARAM_AVC;

typedef struct _DECODE_EVENTDATA_SLICEPARAM_AVC
{
    uint32_t slice_data_size;
    uint32_t slice_data_offset;
} DECODE_EVENTDATA_SLICEPARAM_AVC;

typedef struct _DECODE_EVENTDATA_LONGSLICEPARAM_AVC
{
    uint32_t slice_data_size;     
    uint32_t slice_data_offset; 
    uint32_t slice_data_bit_offset;
    uint32_t first_mb_in_slice; 
    uint32_t NumMbsForSlice;
    uint32_t slice_type;      
    uint32_t direct_spatial_mv_pred_flag;
    uint32_t num_ref_idx_l0_active_minus1;
    uint32_t num_ref_idx_l1_active_minus1;
    uint32_t cabac_init_idc;
    uint32_t slice_qp_delta;
    uint32_t disable_deblocking_filter_idc;
    uint32_t slice_alpha_c0_offset_div2;
    uint32_t slice_beta_offset_div2;
    DECODE_EVENTDATA_FRAME RefPicList0[32];
    DECODE_EVENTDATA_FRAME RefPicList1[32];
    uint32_t luma_log2_weight_denom;
    uint32_t chroma_log2_weight_denom;
    uint32_t Weights0x00[32];
    uint32_t Weights0x01[32];
    uint32_t Weights0x10[32];
    uint32_t Weights0x11[32];
    uint32_t Weights0x20[32];
    uint32_t Weights0x21[32];
    uint32_t Weights1x00[32];
    uint32_t Weights1x01[32];
    uint32_t Weights1x10[32];
    uint32_t Weights1x11[32];
    uint32_t Weights1x20[32];
    uint32_t Weights1x21[32];
    uint32_t slice_id;
    uint32_t first_mb_in_next_slice;
} DECODE_EVENTDATA_LONGSLICEPARAM_AVC;

void DecodeEventDataAVCPicParamInit(
    DECODE_EVENTDATA_PICPARAM_AVC *pEventData,
    PCODEC_AVC_PIC_PARAMS          pAvcPicParams);

void DecodeEventDataAVCSliceParamInit(
    DECODE_EVENTDATA_SLICEPARAM_AVC *pEventData,
    PCODEC_AVC_SLICE_PARAMS          pAvcSliceParams,
    uint32_t                         dwNumSlices);

void DecodeEventDataAVCLongSliceParamInit(
    DECODE_EVENTDATA_LONGSLICEPARAM_AVC *pEventData,
    PCODEC_AVC_SLICE_PARAMS              pAvcSliceParams,
    uint32_t                             dwNumSlices);

typedef struct _DECODE_EVENTDATA_PICPARAM_HEVC
{
    uint32_t CurrFrameIdx;            
    uint32_t CurrPicFlags;              
    uint32_t CurrPicEntry;              
    uint32_t PicWidthInMinCbsY;       
    uint32_t PicHeightInMinCbsY;       
    uint32_t wFormatAndSequenceInfoFlags;
    uint32_t sps_max_dec_pic_buffering_minus1;    
    uint32_t log2_min_luma_coding_block_size_minus3;
    uint32_t log2_diff_max_min_luma_coding_block_size;
    uint32_t log2_min_transform_block_size_minus2;  
    uint32_t log2_diff_max_min_transform_block_size; 
    uint32_t max_transform_hierarchy_depth_inter;  
    uint32_t max_transform_hierarchy_depth_intra;  
    uint32_t num_short_term_ref_pic_sets;    
    uint32_t num_long_term_ref_pic_sps;        
    uint32_t num_ref_idx_l0_default_active_minus1;  
    uint32_t num_ref_idx_l1_default_active_minus1;   
    uint32_t init_qp_minus26;             
    uint32_t ucNumDeltaPocsOfRefRpsIdx;            
    uint32_t wNumBitsForShortTermRPSInSlice;         
    uint32_t dwCodingParamToolFlags;          
    uint32_t dwCodingSettingPicturePropertyFlags; 
    uint32_t pps_cb_qp_offset;                
    uint32_t pps_cr_qp_offset;                
    uint32_t num_tile_columns_minus1;              
    uint32_t num_tile_rows_minus1;            
    uint32_t deblocking_filter_override_enabled_flag;
    uint32_t pps_deblocking_filter_disabled_flag;
    DECODE_EVENTDATA_FRAME RefFrameList[CODEC_MAX_NUM_REF_FRAME_HEVC];
    uint32_t column_width_minus1[19];
    uint32_t row_height_minus1[21];
    uint32_t diff_cu_qp_delta_depth;          
    uint32_t pps_beta_offset_div2;            
    uint32_t pps_tc_offset_div2;              
    uint32_t log2_parallel_merge_level_minus2;
    uint32_t CurrPicOrderCntVal;
    uint32_t PicOrderCntValList[15];
    uint32_t RefPicSetStCurrBefore[8];
    uint32_t RefPicSetStCurrAfter[8]; 
    uint32_t RefPicSetLtCurr[8];
    uint32_t RefFieldPicFlag;           
    uint32_t RefBottomFieldFlag;        
    uint32_t TotalNumEntryPointOffsets; 
    uint32_t StatusReportFeedbackNumber;
} DECODE_EVENTDATA_PICPARAM_HEVC;

typedef struct _DECODE_EVENTDATA_REXTPICPARAM_HEVC
{
    uint32_t CurrFrameIdx;;
    uint32_t RangeExtensionPropertyFlags;             
    uint32_t diff_cu_chroma_qp_offset_depth;          
    uint32_t chroma_qp_offset_list_len_minus1;         
    uint32_t log2_sao_offset_scale_luma;               
    uint32_t log2_sao_offset_scale_chroma;             
    uint32_t log2_max_transform_skip_block_size_minus2;
    uint32_t cb_qp_offset_list[6];
    uint32_t cr_qp_offset_list[6];
} DECODE_EVENTDATA_REXTPICPARAM_HEVC;

typedef struct _DECODE_EVENTDATA_SCCPICPARAM_HEVC
{
    uint32_t CurrFrameIdx;
    uint32_t ScreenContentCodingPropertyFlags;
    uint32_t palette_max_size;                
    uint32_t delta_palette_max_predictor_size;
    uint32_t PredictorPaletteSize;            
    uint32_t pps_act_y_qp_offset_plus5;       
    uint32_t pps_act_cb_qp_offset_plus5;      
    uint32_t pps_act_cr_qp_offset_plus3;
    uint32_t PredictorPaletteEntries[3][128];
} DECODE_EVENTDATA_SCCPICPARAM_HEVC;

typedef struct _DECODE_EVENTDATA_SLICEPARAM_HEVC
{
    uint32_t NumEmuPrevnBytesInSliceHdr;
    uint32_t slice_data_size;
    uint32_t slice_data_offset;
} DECODE_EVENTDATA_SLICEPARAM_HEVC;

typedef struct _DECODE_EVENTDATA_LONGSLICEPARAM_HEVC
{
    uint32_t slice_data_size;
    uint32_t slice_data_offset;
    uint32_t ByteOffsetToSliceData;
    uint32_t NumEmuPrevnBytesInSliceHdr;
    uint32_t slice_segment_address;
    DECODE_EVENTDATA_FRAME RefPicList0[15];
    DECODE_EVENTDATA_FRAME RefPicList1[15];
    uint32_t LastSliceOfPic;                              
    uint32_t dependent_slice_segment_flag;                
    uint32_t slice_type;                                  
    uint32_t color_plane_id;                              
    uint32_t slice_sao_luma_flag;                         
    uint32_t slice_sao_chroma_flag;                       
    uint32_t mvd_l1_zero_flag;                            
    uint32_t cabac_init_flag;                             
    uint32_t slice_temporal_mvp_enabled_flag;             
    uint32_t slice_deblocking_filter_disabled_flag;       
    uint32_t collocated_from_l0_flag;                     
    uint32_t slice_loop_filter_across_slices_enabled_flag;
    
    uint32_t collocated_ref_idx;
    uint32_t num_ref_idx_l0_active_minus1;
    uint32_t num_ref_idx_l1_active_minus1;
    uint32_t slice_qp_delta;
    uint32_t slice_cb_qp_offset;
    uint32_t slice_cr_qp_offset;
    uint32_t slice_beta_offset_div2;     
    uint32_t slice_tc_offset_div2;          
    uint32_t luma_log2_weight_denom;        
    uint32_t delta_chroma_log2_weight_denom;

    char delta_luma_weight_l0[15];    
    char delta_chroma_weight_l0[15][2];
    char delta_luma_weight_l1[15];
    char delta_chroma_weight_l1[15][2];

    char luma_offset_l0[15];
    char ChromaOffsetL0[15][2];    
    char luma_offset_l1[15];    
    char ChromaOffsetL1[15][2];

    uint32_t five_minus_max_num_merge_cand;
    uint32_t num_entry_point_offsets;
    uint32_t EntryOffsetToSubsetArray;
} DECODE_EVENTDATA_LONGSLICEPARAM_HEVC;

typedef struct _DECODE_EVENTDATA_REXTLONGSLICEPARAM_HEVC
{
    int16_t luma_offset_l0[15];
    int16_t ChromaOffsetL0[15][2];
    int16_t luma_offset_l1[15];
    int16_t ChromaOffsetL1[15][2];
    uint32_t cu_chroma_qp_offset_enabled_flag;
    uint32_t isHevcScc;
    uint32_t slice_act_y_qp_offset; 
    uint32_t slice_act_cb_qp_offset;
    uint32_t slice_act_cr_qp_offset;
    uint32_t use_integer_mv_flag;
} DECODE_EVENTDATA_REXTLONGSLICEPARAM_HEVC;

void DecodeEventDataHEVCPicParamInit(
    DECODE_EVENTDATA_PICPARAM_HEVC *pEventData,
    PCODEC_HEVC_PIC_PARAMS          pHevcPicParams);

void DecodeEventDataHEVCRExtPicParamInit(
    DECODE_EVENTDATA_REXTPICPARAM_HEVC *pEventData,
    PCODEC_HEVC_PIC_PARAMS              pHevcPicParams,
    PCODEC_HEVC_EXT_PIC_PARAMS          pHevcExtPicParams);

void DecodeEventDataHEVCSccPicParamInit(
    DECODE_EVENTDATA_SCCPICPARAM_HEVC *pEventData,
    PCODEC_HEVC_PIC_PARAMS             pHevcPicParams,
    PCODEC_HEVC_SCC_PIC_PARAMS         pHevcSccPicParams);

void DecodeEventDataHEVCSliceParamInit(
    DECODE_EVENTDATA_SLICEPARAM_HEVC *pEventData,
    PCODEC_HEVC_SLICE_PARAMS          pHevcSliceParams,
    uint32_t                          dwNumSlices);

void DecodeEventDataHEVCLongSliceParamInit(
    DECODE_EVENTDATA_LONGSLICEPARAM_HEVC *pEventData,
    PCODEC_HEVC_SLICE_PARAMS              pHevcSliceParams,
    uint32_t                              dwNumSlices);

void DecodeEventDataHEVCRExtLongSliceParamInit(
    DECODE_EVENTDATA_REXTLONGSLICEPARAM_HEVC *pEventData,
    PCODEC_HEVC_EXT_SLICE_PARAMS              pHevcRextSliceParams,
    uint32_t                                  dwNumSlices,
    bool                                      isHevcScc);

typedef struct _DECODE_EVENTDATA_PICPARAM_VP9
{
    uint32_t CurrPic_FrameIdx;
    uint32_t CurrPic_PicFlags;
    uint32_t FrameWidthMinus1;
    uint32_t FrameHeightMinus1;
    uint32_t profile;
    uint32_t BitDepthMinus8;
    uint32_t subsampling_x;
    uint32_t subsampling_y;
    uint32_t frame_type;
    uint32_t show_frame;
    uint32_t error_resilient_mode;
    uint32_t intra_only;
    uint32_t LastRefIdx;
    uint32_t LastRefSignBias;
    uint32_t GoldenRefIdx;
    uint32_t GoldenRefSignBias;
    uint32_t AltRefIdx;
    uint32_t AltRefSignBias;
    uint32_t allow_high_precision_mv;
    uint32_t mcomp_filter_type;
    uint32_t frame_parallel_decoding_mode;
    uint32_t segmentation_enabled;
    uint32_t segmentation_temporal_update;
    uint32_t segmentation_update_map;
    uint32_t reset_frame_context;
    uint32_t refresh_frame_context;
    uint32_t frame_context_idx;
    uint32_t LosslessFlag;
    uint32_t filter_level;
    uint32_t sharpness_level;
    uint32_t log2_tile_rows;
    uint32_t log2_tile_columns;
    uint32_t RefFrameList[8];
    uint32_t UncompressedHeaderLengthInBytes;
    uint32_t FirstPartitionSize;
    uint32_t StatusReportFeedbackNumber;
} DECODE_EVENTDATA_PICPARAM_VP9;

typedef struct _DECODE_EVENTDATA_SEGDATA_VP9
{
    uint16_t SegmentReferenceEnabled;
    uint16_t SegmentReference;
    uint16_t SegmentReferenceSkipped;

    uint16_t LumaACQuantScale;
    uint16_t LumaDCQuantScale;
    uint16_t ChromaACQuantScale;
    uint16_t ChromaDCQuantScale;
} DECODE_EVENTDATA_SEGDATA_VP9;

typedef struct _DECODE_EVENTDATA_SEGPARAM_VP9
{
    uint32_t                       SegmentEnabled;
    uint32_t                       SegmentTemporalUpdate;
    uint32_t                       SegmentUpdateMap;
    DECODE_EVENTDATA_SEGDATA_VP9 SegParam[8];
} DECODE_EVENTDATA_SEGPARAM_VP9;

void DecodeEventDataVP9PicParamInit(
    DECODE_EVENTDATA_PICPARAM_VP9 *pEventData,
    void                          *pPicParams);

void DecodeEventDataVP9SegParamInit(
    DECODE_EVENTDATA_SEGPARAM_VP9 *pEventData,
    void                          *pPicParams,
    void                          *pSegData);

typedef struct _DECODE_EVENTDATA_PICPARAM_AV1
{
    uint32_t CurrFrameIdx;
    uint32_t profile;
    uint32_t enableOrderHint;
    uint32_t orderHintBitsMinus1;
    uint32_t bitDepthIdx;
    uint32_t frameWidthMinus1;
    uint32_t frameHeightMinus1;
    uint32_t superResUpscaledWidthMinus1;
    uint32_t superResUpscaledHeightMinus1;
    uint32_t use128x128Superblock;
    uint32_t enableIntraEdgeFilter;
    uint32_t enableInterintraCompound;
    uint32_t enableMaskedCompound;
    uint32_t enableDualFilter;
    uint32_t enableJntComp;
    uint32_t enableCdef;
    uint32_t filmGrainParamsPresent;
    uint32_t enableFilterIntra;
    uint32_t monoChrome;
    uint32_t subsamplingX;
    uint32_t subsamplingY;
    uint32_t allowWarpedMotion;
    uint32_t allowScreenContentTools;
    uint32_t forceIntegerMv;
    uint32_t allowIntrabc;
    uint32_t allowHighPrecisionMv;
    uint32_t isMotionModeSwitchable;
    uint32_t disableFrameEndUpdateCdf;
    uint32_t disableCdfUpdate;
    uint32_t useSuperres;
    uint32_t useRefFrameMvs;
    uint32_t showFrame;
    uint32_t showableFrame;
    uint32_t frameType;
    uint32_t RefFrameMapIdx[8];
    uint32_t RefFrameIdx[7];
    uint32_t primaryRefFrame;
    uint32_t orderHint;
    uint32_t superresScaleDenominator;
    uint32_t interpFilter;
    uint32_t filterLevel_0;
    uint32_t filterLevel_1;
    uint32_t filterLevelU;
    uint32_t filterLevelV;
    uint32_t sharpnessLevel;
    uint32_t modeRefDeltaEnabled;
    uint32_t modeRefDeltaUpdate;
    uint32_t RefDeltas[8];
    uint32_t ModeDeltas[2];
    uint32_t baseQindex;
    uint32_t yDcDeltaQ;
    uint32_t uDcDeltaQ;
    uint32_t uAcDeltaQ;
    uint32_t vDcDeltaQ;
    uint32_t vAcDeltaQ;
    uint32_t qmY;
    uint32_t qmU;
    uint32_t qmV;
    uint32_t usingQmatrix;
    uint32_t deltaQPresentFlag;
    uint32_t log2DeltaQRes;
    uint32_t referenceMode;
    uint32_t skipModePresent;
    uint32_t reducedTxSetUsed;
    uint32_t txMode;
    uint32_t deltaLfMulti;
    uint32_t deltaLfPresentFlag;
    uint32_t log2DeltaLfRes;
    uint32_t losslessMode;    
    uint32_t SegData_enabled;
    uint32_t tileCols;
    uint32_t tileRows;
    uint32_t widthInSbsMinus1[64];
    uint32_t heightInSbsMinus1[64];
    uint32_t m_contextUpdateTileId;
    uint32_t m_cdefDampingMinus3;
    uint32_t m_cdefBits;
    uint32_t cdefYStrengths[8];
    uint32_t cdefUvStrengths[8];
    uint32_t yframeRestorationType;
    uint32_t cbframeRestorationType;
    uint32_t crframeRestorationType;
    uint32_t lrUnitShift;
    uint32_t lrUvShift;     
    CodecAv1WarpedMotionParams WarpMotion[7];
    uint32_t matrixCoefficients;
    uint32_t applyGrain;
    uint32_t statusReportFeedbackNumber;
} DECODE_EVENTDATA_PICPARAM_AV1;

typedef struct _DECODE_EVENTDATA_SEGPARAM_AV1
{
    uint32_t CurrFrameIdx;
    uint32_t SegData_enabled;
    uint32_t SegData_updateMap;
    uint32_t SegData_temporalUpdate;
    uint32_t SegData_updateData;
    uint32_t SegData_featureMask[8];
    uint32_t SegData_featureData[8][8];
    uint32_t SegData_losslessFlag[8];
    uint32_t SegData_qmLevelY[8];
    uint32_t SegData_qmLevelU[8];
    uint32_t SegData_qmLevelV[8];
} DECODE_EVENTDATA_SEGPARAM_AV1;

typedef struct _DECODE_EVENTDATA_FILMGRAINPARAM_AV1
{
    uint32_t CurrFrameIdx;
    uint32_t applyGrain;
    uint32_t chromaScalingFromLuma;
    uint32_t grainScalingMinus8;
    uint32_t arCoeffLag;
    uint32_t arCoeffShiftMinus6;
    uint32_t grainScaleShift;
    uint32_t clipToRestrictedRange;
    uint32_t overlapFlag;
    uint32_t randomSeed;
    uint32_t numYPoints;
    uint32_t numCbPoints;
    uint32_t numCrPoints;
    uint32_t cbMult;
    uint32_t cbLumaMult;
    uint32_t cbOffset;
    uint32_t crMult;
    uint32_t crLumaMult;
    uint32_t crOffset;
    uint32_t pointYValue[14];
    uint32_t pointYScaling[14];
    uint32_t pointCbValue[10];
    uint32_t pointCbScaling[10];
    uint32_t pointCrValue[10];
    uint32_t pointCrScaling[10];
    uint32_t arCoeffsY[24];
    uint32_t arCoeffsCb[25];
    uint32_t arCoeffsCr[25];
} DECODE_EVENTDATA_FILMGRAINPARAM_AV1;

typedef struct _DECODE_EVENTDATA_TILEPARAM_AV1
{
    uint32_t NumTiles;
    uint32_t largeScaleTile;
    uint32_t tileCountMinus1;
    uint32_t outputFrameWidthInTilesMinus1;
    uint32_t outputFrameHeightInTilesMinus1;
    uint32_t anchorFrameInsertion;
} DECODE_EVENTDATA_TILEPARAM_AV1;

typedef struct _DECODE_EVENTDATA_TILEINFO_AV1
{
    uint32_t tileIndex;
    uint32_t bsTileDataLocation;
    uint32_t bsTileBytesInBuffer;
    uint32_t badBSBufferChopping;
    uint32_t tileRow;
    uint32_t tileColumn;
    uint32_t anchorFrameIdx;
    uint32_t bsTilePayloadSizeInBytes;
} DECODE_EVENTDATA_TILEINFO_AV1;

void DecodeEventDataAV1PicParamInit(
    DECODE_EVENTDATA_PICPARAM_AV1 *pEventData,
    CodecAv1PicParams             *pAv1PicParams);

void DecodeEventDataAV1SegParamInit(
    DECODE_EVENTDATA_SEGPARAM_AV1 *pEventData,
    CodecAv1PicParams *            pAv1PicParams);

void DecodeEventDataAV1FilmGrainParamInit(
    DECODE_EVENTDATA_FILMGRAINPARAM_AV1 *pEventData,
    CodecAv1PicParams                   *pAv1PicParams);

void DecodeEventDataAV1TileParamInit(
    DECODE_EVENTDATA_TILEPARAM_AV1 *pEventData,
    DECODE_EVENTDATA_TILEINFO_AV1  *pEventTileData,
    CodecAv1PicParams              *pAv1PicParams,
    CodecAv1TileParams             *pAv1TileParams,
    uint32_t                        dwNumTiles);
#endif

//!
//! \struct DDI_DECODE_CONFIG_ATTR
//! \brief  Ddi decode configuration attribute
//!
struct DDI_DECODE_CONFIG_ATTR
{
    VAProfile           profile;
    VAEntrypoint        entrypoint;
    uint32_t            uiDecSliceMode;
    uint32_t            uiEncryptionType;
    uint32_t            uiDecProcessingType;
};

typedef struct DDI_DECODE_CONFIG_ATTR *PDDI_DECODE_CONFIG_ATTR;

class DdiMediaDecode;

//!
//! \struct DDI_DECODE_CONTEXT
//! \brief  Ddi decode context
//!
struct DDI_DECODE_CONTEXT
{
    // Decoder data related with the specific codec
    // The instance of DdiDecodeXXX. For example: DdiDecodeAvc, DdiDecodeJPEG
    DdiMediaDecode                  *m_ddiDecode;
    // Decoder private data
    DdiCpInterface                  *pCpDdiInterface;
    // Parameters
    CodechalDecodeParams            DecodeParams;
    uint16_t                        wMode;                  // Get the info during hand shaking
    Codechal                        *pCodecHal;
    bool                            bShortFormatInUse;
    bool                            bDecodeModeReported;
    VASurfaceDecodeMBErrors         vaSurfDecErrOutput[2];
    DDI_CODEC_RENDER_TARGET_TABLE   RTtbl;
    DDI_CODEC_COM_BUFFER_MGR        BufMgr;
    PDDI_MEDIA_CONTEXT              pMediaCtx;
    // Add a list to track DPB.
    VASurfaceID                     RecListSurfaceID[CODEC_AVC_NUM_UNCOMPRESSED_SURFACE];
    uint32_t                        dwSliceParamBufNum;
    uint32_t                        dwSliceCtrlBufNum;
    uint32_t                        uiDecProcessingType;
};

typedef struct DDI_DECODE_CONTEXT *PDDI_DECODE_CONTEXT;

static __inline PDDI_DECODE_CONTEXT DdiDecode_GetDecContextFromPVOID (void *decCtx)
{
    return (PDDI_DECODE_CONTEXT)decCtx;
}

//!
//! \brief  Status report
//!
//! \param  [in] decoder
//!     CodechalDecode decoder
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiDecode_StatusReport(
    PDDI_MEDIA_CONTEXT mediaCtx,
    CodechalDecode *decoder,
    DDI_MEDIA_SURFACE *surface);

//!
//! \brief  Status report
//!
//! \param  [in] decoder
//!     DecodePipelineAdapter decoder
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiDecode_StatusReport(
    PDDI_MEDIA_CONTEXT mediaCtx,
    DecodePipelineAdapter *decoder,
    DDI_MEDIA_SURFACE *surface);

//!
//! \brief  Create buffer
//!
//! \param  [in] ctx
//!     Pointer to VA driver context
//! \param  [in] decCtx
//!     Pointer to ddi decode context
//! \param  [in] type
//!     VA buffer type
//! \param  [in] size
//!     Size
//! \param  [in] numElements
//!     Number of elements
//! \param  [in] data
//!     DAta
//! \param  [in] bufId
//!     VA buffer ID
//! 
//! \return     VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiDecode_CreateBuffer(
    VADriverContextP         ctx,
    PDDI_DECODE_CONTEXT      decCtx,
    VABufferType             type,
    uint32_t                 size,
    uint32_t                 numElements,
    void                    *data,
    VABufferID              *bufId
);

//!
//! \brief  Begin picture
//!
//! \param  [in] ctx
//!     Pointer to VA driver context
//! \param  [in] context
//!     VA context ID
//! \param  [in] renderTarget
//!     VA surface ID
//! 
//! \return     VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiDecode_BeginPicture (
    VADriverContextP    ctx,
    VAContextID         context,
    VASurfaceID         renderTarget
);

//!
//! \brief  End picture
//!
//! \param  [in] ctx
//!     Pointer to VA driver context
//! \param  [in] context
//!     VA context ID
//! 
//! \return     VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiDecode_EndPicture (
    VADriverContextP    ctx,
    VAContextID         context
);

//!
//! \brief  Render picture
//!
//! \param  [in] ctx
//!     Pointer to VA driver context
//! \param  [in] context
//!     VA context ID
//! \param  [in] buffers
//!     VA buffer ID
//! \param  [in] numBuffers
//!     Number of buffers
//! 
//! \return     VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiDecode_RenderPicture (
    VADriverContextP    ctx,
    VAContextID         context,
    VABufferID         *buffers,
    int32_t             numBuffers
);

//!
//! \brief  Create context
//!
//! \param  [in] ctx
//!     Pointer to VA driver context
//! \param  [in] configId
//!     VA configuration ID
//! \param  [in] pictureWidth
//!     The width of picture
//! \param  [in] pictureHeight
//!     The height of picture
//! \param  [in] flag
//!     Flag
//! \param  [in] renderTargets
//!     VA surface ID
//! \param  [in] numRenderTargets
//!     Number of render targets
//! \param  [in] context
//!     VA context ID
//! 
//! \return     VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiDecode_CreateContext (
    VADriverContextP    ctx,
    VAConfigID          configId,
    int32_t             pictureWidth,
    int32_t             pictureHeight,
    int32_t             flag,
    VASurfaceID        *renderTargets,
    int32_t             numRenderTargets,
    VAContextID        *context
);

//!
//! \brief  Destroy context
//!
//! \param  [in] ctx
//!     Pointer to VA driver context
//! \param  [in] context
//!     VA context ID
//! 
//! \return     VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiDecode_DestroyContext (
    VADriverContextP    ctx,
    VAContextID         context
);

//!
//! \brief  Set Decode Gpu priority
//!
//! \param  [in] ctx
//!     Pointer to VA driver context
//! \param  [in] decode context
//!     Pointer to decode context
//! \param  [in] priority
//!     priority
//! \return VAStatus
//!
VAStatus DdiDecode_SetGpuPriority(
    VADriverContextP     ctx,
    PDDI_DECODE_CONTEXT  decCtx,
    int32_t              priority
);

#endif
