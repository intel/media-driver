/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     mhw_vdbox_hcp_cmdpar.h
//! \brief    MHW command parameters
//! \details
//!

#ifndef __MHW_VDBOX_HCP_CMDPAR_H__
#define __MHW_VDBOX_HCP_CMDPAR_H__

#include <functional>
#include "codec_def_decode_hevc.h"
#include "codec_def_encode_hevc.h"
#include "mhw_vdbox.h"
#include "mhw_vdbox_cmdpar.h"

namespace mhw
{
namespace vdbox
{
namespace hcp
{
enum class SURFACE_FORMAT
{
    SURFACE_FORMAT_YUY2FORMAT           = 0,   //!< No additional details
    SURFACE_FORMAT_RGB8FORMAT           = 1,   //!< No additional details
    SURFACE_FORMAT_AYUV4444FORMAT       = 2,   //!< No additional details
    SURFACE_FORMAT_P010VARIANT          = 3,   //!< No additional details
    SURFACE_FORMAT_PLANAR4208           = 4,   //!< No additional details
    SURFACE_FORMAT_YCRCBSWAPYFORMAT     = 5,   //!< No additional details
    SURFACE_FORMAT_YCRCBSWAPUVFORMAT    = 6,   //!< No additional details
    SURFACE_FORMAT_YCRCBSWAPUVYFORMAT   = 7,   //!< No additional details
    SURFACE_FORMAT_Y216Y210FORMAT       = 8,   //!< Same value is used to represent Y216 and Y210
    SURFACE_FORMAT_RGB10FORMAT          = 9,   //!< No additional details
    SURFACE_FORMAT_Y410FORMAT           = 10,  //!< No additional details
    SURFACE_FORMAT_NV21PLANAR4208FORMAT = 11,  //!< No additional details
    SURFACE_FORMAT_Y416FORMAT           = 12,  //!< No additional details
    SURFACE_FORMAT_P010                 = 13,  //!< No additional details
    SURFACE_FORMAT_P016                 = 14,  //!< No additional details
    SURFACE_FORMAT_Y8FORMAT             = 15,  //!< No additional details
    SURFACE_FORMAT_Y16FORMAT            = 16,  //!< No additional details
    SURFACE_FORMAT_Y216VARIANT          = 17,  //!< Y216Variant is the modifed Y210/Y216 format, 8 bit planar 422 with MSB bytes packed together and LSB bytes at an offset in the X-direction where the x-offset is 32-bit aligned.   The chroma is UV interleaved with identical MSB and LSB split as luma and is at an offset in the Y-direction (similar to NV12) but is the same height as the luma.
    SURFACE_FORMAT_Y416VARIANT          = 18,  //!< Y416Variant is the modifed Y410/Y412/Y416 format,8 bit planar 444 with MSB bytes packed together and LSB bytes at an offset in the X-direction where the x-offset is 32-bit aligned.   The U channel is below the luma, has identical MSB and LSB split as luma and is at an offset in the Y-direction (similar to NV12) but is the same height as the luma  The V channel is below the U, has identical MSB and LSB split as luma and is at an offset in the Y-direction (similar to NV12) but is the same height as the luma.
    SURFACE_FORMAT_YUY2VARIANT          = 19,  //!< YUY2Variant is the modifed YUY2 format, 8 bit planar 422. The chroma is UV interleaved and is at an offset in the Y-direction (similar to NV12) but is the same height as the luma.
    SURFACE_FORMAT_AYUV4444VARIANT      = 20,  //!< AYUV4444Variant is the modifed AYUV4444 format, 8 bit planar 444 format.  The U channel is below the luma and is at an offset in the Y-direction (similar to NV12) but is the same height as the luma.  The V channel is below the and is at an offset in the Y-direction (similar to NV12) but is the same height as the luma.
};

enum class HCP_INTERNAL_BUFFER_TYPE
{
    DBLK_LINE = 0x0,
    DBLK_TILE_LINE,
    DBLK_TILE_COL,
    MV_UP_RT_COL,
    META_LINE,
    META_TILE_LINE,
    META_TILE_COL,
    TR_NBR,
    SAO_LINE,
    SAO_TILE_LINE,
    SAO_TILE_COL,
    HSSE_RS,
    HSAO_RS,
    CURR_MV_TEMPORAL,
    COLL_MV_TEMPORAL,
    SLC_STATE_STREAMOUT,
    CABAC_STREAMOUT,
    MV_UP_RIGHT_COL,
    INTRA_PRED_UP_RIGHT_COL,
    INTRA_PRED_LFT_RECON_COL,
    SEGMENT_ID,
    HVD_LINE,
    HVD_TILE
};

struct HcpMmioRegisters
{
    uint32_t watchdogCountCtrlOffset;
    uint32_t watchdogCountThresholdOffset;
    uint32_t hcpDebugFEStreamOutSizeRegOffset;
    uint32_t hcpEncImageStatusMaskRegOffset;
    uint32_t hcpEncImageStatusCtrlRegOffset;
    uint32_t hcpEncBitstreamBytecountFrameRegOffset;
    uint32_t hcpEncBitstreamSeBitcountFrameRegOffset;
    uint32_t hcpEncBitstreamBytecountFrameNoHeaderRegOffset;
    uint32_t hcpEncQpStatusCountRegOffset;
    uint32_t hcpEncSliceCountRegOffset;
    uint32_t hcpEncVdencModeTimerRegOffset;
    uint32_t hcpVp9EncBitstreamBytecountFrameRegOffset;
    uint32_t hcpVp9EncBitstreamBytecountFrameNoHeaderRegOffset;
    uint32_t hcpVp9EncImageStatusMaskRegOffset;
    uint32_t hcpVp9EncImageStatusCtrlRegOffset;
    uint32_t csEngineIdOffset;
    uint32_t hcpDecStatusRegOffset;
    uint32_t hcpCabacStatusRegOffset;
    uint32_t hcpFrameCrcRegOffset;
};

struct HcpBufferSizePar
{
    HCP_INTERNAL_BUFFER_TYPE bufferType;
    uint8_t                  ucMaxBitDepth;
    uint8_t                  ucChromaFormat;
    uint32_t                 dwCtbLog2SizeY;
    uint32_t                 dwPicWidth;
    uint32_t                 dwPicHeight;
    uint32_t                 dwMaxFrameSize;
    uint32_t                 dwBufferSize;
};

struct HcpVdboxRowStorePar
{
    uint32_t Mode;
    uint32_t dwPicWidth;
    uint32_t bMbaff;
    bool     bIsFrame;
    uint8_t  ucBitDepthMinus8;
    uint8_t  ucChromaFormat;
    uint8_t  ucLCUSize;
};

struct HCPPakHWTileSizeRecord
{
    uint32_t Address_31_0;   //DW0
    uint32_t Address_63_32;  //DW1
    uint32_t Length;         //DW2 Bitstream length per tile; includes header len in first tile, and tail len in last tile
    uint32_t TileSize;       //DW3 In Vp9, it is used for back annotation, In Hevc, it is the mmio register bytecountNoHeader
    uint32_t AddressOffset;  //DW4 Cacheline offset

    //DW5
    uint32_t
        ByteOffset : 6,  //[5:0] // Byte offset within cacheline
        Res_95_70 : 26;  //[31:6]

    uint32_t Hcp_Bs_SE_Bitcount_Tile;  //DW6 Bitstream size for syntax element per tile
    uint32_t Hcp_Cabac_BinCnt_Tile;    //DW7 Bitstream size for syntax element per tile
    uint32_t Res_DW8_31_0;             //DW8
    uint32_t Hcp_Image_Status_Ctrl;    //DW9 Image status control per tile
    uint32_t Hcp_Qp_Status_Count;      //DW10 Qp status count per tile
    uint32_t Hcp_Slice_Count_Tile;     //DW11 Number of slices per tile
    uint32_t Res_DW12_DW15[4];         //DW12-15 Reserved bits added so that QwordDisables are set correctly
};

enum CommandsNumberOfAddresses
{
    MI_BATCH_BUFFER_START_CMD_NUMBER_OF_ADDRESSES           = 1,  //  2 DW for  1 address field
    MI_STORE_DATA_IMM_CMD_NUMBER_OF_ADDRESSES               = 1,  //  2 DW for  1 address field
    MI_FLUSH_DW_CMD_NUMBER_OF_ADDRESSES                     = 1,  //  2 DW for  1 address field
    MI_CONDITIONAL_BATCH_BUFFER_END_CMD_NUMBER_OF_ADDRESSES = 1,  //  2 DW for  1 address field
    MI_STORE_REGISTER_MEM_CMD_NUMBER_OF_ADDRESSES           = 1,  //  2 DW for  1 address field
    MI_COPY_MEM_MEM_CMD_NUMBER_OF_ADDRESSES                 = 4,  //  4 DW for  2 address fields
    MI_SEMAPHORE_WAIT_CMD_NUMBER_OF_ADDRESSES               = 1,  //  2 DW for  1 address fields
    MI_ATOMIC_CMD_NUMBER_OF_ADDRESSES                       = 1,  //  2 DW for  1 address field

    MFX_WAIT_CMD_NUMBER_OF_ADDRESSES = 0,  //  0 DW for    address fields

    HCP_PIPE_MODE_SELECT_CMD_NUMBER_OF_ADDRESSES          = 0,   //  0 DW for    address fields
    HCP_SURFACE_STATE_CMD_NUMBER_OF_ADDRESSES             = 0,   //  0 DW for    address fields
    HCP_PIPE_BUF_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES       = 45,  //           45 address fields
    HCP_IND_OBJ_BASE_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES   = 11,  // 22 DW for 11 address field
    HCP_QM_STATE_CMD_NUMBER_OF_ADDRESSES                  = 0,   //  0 DW for    address fields
    HCP_FQM_STATE_CMD_NUMBER_OF_ADDRESSES                 = 0,   //  0 DW for    address fields
    HCP_PIC_STATE_CMD_NUMBER_OF_ADDRESSES                 = 0,   //  0 DW for    address fields
    HCP_REF_IDX_STATE_CMD_NUMBER_OF_ADDRESSES             = 0,   //  0 DW for    address fields
    HCP_WEIGHTOFFSET_STATE_CMD_NUMBER_OF_ADDRESSES        = 0,   //  0 DW for    address fields
    HCP_SLICE_STATE_CMD_NUMBER_OF_ADDRESSES               = 0,   //  0 DW for    address fields
    HCP_PAK_INSERT_OBJECT_CMD_NUMBER_OF_ADDRESSES         = 0,   //  0 DW for    address fields
    HCP_TILE_STATE_CMD_NUMBER_OF_ADDRESSES                = 0,   //  0 DW for    address fields
    HCP_BSD_OBJECT_CMD_NUMBER_OF_ADDRESSES                = 0,   //  0 DW for    address fields
    HCP_VP9_SEGMENT_STATE_CMD_NUMBER_OF_ADDRESSES         = 0,   //  0 DW for    address fields
    HCP_VP9_PIC_STATE_CMD_NUMBER_OF_ADDRESSES             = 0,   //  0 DW for    address fields
    HCP_TILE_CODING_COMMAND_NUMBER_OF_ADDRESSES           = 1,   //  0 DW for    address fields
    HCP_PALETTE_INITIALIZER_STATE_CMD_NUMBER_OF_ADDRESSES = 0,   //  0 DW for    address fields

    VDENC_PIPE_BUF_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES = 12,  // 12 DW for 12 address fields
    VD_PIPELINE_FLUSH_CMD_NUMBER_OF_ADDRESSES         = 0,   //  0 DW for  0 address fields
};

static constexpr uint32_t MAX_REF_FRAME_NUM = 15;

struct _MHW_PAR_T(HCP_PIC_STATE)
{
    bool     bDecodeInUse                            = false;
    uint16_t framewidthinmincbminus1                 = 0;
    bool     transformSkipEnabled                    = false;
    uint16_t frameheightinmincbminus1                = 0;
    uint8_t  mincusize                               = 0;
    uint8_t  ctbsizeLcusize                          = 0;
    uint8_t  maxtusize                               = 0;
    uint8_t  mintusize                               = 0;
    uint8_t  maxpcmsize                              = 0;
    uint8_t  minpcmsize                              = 0;
    bool     sampleAdaptiveOffsetEnabled             = false;
    bool     pcmEnabledFlag                          = false;
    bool     cuQpDeltaEnabledFlag                    = false;
    uint8_t  diffCuQpDeltaDepth                      = 0;
    bool     pcmLoopFilterDisableFlag                = false;
    bool     constrainedIntraPredFlag                = false;
    uint8_t  log2ParallelMergeLevelMinus2            = 0;
    bool     signDataHidingFlag                      = false;
    bool     weightedPredFlag                        = false;
    bool     weightedBipredFlag                      = false;
    bool     fieldpic                                = false;
    bool     bottomfield                             = false;
    bool     ampEnabledFlag                          = false;
    bool     transquantBypassEnableFlag              = false;
    bool     strongIntraSmoothingEnableFlag          = false;
    uint8_t  picCbQpOffset                           = 0;
    uint8_t  picCrQpOffset                           = 0;
    uint8_t  maxTransformHierarchyDepthIntra         = 0;
    uint8_t  maxTransformHierarchyDepthInter         = 0;
    uint8_t  pcmSampleBitDepthChromaMinus1           = 0;
    uint8_t  pcmSampleBitDepthLumaMinus1             = 0;
    uint8_t  bitDepthChromaMinus8                    = 0;
    uint8_t  bitDepthLumaMinus8                      = 0;
    uint16_t lcuMaxBitsizeAllowed                    = 0;
    uint8_t  lcuMaxBitSizeAllowedMsb2its             = 0;
    bool     rdoqEnable                              = false;
    bool     sseEnable                               = true;
    bool     rhodomainRateControlEnable              = true;
    uint8_t  rhodomainframelevelqp                   = 0;
    bool     fractionalQpAdjustmentEnable            = true;
    bool     pakDynamicSliceModeEnable               = false;
    uint8_t  slicePicParameterSetId                  = false;
    bool     nalunittypeflag                         = false;
    bool     noOutputOfPriorPicsFlag                 = false;
    uint32_t sliceSizeThresholdInBytes               = 0;
    uint32_t targetSliceSizeInBytes                  = 0;
    bool     tilesEnabledFlag                        = false;
    uint8_t  chromaSubsampling                       = 0;
    uint8_t  log2Maxtransformskipsize                = 0;
    bool     loopFilterAcrossTilesEnabled            = false;
    bool     entropyCodingSyncEnabled                = false;
    bool     intratucountbasedrdoqdisable            = false;
    uint16_t rdoqintratuthreshold                    = 0;
    bool     intraBoundaryFilteringDisabledFlag      = false;
    uint8_t  motionVectorResolutionControlIdc        = 0;
    bool     ppsCurrPicRefEnabledFlag                = false;
    uint8_t  ibcMotionCompensationBufferReferenceIdc = 0;
    uint8_t  ibcConfiguration                        = 0;
    bool     paletteModeEnabledFlag                  = 0;
    uint8_t  paletteMaxSize                          = 0;
    uint8_t  deltaPaletteMaxPredictorSize            = 0;
    uint8_t  lumaBitDepthEntryMinus8                 = 0;
    uint8_t  chromaBitDepthEntryMinus8               = 0;
    bool     partialFrameUpdateMode                  = false;
    bool     temporalMvPredDisable                   = false;
    uint16_t minframesize                            = 0;
    uint8_t  minframesizeunits                       = 0;
    uint8_t  ucRecNotFilteredID                      = 0;
    bool     deblockingFilterOverrideEnabled         = false;
    bool     ppsDeblockingFilterDisabled             = false;
    bool     requestCRC                              = false;
    uint8_t  bNotFirstPass                           = 0;
    PCODEC_HEVC_EXT_PIC_PARAMS pHevcExtPicParams     = nullptr;
    PCODEC_HEVC_SCC_PIC_PARAMS pHevcSccPicParams     = nullptr;
    bool     vdaqmEnable                             = false;
};

struct _MHW_PAR_T(HCP_SURFACE_STATE)
{
    uint8_t           surfaceStateId       = 0;
    uint32_t          surfacePitchMinus1   = 0;
    SURFACE_FORMAT    surfaceFormat        = SURFACE_FORMAT::SURFACE_FORMAT_PLANAR4208;
    uint32_t          yOffsetForUCbInPixel = 0;
    uint32_t          defaultAlphaValue    = 0;
    uint16_t          yOffsetForVCr        = 0;
    MOS_MEMCOMP_STATE mmcState             = MOS_MEMCOMP_DISABLED;
    uint8_t           mmcSkipMask          = 0;
    uint32_t          dwCompressionFormat  = 0;
    uint8_t           refsMmcEnable        = 0;
    uint8_t           refsMmcType          = 0;
};

struct _MHW_PAR_T(HCP_PIPE_MODE_SELECT)
{
    std::function<MOS_STATUS(uint32_t *cmdData)> setProtectionSettings;

    uint8_t  codecStandardSelect                = 0;
    bool     bAdvancedRateControlEnable         = false;
    bool     bStreamOutEnabled                  = false;
    bool     bBRCEnabled                        = false;
    bool     pakPiplnStrmoutEnabled             = false;
    bool     bDeblockerStreamOutEnable          = false;
    bool     bVdencEnabled                      = false;
    bool     bRdoqEnable                        = false;
    bool     pakFrmLvlStrmoutEnable             = false;
    bool     bTileBasedReplayMode               = false;
    bool     bDynamicScalingEnabled             = false;
    uint8_t  codecSelect                        = 1;
    uint8_t  ucPhaseIndicator                   = 0;
    bool     bHEVCSeparateTileProgramming       = false;
    bool     prefetchDisable                    = false;
    uint32_t mediaSoftResetCounterPer1000Clocks = 0;

    MHW_VDBOX_HCP_PIPE_WORK_MODE    pipeWorkMode    = MHW_VDBOX_HCP_PIPE_WORK_MODE_LEGACY;
    MHW_VDBOX_HCP_MULTI_ENGINE_MODE multiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY;
};

struct _MHW_PAR_T(HCP_SLICE_STATE)
{
    uint32_t slicestartctbxOrSliceStartLcuXEncoder         = 0;
    uint32_t slicestartctbyOrSliceStartLcuYEncoder         = 0;
    uint32_t nextslicestartctbxOrNextSliceStartLcuXEncoder = 0;
    uint32_t nextslicestartctbyOrNextSliceStartLcuYEncoder = 0;
    uint8_t  sliceType                                     = 0;
    bool     lastsliceofpic                                = false;
    bool     sliceqpSignFlag                               = false;
    bool     dependentSliceFlag                            = false;
    bool     sliceTemporalMvpEnableFlag                    = false;
    uint8_t  sliceqp                                       = 0;
    uint8_t  sliceCbQpOffset                               = 0;
    uint8_t  sliceCrQpOffset                               = 0;
    bool     intrareffetchdisable                          = false;
    bool     deblockingFilterDisable                       = false;
    char     tcOffsetDiv2                                  = 0;
    char     betaOffsetDiv2                                = 0;
    bool     loopFilterAcrossSlicesEnabled                 = false;
    bool     saoLumaFlag                                   = false;
    bool     saoChromaFlag                                 = false;
    bool     mvdL1ZeroFlag                                 = false;
    bool     isLowDelay                                    = false;
    bool     collocatedFromL0Flag                          = false;
    uint8_t  chromalog2Weightdenom                         = 0;
    uint8_t  lumaLog2WeightDenom                           = 0;
    bool     cabacInitFlag                                 = false;
    uint8_t  maxmergeidx                                   = 0;
    uint8_t  collocatedrefidx                              = 0;
    uint32_t sliceheaderlength                             = 0;
    bool     cabaczerowordinsertionenable                  = false;
    bool     emulationbytesliceinsertenable                = false;
    bool     tailInsertionEnable                           = false;
    bool     slicedataEnable                               = false;
    bool     headerInsertionEnable                         = false;
    uint32_t indirectPakBseDataStartOffsetWrite            = 0;
    uint16_t transformskiplambda                           = 0;
    uint8_t  transformskipNumzerocoeffsFactor0             = 0;
    uint8_t  transformskipNumnonzerocoeffsFactor0          = 0;
    uint8_t  transformskipNumzerocoeffsFactor1             = 0;
    uint8_t  transformskipNumnonzerocoeffsFactor1          = 0;
    bool     lastSliceInTile                               = false;
    bool     lastSliceInTileColumn                         = false;
    uint32_t roundinter                                    = 0;
    uint32_t roundintra                                    = 0;
    bool     cuChromaQpOffsetEnable                        = false;
    bool     bIsNotFirstTile                               = false;  //!< Not first tile in slice
    uint32_t originalSliceStartCtbX                        = 0;
    uint32_t originalSliceStartCtbY                        = 0;
    uint32_t dependentSliceDueToTileSplit                  = 0;
    uint32_t sliceActYQpOffset                             = 0;
    uint32_t sliceActCbQpOffset                            = 0;
    uint32_t sliceActCrQpOffset                            = 0;
    uint32_t useIntegerMvFlag                              = 0;
};

struct _MHW_PAR_T(HCP_IND_OBJ_BASE_ADDR_STATE)
{
    bool          bDecodeInUse                 = 0;
    PMOS_RESOURCE presDataBuffer               = nullptr;
    uint32_t      dwDataSize                   = 0;
    uint32_t      dwDataOffset                 = 0;
    PMOS_RESOURCE presMvObjectBuffer           = nullptr;
    uint32_t      dwMvObjectSize               = 0;
    uint32_t      dwMvObjectOffset             = 0;
    PMOS_RESOURCE presPakBaseObjectBuffer      = nullptr;
    uint32_t      dwPakBaseObjectSize          = 0;
    uint32_t      dwPakBaseObjectOffset        = 0;
    PMOS_RESOURCE presPakTileSizeStasBuffer    = nullptr;
    uint32_t      dwPakTileSizeStasBufferSize  = 0;
    uint32_t      dwPakTileSizeRecordOffset    = 0;
    PMOS_RESOURCE presCompressedHeaderBuffer   = nullptr;
    uint32_t      dwCompressedHeaderSize       = 0;
    PMOS_RESOURCE presProbabilityDeltaBuffer   = nullptr;
    uint32_t      dwProbabilityDeltaSize       = 0;
    PMOS_RESOURCE presProbabilityCounterBuffer = nullptr;
    uint32_t      dwProbabilityCounterOffset   = 0;
    uint32_t      dwProbabilityCounterSize     = 0;
    PMOS_RESOURCE presTileRecordBuffer         = nullptr;
    uint32_t      dwTileRecordSize             = 0;
    PMOS_RESOURCE presCuStatsBuffer            = nullptr;
    uint32_t      dwCuStatsSize                = 0;
    PMOS_RESOURCE presStreamOutObjectBuffer    = nullptr;
    uint32_t      dwStreamOutObjectSize        = 0;
    uint32_t      dwStreamOutObjectOffset      = 0;
};

struct _MHW_PAR_T(HCP_QM_STATE)
{
    uint8_t  predictionType      = 0;
    uint8_t  sizeid              = 0;
    uint8_t  colorComponent      = 0;
    uint8_t  dcCoefficient       = 0;
    uint32_t quantizermatrix[16] = {};
};

struct _MHW_PAR_T(HCP_FQM_STATE)
{
    uint8_t  intraInter          = 0;
    uint8_t  sizeid              = 0;
    uint8_t  colorComponent      = 0;
    uint16_t fqmDcValue1Dc       = 0;
    uint32_t quantizermatrix[32] = {};
};

struct _MHW_PAR_T(HCP_BSD_OBJECT)
{
    uint32_t bsdDataLength      = 0;
    uint32_t bsdDataStartOffset = 0;
};

struct _MHW_PAR_T(HCP_TILE_STATE)
{
    uint16_t *pTileColWidth        = nullptr;
    uint16_t *pTileRowHeight       = nullptr;
    uint8_t   numTileColumnsMinus1 = 0;
    uint8_t   numTileRowsMinus1    = 0;
};

struct _MHW_PAR_T(HCP_REF_IDX_STATE)
{
    uint8_t  ucList                                                             = 0;
    uint8_t  ucNumRefForList                                                    = 0;
    uint8_t  numRefIdxLRefpiclistnumActiveMinus1                                = 0;
    uint8_t  listEntryLxReferencePictureFrameIdRefaddr07[MAX_REF_FRAME_NUM + 1] = {};
    uint32_t referencePictureTbValue[MAX_REF_FRAME_NUM + 1]                     = {};
    bool     longtermreference[MAX_REF_FRAME_NUM + 1]                           = {};
    bool     fieldPicFlag[MAX_REF_FRAME_NUM + 1]                                = {};
    bool     bottomFieldFlag[MAX_REF_FRAME_NUM + 1]                             = {};
    bool     bDummyReference                                                    = false;
    bool     bDecodeInUse                                                       = false;
};

struct _MHW_PAR_T(HCP_WEIGHTOFFSET_STATE)
{
    uint8_t ucList                  = 0;
    char    LumaWeights[2][15]      = {};
    int16_t LumaOffsets[2][15]      = {};
    char    ChromaWeights[2][15][2] = {};
    int16_t ChromaOffsets[2][15][2] = {};
};

struct _MHW_PAR_T(HCP_PIPE_BUF_ADDR_STATE)
{
    uint32_t          Mode                                         = 0;
    PMOS_SURFACE      psPreDeblockSurface                          = nullptr;  // Pointer to MOS_SURFACE of render surface
    MOS_MEMCOMP_STATE PreDeblockSurfMmcState                       = MOS_MEMCOMP_DISABLED;
    PMOS_SURFACE      psPostDeblockSurface                         = nullptr;  // Pointer to MOS_SURFACE of render surface
    MOS_MEMCOMP_STATE PostDeblockSurfMmcState                      = MOS_MEMCOMP_DISABLED;
    PMOS_SURFACE      psRawSurface                                 = nullptr;  // Pointer to MOS_SURFACE of raw surface
    MOS_MEMCOMP_STATE RawSurfMmcState                              = MOS_MEMCOMP_DISABLED;
    PMOS_SURFACE      ps4xDsSurface                                = nullptr;
    MOS_MEMCOMP_STATE Ps4xDsSurfMmcState                           = MOS_MEMCOMP_DISABLED;
    PMOS_SURFACE      ps8xDsSurface                                = nullptr;
    MOS_MEMCOMP_STATE Ps8xDsSurfMmcState                           = MOS_MEMCOMP_DISABLED;
    PMOS_RESOURCE     presDataBuffer                               = nullptr;  // Handle of residual difference surface
    PMOS_RESOURCE     presReferences[CODEC_MAX_NUM_REF_FRAME]      = {};
    PMOS_RESOURCE     presMfdIntraRowStoreScratchBuffer            = nullptr;  // Handle of MFD Intra Row Store Scratch data surface
    PMOS_RESOURCE     presMfdDeblockingFilterRowStoreScratchBuffer = nullptr;  // Handle of MFD Deblocking Filter Row Store Scratch data surface
    PMOS_RESOURCE     presStreamOutBuffer                          = nullptr;
    MOS_MEMCOMP_STATE StreamOutBufMmcState                         = MOS_MEMCOMP_DISABLED;
    PMOS_RESOURCE     presMacroblockIldbStreamOutBuffer1           = nullptr;
    PMOS_RESOURCE     presMacroblockIldbStreamOutBuffer2           = nullptr;
    PMOS_RESOURCE     presSliceSizeStreamOutBuffer                 = nullptr;
    PMOS_SURFACE      psFwdRefSurface0                             = nullptr;
    PMOS_SURFACE      psFwdRefSurface1                             = nullptr;
    PMOS_SURFACE      psFwdRefSurface2                             = nullptr;
    bool              bDynamicScalingEnable                        = false;

    PMOS_RESOURCE presVdencIntraRowStoreScratchBuffer           = nullptr;  // For VDEnc, Handle of VDEnc Intra Row Store Scratch data surface
    PMOS_RESOURCE presVdencTileRowStoreBuffer                   = nullptr;
    PMOS_RESOURCE presVdencStreamOutBuffer                      = nullptr;
    PMOS_RESOURCE presVdencCuObjStreamOutBuffer                 = nullptr;
    PMOS_RESOURCE presVdencPakObjCmdStreamOutBuffer             = nullptr;
    PMOS_RESOURCE presVdencStreamInBuffer                       = nullptr;
    PMOS_RESOURCE presVdencReferences[CODEC_MAX_NUM_REF_FRAME]  = {};
    PMOS_RESOURCE presVdenc4xDsSurface[CODEC_MAX_NUM_REF_FRAME] = {};
    PMOS_RESOURCE presVdenc8xDsSurface[CODEC_MAX_NUM_REF_FRAME] = {};

    PMOS_RESOURCE             presVdencColocatedMVWriteBuffer                 = nullptr;  // For AVC only
    PMOS_RESOURCE             presVdencColocatedMVReadBuffer                  = nullptr;  // For AVC only
    PMOS_RESOURCE             presDeblockingFilterTileRowStoreScratchBuffer   = nullptr;  // For HEVC, VP9
    PMOS_RESOURCE             presDeblockingFilterColumnRowStoreScratchBuffer = nullptr;  // For HEVC, VP9
    PMOS_RESOURCE             presMetadataLineBuffer                          = nullptr;  // For HEVC, VP9
    PMOS_RESOURCE             presMetadataTileLineBuffer                      = nullptr;  // For HEVC, VP9
    PMOS_RESOURCE             presMetadataTileColumnBuffer                    = nullptr;  // For HEVC, VP9
    PMOS_RESOURCE             presSaoLineBuffer                               = nullptr;  // For HEVC only
    PMOS_RESOURCE             presSaoTileLineBuffer                           = nullptr;  // For HEVC only
    PMOS_RESOURCE             presSaoTileColumnBuffer                         = nullptr;  // For HEVC only
    PMOS_RESOURCE             presCurMvTempBuffer                             = nullptr;  // For HEVC, VP9
    PMOS_RESOURCE             presColMvTempBuffer[CODEC_MAX_NUM_REF_FRAME]    = {};       // For HEVC, VP9
    PMOS_RESOURCE             presLcuBaseAddressBuffer                        = nullptr;  // For HEVC only
    PMOS_RESOURCE             presLcuILDBStreamOutBuffer                      = nullptr;  // For HEVC only
    PMOS_RESOURCE             presVp9ProbBuffer                               = nullptr;  // For VP9 only
    PMOS_RESOURCE             presVp9SegmentIdBuffer                          = nullptr;  // For VP9 only
    PMOS_RESOURCE             presHvdLineRowStoreBuffer                       = nullptr;  // For VP9 only
    PMOS_RESOURCE             presHvdTileRowStoreBuffer                       = nullptr;  // For VP9 only
    PMOS_RESOURCE             presSaoStreamOutBuffer                          = nullptr;  // For HEVC only
    PMOS_RESOURCE             presSaoRowStoreBuffer                           = nullptr;  // For HEVC only
    PMOS_SURFACE              presP010RTSurface                               = nullptr;  // For HEVC only
    PMOS_RESOURCE             presFrameStatStreamOutBuffer                    = nullptr;
    PMOS_RESOURCE             presSseSrcPixelRowStoreBuffer                   = nullptr;
    PMOS_RESOURCE             presSegmentMapStreamIn                          = nullptr;
    PMOS_RESOURCE             presSegmentMapStreamOut                         = nullptr;
    PMOS_RESOURCE             presPakCuLevelStreamoutBuffer                   = nullptr;
    PMHW_VDBOX_SURFACE_PARAMS pRawSurfParam                                   = nullptr;
    PMHW_VDBOX_SURFACE_PARAMS pDecodedReconParam                              = nullptr;
    bool                      bVdencEnabled                                   = false;
    bool                      bRawIs10Bit                                     = false;
    bool                      bDecodecReconIs10Bit                            = false;
    uint32_t                  dwNumRefIdxL0ActiveMinus1                       = 0;
    uint32_t                  dwNumRefIdxL1ActiveMinus1                       = 0;
    uint32_t                  dwLcuStreamOutOffset                            = 0;
    uint32_t                  dwFrameStatStreamOutOffset                      = 0;
    uint32_t                  dwVdencStatsStreamOutOffset                     = 0;
    bool                      oneOnOneMapping                                 = false;  // Flag for indicating using 1:1 ref index mapping for vdenc
    bool                      isLowDelayB                                     = true;   // Flag to indicate if it is LDB
    bool                      isIFrame                                        = false;  // Flag to indicate if it is I frame
    bool                      isPFrame                                        = false;  // Flag to indicate if it is P frame
    bool                      bIBCEnabled                                     = false;
    uint8_t                   IBCRefIdxMask                                   = 0;
    PMOS_RESOURCE             presVdencCumulativeCuCountStreamoutSurface      = nullptr;

    //Scalable
    PMOS_RESOURCE presSliceStateStreamOutBuffer        = nullptr;
    PMOS_RESOURCE presMvUpRightColStoreBuffer          = nullptr;
    PMOS_RESOURCE presIntraPredUpRightColStoreBuffer   = nullptr;
    PMOS_RESOURCE presIntraPredLeftReconColStoreBuffer = nullptr;
    PMOS_RESOURCE presCABACSyntaxStreamOutBuffer       = nullptr;
    PMOS_RESOURCE presCABACSyntaxStreamOutMaxAddr      = nullptr;
};

struct _MHW_PAR_T(HCP_PAK_INSERT_OBJECT)
{
    PBSBuffer         pBsBuffer                        = nullptr;
    uint32_t          dwBitSize                        = 0;
    uint32_t          dwOffset                         = 0;
    uint32_t          uiSkipEmulationCheckCount        = 0;
    bool              bLastPicInSeq                    = false;
    bool              bLastPicInStream                 = false;
    bool              bLastHeader                      = false;
    bool              bEmulationByteBitsInsert         = false;
    bool              bSetLastPicInStreamData          = false;
    bool              bSliceHeaderIndicator            = false;
    bool              bHeaderLengthExcludeFrmSize      = false;
    uint32_t *        pdwMpeg2PicHeaderTotalBufferSize = nullptr;
    uint32_t *        pdwMpeg2PicHeaderDataStartOffset = nullptr;
    bool              bResetBitstreamStartingPos       = false;
    bool              bEndOfSlice                      = false;
    uint32_t          dwLastPicInSeqData               = 0;
    uint32_t          dwLastPicInStreamData            = 0;
    PMHW_BATCH_BUFFER pBatchBufferForPakSlices         = nullptr;
    bool              bVdencInUse                      = false;
    uint32_t          dataBitsInLastDw                 = 0;
    uint8_t           databyteoffset                   = 0;
    uint32_t          dwPadding                        = 0;
    bool              bIndirectPayloadEnable           = false;
};

struct _MHW_PAR_T(HCP_VP9_PIC_STATE)
{
    uint32_t frameWidthInPixelsMinus1          = 0;
    uint32_t frameHeightInPixelsMinus1         = 0;
    uint32_t frameType                         = 0;
    uint32_t adaptProbabilitiesFlag            = 0;
    uint32_t intraOnlyFlag                     = 0;
    uint32_t allowHiPrecisionMv                = 0;
    uint32_t mcompFilterType                   = 0;
    uint32_t refFrameSignBias02                = 0;
    uint32_t hybridPredictionMode              = 0;
    uint32_t selectableTxMode                  = 0;
    uint32_t usePrevInFindMvReferences         = 0;
    uint32_t lastFrameType                     = 0;
    uint32_t refreshFrameContext               = 0;
    uint32_t errorResilientMode                = 0;
    uint32_t frameParallelDecodingMode         = 0;
    uint32_t filterLevel                       = 0;
    uint32_t sharpnessLevel                    = 0;
    uint32_t segmentationEnabled               = 0;
    uint32_t segmentationUpdateMap             = 0;
    uint32_t segmentationTemporalUpdate        = 0;
    uint32_t losslessMode                      = 0;
    uint32_t segmentIdStreamOutEnable          = 0;
    uint32_t segmentIdStreamInEnable           = 0;
    uint32_t log2TileColumn                    = 0;
    uint32_t log2TileRow                       = 0;
    uint32_t sseEnable                         = 0;
    uint32_t chromaSamplingFormat              = 0;
    uint32_t bitdepthMinus8                    = 0;
    uint32_t profileLevel                      = 0;
    uint32_t verticalScaleFactorForLast        = 0;
    uint32_t horizontalScaleFactorForLast      = 0;
    uint32_t verticalScaleFactorForGolden      = 0;
    uint32_t horizontalScaleFactorForGolden    = 0;
    uint32_t verticalScaleFactorForAltref      = 0;
    uint32_t horizontalScaleFactorForAltref    = 0;
    uint32_t lastFrameWidthInPixelsMinus1      = 0;
    uint32_t lastFrameHeightInPixelsMinus1     = 0;
    uint32_t goldenFrameWidthInPixelsMinus1    = 0;
    uint32_t goldenFrameHeightInPixelsMinus1   = 0;
    uint32_t altrefFrameWidthInPixelsMinus1    = 0;
    uint32_t altrefFrameHeightInPixelsMinus1   = 0;
    uint32_t uncompressedHeaderLengthInBytes70 = 0;
    uint32_t firstPartitionSizeInBytes150      = 0;
    uint32_t baseQIndexSameAsLumaAc            = 0;
    uint32_t headerInsertionEnable             = 0;
    uint32_t chromaAcQIndexDelta               = 0;
    uint32_t chromaDcQIndexDelta               = 0;
    uint32_t lumaDcQIndexDelta                 = 0;
    uint32_t lfRefDelta0                       = 0;
    uint32_t lfRefDelta1                       = 0;
    uint32_t lfRefDelta2                       = 0;
    uint32_t lfRefDelta3                       = 0;
    uint32_t lfModeDelta0                      = 0;
    uint32_t lfModeDelta1                      = 0;
    uint32_t bitOffsetForLfRefDelta            = 0;
    uint32_t bitOffsetForLfModeDelta           = 0;
    uint32_t bitOffsetForQIndex                = 0;
    uint32_t bitOffsetForLfLevel               = 0;
    uint32_t vdencPakOnlyPass                  = 0;
    uint32_t bitOffsetForFirstPartitionSize    = 0;
    uint32_t dWordLength                       = 0;
    bool     bDecodeInUse                      = false;
};

struct _MHW_PAR_T(HCP_VP9_SEGMENT_STATE)
{
    uint32_t segmentId                         = 0;
    uint32_t segmentSkipped                    = 0;
    uint32_t segmentReference                  = 0;
    uint32_t segmentReferenceEnabled           = 0;
    uint32_t filterLevelRef0Mode0              = 0;
    uint32_t filterLevelRef0Mode1              = 0;
    uint32_t filterLevelRef1Mode0              = 0;
    uint32_t filterLevelRef1Mode1              = 0;
    uint32_t filterLevelRef2Mode0              = 0;
    uint32_t filterLevelRef2Mode1              = 0;
    uint32_t filterLevelRef3Mode0              = 0;
    uint32_t filterLevelRef3Mode1              = 0;
    uint32_t lumaDcQuantScaleDecodeModeOnly    = 0;
    uint32_t lumaAcQuantScaleDecodeModeOnly    = 0;
    uint32_t chromaDcQuantScaleDecodeModeOnly  = 0;
    uint32_t chromaAcQuantScaleDecodeModeOnly  = 0;
    uint32_t segmentQindexDeltaEncodeModeOnly  = 0;
    uint32_t segmentLfLevelDeltaEncodeModeOnly = 0;
};

struct _MHW_PAR_T(HEVC_VP9_RDOQ_STATE)
{
    bool     disableHtqPerformanceFix0 = false;
    bool     disableHtqPerformanceFix1 = false;
    uint16_t lambdaTab[2][2][76]       = {};
};

struct _MHW_PAR_T(HCP_TILE_CODING)
{
    uint32_t numOfTileColumnsInFrame              = 0;
    uint32_t tileStartLCUX                        = 0;
    uint32_t tileStartLCUY                        = 0;
    uint16_t tileHeightInMinCbMinus1              = 0;
    uint16_t tileWidthInMinCbMinus1               = 0;
    bool     isLastTileofColumn                   = false;
    bool     isLastTileofRow                      = false;
    uint32_t tileRowStoreSelect                   = 0;
    uint32_t tileColumnStoreSelect                = 0;
    bool     nonFirstPassTile                     = false;
    bool     bitstreamByteOffsetEnable            = false;
    uint32_t numberOfActiveBePipes                = 0;
    uint32_t bitstreamByteOffset                  = 0;
    uint32_t pakTileStatisticsOffset              = 0;
    uint32_t cuLevelStreamoutOffset               = 0;
    uint32_t sliceSizeStreamoutOffset             = 0;
    uint32_t cuRecordOffset                       = 0;
    uint32_t sseRowstoreOffset                    = 0;
    uint32_t saoRowstoreOffset                    = 0;
    uint32_t tileSizeStreamoutOffset              = 0;
    uint32_t vp9ProbabilityCounterStreamoutOffset = 0;
};

struct _MHW_PAR_T(HCP_PALETTE_INITIALIZER_STATE)
{
    uint8_t  predictorPaletteSize            = 0;
    uint16_t predictorPaletteEntries[3][128] = {};
    uint32_t hevcSccPaletteSize              = 0;
};

}  // namespace hcp
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_HCP_CMDPAR_H__
