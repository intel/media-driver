/*
* Copyright (c) 2021-2023 Intel Corporation
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
//! \file     mhw_vdbox_mfx_cmdpar.h
//! \brief    MHW command parameters
//! \details
//!

#ifndef __MHW_VDBOX_MFX_CMDPAR_H__
#define __MHW_VDBOX_MFX_CMDPAR_H__

#include "mhw_vdbox_cmdpar.h"
#include "codec_def_common_jpeg.h"
#include "codec_def_common_mpeg2.h"

#ifdef IGFX_MFX_INTERFACE_EXT_SUPPORT
#include "mhw_vdbox_mfx_cmdpar_ext.h"
#define __MHW_VDBOX_MFX_WRAPPER_EXT(STUFF) STUFF
#else
#define __MHW_VDBOX_MFX_WRAPPER_EXT(STUFF)
#endif

namespace mhw
{
namespace vdbox
{
namespace mfx
{

enum SURFACE_FORMAT
{
    SURFACE_FORMAT_YCRCBNORMAL      = 0,   //!< No additional details
    SURFACE_FORMAT_YCRCBSWAPUVY     = 1,   //!< No additional details
    SURFACE_FORMAT_YCRCBSWAPUV      = 2,   //!< No additional details
    SURFACE_FORMAT_YCRCBSWAPY       = 3,   //!< No additional details
    SURFACE_FORMAT_PLANAR4208       = 4,   //!< (NV12, IMC1,2,3,4, YV12)
    SURFACE_FORMAT_PLANAR4118       = 5,   //!< Deinterlace Only
    SURFACE_FORMAT_PLANAR4228       = 6,   //!< Deinterlace Only
    SURFACE_FORMAT_STMMDNSTATISTICS = 7,   //!< Deinterlace Only
    SURFACE_FORMAT_R10G10B10A2UNORM = 8,   //!< Sample_8x8 Only
    SURFACE_FORMAT_R8G8B8A8UNORM    = 9,   //!< Sample_8x8 Only
    SURFACE_FORMAT_R8B8UNORM_CRCB   = 10,  //!< Sample_8x8 Only
    SURFACE_FORMAT_R8UNORM_CRCB     = 11,  //!< Sample_8x8 Only
    SURFACE_FORMAT_Y8UNORM          = 12,  //!< Sample_8x8 Only
};

//!
//! \enum     MfxDecoderModeSelect
//! \brief    MFX decoder mode select
//!
enum MfxDecoderModeSelect
{
    mfxDecoderModeVld = 0,
    mfxDecoderModeIt  = 1
};

enum CommandsNumberOfAddresses
{
    // MFX Engine Commands
    MI_BATCH_BUFFER_START_CMD_NUMBER_OF_ADDRESSES           = 1,   //  2 DW for  1 address field
    MI_STORE_DATA_IMM_CMD_NUMBER_OF_ADDRESSES               = 1,   //  2 DW for  1 address field
    MI_FLUSH_DW_CMD_NUMBER_OF_ADDRESSES                     = 1,   //  2 DW for  1 address field
    MI_CONDITIONAL_BATCH_BUFFER_END_CMD_NUMBER_OF_ADDRESSES = 1,   //  2 DW for  1 address field
    MI_STORE_REGISTER_MEM_CMD_NUMBER_OF_ADDRESSES           = 1,   //  2 DW for  1 address field
    MFX_PIPE_MODE_SELECT_CMD_NUMBER_OF_ADDRESSES            = 0,   //  0 DW for    address fields
    MFX_SURFACE_STATE_CMD_NUMBER_OF_ADDRESSES               = 0,   //  0 DW for    address fields
    MFX_PIPE_BUF_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES         = 27,  // 50 DW for 25 address fields, added 2 for DownScaledReconPicAddr
    MFX_IND_OBJ_BASE_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES     = 5,   // 10 DW for  5 address fields
    MFX_WAIT_CMD_NUMBER_OF_ADDRESSES                        = 0,   //  0 DW for    address fields
    MFX_BSP_BUF_BASE_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES     = 3,   //  2 DW for  3 address fields
    MFD_AVC_PICID_STATE_CMD_NUMBER_OF_ADDRESSES             = 0,   //  0 DW for    address fields
    MFX_AVC_DIRECTMODE_STATE_CMD_NUMBER_OF_ADDRESSES        = 17,  // 50 DW for 17 address fields
    MFX_AVC_IMG_STATE_CMD_NUMBER_OF_ADDRESSES               = 0,   //  0 DW for    address fields
    MFX_QM_STATE_CMD_NUMBER_OF_ADDRESSES                    = 0,   //  0 DW for    address fields
    MFX_FQM_STATE_CMD_NUMBER_OF_ADDRESSES                   = 0,   //  0 DW for    address fields
    MFX_MPEG2_PIC_STATE_CMD_NUMBER_OF_ADDRESSES             = 0,   //  0 DW for    address fields
    MFX_DBK_OBJECT_CMD_NUMBER_OF_ADDRESSES                  = 4,   //  2 DW for  4 address fields
    MFX_VP8_PIC_STATE_CMD_NUMBER_OF_ADDRESSES               = 2,   //  2 DW for  2 address fields
    MFX_AVC_SLICE_STATE_CMD_NUMBER_OF_ADDRESSES             = 0,   //  0 DW for    address fields
    MFD_AVC_BSD_OBJECT_CMD_NUMBER_OF_ADDRESSES              = 0,   //  0 DW for    address fields
    MFD_AVC_DPB_STATE_CMD_NUMBER_OF_ADDRESSES               = 0,   //  0 DW for    address fields
    MFD_AVC_SLICEADDR_CMD_NUMBER_OF_ADDRESSES               = 0,   //  0 DW for    address fields
    MFX_AVC_REF_IDX_STATE_CMD_NUMBER_OF_ADDRESSES           = 0,   //  0 DW for    address fields
    MFX_AVC_WEIGHTOFFSET_STATE_CMD_NUMBER_OF_ADDRESSES      = 0,   //  0 DW for    address fields
    MFC_AVC_PAK_INSERT_OBJECT_CMD_NUMBER_OF_ADDRESSES       = 0,   //  0 DW for    address fields
    MFD_MPEG2_BSD_OBJECT_CMD_NUMBER_OF_ADDRESSES            = 0,   //  0 DW for    address fields
    MFD_MPEG2_IT_OBJECT_CMD_NUMBER_OF_ADDRESSES             = 0,   //  0 DW for    address fields
    MFD_VP8_BSD_OBJECT_CMD_NUMBER_OF_ADDRESSES              = 0,   //  0 DW for    address fields
};

/*MFX COMMON CMDS BELOW*/
struct _MHW_PAR_T(MFX_QM_STATE)
{
    uint32_t qmType              = 0;
    uint32_t quantizermatrix[16] = {};
};

struct _MHW_PAR_T(MFX_FQM_STATE)
{
    uint8_t  qmType              = 0;
    uint32_t quantizermatrix[32] = {};
};

struct _MHW_PAR_T(MFX_PIPE_MODE_SELECT)
{
    uint32_t Mode                                           = 0;
    uint32_t standardSelect                                 = 0;
    uint8_t  codecSelect                                    = 0;
    bool     frameStatisticsStreamoutEnable                 = false;
    bool     scaledSurfaceEnable                            = false;
    bool     preDeblockingOutputEnablePredeblockoutenable   = false;
    bool     postDeblockingOutputEnablePostdeblockoutenable = false;
    bool     streamOutEnable                                = false;
    bool     deblockerStreamOutEnable                       = false;
    uint8_t  vdencMode                                      = 0;
    uint8_t  decoderModeSelect                              = 0;
    uint8_t  decoderShortFormatMode                         = 0;
    bool     extendedStreamOutEnable                        = false;
    uint8_t  vlf720IOddHeightInVc1Mode                      = 0;
    bool     shortFormatInUse                               = false;
    bool     vc1OddFrameHeight                              = false;
    uint32_t mediaSoftResetCounterPer1000Clocks             = 0;
    bool     sliceSizeStreamout32bit                        = false;
};

struct _MHW_PAR_T(MFX_SURFACE_STATE)
{
    uint8_t      surfaceId         = 0;
    uint32_t     width             = 0;
    uint32_t     height            = 0;
    uint32_t     tilemode          = 0;
    uint32_t     surfacePitch      = 0;
    uint32_t     compressionFormat = 0;
    uint8_t      interleaveChroma  = 0;
    uint32_t     surfaceFormat     = 0;
    uint32_t     yOffsetForUCb     = 0;
    uint32_t     yOffsetForVCr     = 0;
    PMOS_SURFACE psSurface         = nullptr;
    uint32_t     uvPlaneAlignment  = 0;
    MOS_MEMCOMP_STATE mmcState     = MOS_MEMCOMP_DISABLED;
};

struct _MHW_PAR_T(MFX_PIPE_BUF_ADDR_STATE)
{
    uint32_t          Mode                                         = 0;
    bool              decodeInUse                                  = false;
    bool              oneOnOneMapping                              = false;                // Flag for indicating using 1:1 ref index mapping for vdenc
    PMOS_SURFACE      psPreDeblockSurface                          = nullptr;              // Pointer to MOS_SURFACE of render surface
    MOS_MEMCOMP_STATE PreDeblockSurfMmcState                       = MOS_MEMCOMP_DISABLED;
    PMOS_SURFACE      psPostDeblockSurface                         = nullptr;              // Pointer to MOS_SURFACE of render surface
    MOS_MEMCOMP_STATE PostDeblockSurfMmcState                      = MOS_MEMCOMP_DISABLED;
    PMOS_SURFACE      psRawSurface                                 = nullptr;              // Pointer to MOS_SURFACE of raw surface
    MOS_MEMCOMP_STATE RawSurfMmcState                              = MOS_MEMCOMP_DISABLED;
    PMOS_RESOURCE     presStreamOutBuffer                          = nullptr;
    PMOS_RESOURCE     presMfdIntraRowStoreScratchBuffer            = nullptr;              // Handle of MFD Intra Row Store Scratch data surface
    PMOS_RESOURCE     presMfdDeblockingFilterRowStoreScratchBuffer = nullptr;              // Handle of MFD Deblocking Filter Row Store Scratch data surface
    PMOS_RESOURCE     presReferences[CODEC_MAX_NUM_REF_FRAME]      = {};
    PMOS_RESOURCE     presVdencReferences[CODEC_MAX_NUM_REF_FRAME] = {};
    PMOS_RESOURCE     presSliceSizeStreamOutBuffer                 = nullptr;
    PMOS_SURFACE      ps4xDsSurface                                = nullptr;
    PMOS_RESOURCE     presMacroblockIldbStreamOutBuffer1           = nullptr;
    PMOS_RESOURCE     presMacroblockIldbStreamOutBuffer2           = nullptr;
    PMOS_RESOURCE *   references                                   = nullptr;
    MOS_MEMCOMP_STATE Ps4xDsSurfMmcState                           = MOS_MEMCOMP_DISABLED;
};

struct _MHW_PAR_T(MFX_IND_OBJ_BASE_ADDR_STATE)
{
    uint32_t      Mode                    = 0;
    PMOS_RESOURCE presDataBuffer          = nullptr;
    uint32_t      dwDataSize              = 0;
    uint32_t      dwDataOffset            = 0;
    PMOS_RESOURCE presMvObjectBuffer      = nullptr;
    uint32_t      dwMvObjectSize          = 0;
    uint32_t      dwMvObjectOffset        = 0;
    PMOS_RESOURCE presPakBaseObjectBuffer = nullptr;
    uint32_t      dwPakBaseObjectSize     = 0;
};

struct _MHW_PAR_T(MFX_BSP_BUF_BASE_ADDR_STATE)
{
    PMOS_RESOURCE presBsdMpcRowStoreScratchBuffer = nullptr;
    PMOS_RESOURCE presMprRowStoreScratchBuffer    = nullptr;
    PMOS_RESOURCE presBitplaneBuffer              = nullptr;
};

struct _MHW_PAR_T(MFX_PAK_INSERT_OBJECT)
{
    uint32_t dwPadding                                        = 0;
    bool     bitstreamstartresetResetbitstreamstartingpos     = false;
    bool     endofsliceflagLastdstdatainsertcommandflag       = false;
    bool     lastheaderflagLastsrcheaderdatainsertcommandflag = false;
    bool     emulationflagEmulationbytebitsinsertenable       = false;
    uint32_t skipemulbytecntSkipEmulationByteCount            = 0;
    uint32_t databitsinlastdwSrcdataendingbitinclusion50      = 0;
    bool     sliceHeaderIndicator                             = false;
    bool     headerlengthexcludefrmsize                       = false;
};

/*AVC CMDS BELOW*/
struct _MHW_PAR_T(MFX_AVC_IMG_STATE)
{
    uint32_t                  frameSize                                    = 0;
    uint16_t                  frameWidth                                   = 0;
    uint16_t                  frameHeight                                  = 0;
    uint8_t                   imgstructImageStructureImgStructure10        = 0;
    uint16_t                  weightedBipredIdc                            = 0;
    uint16_t                  weightedPredFlag                             = 0;
    char                      firstChromaQpOffset                          = 0;
    char                      secondChromaQpOffset                         = 0;
    bool                      fieldpicflag                                 = false;
    bool                      mbaffflameflag                               = false;
    bool                      vdencEnabled                                 = true;
    uint16_t                  framembonlyflag                              = 0;
    uint16_t                  transform8X8Flag                             = 0;
    uint16_t                  direct8X8Infflag                             = 0;
    uint16_t                  constrainedipredflag                         = 0;
    uint32_t                  imgdisposableflag                            = 0;
    uint16_t                  entropycodingflag                            = 0;
    uint8_t                   mbmvformatflag                               = 0;
    uint8_t                   chromaformatidc                              = 0;
    uint8_t                   mvunpackedflag                               = 0;
    bool                      mbstatenabled                                = false;
    uint8_t                   intrambmaxbitflagIntrambmaxsizereportmask    = 0;
    uint8_t                   intermbmaxbitflagIntermbmaxsizereportmask    = 0;
    uint8_t                   frameszoverflagFramebitratemaxreportmask     = 0;
    uint8_t                   frameszunderflagFramebitrateminreportmask    = 0;
    uint8_t                   intraIntermbipcmflagForceipcmcontrolmask     = 0;
    uint8_t                   mbratectrlflagMbLevelRateControlEnablingFlag = 0;
    uint32_t                  trellisQuantizationRoundingTqr               = 0;
    uint32_t                  trellisQuantizationEnabledTqenb              = 0;
    uint32_t                  intrambmaxsz                                 = 0;
    uint32_t                  intermbmaxsz                                 = 0;
    uint32_t                  framebitratemin                              = 0;
    uint32_t                  framebitrateminunitmode                      = 0;
    uint32_t                  framebitrateminunit                          = 0;
    uint32_t                  framebitratemax                              = 0;
    uint32_t                  framebitratemaxunitmode                      = 0;
    uint32_t                  framebitratemaxunit                          = 0;
    uint32_t                  framebitratemindelta                         = 0;
    uint32_t                  framebitratemaxdelta                         = 0;
    bool                      sliceStatsStreamoutEnable                    = false;
    char                      initialQpValue                               = 0;
    uint8_t                   numberOfActiveReferencePicturesFromL0        = 0;
    uint8_t                   numberOfActiveReferencePicturesFromL1        = 0;
    uint8_t                   numberOfReferenceFrames                      = 0;
    bool                      picOrderPresentFlag                          = false;
    bool                      deltaPicOrderAlwaysZeroFlag                  = false;
    uint32_t                  picOrderCntType                              = 0;
    uint8_t                   sliceGroupMapType                            = 0;
    bool                      redundantPicCntPresentFlag                   = false;
    uint8_t                   numSliceGroupsMinus1                         = 0;
    bool                      deblockingFilterControlPresentFlag           = false;
    uint32_t                  log2MaxFrameNumMinus4                        = 0;
    uint32_t                  log2MaxPicOrderCntLsbMinus4                  = 0;
    uint16_t                  sliceGroupChangeRate                         = 0;
    uint16_t                  currPicFrameNum                              = 0;
    uint16_t                  currentFrameViewId                           = 0;
    uint8_t                   maxViewIdxl0                                 = 0;
    uint8_t                   maxViewIdxl1                                 = 0;
    bool                      extendedRhodomainStatisticsEnable            = false;
    uint32_t                  thresholdSizeInBytes                         = 0;
    uint32_t                  targetSliceSizeInBytes                       = 0;
    PCODEC_AVC_PIC_PARAMS     avcPicParams                                 = nullptr;
    PCODEC_MVC_EXT_PIC_PARAMS mvcExtPicParams                              = nullptr;
    uint8_t                   activeFrameCnt                               = 0;
    bool                      decodeInUse                                  = false;
    uint32_t                  numMBs                                       = 0;
    uint32_t                  imgStructImageStructureImgStructure10        = 0;
    uint32_t                  interViewOrderDisable                        = 0;
    bool                      vdaqmEnable                                  = false;  
    uint8_t                   bitDepthLumaMinus8                           = 0;                
    uint8_t                   bitDepthChromaMinus8                         = 0;
};

struct _MHW_PAR_T(MFX_AVC_REF_IDX_STATE)
{
    uint32_t      uiList                = 0;
    uint32_t      referenceListEntry[8] = {};
    CODEC_PICTURE CurrPic               = {};
    uint32_t      numRefForList[2]      = {};
    PCODEC_PIC_ID pAvcPicIdx            = nullptr;
    void **       avcRefList            = nullptr;
    bool          intelEntrypointInUse  = false;
    bool          picIdRemappingInUse   = false;
    CODEC_PICTURE refPicList[2][32]     = {};
    bool          dummyReference        = false;
    bool          oneOnOneMapping       = false;
};

struct _MHW_PAR_T(MFX_AVC_WEIGHTOFFSET_STATE)
{
    uint32_t uiList               = 0;
    int16_t  Weights[2][32][3][2] = {};
    bool     decodeInUse          = false;
    uint32_t weightoffset[96]     = {};
};

struct _MHW_PAR_T(MFX_AVC_SLICE_STATE)
{
    uint8_t                   sliceType                                       = 0;
    uint8_t                   log2WeightDenomLuma                             = 0;
    uint8_t                   log2WeightDenomChroma                           = 0;
    uint8_t                   numberOfReferencePicturesInInterPredictionList0 = 0;
    uint8_t                   numberOfReferencePicturesInInterPredictionList1 = 0;
    uint8_t                   sliceAlphaC0OffsetDiv2                          = 0;
    uint8_t                   sliceBetaOffsetDiv2                             = 0;
    int8_t                    sliceQuantizationParameter                      = 0;
    uint8_t                   cabacInitIdc10                                  = 0;
    uint8_t                   disableDeblockingFilterIndicator                = 0;
    uint8_t                   directPredictionType                            = 0;
    uint8_t                   weightedPredictionIndicator                     = 0;
    uint16_t                  sliceStartMbNum                                 = 0;
    uint32_t                  sliceHorizontalPosition                         = 0;
    uint32_t                  sliceVerticalPosition                           = 0;
    uint32_t                   nextSliceHorizontalPosition                     = 0;
    uint32_t                   nextSliceVerticalPosition                       = 0;
    uint8_t                   sliceId30                                       = 0;
    bool                      cabaczerowordinsertionenable                    = false;
    bool                      emulationbytesliceinsertenable                  = false;
    bool                      tailInsertionPresentInBitstream                 = false;
    bool                      slicedataInsertionPresentInBitstream            = false;
    bool                      headerInsertionPresentInBitstream               = false;
    bool                      isLastSlice                                     = false;
    uint8_t                   roundintra                                      = 0;
    uint8_t                   roundinter                                      = 0;
    bool                      roundinterenable                                = false;
    bool                      intelEntrypointInUse                            = false;
    bool                      picIdRemappingInUse                             = false;
    bool                      shortFormatInUse                                = false;
    PMOS_RESOURCE             presDataBuffer                                  = nullptr;
    PCODEC_AVC_PIC_PARAMS     avcPicParams                                    = nullptr;
    PCODEC_MVC_EXT_PIC_PARAMS mvcExtPicParams                                 = nullptr;
    PCODEC_PIC_ID             avcPicIdx                                       = {};
    bool                      phantomSlice                                    = false;
    PCODEC_AVC_SLICE_PARAMS   avcSliceParams                                  = nullptr;
    uint32_t                  Offset                                          = 0;
    uint32_t                  Length                                          = 0;
    uint32_t                  nextOffset                                      = 0;
    uint32_t                  nextLength                                      = 0;
    uint32_t                  sliceIndex                                      = 0;
    bool                      fullFrameData                                   = 0;
    uint32_t                  totalBytesConsumed                              = 0;
    bool                      decodeInUse                                     = false;
};

struct _MHW_PAR_T(MFX_AVC_DIRECTMODE_STATE)
{
    CODEC_PICTURE CurrPic                 = {};
    uint32_t      uiUsedForReferenceFlags = 0;
    PMOS_RESOURCE presAvcDmvBuffers       = nullptr;
    uint8_t       ucAvcDmvIdx             = 0;
    PCODEC_PIC_ID pAvcPicIdx              = nullptr;
    void**        avcRefList              = nullptr;
    bool          bPicIdRemappingInUse    = false;
    bool          bDisableDmvBuffers      = false;
    MOS_RESOURCE  resAvcDmvBuffers[18]    = {};
};

struct _MHW_PAR_T(MFD_AVC_SLICEADDR)
{
    bool          decodeInUse                 = false;
    uint32_t      IndirectBsdDataLength       = 0;
    uint32_t      IndirectBsdDataStartAddress = 0;
    PMOS_RESOURCE presDataBuffer              = nullptr;
    uint32_t      dwSliceIndex                = 0;
    uint32_t      dwTotalBytesConsumed        = 0;
    PCODEC_AVC_SLICE_PARAMS avcSliceParams    = nullptr;
};

struct _MHW_PAR_T(MFD_AVC_BSD_OBJECT)
{
    bool     LastsliceFlag                          = false;
    uint32_t IntraPredmode4X48X8LumaErrorControlBit = 0;

    uint32_t                IntraPredictionErrorControlBitAppliedToIntra16X16Intra8X8Intra4X4LumaAndChroma = 0;
    uint32_t                Intra8X84X4PredictionErrorConcealmentControlBit                                = 0;
    uint32_t                ISliceConcealmentMode                                                          = 0;
    uint32_t                IndirectBsdDataLength                                                          = 0;
    uint32_t                IndirectBsdDataStartAddress                                                    = 0;
    uint32_t                FirstMbByteOffsetOfSliceDataOrSliceHeader                                      = 0;
    uint32_t                FirstMacroblockMbBitOffset                                                     = 0;
    uint32_t                FixPrevMbSkipped                                                               = 0;
    PMOS_RESOURCE           presDataBuffer                                                                 = nullptr;
    uint32_t                dwSliceIndex                                                                   = 0;
    uint32_t                dwTotalBytesConsumed                                                           = 0;
    bool                    decodeInUse                                                                    = false;
    PCODEC_AVC_SLICE_PARAMS pAvcSliceParams                                                                = nullptr;
};

struct _MHW_PAR_T(MFD_AVC_PICID_STATE)
{
    bool          bPicIdRemappingInUse      = false;
    PCODEC_PIC_ID pAvcPicIdx                = nullptr;
    uint32_t      Pictureidlist1616Bits[8]  = {};
    uint32_t      PictureidRemappingDisable = 0;
};

struct _MHW_PAR_T(MFD_AVC_DPB_STATE)
{
    PCODEC_AVC_PIC_PARAMS     pAvcPicParams                          = nullptr;
    PCODEC_MVC_EXT_PIC_PARAMS pMvcExtPicParams                       = nullptr;
    PCODEC_REF_LIST *         ppAvcRefList                           = nullptr;
    bool                      bPicIdRemappingInUse                   = false;
    uint32_t                  NonExistingframeFlag161Bit             = 0;
    uint32_t                  LongtermframeFlag161Bit                = 0;
    uint32_t                  usedForRef                             = 0;
    int16_t                   refFrameOrder[CODEC_MAX_NUM_REF_FRAME] = {0};
    PCODEC_PIC_ID             pAvcPicIdx                             = nullptr;
};

struct _MHW_PAR_T(MFX_JPEG_PIC_STATE)
{
    bool     decodeInUse                  = false;

    uint8_t  inputFormatYuv               = 0;
    uint8_t  rotation                     = 0;
    uint8_t  outputFormatYuv              = 0;
    bool     verticalDownSamplingEnable   = false;
    bool     horizontalDownSamplingEnable = false;
    bool     verticalUpSamplingEnable     = false;

    uint8_t  outputMcuStructure           = 0;
    uint8_t  inputSurfaceFormatYuv        = 0;
    uint8_t  pixelsInVerticalLastMcu      = 0;
    uint8_t  pixelsInHorizontalLastMcu    = 0;
    uint32_t frameWidthInBlocksMinus1     = 0; 
    uint32_t frameHeightInBlocksMinus1    = 0;

    uint32_t                  Mode             = 0;
    CodecDecodeJpegPicParams *pJpegPicParams   = nullptr;
    uint32_t                  dwOutputFormat   = 0;
    uint32_t                  dwWidthInBlocks  = 0;
    uint32_t                  dwHeightInBlocks = 0;
};

struct _MHW_PAR_T(MFC_JPEG_HUFF_TABLE_STATE)
{
    uint8_t  huffTableId  = 0;
    uint32_t dcTable[12]  = {};
    uint32_t acTable[162] = {};
};

struct _MHW_PAR_T(MFC_JPEG_SCAN_OBJECT)
{
    uint32_t mcuCount        = 0;
    uint16_t restartInterval = 0;
    uint8_t  huffmanDcTable  = 0;
    uint8_t  huffmanAcTable  = 0;
};

struct _MHW_PAR_T(MFX_JPEG_HUFF_TABLE_STATE)
{
    uint32_t huffTableID = 0;
    void *   pDCBits     = nullptr;
    void *   pDCValues   = nullptr;
    void *   pACBits     = nullptr;
    void *   pACValues   = nullptr;
};

struct _MHW_PAR_T(MFD_JPEG_BSD_OBJECT)
{
    uint32_t indirectDataLength     = 0;
    uint32_t dataStartAddress       = 0;
    uint32_t scanHorizontalPosition = 0;
    uint32_t scanVerticalPosition   = 0;
    bool     interleaved            = false;
    int16_t  scanComponent          = 0;
    uint32_t mcuCount               = 0;
    uint32_t restartInterval        = 0;
};

/*MPEG2 CMDS BELOW*/
struct _MHW_PAR_T(MFX_MPEG2_PIC_STATE)
{
    uint16_t ScanOrder                   = 0;
    uint16_t IntraVlcFormat              = 0;
    uint16_t QuantizerScaleType          = 0;
    uint16_t ConcealmentMotionVectorFlag = 0;
    uint16_t FramePredictionFrameDct     = 0;
    uint16_t TffTopFieldFirst            = 0;
    uint16_t PictureStructure            = 0;
    uint16_t IntraDcPrecision            = 0;
    uint16_t FCode00                     = 0;
    uint16_t FCode01                     = 0;
    uint16_t FCode10                     = 0;
    uint16_t FCode11                     = 0;
    
    int32_t  PictureCodingType                                                = 0;
    uint32_t ISliceConcealmentMode                                            = 0;
    uint32_t PBSliceConcealmentMode                                           = 0;
    uint32_t PBSlicePredictedBidirMotionTypeOverrideBiDirectionMvTypeOverride = 0;
    uint32_t PBSlicePredictedMotionVectorOverrideFinalMvValueOverride         = 0;
    uint32_t SliceConcealmentDisableBit                                       = 0;

    uint16_t Framewidthinmbsminus170PictureWidthInMacroblocks   = 0;
    uint16_t Frameheightinmbsminus170PictureHeightInMacroblocks = 0;

    uint16_t mfxMpeg2PicStatePar0 = 0;

    uint32_t Roundintradc = 0;
    uint32_t Roundinterdc = 0;
    uint32_t Roundintraac = 0;
    uint32_t Roundinterac = 0;
};

struct _MHW_PAR_T(MFD_MPEG2_BSD_OBJECT)
{
    bool          decodeInUse       = false;
    PMOS_RESOURCE presDataBuffer    = nullptr;
    uint32_t      dwDataStartOffset = 0;

    uint32_t IndirectBsdDataLength    = 0;
    uint32_t IndirectDataStartAddress = 0;
    
    uint32_t FirstMacroblockBitOffset = 0;
    bool     IsLastMb                 = false;
    bool     LastPicSlice             = false;
    bool     MbRowLastSlice           = false;
    uint32_t MacroblockCount          = 0;
    uint32_t SliceHorizontalPosition  = 0;
    uint32_t SliceVerticalPosition    = 0;

    int32_t  QuantizerScaleCode          = 0;
    uint16_t NextSliceHorizontalPosition = 0;
    uint16_t NextSliceVerticalPosition   = 0;
};

/*VP8 CMDS BELOW*/
struct _MHW_PAR_T(MFX_VP8_PIC_STATE)
{
    uint32_t dwCoefProbTableOffset                  = 0;
    PMOS_RESOURCE presCoefProbBuffer                = nullptr;
    PMOS_RESOURCE presSegmentationIdStreamBuffer    = nullptr;
    // DW1
    uint32_t FrameWidthMinus1                       = 0;
    uint32_t FrameHeightMinus1                      = 0;
    // DW2
    uint32_t McFilterSelect                         = 0;
    uint32_t ChromaFullPixelMcFilterMode            = 0;
    uint32_t Dblkfiltertype                         = 0;
    uint32_t Skeyframeflag                          = 0;
    uint32_t SegmentationIdStreamoutEnable          = 0;
    uint32_t SegmentationIdStreaminEnable           = 0;
    uint32_t SegmentEnableFlag                      = 0;
    uint32_t UpdateMbsegmentMapFlag                 = 0;
    uint32_t MbNocoeffSkipflag                      = 0;
    uint32_t ModeReferenceLoopFilterDeltaEnabled    = 0;
    uint32_t GoldenRefPictureMvSignbiasFlag         = 0;
    uint32_t AlternateRefPicMvSignbiasFlag          = 0;
    uint32_t DeblockSharpnessLevel                  = 0;
    // DW3
    uint32_t DblkfilterlevelForSegment3             = 0;
    uint32_t DblkfilterlevelForSegment2             = 0;
    uint32_t DblkfilterlevelForSegment1             = 0;
    uint32_t DblkfilterlevelForSegment0             = 0;
    // DW4
    uint32_t QuantizerValue0Blocktype0Y1Dc          = 0;
    uint32_t QuantizerValue0Blocktype1Y1Ac          = 0;
    // DW5
    uint32_t QuantizerValue0Blocktype2Uvdc          = 0;
    uint32_t QuantizerValue0Blocktype3Uvac          = 0;
    // DW6
    uint32_t QuantizerValue0Blocktype4Y2Dc          = 0;
    uint32_t QuantizerValue0Blocktype5Y2Ac          = 0;
    // DW7
    uint32_t QuantizerValue1Blocktype0Y1Dc          = 0;
    uint32_t QuantizerValue1Blocktype1Y1Ac          = 0;
    // DW8
    uint32_t QuantizerValue1Blocktype2Uvdc          = 0;
    uint32_t QuantizerValue1Blocktype3Uvac          = 0;
    // DW9
    uint32_t QuantizerValue1Blocktype4Y2Dc          = 0;
    uint32_t QuantizerValue1Blocktype5Y2Ac          = 0;
    // DW10
    uint32_t QuantizerValue2Blocktype0Y1Dc          = 0;
    uint32_t QuantizerValue2Blocktype1Y1Ac          = 0;
    // DW11
    uint32_t QuantizerValue2Blocktype2Uvdc          = 0;
    uint32_t QuantizerValue2Blocktype3Uvac          = 0;
    // DW12
    uint32_t QuantizerValue2Blocktype4Y2Dc          = 0;
    uint32_t QuantizerValue2Blocktype5Y2Ac          = 0;
    // DW13
    uint32_t QuantizerValue3Blocktype0Y1Dc          = 0;
    uint32_t QuantizerValue3Blocktype1Y1Ac          = 0;
    // DW14
    uint32_t QuantizerValue3Blocktype2Uvdc          = 0;
    uint32_t QuantizerValue3Blocktype3Uvac          = 0;
    // DW15
    uint32_t QuantizerValue3Blocktype4Y2Dc          = 0;
    uint32_t QuantizerValue3Blocktype5Y2Ac          = 0;
    // DW19
    uint32_t Mbsegmentidtreeprobs0                  = 0;
    uint32_t Mbsegmentidtreeprobs1                  = 0;
    uint32_t Mbsegmentidtreeprobs2                  = 0;
    // DW20
    uint32_t Mbnocoeffskipfalseprob                 = 0;
    uint32_t Intrambprob                            = 0;
    uint32_t Interpredfromlastrefprob               = 0;
    uint32_t Interpredfromgrefrefprob               = 0;
    // DW21
    uint32_t Ymodeprob3                             = 0;
    uint32_t Ymodeprob2                             = 0;
    uint32_t Ymodeprob1                             = 0;
    uint32_t Ymodeprob0                             = 0;
    // DW22
    uint32_t Uvmodeprob2                            = 0;
    uint32_t Uvmodeprob1                            = 0;
    uint32_t Uvmodeprob0                            = 0;
    // DW23
    uint32_t Mvupdateprobs00                        = 0;
    uint32_t Mvupdateprobs01                        = 0;
    uint32_t Mvupdateprobs02                        = 0;
    uint32_t Mvupdateprobs03                        = 0;
    // DW24
    uint32_t Mvupdateprobs04                        = 0;
    uint32_t Mvupdateprobs05                        = 0;
    uint32_t Mvupdateprobs06                        = 0;
    uint32_t Mvupdateprobs07                        = 0;
    // DW25
    uint32_t Mvupdateprobs08                        = 0;
    uint32_t Mvupdateprobs09                        = 0;
    uint32_t Mvupdateprobs010                       = 0;
    uint32_t Mvupdateprobs011                       = 0;
    // DW26
     uint32_t Mvupdateprobs012                      = 0;
    uint32_t Mvupdateprobs013                       = 0;
    uint32_t Mvupdateprobs014                       = 0;
    uint32_t Mvupdateprobs015                       = 0;
    // DW27
     uint32_t Mvupdateprobs016                      = 0;
    uint32_t Mvupdateprobs017                       = 0;
    uint32_t Mvupdateprobs018                       = 0;
    // DW28
    uint32_t Mvupdateprobs10                        = 0;
    uint32_t Mvupdateprobs11                        = 0;
    uint32_t Mvupdateprobs12                        = 0;
    uint32_t Mvupdateprobs13                        = 0;
    // DW29
    uint32_t Mvupdateprobs14                        = 0;
    uint32_t Mvupdateprobs15                        = 0;
    uint32_t Mvupdateprobs16                        = 0;
    uint32_t Mvupdateprobs17                        = 0;
    // DW30
    uint32_t Mvupdateprobs18                        = 0;
    uint32_t Mvupdateprobs19                        = 0;
    uint32_t Mvupdateprobs110                       = 0;
    uint32_t Mvupdateprobs111                       = 0;
    // DW31
     uint32_t Mvupdateprobs112                      = 0;
    uint32_t Mvupdateprobs113                       = 0;
    uint32_t Mvupdateprobs114                       = 0;
    uint32_t Mvupdateprobs115                       = 0;
    // DW32
    uint32_t Mvupdateprobs116                       = 0;
    uint32_t Mvupdateprobs117                       = 0;
    uint32_t Mvupdateprobs118                       = 0;
    // DW33
    uint32_t Reflfdelta0ForIntraFrame               = 0;
    uint32_t Reflfdelta1ForLastFrame                = 0;
    uint32_t Reflfdelta2ForGoldenFrame              = 0;
    uint32_t Reflfdelta3ForAltrefFrame              = 0;
    // DW34
    uint32_t Modelfdelta0ForBPredMode               = 0;
    uint32_t Modelfdelta1ForZeromvMode              = 0;
    uint32_t Modelfdelta2ForNearestNearAndNewMode   = 0;
    uint32_t Modelfdelta3ForSplitmvMode             = 0;
};

struct _MHW_PAR_T(MFD_VP8_BSD_OBJECT)
{
    // DW1
    uint32_t CodedNumOfCoeffTokenPartitions     = 0;
    uint32_t Partition0CpbacEntropyRange        = 0;
    uint32_t Partition0CpbacEntropyCount        = 0;
    // DW2
    uint32_t Partition0CpbacEntropyValue        = 0;
    // DW3
    uint32_t IndirectPartition0DataLength       = 0;
    // DW4
    uint32_t IndirectPartition0DataStartOffset  = 0;
    // DW5
    uint32_t IndirectPartition1DataLength       = 0;
    // DW6
    uint32_t IndirectPartition1DataStartOffset  = 0;
    // DW7-8
    uint32_t IndirectPartition2DataLength       = 0;
    uint32_t IndirectPartition2DataStartOffset  = 0;
    // DW9-10
    uint32_t IndirectPartition3DataLength       = 0;
    uint32_t IndirectPartition3DataStartOffset  = 0;
    // DW11-12
    uint32_t IndirectPartition4DataLength       = 0;
    uint32_t IndirectPartition4DataStartOffset  = 0;
    // DW13-14
    uint32_t IndirectPartition5DataLength       = 0;
    uint32_t IndirectPartition5DataStartOffset  = 0;
    // DW15-16
    uint32_t IndirectPartition6DataLength       = 0;
    uint32_t IndirectPartition6DataStartOffset  = 0;
    // DW17-18
    uint32_t IndirectPartition7DataLength       = 0;
    uint32_t IndirectPartition7DataStartOffset  = 0;
    // DW19-20
    uint32_t IndirectPartition8DataLength       = 0;
    uint32_t IndirectPartition8DataStartOffset  = 0;
};

struct _MHW_PAR_T(MFD_IT_OBJECT)
{
    uint32_t dwDCTLength                           = 0;
    uint32_t DwordLength                           = 0;
    uint32_t IndirectItCoeffDataStartAddressOffset = 0;
};

struct _MHW_PAR_T(MFD_IT_OBJECT_MPEG2_INLINE_DATA)
{
    CodecDecodeMpeg2MbParams *pMBParams = nullptr;

    int32_t CodingType     = 0;
    int16_t sPackedMVs0[4] = {};
    int16_t sPackedMVs1[4] = {};

    uint16_t CodedBlockPattern = 0;
    bool     Lastmbinrow       = false;

    uint32_t Horzorigin = 0;
    uint32_t Vertorigin = 0;
};

}  // namespace mfx
}  // namespace vdbox
}  // namespace mhw

#endif  //__MHW_VDBOX_MFX_CMDPAR_H__

