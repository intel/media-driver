/*
* Copyright (c) 2014-2018, Intel Corporation
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
//! \file      mhw_vdbox.h 
//! \brief     This modules implements HW interface layer to be used on all platforms on all operating systems/DDIs, across MHW components. 
//!
#ifndef _MHW_VDBOX_H_
#define _MHW_VDBOX_H_

#include "codec_def_encode_avc.h"
#include "codec_def_encode_jpeg.h"
#include "codec_def_encode_mpeg2.h"
#include "codec_def_encode_vp9.h"
#include "codec_def_decode_vp9.h"
#include "codec_def_decode_vc1.h"
#include "codec_def_decode_avc.h"
#include "codec_def_decode_hevc.h"
#include "mos_os.h"
#include "mhw_utilities.h"
#include "mhw_cp_interface.h"

#define MHW_VDBOX_VC1_BITPLANE_BUFFER_PITCH_SMALL         64
#define MHW_VDBOX_VC1_BITPLANE_BUFFER_PITCH_LARGE         128

#define MHW_VDBOX_MFX_RAW_UV_PLANE_ALIGNMENT_GEN9         4         // starting Gen9 the alignment is relaxed to 4x instead of 16x
#define MHW_VDBOX_MFX_UV_PLANE_ALIGNMENT_LEGACY           16
#define MHW_VDBOX_MFX_RECON_UV_PLANE_ALIGNMENT            16
#define MHW_VDBOX_HCP_RAW_UV_PLANE_ALIGNMENT              4         // starting Gen9 the alignment is relaxed to 4x instead of 16x
#define MHW_VDBOX_HCP_RECON_UV_PLANE_ALIGNMENT            8

#define MHW_VDBOX_PAK_BITSTREAM_OVERFLOW_SIZE             400
#define MHW_VDBOX_VDENC_DYNAMIC_SLICE_WA_COUNT            1500

// Rowstore Cache values
#define MHW_VDBOX_PICWIDTH_1K                                                 1024
#define MHW_VDBOX_PICWIDTH_2K                                                 2048
#define MHW_VDBOX_PICWIDTH_3K                                                 3072
#define MHW_VDBOX_PICWIDTH_4K                                                 4096
#define MHW_VDBOX_PICWIDTH_8K                                                 8192
#define INTRAROWSTORE_FRAME_FIELD_BASEADDRESS_PICWIDTH_LESS_THAN_2K           256
#define INTRAROWSTORE_FRAME_FIELD_BASEADDRESS_PICWIDTH_BETWEEN_2K_AND_3K      384
#define INTRAROWSTORE_MBAFF_BASEADDRESS_PICWIDTH_LESS_THAN_2K                 512
#define INTRAROWSTORE_FRAME_FIELD_BASEADDRESS_PICWIDTH_BETWEEN_3K_AND_4K      384
#define DEBLOCKINGROWSTORE_FRAME_FIELD_BASEADDRESS_PICWIDTH_LESS_THAN_2K      384
#define BSDMPCROWSTORE_BASEADDRESS                                            0
#define MPRROWSTORE_FRAME_FIELD_BASEADDRESS_PICWIDTH_LESS_THAN_2K             128
#define MPRROWSTORE_FRAME_FIELD_BASEADDRESS_PICWIDTH_BETWEEN_2K_AND_3K        192
#define MPRROWSTORE_FRAME_FIELD_BASEADDRESS_PICWIDTH_GREATER_THAN_3K          256
#define MPRROWSTORE_MBAFF_BASEADDRESS_PICWIDTH_LESS_THAN_2K                   256
#define VDENCROWSTORE_FRAME_BASEADDRESS_PICWIDTH_LESS_THAN_2K                 128
#define VDENCROWSTORE_FRAME_BASEADDRESS_PICWIDTH_BETWEEN_2K_AND_3K            192
#define VDENCROWSTORE_FRAME_BASEADDRESS_PICWIDTH_BETWEEN_3K_AND_4K            256
#define HEVCDATROWSTORE_BASEADDRESS                                           0
#define HEVCDFROWSTORE_BASEADDRESS_PICWIDTH_LESS_THAN_OR_EQU_TO_2K            64
#define HEVCDFROWSTORE_BASEADDRESS_PICWIDTH_BETWEEN_2K_AND_4K                 128
#define HEVCSAOROWSTORE_BASEADDRESS_PICWIDTH_LESS_THAN_OR_EQU_TO_2K           320
#define VP9HVDROWSTORE_BASEADDRESS                                            0
#define VP9DFROWSTORE_BASEADDRESS_PICWIDTH_LESS_THAN_OR_EQU_TO_2K             32
#define MHW_CACHELINE_SIZE                                                    64
#define BYTES_PER_DWORD                                                       4
#define NUM_PAK_DWS_PER_LCU                                                   5
#define NUM_DWS_PER_CU                                                        8

#define VP9DFROWSTORE_BASEADDRESS_8BIT_PICWIDTH_LESS_THAN_OR_EQU_TO_4K          384
#define VP9DATROWSTORE_BASEADDRESS_8BIT_PICWIDTH_LESS_THAN_OR_EQU_TO_4K         64

typedef enum _MHW_VDBOX_ADDRESS_SHIFT
{
    MHW_VDBOX_SURFACE_STATE_SHIFT                    = 0,
    MHW_VDBOX_MFX_GENERAL_STATE_SHIFT                = 6,
    MHW_VDBOX_HCP_GENERAL_STATE_SHIFT                = 6,
    MHW_VDBOX_HUC_GENERAL_STATE_SHIFT                = 6,
    MHW_VDBOX_MFX_UPPER_BOUND_STATE_SHIFT            = 12,
    MHW_VDBOX_STATE_BASE_ADDRESS_SHIFT               = 12,
    MHW_VDBOX_HCP_UPPER_BOUND_STATE_SHIFT            = 12,
    MHW_VDBOX_HUC_UPPER_BOUND_STATE_SHIFT            = 12,
    MHW_VDBOX_HUC_IMEM_STATE_SHIFT                   = 15,
    MHW_VDBOX_HCP_DECODED_BUFFER_SHIFT               = 12
} MHW_VDBOX_ADDRESS_SHIFT;

typedef enum _MHW_VDBOX_NODE_IND
{
    MHW_VDBOX_NODE_1           = 0x0,
    MHW_VDBOX_NODE_2           = 0x1,
    MHW_VDBOX_NODE_MAX
} MHW_VDBOX_NODE_IND;

typedef struct _MHW_VDBOX_AVC_QM_PARAMS
{
    uint8_t List4x4[6][16];
    uint8_t List8x8[2][64];
} MHW_VDBOX_AVC_QM_PARAMS, *PMHW_VDBOX_AVC_QM_PARAMS;

typedef struct _MHW_VDBOX_HEVC_QM_PARAMS
{
    uint8_t List4x4[6][16];
    uint8_t List8x8[6][64];
    uint8_t List16x16[6][64];
    uint8_t List32x32[2][64];
    uint8_t ListDC16x16[6];
    uint8_t ListDC32x32[2];
} MHW_VDBOX_HEVC_QM_PARAMS, *PMHW_VDBOX_HEVC_QM_PARAMS;

typedef enum _HCP_SURFACE_FORMAT
{
    HCP_SURFACE_FORMAT_YUY2 = 0x0,
    HCP_SURFACE_FORMAT_RGBX8888 = 0x1,
    HCP_SURFACE_FORMAT_AYUV4444 = 0x2,
    HCP_SURFACE_FORMAT_P010_VARIANT = 0x3,
    HCP_SURFACE_FORMAT_PLANAR_420_8 = 0x4,
    HCP_SURFACE_FORMAT_UYVY = 0x5,
    HCP_SURFACE_FORMAT_YVYU = 0x6,
    HCP_SURFACE_FORMAT_VYUY = 0x7,
    HCP_SURFACE_FORMAT_Y210 = 0x8,
    HCP_SURFACE_FORMAT_Y216 = 0x8,
    HCP_SURFACE_FORMAT_RGBA1010102 = 0x9,
    HCP_SURFACE_FORMAT_Y410 = 0xA,
    HCP_SURFACE_FORMAT_NV21 = 0xB,
    HCP_SURFACE_FORMAT_Y416 = 0xC,
    HCP_SURFACE_FORMAT_P010 = 0xD,
    HCP_SURFACE_FORMAT_P016 = 0xE,
    HCP_SURFACE_FORMAT_Y8 = 0xF,
    HCP_SURFACE_FORMAT_Y16 = 0x10,
    HCP_SURFACE_FORMAT_Y216_VARIANT = 0x11,
    HCP_SURFACE_FORMAT_Y416_VARIANT = 0x12,
    HCP_SURFACE_FORMAT_YUYV_VARIANT = 0x13,
    HCP_SURFACE_FORMAT_AYUV4444_VARIANT = 0x14,
    HCP_SURFACE_FORMAT_RESERVED = 0x15,
} HCP_SURFACE_FORMAT;

typedef enum _PIPE_WORK_MODE
{
    MHW_VDBOX_HCP_PIPE_WORK_MODE_LEGACY             = 0,
    MHW_VDBOX_HCP_PIPE_WORK_MODE_CABAC_FE           = 1,
    MHW_VDBOX_HCP_PIPE_WORK_MODE_CODEC_BE           = 2,
    MHW_VDBOX_HCP_PIPE_WORK_MODE_CABAC_REAL_TILE    = 3,
}MHW_VDBOX_HCP_PIPE_WORK_MODE;

typedef enum _MULTI_ENGINE_MODE
{
    MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY     = 0,
    MHW_VDBOX_HCP_MULTI_ENGINE_MODE_LEFT          = 1,
    MHW_VDBOX_HCP_MULTI_ENGINE_MODE_RIGHT         = 2,
    MHW_VDBOX_HCP_MULTI_ENGINE_MODE_MIDDLE        = 3,
}MHW_VDBOX_HCP_MULTI_ENGINE_MODE;

typedef enum _VDENC_PIPE_NUM_OF_PIPE
{
    VDENC_PIPE_SINGLE_PIPE          = 0,
    VDENC_PIPE_TWO_PIPE             = 1,
    VDENC_PIPE_INVALID              = 2,
    VDENC_PIPE_FOUR_PIPE            = 3,
}VDENC_PIPE_NUM_OF_PIPE;

typedef enum
{
    HCP_CHROMA_FORMAT_MONOCHROME = 0,
    HCP_CHROMA_FORMAT_YUV420 = 1,
    HCP_CHROMA_FORMAT_YUV422 = 2,
    HCP_CHROMA_FORMAT_YUV444 = 3
} HCP_CHROMA_FORMAT_IDC;

// Media memory compression trigger
typedef enum _MHW_MEDIA_MEMORY_COMPRESSION_EN
{
    MHW_MEDIA_MEMCOMP_DISABLED = 0x0,
    MHW_MEDIA_MEMCOMP_ENABLED = 0x1
} MHW_MEDIA_MEMORY_COMPRESSION_EN;

// Media memory compression mode
typedef enum _MHW_MEDIA_MEMORY_COMPRESSION_MODE
{
    MHW_MEDIA_MEMCOMP_MODE_HORIZONTAL = 0x0,
    MHW_MEDIA_MEMCOMP_MODE_VERTICAL = 0x1,
} MHW_MEDIA_MEMORY_COMPRESSION_MODE;

//!
//! \enum     ROWSTORE_SCRATCH_BUFFER_CACHE
//! \brief    Rowstore scratch buffer cache select
//!
enum ROWSTORE_SCRATCH_BUFFER_CACHE
{
    BUFFER_TO_LLC                  = 0x0,
    BUFFER_TO_INTERNALMEDIASTORAGE = 0x1
};

//!
//! \enum     SLICE_THRESHOLD_TABLE_MODE
//! \brief    Slice thershold table mode, dynamic slice tuning params
//!
enum SLICE_THRESHOLD_TABLE_MODE
{
    NO_SLICE_THRESHOLD_TABLE = 0,
    USE_SLICE_THRESHOLD_TABLE_100_PERCENT = 1,
    USE_SLICE_THRESHOLD_TABLE_90_PERCENT = 2
};

struct MHW_VDBOX_PIPE_MODE_SELECT_PARAMS
{
    uint32_t                    Mode = 0;
    bool                        bStreamOutEnabled = false;
    bool                        bShortFormatInUse = false;
    bool                        bVC1OddFrameHeight = false;
    bool                        pakFrmLvlStrmoutEnable = false;
    bool                        pakPiplnStrmoutEnabled = false;

    bool                        bDeblockerStreamOutEnable = false;
    bool                        bPostDeblockOutEnable = false;
    bool                        bPreDeblockOutEnable = false;
    bool                        bDynamicSliceEnable = false;
    bool                        bSaoFirstPass = false;
    bool                        bRdoqEnable = false;
    bool                        bDynamicScalingEnabled = false;

    // VDEnc specific
    bool                        bVdencEnabled = false;
    bool                        bVdencStreamInEnable = false;
    uint8_t                     ucVdencBitDepthMinus8 = 0;
    bool                        bPakThresholdCheckEnable = false;
    bool                        bVdencPakObjCmdStreamOutEnable = false;
    bool                        bBatchBufferInUse = false;
    bool                        bTlbPrefetchEnable = 0;
    PMHW_BATCH_BUFFER           pBatchBuffer = nullptr;
    uint32_t                    ChromaType = 0;
    MOS_FORMAT                  Format = {};

    // HuC specific
    uint32_t                    dwMediaSoftResetCounterValue = 0;
    bool                        bAdvancedRateControlEnable = false;
    bool                        bStreamObjectUsed = false;
    // No need to set protection settings
    bool                        disableProtectionSetting = false;
    virtual ~MHW_VDBOX_PIPE_MODE_SELECT_PARAMS() {}
};
using PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS = MHW_VDBOX_PIPE_MODE_SELECT_PARAMS * ;

typedef struct _MHW_VDBOX_SURFACE_PARAMS
{
    uint32_t                    Mode;
    PMOS_SURFACE                psSurface;              // 2D surface parameters
    uint8_t                     ucVDirection;
    uint8_t                     ChromaType;
    uint8_t                     ucSurfaceStateId;
    uint8_t                     ucBitDepthLumaMinus8;
    uint8_t                     ucBitDepthChromaMinus8;
    uint32_t                    dwUVPlaneAlignment;
    bool                        bDisplayFormatSwizzle;
    bool                        bSrc8Pak10Mode;
    bool                        bColorSpaceSelection;
    bool                        bVdencDynamicScaling;
    uint32_t                    dwActualWidth;
    uint32_t                    dwActualHeight;
    uint32_t                    dwReconSurfHeight;
    MOS_MEMCOMP_STATE           mmcState;
    uint8_t                     mmcSkipMask;
    uint32_t                    dwCompressionFormat;
} MHW_VDBOX_SURFACE_PARAMS, *PMHW_VDBOX_SURFACE_PARAMS;

struct MHW_VDBOX_PIPE_BUF_ADDR_PARAMS
{
    uint32_t                    Mode = 0;
    PMOS_SURFACE                psPreDeblockSurface = nullptr;                            // Pointer to MOS_SURFACE of render surface
    MOS_MEMCOMP_STATE           PreDeblockSurfMmcState = MOS_MEMCOMP_DISABLED;
    PMOS_SURFACE                psPostDeblockSurface = nullptr;                           // Pointer to MOS_SURFACE of render surface
    MOS_MEMCOMP_STATE           PostDeblockSurfMmcState = MOS_MEMCOMP_DISABLED;
    PMOS_SURFACE                psRawSurface = nullptr;                                   // Pointer to MOS_SURFACE of raw surface
    MOS_MEMCOMP_STATE           RawSurfMmcState = MOS_MEMCOMP_DISABLED;
    PMOS_SURFACE                ps4xDsSurface = nullptr;
    MOS_MEMCOMP_STATE           Ps4xDsSurfMmcState = MOS_MEMCOMP_DISABLED;
    PMOS_SURFACE                ps8xDsSurface = nullptr;
    MOS_MEMCOMP_STATE           Ps8xDsSurfMmcState = MOS_MEMCOMP_DISABLED;
    PMOS_RESOURCE               presDataBuffer = nullptr;                                 // Handle of residual difference surface
    PMOS_RESOURCE               presReferences[CODEC_MAX_NUM_REF_FRAME] = {};
    PMOS_RESOURCE               presMfdIntraRowStoreScratchBuffer = nullptr;              // Handle of MFD Intra Row Store Scratch data surface
    PMOS_RESOURCE               presMfdDeblockingFilterRowStoreScratchBuffer = nullptr;   // Handle of MFD Deblocking Filter Row Store Scratch data surface
    PMOS_RESOURCE               presStreamOutBuffer = nullptr;
    MOS_MEMCOMP_STATE           StreamOutBufMmcState = MOS_MEMCOMP_DISABLED;
    PMOS_RESOURCE               presMacroblockIldbStreamOutBuffer1 = nullptr;
    PMOS_RESOURCE               presMacroblockIldbStreamOutBuffer2 = nullptr;
    PMOS_RESOURCE               presSliceSizeStreamOutBuffer = nullptr;
    PMOS_SURFACE                psFwdRefSurface0 = nullptr;
    PMOS_SURFACE                psFwdRefSurface1 = nullptr;
    PMOS_SURFACE                psFwdRefSurface2 = nullptr;
    bool                        bDynamicScalingEnable = false;

    PMOS_RESOURCE               presVdencIntraRowStoreScratchBuffer = nullptr;            // For VDEnc, Handle of VDEnc Intra Row Store Scratch data surface
    PMOS_RESOURCE               presVdencTileRowStoreBuffer = nullptr;
    PMOS_RESOURCE               presVdencStreamOutBuffer = nullptr;
    PMOS_RESOURCE               presVdencCuObjStreamOutBuffer = nullptr;
    PMOS_RESOURCE               presVdencPakObjCmdStreamOutBuffer = nullptr;
    PMOS_RESOURCE               presVdencStreamInBuffer = nullptr;
    PMOS_RESOURCE               presVdencReferences[CODEC_MAX_NUM_REF_FRAME] = {};
    PMOS_RESOURCE               presVdenc4xDsSurface[CODEC_MAX_NUM_REF_FRAME] = {};
    PMOS_RESOURCE               presVdenc8xDsSurface[CODEC_MAX_NUM_REF_FRAME] = {};

    PMOS_RESOURCE               presVdencColocatedMVWriteBuffer = nullptr;                      // For AVC only
    PMOS_RESOURCE               presVdencColocatedMVReadBuffer    = nullptr;                      // For AVC only
    PMOS_RESOURCE               presDeblockingFilterTileRowStoreScratchBuffer = nullptr;   // For HEVC, VP9
    PMOS_RESOURCE               presDeblockingFilterColumnRowStoreScratchBuffer = nullptr; // For HEVC, VP9
    PMOS_RESOURCE               presMetadataLineBuffer = nullptr;                          // For HEVC, VP9
    PMOS_RESOURCE               presMetadataTileLineBuffer = nullptr;                      // For HEVC, VP9
    PMOS_RESOURCE               presMetadataTileColumnBuffer = nullptr;                    // For HEVC, VP9
    PMOS_RESOURCE               presSaoLineBuffer = nullptr;                               // For HEVC only
    PMOS_RESOURCE               presSaoTileLineBuffer = nullptr;                           // For HEVC only
    PMOS_RESOURCE               presSaoTileColumnBuffer = nullptr;                         // For HEVC only
    PMOS_RESOURCE               presCurMvTempBuffer = nullptr;                             // For HEVC, VP9
    PMOS_RESOURCE               presColMvTempBuffer[CODEC_MAX_NUM_REF_FRAME] = {};    // For HEVC, VP9
    PMOS_RESOURCE               presLcuBaseAddressBuffer = nullptr;                        // For HEVC only
    PMOS_RESOURCE               presLcuILDBStreamOutBuffer = nullptr;                      // For HEVC only
    PMOS_RESOURCE               presVp9ProbBuffer = nullptr;                               // For VP9 only
    PMOS_RESOURCE               presVp9SegmentIdBuffer = nullptr;                          // For VP9 only
    PMOS_RESOURCE               presHvdLineRowStoreBuffer = nullptr;                       // For VP9 only
    PMOS_RESOURCE               presHvdTileRowStoreBuffer = nullptr;                       // For VP9 only
    PMOS_RESOURCE               presSaoStreamOutBuffer = nullptr;                          // For HEVC only
    PMOS_RESOURCE               presSaoRowStoreBuffer = nullptr;                           // For HEVC only
    PMOS_SURFACE                presP010RTSurface = nullptr;                               // For HEVC only
    PMOS_RESOURCE               presFrameStatStreamOutBuffer = nullptr;
    PMOS_RESOURCE               presSseSrcPixelRowStoreBuffer = nullptr;
    PMOS_RESOURCE               presSegmentMapStreamIn  = nullptr;
    PMOS_RESOURCE               presSegmentMapStreamOut = nullptr;
    PMOS_RESOURCE               presPakCuLevelStreamoutBuffer = nullptr;
    PMHW_VDBOX_SURFACE_PARAMS   pRawSurfParam = nullptr;
    PMHW_VDBOX_SURFACE_PARAMS   pDecodedReconParam = nullptr;
    bool                        bVdencEnabled = false;
    bool                        bRawIs10Bit = false;
    bool                        bDecodecReconIs10Bit = false;
    uint32_t                    dwNumRefIdxL0ActiveMinus1 = 0;
    uint32_t                    dwNumRefIdxL1ActiveMinus1 = 0;
    uint32_t                    dwLcuStreamOutOffset = 0;
    uint32_t                    dwFrameStatStreamOutOffset = 0;
    uint32_t                    dwVdencStatsStreamOutOffset = 0;
    bool                        oneOnOneMapping = false;                 // Flag for indicating using 1:1 ref index mapping for vdenc
    bool                        isLowDelayB = true;                      // Flag to indicate if it is LDB
    uint8_t                     IBCRefIdxMask = 0;
    PMOS_RESOURCE               presVdencCumulativeCuCountStreamoutSurface = nullptr;
    virtual ~MHW_VDBOX_PIPE_BUF_ADDR_PARAMS() {}
};
using PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS = MHW_VDBOX_PIPE_BUF_ADDR_PARAMS * ;

typedef struct _MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS
{
    uint32_t                    Mode;
    PMOS_RESOURCE               presDataBuffer;
    uint32_t                    dwDataSize;
    uint32_t                    dwDataOffset;
    PMOS_RESOURCE               presMvObjectBuffer;
    uint32_t                    dwMvObjectSize;
    uint32_t                    dwMvObjectOffset;
    PMOS_RESOURCE               presPakBaseObjectBuffer;
    uint32_t                    dwPakBaseObjectSize;
    PMOS_RESOURCE               presPakTileSizeStasBuffer;
    uint32_t                    dwPakTileSizeStasBufferSize;
    uint32_t                    dwPakTileSizeRecordOffset;
    // used by VP9
    PMOS_RESOURCE               presCompressedHeaderBuffer;
    uint32_t                    dwCompressedHeaderSize;
    PMOS_RESOURCE               presProbabilityDeltaBuffer;
    uint32_t                    dwProbabilityDeltaSize;
    PMOS_RESOURCE               presProbabilityCounterBuffer;
    uint32_t                    dwProbabilityCounterOffset;
    uint32_t                    dwProbabilityCounterSize;
    PMOS_RESOURCE               presTileRecordBuffer;
    uint32_t                    dwTileRecordSize;
    PMOS_RESOURCE               presCuStatsBuffer;
    uint32_t                    dwCuStatsSize;

    PMOS_RESOURCE               presStreamOutObjectBuffer;
    uint32_t                    dwStreamOutObjectSize;
    uint32_t                    dwStreamOutObjectOffset;
} MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS, *PMHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS;

struct MHW_VDBOX_AVC_IMG_PARAMS
{
    // Decoding Params
    PCODEC_AVC_PIC_PARAMS                   pAvcPicParams = nullptr;
    PCODEC_MVC_EXT_PIC_PARAMS               pMvcExtPicParams = nullptr;
    uint8_t                                 ucActiveFrameCnt = 0;
    // Encoding Params
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS       pEncodeAvcSeqParams = nullptr;
    PCODEC_AVC_ENCODE_PIC_PARAMS            pEncodeAvcPicParams = nullptr;
    PCODEC_AVC_ENCODE_SLICE_PARAMS          pEncodeAvcSliceParams = nullptr;
    PCODEC_REF_LIST                        *ppRefList = nullptr;
    CODEC_PIC_ID                           *pPicIdx = nullptr;
    uint32_t                                dwTqEnabled = 0;
    uint32_t                                dwTqRounding = 0;
    uint32_t                                dwMaxVmvR = 0;
    uint16_t                                wPicWidthInMb = 0;
    uint16_t                                wPicHeightInMb = 0;
    uint16_t                                wSlcHeightInMb = 0;
    uint8_t                                 ucKernelMode = 0;     // normal, performance, quality.

    //FEI multiple passes PAK ---max frame size
    uint8_t                                 ucCurrPass = 0;
    uint8_t                                *pDeltaQp = nullptr;
    uint32_t                                dwMaxFrameSize = 0;

    bool                                    bIPCMPass = false;
    // VDEnc specific
    bool                                    bVdencEnabled = false;
    bool                                    bVDEncPerfModeEnabled = false;
    bool                                    bVdencStreamInEnabled = false;
    bool                                    bVdencBRCEnabled = false;
    bool                                    bSliceSizeStreamOutEnabled = false;
    bool                                    bCrePrefetchEnable = false;

    uint32_t                                dwMbSlcThresholdValue = 0;  // For VDENC dynamic slice size control
    uint32_t                                dwSliceThresholdTable = 0;
    uint32_t                                dwVdencSliceMinusBytes = 0;
    uint8_t                                *pVDEncModeCost = nullptr;
    uint8_t                                *pVDEncMvCost = nullptr;
    uint8_t                                *pVDEncHmeMvCost = nullptr;
    uint32_t                               biWeight = 0;
    virtual ~MHW_VDBOX_AVC_IMG_PARAMS(){}
};
using PMHW_VDBOX_AVC_IMG_PARAMS = MHW_VDBOX_AVC_IMG_PARAMS * ;

typedef struct _MHW_VDBOX_QM_PARAMS
{
    uint32_t                             Standard;
    uint32_t                             Mode;
    PMHW_VDBOX_AVC_QM_PARAMS             pAvcIqMatrix;
    CodecMpeg2IqMatrix                  *pMpeg2IqMatrix;
    CodecJpegQuantMatrix                *pJpegQuantMatrix;
    uint32_t                             JpegQMTableSelector;
    bool                                 bJpegQMRotation;
    PMHW_VDBOX_HEVC_QM_PARAMS            pHevcIqMatrix;
} MHW_VDBOX_QM_PARAMS, *PMHW_VDBOX_QM_PARAMS;

typedef struct _MHW_VDBOX_AVC_WEIGHTOFFSET_PARAMS
{
    uint32_t                        uiList;
    uint32_t                        uiLumaLogWeightDenom;
    uint32_t                        uiChromaLogWeightDenom;
    uint32_t                        uiLumaWeightFlag;
    uint32_t                        uiChromaWeightFlag;
    uint32_t                        uiNumRefForList;
    int16_t                         Weights[2][32][3][2];
    PCODEC_AVC_ENCODE_PIC_PARAMS    pAvcPicParams;
} MHW_VDBOX_AVC_WEIGHTOFFSET_PARAMS, *PMHW_VDBOX_AVC_WEIGHTOFFSET_PARAMS;

typedef struct _MHW_VDBOX_PAK_INSERT_PARAMS
{
    PBSBuffer                               pBsBuffer;
    // also reuse dwBitSize for passing SrcDataEndingBitInclusion when (pEncoder->bLastPicInStream || pEncoder->bLastPicInSeq)
    uint32_t                                dwBitSize;
    uint32_t                                dwOffset;
    uint32_t                                uiSkipEmulationCheckCount;
    bool                                    bLastPicInSeq;
    bool                                    bLastPicInStream;
    bool                                    bLastHeader;
    bool                                    bEmulationByteBitsInsert;
    bool                                    bSetLastPicInStreamData;
    bool                                    bSliceHeaderIndicator;
    bool                                    bHeaderLengthExcludeFrmSize;
    uint32_t                               *pdwMpeg2PicHeaderTotalBufferSize;
    uint32_t                               *pdwMpeg2PicHeaderDataStartOffset;
    bool                                    bResetBitstreamStartingPos;
    bool                                    bEndOfSlice;
    uint32_t                                dwLastPicInSeqData;
    uint32_t                                dwLastPicInStreamData;
    PMHW_BATCH_BUFFER                       pBatchBufferForPakSlices;
    bool                                    bVdencInUse;
} MHW_VDBOX_PAK_INSERT_PARAMS, *PMHW_VDBOX_PAK_INSERT_PARAMS;

typedef struct _MHW_VDBOX_VP9_SEGMENT_STATE
{
    uint32_t                            Mode;
    PCODEC_VP9_SEGMENT_PARAMS           pVp9SegmentParams;
    PCODEC_VP9_ENCODE_SEGMENT_PARAMS    pVp9EncodeSegmentParams;
    uint8_t                             ucCurrentSegmentId;
    uint8_t                             ucQPIndexLumaAC;
    const uint8_t                      *pcucLfQpLookup;
    uint8_t                            *pbSegStateBufferPtr;
} MHW_VDBOX_VP9_SEGMENT_STATE, *PMHW_VDBOX_VP9_SEGMENT_STATE;

typedef struct _MHW_VDBOX_HCP_BSD_PARAMS
{
    uint32_t                            dwBsdDataLength;
    uint32_t                            dwBsdDataStartOffset;
} MHW_VDBOX_HCP_BSD_PARAMS, *PMHW_VDBOX_HCP_BSD_PARAMS;

typedef struct _MHW_VDBOX_ROWSTORE_PARAMS
{
    uint32_t   Mode;
    uint32_t   dwPicWidth;
    uint32_t   bMbaff;
    bool       bIsFrame;
    uint8_t    ucBitDepthMinus8;
    uint8_t    ucChromaFormat;
    uint8_t    ucLCUSize;
} MHW_VDBOX_ROWSTORE_PARAMS, *PMHW_VDBOX_ROWSTORE_PARAMS;

typedef struct _MHW_VDBOX_ROWSTORE_CACHE
{
    bool       bSupported;
    bool       bEnabled;
    uint32_t   dwAddress;
} MHW_VDBOX_ROWSTORE_CACHE, *PMHW_VDBOX_ROWSTORE_CACHE;

struct MHW_VDBOX_STATE_CMDSIZE_PARAMS
{
    bool       bShortFormat = false;
    bool       bHucDummyStream = false;
    bool       bSfcInUse = false;
    virtual ~MHW_VDBOX_STATE_CMDSIZE_PARAMS() {}
};
using PMHW_VDBOX_STATE_CMDSIZE_PARAMS = MHW_VDBOX_STATE_CMDSIZE_PARAMS * ;

typedef struct _MHW_VDBOX_AVC_SLICE_STATE
{
    PCODEC_PIC_ID                           pAvcPicIdx;
    PMOS_RESOURCE                           presDataBuffer;
    uint32_t                                dwDataBufferOffset;
    uint32_t                                dwOffset;
    uint32_t                                dwLength;
    uint32_t                                dwSliceIndex;
    bool                                    bLastSlice;
    uint32_t                                dwTotalBytesConsumed;

    // Decoding Only
    PCODEC_AVC_PIC_PARAMS                   pAvcPicParams;
    PCODEC_MVC_EXT_PIC_PARAMS               pMvcExtPicParams;
    PCODEC_AVC_SLICE_PARAMS                 pAvcSliceParams;
    uint32_t                                dwNextOffset;
    uint32_t                                dwNextLength;
    bool                                    bIntelEntrypointInUse;
    bool                                    bPicIdRemappingInUse;
    bool                                    bShortFormatInUse;
    bool                                    bPhantomSlice;
    uint8_t                                 ucDisableDeblockingFilterIdc;
    uint8_t                                 ucSliceBetaOffsetDiv2;
    uint8_t                                 ucSliceAlphaC0OffsetDiv2;

    // Encoding Only
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS       pEncodeAvcSeqParams;
    PCODEC_AVC_ENCODE_PIC_PARAMS            pEncodeAvcPicParams;
    PCODEC_AVC_ENCODE_SLICE_PARAMS          pEncodeAvcSliceParams;
    PBSBuffer                               pBsBuffer;
    PCODECHAL_NAL_UNIT_PARAMS              *ppNalUnitParams;
    PMHW_BATCH_BUFFER                       pBatchBufferForPakSlices;
    bool                                    bSingleTaskPhaseSupported;
    bool                                    bFirstPass;
    bool                                    bLastPass;
    bool                                    bBrcEnabled;
    bool                                    bRCPanicEnable;
    bool                                    bInsertBeforeSliceHeaders;
    bool                                    bAcceleratorHeaderPackingCaps;
    uint32_t                                dwBatchBufferForPakSlicesStartOffset;
    uint32_t                                uiSkipEmulationCheckCount;
    uint32_t                                dwRoundingValue;
    uint32_t                                dwRoundingIntraValue;
    bool                                    bRoundingInterEnable;
    uint16_t                                wFrameFieldHeightInMB;  // Frame/field Height in MB
    bool                                    bVdencInUse;
    bool                                    bVdencNoTailInsertion;
    bool                                    oneOnOneMapping = false;
    bool                                    bFullFrameData;
} MHW_VDBOX_AVC_SLICE_STATE, *PMHW_VDBOX_AVC_SLICE_STATE;

#endif
