/*
* Copyright (c) 2020-2024, Intel Corporation
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
//! \file     mhw_vdbox_avp_interface.h
//! \brief    MHW interface for constructing AVP commands for the Vdbox engine
//! \details  Defines the interfaces for constructing MHW Vdbox AVP commands across all platforms
//!

#ifndef _MHW_VDBOX_AVP_INTERFACE_H_
#define _MHW_VDBOX_AVP_INTERFACE_H_

#include "mhw_vdbox.h"
#include "mhw_mi.h"
#include "codec_def_decode_av1.h"

struct MhwVdboxAvpTileCodingParams
{
    uint16_t                        m_tileId;
    uint16_t                        m_tgTileNum;  //!< Tile ID in its Tile group
    uint16_t                        m_tileGroupId;

    uint16_t                        m_tileColPositionInSb;
    uint16_t                        m_tileRowPositionInSb;

    uint16_t                        m_tileWidthInSbMinus1;              //!< Tile width minus 1 in SB unit
    uint16_t                        m_tileHeightInSbMinus1;             //!< Tile height minus 1 in SB unit

    bool                            m_tileRowIndependentFlag;
    bool                            m_firstTileInAFrame;
    bool                            m_isLastTileOfColumn;
    bool                            m_isLastTileOfRow;
    bool                            m_isFirstTileOfTileGroup;
    bool                            m_isLastTileOfTileGroup;
    bool                            m_isLastTileOfFrame;
    bool                            m_disableCdfUpdateFlag;
    bool                            m_disableFrameContextUpdateFlag;

    uint8_t                         m_numOfActiveBePipes;
    uint16_t                        m_numOfTileColumnsInFrame;
    uint16_t                        m_numOfTileRowsInFrame;
    uint16_t                        m_outputDecodedTileColumnPositionInSBUnit;
    uint16_t                        m_outputDecodedTileRowPositionInSBUnit;
};

struct MhwVdboxAvpPicStateParams
{
    // Decode
    CodecAv1PicParams               *m_picParams;
    PCODEC_REF_LIST_AV1             *m_refList;

    union
    {
        struct
        {
            uint8_t                 m_keyFrame      : 1;        // [0..1]
            uint8_t                 m_intraOnly     : 1;        // [0..1]
            uint8_t                 m_display       : 1;        // [0..1]
            uint8_t                 m_reservedField : 5;        // [0]
        } m_fields;
        uint8_t                     m_value;
    } m_prevFrameParams;

    uint32_t                        m_prevFrmWidth;
    uint32_t                        m_prevFrmHeight;

    //driver internally
    uint8_t                         m_referenceFrameSignBias[8];
    int8_t                          m_skipModeFrame[2]; //offset to LAST_FRAME, should be sign value
    uint32_t                        m_lumaPlaneXStepQn;
    uint32_t                        m_lumaPlaneX0Qn;
    uint32_t                        m_chromaPlaneXStepQn;
    uint32_t                        m_chromaPlaneX0Qn;

    uint8_t                         m_refOrderHints[7];
    uint8_t                         m_savedRefOrderHints[7][7];
    uint8_t                         m_refMaskMfProj;
    uint8_t                         m_validRefPicIdx;
};

struct MhwVdboxAvpPakInsertObjParams
{
    PBSBuffer pBsBuffer;
    // also reuse dwBitSize for passing SrcDataEndingBitInclusion when (pEncoder->bLastPicInStream || pEncoder->bLastPicInSeq)
    uint32_t          dwBitSize;
    uint32_t          dwOffset;
    uint32_t          uiSkipEmulationCheckCount;
    bool              bLastPicInSeq;
    bool              bLastPicInStream;
    bool              m_lastHeader;
    bool              bEmulationByteBitsInsert;
    bool              bSetLastPicInStreamData;
    bool              bSliceHeaderIndicator;
    bool              bHeaderLengthExcludeFrmSize;
    uint32_t *        pdwMpeg2PicHeaderTotalBufferSize;
    uint32_t *        pdwMpeg2PicHeaderDataStartOffset;
    bool              bResetBitstreamStartingPos;
    uint32_t          m_endOfHeaderInsertion;
    uint32_t          dwLastPicInSeqData;
    uint32_t          dwLastPicInStreamData;
    PMHW_BATCH_BUFFER pBatchBufferForPakSlices;
    bool              bVdencInUse;
};

struct MhwVdboxAvpSegmentStateParams
{
    CodecAv1SegmentsParams          *m_av1SegmentParams;
    uint8_t                         m_currentSegmentId;
};

struct MhwVdboxAvpBsdParams
{
    uint32_t                        m_bsdDataLength;
    uint32_t                        m_bsdDataStartOffset;
};

class MhwVdboxAvpPipeBufAddrParams
{
public:
    uint32_t                    m_mode                                                      = 0;

    MOS_RESOURCE                *m_references[8]                                            = {};       //!< Reference Frame Buffer
    MOS_SURFACE                 *m_decodedPic                                               = nullptr;  //!< Decoded Output Frame Buffer
    MOS_RESOURCE                *m_intrabcDecodedOutputFrameBuffer                          = nullptr;  //!< IntraBC Decoded output frame buffer
    MOS_RESOURCE                *m_cdfTableInitializationBuffer                             = nullptr;  //!< CDF Tables Initialization Buffer
    uint32_t                    m_cdfTableInitializationBufferOffset                        = 0;        //!< CDF Tables Initialization Buffer Size
    MOS_RESOURCE                *m_cdfTableBwdAdaptationBuffer                              = nullptr;  //!< CDF Tables Backward Adaptation Buffer

    MOS_RESOURCE                *m_segmentIdReadBuffer                                      = nullptr;  //!< AV1 Segment ID Read Buffer
    MOS_RESOURCE                *m_segmentIdWriteBuffer                                     = nullptr;  //!< AV1 Segment ID Write Buffer
    MOS_RESOURCE                *m_colMvTemporalBuffer[9]                                   = {};       //!< Collocated MV temporal buffer
    MOS_RESOURCE                *m_curMvTemporalBuffer                                      = nullptr;  //!< Current MV temporal buffer
    MOS_RESOURCE                *m_bitstreamDecoderEncoderLineRowstoreReadWriteBuffer       = nullptr;  //!< Handle of Bitstream Decode Line Rowstore buffer, can be programmed to use Local Media Storage VMM instead of Memory
    MOS_RESOURCE                *m_bitstreamDecoderEncoderTileLineRowstoreReadWriteBuffer   = nullptr;  //!< Handle of Bitstream Decode Tile Line buffer
    MOS_RESOURCE                *m_intraPredictionLineRowstoreReadWriteBuffer               = nullptr;  //!< Handle of Intra Prediction Line Rowstore Read/Write Buffer
    MOS_RESOURCE                *m_intraPredictionTileLineRowstoreReadWriteBuffer           = nullptr;  //!< Handle of Intra Prediction Tile Line Rowstore Read/Write Buffer
    MOS_RESOURCE                *m_spatialMotionVectorLineReadWriteBuffer                   = nullptr;  //!< Handle of Spatial Motion Vector Line rowstore buffer, can be programmed to use Local Media Storage VMM instead of Memory
    MOS_RESOURCE                *m_spatialMotionVectorCodingTileLineReadWriteBuffer         = nullptr;  //!< Handle of Spatial Motion Vector Tile Line buffer
    MOS_RESOURCE                *m_loopRestorationMetaTileColumnReadWriteBuffer             = nullptr;  //!< Loop Restoration Meta Tile Column Read/Write Buffer Address
    MOS_RESOURCE                *m_loopRestorationFilterTileReadWriteLineYBuffer            = nullptr;  //!< Loop Restoration Filter Tile Read/Write Line Y Buffer Address
    MOS_RESOURCE                *m_loopRestorationFilterTileReadWriteLineUBuffer            = nullptr;  //!< Loop Restoration Filter Tile Read/Write Line U Buffer Address
    MOS_RESOURCE                *m_loopRestorationFilterTileReadWriteLineVBuffer            = nullptr;  //!< Loop Restoration Filter Tile Read/Write Line V Buffer Address
    MOS_RESOURCE                *m_deblockerFilterLineReadWriteYBuffer                      = nullptr;  //!< Deblocker Filter Line Read/Write Y Buffer Address
    MOS_RESOURCE                *m_deblockerFilterLineReadWriteUBuffer                      = nullptr;  //!< Deblocker Filter Line Read/Write U Buffer Address
    MOS_RESOURCE                *m_deblockerFilterLineReadWriteVBuffer                      = nullptr;  //!< Deblocker Filter Line Read/Write V Buffer Address
    MOS_RESOURCE                *m_deblockerFilterTileLineReadWriteYBuffer                  = nullptr;  //!< Deblocker Filter Tile Line Read/Write Y Buffer Address
    MOS_RESOURCE                *m_deblockerFilterTileLineReadWriteVBuffer                  = nullptr;  //!< Deblocker Filter Tile Line Read/Write V Buffer Address
    MOS_RESOURCE                *m_deblockerFilterTileLineReadWriteUBuffer                  = nullptr;  //!< Deblocker Filter Tile Line Read/Write U Buffer Address
    MOS_RESOURCE                *m_deblockerFilterTileColumnReadWriteYBuffer                = nullptr;  //!< Deblocker Filter Tile Column Read/Write Y Buffer Address
    MOS_RESOURCE                *m_deblockerFilterTileColumnReadWriteUBuffer                = nullptr;  //!< Deblocker Filter Tile Column Read/Write U Buffer Address
    MOS_RESOURCE                *m_deblockerFilterTileColumnReadWriteVBuffer                = nullptr;  //!< Deblocker Filter Tile Column Read/Write V Buffer Address
    MOS_RESOURCE                *m_cdefFilterLineReadWriteBuffer                            = nullptr;  //!< CDEF Filter Line Read/Write Y Buffer Address
    MOS_RESOURCE                *m_cdefFilterTileLineReadWriteBuffer                        = nullptr;  //!< CDEF Filter Tile Line Read/Write Y Buffer Address
    MOS_RESOURCE                *m_cdefFilterTileColumnReadWriteBuffer                      = nullptr;  //!< CDEF Filter Tile Column Read/Write Y Buffer Address
    MOS_RESOURCE                *m_cdefFilterMetaTileLineReadWriteBuffer                    = nullptr;  //!< CDEF Filter Meta Tile Line Read/Write Buffer Address
    MOS_RESOURCE                *m_cdefFilterMetaTileColumnReadWriteBuffer                  = nullptr;  //!< CDEF Filter Meta Tile Column Read/Write Buffer Address
    MOS_RESOURCE                *m_cdefFilterTopLeftCornerReadWriteBuffer                   = nullptr;  //!< CDEF Filter Top-Left Corner Read/Write Buffer Address
    MOS_RESOURCE                *m_superResTileColumnReadWriteYBuffer                       = nullptr;  //!< Super-Res Tile Column Read/Write Y Buffer Address
    MOS_RESOURCE                *m_superResTileColumnReadWriteUBuffer                       = nullptr;  //!< Super-Res Tile Column Read/Write U Buffer Address
    MOS_RESOURCE                *m_superResTileColumnReadWriteVBuffer                       = nullptr;  //!< Super-Res Tile Column Read/Write V Buffer Address
    MOS_RESOURCE                *m_loopRestorationFilterTileColumnReadWriteYBuffer          = nullptr;  //!< Loop Restoration Filter Tile Column Read/Write Y Buffer Address
    MOS_RESOURCE                *m_loopRestorationFilterTileColumnReadWriteUBuffer          = nullptr;  //!< Loop Restoration Filter Tile Column Read/Write U Buffer Address
    MOS_RESOURCE                *m_loopRestorationFilterTileColumnReadWriteVBuffer          = nullptr;  //!< Loop Restoration Filter Tile Column Read/Write V Buffer Address
    MOS_RESOURCE                *m_decodedFrameStatusErrorBuffer                            = nullptr;  //!< Decoded Frame Status/Error Buffer Base Address
    MOS_RESOURCE                *m_decodedBlockDataStreamoutBuffer                          = nullptr;  //!< Decoded Block Data Streamout Buffer Address

    //MMC supported
    MOS_MEMCOMP_STATE           m_preDeblockSurfMmcState                                    = {};
    MOS_MEMCOMP_STATE           m_streamOutBufMmcState                                      = {};
    MOS_MEMCOMP_STATE           m_rawSurfMmcState                                           = {};

    virtual void Initialize();
    //!
    //! \brief    Destructor
    //!
    virtual ~MhwVdboxAvpPipeBufAddrParams() { }
};

struct MhwVdboxAvpBufferSizeParams
{
    uint8_t                         m_bitDepthIdc;
    uint32_t                        m_picWidth;                // picWidth in SB
    uint32_t                        m_picHeight;               // picHeight in SB
    uint32_t                        m_tileWidth;               // tileWidth in SB
    uint32_t                        m_bufferSize;
    bool                            m_isSb128x128;
    uint32_t                        m_curFrameTileNum;
    uint32_t                        m_numTileCol;
    uint8_t                         m_numOfActivePipes;
};

struct MhwVdboxAvpBufferReallocParams
{
    uint8_t                         m_bitDepthIdc;
    uint16_t                        m_prevFrameTileNum;
    uint16_t                        m_curFrameTileNum;
    bool                            m_isSb128x128;
    uint32_t                        m_tileWidth;
    uint32_t                        m_numTileCol;
    uint32_t                        m_numTileColAllocated;
    uint32_t                        m_picWidth;
    uint32_t                        m_picHeight;
    uint32_t                        m_picWidthAlloced;
    uint32_t                        m_picHeightAlloced;
    uint32_t                        m_bufferSizeAlloced;
    bool                            m_needBiggerSize;
};

//NOTE: When changing MhwVdboxAvpInternalBufferType, please change CodecAv1BufferSize[][][]/CodecAv1BufferSizeExt[][][] accordingly!!!
enum MhwVdboxAvpInternalBufferType
{
    segmentIdBuf        = 0,    //!< segment ID temporal buffers
    mvTemporalBuf,              //!< MV temporal buffers of both current and collocated
    bsdLineBuf,                 //!< bitstream decode line buffer
    bsdTileLineBuf,             //!< bitstream decode tile line buffer
    intraPredLine,              //!< intra prediction line buffer
    intraPredTileLine,
    spatialMvLineBuf,
    spatialMvTileLineBuf,
    lrMetaTileCol,              //!< Loop Restoration Meta Tile Column Read/Write Buffer Address
    lrTileLineY,                //!< Loop Restoration Filter Tile Read/Write Line Y Buffer Address
    lrTileLineU,                //!< Loop Restoration Filter Tile Read/Write Line U Buffer Address
    lrTileLineV,                //!< Loop Restoration Filter Tile Read/Write Line V Buffer Address
    deblockLineYBuf,
    deblockLineUBuf,
    deblockLineVBuf,
    deblockTileLineYBuf,
    deblockTileLineVBuf,
    deblockTileLineUBuf,
    deblockTileColYBuf,
    deblockTileColUBuf,
    deblockTileColVBuf,
    cdefLineBuf,
    cdefTileLineBuf,
    cdefTileColBuf,
    cdefMetaTileLine,
    cdefMetaTileCol,
    cdefTopLeftCornerBuf,
    superResTileColYBuf,
    superResTileColUBuf,
    superResTileColVBuf,
    lrTileColYBuf,
    lrTileColUBuf,
    lrTileColVBuf,
    frameStatusErrBuf,
    dbdStreamoutBuf,
    fgTileColBuf,
    fgSampleTmpBuf,
    lrTileColAlignBuf,
    tileSzStreamOutBuf,
    tileStatStreamOutBuf,
    cuStreamoutBuf,
    sseLineBuf,
    sseTileLineBuf,
    avpInternalBufMax
};

//AVP internal buffer size table [buffer_index][bitdepthIdc][IsSb128x128]
static const uint8_t CodecAv1BufferSize[avpInternalBufMax][2][2] =
{
    { 2 ,   8   ,   2   ,   8 }, //segmentIdBuf,
    { 4 ,   16  ,   4   ,    16 }, //mvTemporalBuf,
    { 2 ,   4   ,   2   ,    4 }, //bsdLineBuf,
    { 2 ,   4   ,   2   ,    4 }, //bsdTileLineBuf,
    { 2 ,   4   ,   4   ,    8 }, //intraPredLine,
    { 2 ,   4   ,   4   ,    8 }, //intraPredTileLine,
    { 4 ,   8   ,   4   ,    8 }, //spatialMvLineBuf,
    { 4 ,   8   ,   4   ,    8 }, //spatialMvTileLineBuf,
    { 1 ,   1   ,   1   ,    1 }, //lrMetaTileCol,
    { 7 ,   7   ,   7   ,    7 }, //lrTileLineY,
    { 5 ,   5   ,   5   ,    5 }, //lrTileLineU,
    { 5 ,   5   ,   5   ,    5 }, //lrTileLineV,
    { 9 ,   17  ,   11  ,    21 }, //deblockLineYBuf,
    { 3 ,   4   ,   3   ,    5 }, //deblockLineUBuf,
    { 3 ,   4   ,   3   ,    5 }, //deblockLineVBuf,
    { 9 ,   17  ,   11  ,    21 }, //deblockTileLineYBuf,
    { 3 ,   4   ,   3   ,    5 }, //deblockTileLineVBuf,
    { 3 ,   4   ,   3   ,    5 }, //deblockTileLineUBuf,
    { 8 ,   16  ,   10  ,    20 }, //deblockTileColYBuf,
    { 2 ,   4   ,   3   ,    5 }, //deblockTileColUBuf,
    { 2 ,   4   ,   3   ,    5 }, //deblockTileColVBuf,
    { 8 ,   16  ,   10  ,    20 }, //cdefLineBuf,
    { 8 ,   16  ,   10  ,    20 }, //cdefTileLineBuf,
    { 8 ,   16  ,   10  ,    20 }, //cdefTileColBuf,
    { 1 ,   1   ,   1   ,    1 }, //cdefMetaTileLine,
    { 1 ,   1   ,   1   ,    1 }, //cdefMetaTileCol,
    { 1 ,   1   ,   1   ,    1 }, //cdefTopLeftCornerBuf,
    { 22,   44  ,   29  ,    58 }, //superResTileColYBuf,
    { 8 ,   16  ,   10  ,    20 }, //superResTileColUBuf,
    { 8 ,   16  ,   10  ,    20 }, //superResTileColVBuf,
    { 9 ,   17  ,   11  ,    22 }, //lrTileColYBuf,
    { 5 ,   9   ,   6   ,    12 }, //lrTileColUBuf,
    { 5 ,   9   ,   6   ,    12 }, //lrTileColVBuf,
    { 0 ,   0   ,   0   ,    0 }, //frameStatusErrBuf,
    { 0 ,   0   ,   0   ,    0 }, //dbdStreamoutBuf,
    { 2 ,   4   ,   3   ,    5 }, //fgTileColBuf
    { 96,   96  ,   192 ,    192 },//fgSampleTmpBuf
    { 4,    8   ,   5   ,    10 }, //lrTileColAlignBuf
};

static const uint8_t CodecAv1BufferSizeExt[avpInternalBufMax][2][2] =
{
    { 0 ,    0    ,    0    ,    0 }, //segmentIdBuf,
    { 0 ,    0    ,    0    ,    0 }, //mvTemporalBuf,
    { 0 ,    0    ,    0    ,    0 }, //bsdLineBuf,
    { 0 ,    0    ,    0    ,    0 }, //bsdTileLineBuf,
    { 0 ,    0    ,    0    ,    0 }, //intraPredLine,
    { 0 ,    0    ,    0    ,    0 }, //intraPredTileLine,
    { 0 ,    0    ,    0    ,    0 }, //spatialMvLineBuf,
    { 0 ,    0    ,    0    ,    0 }, //spatialMvTileLineBuf,
    { 1 ,    1    ,    1    ,    1 }, //lrMetaTileCol,
    { 0 ,    0    ,    0    ,    0 }, //lrTileLineY,
    { 0 ,    0    ,    0    ,    0 }, //lrTileLineU,
    { 0 ,    0    ,    0    ,    0 }, //lrTileLineV,
    { 0 ,    0    ,    0    ,    0 }, //deblockLineYBuf,
    { 0 ,    0    ,    0    ,    0 }, //deblockLineUBuf,
    { 0 ,    0    ,    0    ,    0 }, //deblockLineVBuf,
    { 0 ,    0    ,    0    ,    0 }, //deblockTileLineYBuf,
    { 0 ,    0    ,    0    ,    0 }, //deblockTileLineVBuf,
    { 0 ,    0    ,    0    ,    0 }, //deblockTileLineUBuf,
    { 0 ,    0    ,    0    ,    0 }, //deblockTileColYBuf,
    { 0 ,    0    ,    0    ,    0 }, //deblockTileColUBuf,
    { 0 ,    0    ,    0    ,    0 }, //deblockTileColVBuf,
    { 1 ,    1    ,    2    ,    2 }, //cdefLineBuf,
    { 1 ,    1    ,    2    ,    2 }, //cdefTileLineBuf,
    { 1 ,    1    ,    2    ,    2 }, //cdefTileColBuf,
    { 0 ,    0    ,    0    ,    0 }, //cdefMetaTileLine,
    { 1 ,    1    ,    1    ,    1 }, //cdefMetaTileCol,
    { 0 ,    0    ,    0    ,    0 }, //cdefTopLeftCornerBuf,
    { 22,    44   ,    29   ,    58 }, //superResTileColYBuf,
    { 8 ,    16   ,    10   ,    20 }, //superResTileColUBuf,
    { 8 ,    16   ,    10   ,    20 }, //superResTileColVBuf,
    { 2 ,    2    ,    2    ,    2 }, //lrTileColYBuf,
    { 1 ,    1    ,    1    ,    1 }, //lrTileColUBuf,
    { 1 ,    1    ,    1    ,    1 }, //lrTileColVBuf,
    { 0 ,    0    ,    0    ,    0 }, //frameStatusErrBuf,
    { 0 ,    0    ,    0    ,    0 }, //dbdStreamoutBuf,
    { 1 ,    1    ,    1    ,    1 }, //fgTileColBuf,
    { 0 ,    0    ,    0    ,    0 }, //fgSampleTmpBuf,
    { 1 ,    1    ,    1    ,    1 }, //lrTileColAlignBuf,
};

//!
//! \struct   MmioRegistersAvp
//! \brief    MMIO registers AVP
//!
struct MmioRegistersAvp
{
    uint32_t                   avpAv1BitstreamByteCountTileRegOffset         = 0;
    uint32_t                   avpAv1BitstreamByteCountTileNoHeaderRegOffset = 0;
    uint32_t                   avpAv1CabacBinCountTileRegOffset              = 0;
    uint32_t                   avpAv1CabacInsertionCountRegOffset            = 0;
    uint32_t                   avpAv1MinSizePaddingCountRegOffset            = 0;
    uint32_t                   avpAv1ImageStatusMaskRegOffset                = 0;
    uint32_t                   avpAv1ImageStatusControlRegOffset             = 0;
    uint32_t                   avpAv1QpStatusCountRegOffset                  = 0;
    uint32_t                   avpAv1DecErrorStatusAddrRegOffset             = 0;
};

//!  MHW Vdbox Avp interface
/*!
This class defines the interfaces for constructing Vdbox Avp commands across all platforms
*/
class MhwVdboxAvpInterface
{
protected:
    PMOS_INTERFACE              m_osInterface = nullptr;         //!< Pointer to OS interface
    MhwMiInterface              *m_miInterface = nullptr;        //!< Pointer to MI interface
    MhwCpInterface              *m_cpInterface = nullptr;        //!< Pointer to CP interface
    MEDIA_FEATURE_TABLE         *m_skuTable  = nullptr;          //!< Pointer to SKU table
    MEDIA_WA_TABLE              *m_waTable   = nullptr;          //!< Pointer to WA table
    bool                        m_decodeInUse = false;           //!< Flag to indicate if the interface is for decoder or encoder use

    MHW_MEMORY_OBJECT_CONTROL_PARAMS m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_END_CODEC] = {};    //!< Cacheability settings
    MmioRegistersAvp                 m_mmioRegisters[MHW_VDBOX_NODE_MAX]                        = {};    //!< AVP mmio registers
    bool                             m_rowstoreCachingSupported                                 = false; //!< Flag to indicate if row store cache is supported

    MHW_VDBOX_ROWSTORE_CACHE    m_btdlRowstoreCache = {};         //! Bitstream Decoder/Encode Line Rowstore (BTDL)
    MHW_VDBOX_ROWSTORE_CACHE    m_smvlRowstoreCache = {};         //! Spatial Motion Vector Line Rowstore (SMVL)
    MHW_VDBOX_ROWSTORE_CACHE    m_ipdlRowstoreCache = {};         //! Intra Prediction Line Rowstore (IPDL)
    MHW_VDBOX_ROWSTORE_CACHE    m_dflyRowstoreCache = {};         //! Deblocker Filter Line Y Buffer (DFLY)
    MHW_VDBOX_ROWSTORE_CACHE    m_dfluRowstoreCache = {};         //! Deblocker Filter Line U Buffe (DFLU)
    MHW_VDBOX_ROWSTORE_CACHE    m_dflvRowstoreCache = {};         //! Deblocker Filter Line V Buffe (DFLV)
    MHW_VDBOX_ROWSTORE_CACHE    m_cdefRowstoreCache = {};         //! CDEF Filter Line Buffer (CDEF)

    std::shared_ptr<void> m_avpItfNew = nullptr;

#if MOS_EVENT_TRACE_DUMP_SUPPORTED
    bool bMMCReported = false;
#endif
    //!
    //! \brief    Constructor
    //!
    MhwVdboxAvpInterface(
        PMOS_INTERFACE osInterface,
        MhwMiInterface *miInterface,
        MhwCpInterface *cpInterface,
        bool decodeInUse);

    //!
    //! \brief    Add a resource to the command buffer
    //! \details  Internal function to add either a graphics address of a resource or
    //!           add the resource to the patch list for the requested buffer
    //!
    //! \param    [in] osInterface
    //!           OS interface
    //! \param    [in] cmdBuffer
    //!           Command buffer to which resource is added
    //! \param    [in] params
    //!           Parameters necessary to add the resource
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS(*AddResourceToCmd) (
        PMOS_INTERFACE osInterface,
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_RESOURCE_PARAMS params);

public:
    //!
    //! \brief    Destructor
    //!
    virtual ~MhwVdboxAvpInterface() {}

    //!
    //! \brief    Get new AVP interface, temporal solution before switching from
    //!           old interface to new one
    //!
    //! \return   pointer to new AVP interface
    //!
    virtual std::shared_ptr<void> GetNewAvpInterface() { return nullptr; }

    //!
    //! \brief    Judge if row store caching supported
    //!
    //! \return   bool
    //!           true if supported, else false
    //!
    inline bool IsRowStoreCachingSupported()
    {
        return m_rowstoreCachingSupported;
    }

    //!
    //! \brief    Judge if AV1 Bitstream Decoder/Encode Line Rowstore (BTDL) caching enabled
    //!
    //! \return   bool
    //!           true if enabled, else false
    //!
    inline bool IsBtdlRowstoreCacheEnabled()
    {
        return m_btdlRowstoreCache.bEnabled ? true : false;
    }

    //!
    //! \brief    Judge if AV1 Spatial Motion Vector Line Rowstore caching enabled
    //!
    //! \return   bool
    //!           true if enabled, else false
    //!
    inline bool IsSmvlRowstoreCacheEnabled()
    {
        return m_smvlRowstoreCache.bEnabled ? true : false;
    }

    //!
    //! \brief    Judge if AV1 Intra Prediction Line Rowstore (IPDL) caching enabled
    //!
    //! \return   bool
    //!           true if enabled, else false
    //!
    inline bool IsIpdlRowstoreCacheEnabled()
    {
        return m_ipdlRowstoreCache.bEnabled ? true : false;
    }

    //!
    //! \brief    Judge if AV1 Deblocker Filter Line Y Buffer (DFLY) caching enabled
    //!
    //! \return   bool
    //!           true if enabled, else false
    //!
    inline bool IsDflyRowstoreCacheEnabled()
    {
        return m_dflyRowstoreCache.bEnabled ? true : false;
    }

    //!
    //! \brief    Judge if AV1 Deblocker Filter Line U Buffe (DFLU) caching enabled
    //!
    //! \return   bool
    //!           true if enabled, else false
    //!
    inline bool IsDfluRowstoreCacheEnabled()
    {
        return m_dfluRowstoreCache.bEnabled ? true : false;
    }


    //!
    //! \brief    Judge if AV1 Deblocker Filter Line V Buffe (DFLV) caching enabled
    //!
    //! \return   bool
    //!           true if enabled, else false
    //!
    inline bool IsDflvRowstoreCacheEnabled()
    {
        return m_dflvRowstoreCache.bEnabled ? true : false;
    }

    //!
    //! \brief    Judge if AV1 CDEF Filter Line Buffer (CDEF) caching enabled
    //!
    //! \return   bool
    //!           true if enabled, else false
    //!
    inline bool IsCdefRowstoreCacheEnabled()
    {
        return m_cdefRowstoreCache.bEnabled ? true : false;
    }

    //!
    //! \brief    Get mmio registers
    //!
    //! \param    [in] index
    //!           mmio registers index.
    //!
    //! \return   [out] MmioRegistersAvp*
    //!           mmio registers got.
    //!
    inline MmioRegistersAvp* GetMmioRegisters(MHW_VDBOX_NODE_IND index)
    {
        if (index < MHW_VDBOX_NODE_MAX)
        {
            return &m_mmioRegisters[index];
        }
        else
        {
            MHW_ASSERT("index is out of range!");
            return &m_mmioRegisters[MHW_VDBOX_NODE_1];
        }
    }

    //!
    //! \brief    Set cacheability settings
    //!
    //! \param    [in] cacheabilitySettings
    //!           Cacheability settings
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetCacheabilitySettings(
        MHW_MEMORY_OBJECT_CONTROL_PARAMS cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_END_CODEC])
    {
        MHW_FUNCTION_ENTER;

        uint32_t size = MOS_CODEC_RESOURCE_USAGE_END_CODEC * sizeof(MHW_MEMORY_OBJECT_CONTROL_PARAMS);
        MOS_STATUS eStatus = MOS_SecureMemcpy(m_cacheabilitySettings, size,
            cacheabilitySettings, size);

        return eStatus;
    }

    //!
    //! \brief    Calculates the maximum size for AVP picture level commands
    //! \details  Client facing function to calculate the maximum size for AVP picture level commands
    //! \param    [out] commandsSize
    //!           The maximum command buffer size
    //! \param    [out] patchListSize
    //!           The maximum command patch list size
    //! \param    [in] params
    //!           Indicate the command size parameters
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetAvpStateCommandSize(
        uint32_t                        *commandsSize,
        uint32_t                        *patchListSize,
        PMHW_VDBOX_STATE_CMDSIZE_PARAMS params) = 0;

    //!
    //! \brief    Calculates maximum size for AVP slice/MB level commands
    //! \details  Client facing function to calculate maximum size for AVP slice/MB level commands
    //! \param    [out] commandsSize
    //!            The maximum command buffer size
    //! \param    [out] patchListSize
    //!           The maximum command patch list size
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetAvpPrimitiveCommandSize(
        uint32_t                        *commandsSize,
        uint32_t                        *patchListSize) = 0;

    //!
    //! \brief    Get the required buffer size for VDBOX
    //! \details  Internal function to judge if buffer realloc is needed for AV1 codec
    //!
    //! \param    [in] bufferType
    //!           AV1 Buffer type
    //! \param    [in, out] reallocParam
    //!           AVP Re-allocate parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS IsAv1BufferReallocNeeded(
        MhwVdboxAvpInternalBufferType   bufferType,
        MhwVdboxAvpBufferReallocParams  *reallocParam) = 0;

    //!
    //! \brief    Get the required buffer size for VDBOX
    //! \details  Internal function to get required buffer size for AV1 codec
    //!
    //! \param    [in] bufferType
    //!           AV1 Buffer type
    //! \param    [in, out] avpBufSizeParam
    //!           AVP buffer size parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetAv1BufferSize(
        MhwVdboxAvpInternalBufferType       bufferType,
        MhwVdboxAvpBufferSizeParams    *avpBufSizeParam) = 0;

    //!
    //! \brief    Adds AVP Surface State command in command buffer
    //! \details  Client facing function to add AVP Surface State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddAvpSurfaceCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_SURFACE_PARAMS            params);

    //!
    //! \brief    Adds AVP Segment State command in command buffer
    //! \details  Client facing function to add AVP Segment State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddAvpSegmentStateCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        MhwVdboxAvpSegmentStateParams       *params) = 0;

    //!
    //! \brief    Programs base address of rowstore scratch buffers
    //! \details  Internal function to get base address of rowstore scratch buffers
    //!
    //! \param    [in] rowstoreParams
    //!           Rowstore parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetRowstoreCachingAddrs(
        PMHW_VDBOX_ROWSTORE_PARAMS rowstoreParams) = 0;

    //!
    //! \brief    Adds AVP Pipe Pipe Mode Select command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddAvpPipeModeSelectCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS   params) = 0;

    //!
    //! \brief    Adds AVP Surface State command for decode in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddAvpDecodeSurfaceStateCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_SURFACE_PARAMS            params) = 0;

    //!
    //! \brief    Adds AVP Pipe Buffer Address State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddAvpPipeBufAddrCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        MhwVdboxAvpPipeBufAddrParams         *params) = 0;

    //!
    //! \brief    Adds AVP Indirect Object Base Address State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddAvpIndObjBaseAddrCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS  params) = 0;

    //!
    //! \brief    Adds AVP picture State command for decode in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in]  batchBuffer
    //!           Batch buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddAvpDecodePicStateCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        MhwVdboxAvpPicStateParams           *params) = 0;

    //!
    //! \brief    Adds AVP BSD object state command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddAvpBsdObjectCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_BATCH_BUFFER                batchBuffer,
        MhwVdboxAvpBsdParams             *params) = 0;

    //!
    //! \brief    Adds AVP tile coding command in command buffer for decoder
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddAvpDecodeTileCodingCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_BATCH_BUFFER               batchBuffer,
        MhwVdboxAvpTileCodingParams     *params) = 0;

    //!
    //! \brief    Adds AVP tile coding command in command buffer when Large Scale Tile decoding is supported
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddAvpDecodeTileCodingCmdLst(
        PMOS_COMMAND_BUFFER          cmdBuffer,
        PMHW_BATCH_BUFFER            batchBuffer,
        MhwVdboxAvpTileCodingParams *params) = 0;

    //!
    //! \brief    Adds AVP tile coding command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddAvpTileCodingCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_BATCH_BUFFER                   batchBuffer,
        MhwVdboxAvpTileCodingParams         *params) = 0;

    //!
    //! \brief    Adds AVP Inloop Filter State command for decode in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddAvpInloopFilterStateCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        MhwVdboxAvpPicStateParams            *params) = 0;

    //!
    //! \brief    Adds AVP Inter Prediction State command
    //! \details  function to add AVP Inter Prediction State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddAvpInterPredStateCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        MhwVdboxAvpPicStateParams            *params) = 0;
};

#endif
