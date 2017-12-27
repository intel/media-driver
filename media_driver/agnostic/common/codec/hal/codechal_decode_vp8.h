/*
* Copyright (c) 2012-2017, Intel Corporation
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
//! \file     codechal_decode_vp8.h
//! \brief    Defines the decode interface extension for VP8.
//! \details  Defines all types, macros, and functions required by CodecHal for VP8 decoding.
//!           Definitions are not externally facing.
//!
#ifndef __CODECHAL_DECODER_VP8_H__
#define __CODECHAL_DECODER_VP8_H__

#include "codechal.h"
#include "codechal_hw.h"
#include "codechal_decoder.h"


//*------------------------------------------------------------------------------
//* Codec Definitions
//*------------------------------------------------------------------------------

//!
//! \enum VP8_MB_LVL_FEATURES
//! VP8 MB Level Features
//!
typedef enum
{
    VP8_MB_LVL_ALT_Q = 0,
    VP8_MB_LVL_ALT_LF = 1,
    VP8_MB_LVL_MAX = 2
} VP8_MB_LVL_FEATURES;

//!
//! \enum VP8_MB_PREDICTION_MODE
//! VP8 MB Prediction Mode
//!
typedef enum
{
    VP8_DC_PRED,
    VP8_V_PRED,
    VP8_H_PRED,
    VP8_TM_PRED,
    VP8_B_PRED,

    VP8_NEARESTMV,
    VP8_NEARMV,
    VP8_ZEROMV,
    VP8_NEWMV,
    SPLITMV,

    VP8_MB_MODE_COUNT
} VP8_MB_PREDICTION_MODE;

//!
//! \enum VP8_LF_TYPE
//! VP8 LF Type
//!
typedef enum
{
    VP8_NORMAL_LF = 0,
    VP8_SIMPLE_LF = 1
} VP8_LF_TYPE;

//!
//! \enum VP8_TOKEN_PARTITION
//! VP8 Token Partition
//!
typedef enum
{
    VP8_ONE_PARTITION = 0,
    VP8_TWO_PARTITION = 1,
    VP8_FOUR_PARTITION = 2,
    VP8_EIGHT_PARTITION = 3
} VP8_TOKEN_PARTITION;

//!
//! \enum VP8_MV_REFERENCE_FRAME
//! VP8 MV Reference Frame
//!
typedef enum
{
    VP8_NONE = -1,
    VP8_INTRA_FRAME = 0,
    VP8_LAST_FRAME = 1,
    VP8_GOLDEN_FRAME = 2,
    VP8_ALTREF_FRAME = 3,
    VP8_MAX_REF_FRAMES = 4
} VP8_MV_REFERENCE_FRAME;

//!
//! \def VP8_Y_MODES
//! VP8 Y Mode Max Index
//!
#define VP8_Y_MODES  (VP8_B_PRED + 1)
//!
//! \def VP8_UV_MODES
//! VP8 UV Mode Max Index
//!
#define VP8_UV_MODES (VP8_TM_PRED + 1)

//!
//! \def VP8_MAX_MB_SEGMENTS
//! Max Index of VP8 MB Segment
//!
#define VP8_MAX_MB_SEGMENTS         4
//!
//! \def VP8_MB_SEGMENT_TREE_PROBS
//! Max Index of Prob Tree for MB Segment Id
//!
#define VP8_MB_SEGMENT_TREE_PROBS   3

//!
//! \def VP8_MAX_REF_LF_DELTAS
//! Max Index of VP8 Ref LF Deltas
//!
#define VP8_MAX_REF_LF_DELTAS   4
//!
//! \def VP8_MAX_MODE_LF_DELTAS
//! Max Index of VP8 Mode LF Deltas
//!
#define VP8_MAX_MODE_LF_DELTAS  4

//!
//! \def VP8_MAX_Q
//! VP8 Max Q Index
//!
#define VP8_MAX_Q 127
//!
//! \def VP8_Q_INDEX_RANGE
//! VP8 Q Index range
//!
#define VP8_Q_INDEX_RANGE (VP8_MAX_Q + 1)

//!
//! \def VP8_BLOCK_TYPES
//! VP8 Block Types Max Index
//!
#define VP8_BLOCK_TYPES 4
//!
//! \def VP8_COEF_BANDS
//! VP8 Coef Bands Max Index
//!
#define VP8_COEF_BANDS 8
//!
//! \def VP8_PREV_COEF_CONTEXTS
//! VP8 Prev Coef Context Max Index
//!
#define VP8_PREV_COEF_CONTEXTS 3
//!
//! \def VP8_ENTROPY_NODES
//! VP8 Entropy Nodes Max Index
//!
#define VP8_ENTROPY_NODES 11

//!
//! \struct _VP8_FRAME_CONTEXT
//! \brief Define variables for VP8 Frame Context
//!
typedef struct _VP8_FRAME_CONTEXT
{
    uint8_t    YModeProb[VP8_Y_MODES - 1];
    uint8_t    UVModeProb[VP8_UV_MODES - 1];
    uint8_t    CoefProbs[VP8_BLOCK_TYPES][VP8_COEF_BANDS][VP8_PREV_COEF_CONTEXTS][VP8_ENTROPY_NODES];
    MV_CONTEXT MvContext[2];
} VP8_FRAME_CONTEXT;

typedef struct _CODECHAL_DECODE_VP8_FRAME_HEAD CODECHAL_DECODE_VP8_FRAME_HEAD, *PCODECHAL_DECODE_VP8_FRAME_HEAD;

//!
//! \struct _CODECHAL_DECODE_VP8_FRAME_HEAD
//! \brief Define variables for VP8 Frame Head
//!
struct _CODECHAL_DECODE_VP8_FRAME_HEAD
{

    int16_t Y1DeQuant[VP8_Q_INDEX_RANGE][2];
    int16_t Y2DeQuant[VP8_Q_INDEX_RANGE][2];
    int16_t UVDeQuant[VP8_Q_INDEX_RANGE][2];

    int32_t iNewFrameBufferIdx, iLastFrameBufferIdx, iGoldenFrameBufferIdx, iAltFrameBufferIdx;
    int32_t iLastFrameBufferCurrIdx, iGoldenFrameBufferCurrIdx, iAltFrameBufferCurrIdx;


    int32_t iFrameType;

    int32_t iShowframe;

    int32_t iMbNoCoeffSkip;
    int32_t iProbSkipFalse;

    int32_t iBaseQIndex;

    int32_t iY1DcDeltaQ;
    int32_t iY2DcDeltaQ;
    int32_t iY2AcDeltaQ;
    int32_t iUVDcDeltaQ;
    int32_t iUVAcDeltaQ;

    VP8_LF_TYPE FilterType;

    int32_t iFilterLevel;
    int32_t iSharpnessLevel;

    int32_t iRefreshLastFrame;
    int32_t iRefreshGoldenFrame;
    int32_t iRefreshAltFrame;

    int32_t iCopyBufferToGolden;     //!< 0 - None, 1 - Last to Golden, 2 - ALT to Golden
    int32_t iCopyBufferToAlt;        //!< 0 - None, 1 - Last to ALT,    2 - Golden to ALT

    int32_t iRefreshEntropyProbs;

    int32_t RefFrameSignBias[VP8_MAX_REF_FRAMES];

    VP8_FRAME_CONTEXT LastFrameContext;
    VP8_FRAME_CONTEXT FrameContext;

    int32_t iVersion;

    VP8_TOKEN_PARTITION MultiTokenPartition;

    uint8_t u8SegmentationEnabled;

    uint8_t u8UpdateMbSegmentationMap;
    uint8_t u8UpdateMbSegmentationData;

    uint8_t u8MbSegementAbsDelta;

    uint8_t MbSegmentTreeProbs[VP8_MB_SEGMENT_TREE_PROBS];    //!< Prob Tree for MB Segment Id

    int8_t SegmentFeatureData[VP8_MB_LVL_MAX][VP8_MAX_MB_SEGMENTS];
    int8_t LoopFilterLevel[VP8_MAX_MB_SEGMENTS];

    uint8_t u8ModeRefLfDeltaEnabled;
    uint8_t u8ModeRefLfDeltaUpdate;

    int8_t RefLFDeltas[VP8_MAX_REF_LF_DELTAS];                 //!< Intra, Last, Golden, ALT
    int8_t ModeLFDeltas[VP8_MAX_MODE_LF_DELTAS];               //!< BPRED, ZERO_MV, MV, SPLIT

    uint32_t uiFirstPartitionLengthInBytes;

    uint8_t YModeProbs[4];
    uint8_t UVModeProbs[3];

    uint8_t ProbIntra;
    uint8_t ProbLast;
    uint8_t ProbGf;

    bool bNotFirstCall;
};

//!
//! \class Vp8EntropyState
//! \brief This class defines the member fields, functions etc used by VP8 entropy decoder to parse frame head.
//!
class Vp8EntropyState
{
public:
    const uint8_t KEY_FRAME = 0;                                //!< VP8 Key Frame Flag
    const uint8_t INTER_FRAME = 1;                              //!< VP8 Inter Frame Flag
    const uint32_t BD_VALUE_SIZE = ((uint32_t)sizeof(uint32_t) * CHAR_BIT); // VP8 BD Value Size
    const uint32_t LOTS_OF_BITS = 0x40000000;                   //!< Offset for parsing frame head
    const uint8_t PROB_HALF = 128;                              //!< VP8 Half Probability

    //!
    //! \brief    Constructor
    //!
    Vp8EntropyState() {};
    //!
    //! \brief    Destructor
    //!
    ~Vp8EntropyState() {};

    //!
    //! \brief    Initialize VP8 entropy state
    //! \details  Initialize VP8 bitstream buffer and related index 
    //! \param    [in] vp8FrameHeadIn
    //!           Pointer to VP8 Frame Head
    //! \param    [in] bitstreamBufferIn
    //!           Pointer to VP8 bitstream buffer
    //! \param    [in] bitstreamBufferSizeIn
    //!           VP8 bitstream buffer size
    //! \return   void
    //!
    void Initialize(
        PCODECHAL_DECODE_VP8_FRAME_HEAD vp8FrameHeadIn,
        uint8_t*        bitstreamBufferIn,
        uint32_t        bitstreamBufferSizeIn);

    //!
    //! \brief    Parse VP8 Frame Head
    //! \details  Parse VP8 Frame Head based on VP8 Pic Params
    //! \param    [in] vp8PicParams
    //!           Pointer to VP8 Pic Params
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ParseFrameHead(
        PCODEC_VP8_PIC_PARAMS vp8PicParams);

    //!
    //! \brief    Update Quant Index in VP8 Frame Head
    //! \details  Update Quant Index in VP8 Frame Head for
    //!           Y/UV based on DeltaQ in VP8 Pic Params
    //! \param    [in] vp8PicParams
    //!           Pointer to VP8 Pic Params
    //! \return   void
    //!
    void FrameHeadQuantUpdate(
        PCODEC_VP8_PIC_PARAMS vp8PicParams);

protected:
    //!
    //! \brief    Initialize Frame Head in VP8 Entropy State class
    //! \details  Initialize Frame Head for VP8 Frame Head Parser
    //! \return   void
    //!
    void ParseFrameHeadInit();

    //!
    //! \brief    Start Entropy Decode
    //! \return   int32_t
    //!           1 if Buffer is empty or pointer is nullptr, else call DecodeFill and return 0
    //!
    int32_t StartEntropyDecode();

    //!
    //! \brief    Update Segmentation Info in Frame Head
    //! \details  Parse bitstream to update frame head segmentation info
    //! \return   void
    //!
    void SegmentationEnabled();

    //!
    //! \brief    Update Mv Contexts in Frame Head
    //! \details  Parse bitstream to update Mv Contexts Info
    //! \param    [out] mvContext
    //!           Pointer to Mv Contexts
    //! \return   void
    //!
    void ReadMvContexts(MV_CONTEXT *mvContext);

    PCODECHAL_DECODE_VP8_FRAME_HEAD pFrameHead = nullptr;               //!< Pointer to VP8 Frame Head
    uint8_t* pBitstreamBuffer       = nullptr;                          //!< Pointer to Bitstream Buffer
    uint32_t u32BitstreamBufferSize = 0;                                //!< Size of Bitstream Buffer
    uint8_t* pDataBuffer            = nullptr;                          //!< Pointer to Data Buffer
    uint8_t* pDataBufferEnd         = nullptr;                          //<! Pointer to Data Buffer End

private:
    //!
    //! \brief    Update Entropy Decode State Info
    //! \details  Calculate left bitstream size to update pointer info
    //! \return   void
    //!
    void DecodeFill();

    //!
    //! \brief    Update Entropy Decode State according to probability
    //! \param    [in] probability
    //!           Probability to do entropy decode
    //! \return   uint32_t
    //!           return 1 if entropy decode value meets the requirement of probability, else 0
    //!
    uint32_t DecodeBool(int32_t probability);

    //!
    //! \brief    Update Entropy Decode State according to Bits Number
    //! \param    [in] bits
    //!           Bits to do entropy decode
    //! \return   int32_t
    //!           return value calculated by DecodeBool using 0x80 as probability
    //!
    int32_t DecodeValue(int32_t bits);

    //!
    //! \brief    Update Loop Filter Info in VP8 Frame Header
    //! \param    [in] defaultFilterLvl
    //!           Default Segmentation Level
    //! \return   void
    //!
    void LoopFilterInit(int32_t defaultFilterLvl);

    //!
    //! \brief    Update Loop Filter Deltas and Default Loop Filter Level
    //! \details  Parse Loop Filter Deltas and Default Loop Filter Level Info to
    //!           Initialize Loop Filter
    //! \return   void
    //!
    void LoopFilterEnabled();

    //!
    //! \brief    Get DeltaQ from Frame Header
    //! \details  Parse Frame Header to Get DeltaQ value Based on
    //!           Current DeltaQ Value and Update info
    //! \param    [in] prevVal
    //!           Previous DeltaQ Value
    //! \param    [in] qupdate
    //!           Pointer to DeltaQ Update Value
    //! \return   int32_t
    //!           Return Calculated DeltaQ Value
    //!
    int32_t GetDeltaQ(int32_t prevVal, int32_t * qupdate);

    //!
    //! \brief    Get DC Quant Value from Lookup Table
    //! \param    [in] qindex
    //!           Quant Index
    //! \param    [in] delta
    //!           Quant Index Offset
    //! \return   int32_t
    //!           Return DC Quant Value in Lookup Table with Index and Offset
    //!
    int32_t DcQuant(int32_t qindex, int32_t delta);

    //!
    //! \brief    Get Double DC Quant Value from Lookup Table
    //! \param    [in] qindex
    //!           Quant Index
    //! \param    [in] delta
    //!           Quant Index Offset
    //! \return   int32_t
    //!           Return Double DC Quant Value in Lookup Table with Index and Offset
    //!
    int32_t Dc2Quant(int32_t qindex, int32_t delta);

    //!
    //! \brief    Get DC Quant Value from Lookup Table for UV
    //! \param    [in] qindex
    //!           Quant Index
    //! \param    [in] delta
    //!           Quant Index Offset
    //! \return   int32_t
    //!           Return DC Quant Value in Lookup Table with Index and Offset for UV
    //!
    int32_t DcUVQuant(int32_t qindex, int32_t delta);

    //!
    //! \brief    Get AC Quant Value from Lookup Table for Y
    //! \param    [in] qindex
    //!           Quant Index
    //! \return   int32_t
    //!           Return AC Quant Value in Lookup Table with Index and Offset for Y
    //!
    int32_t AcYQuant(int32_t qindex);

    //!
    //! \brief    Get ~Double AC Quant Value from Lookup Table
    //! \param    [in] qindex
    //!           Quant Index
    //! \param    [in] delta
    //!           Quant Index Offset
    //! \return   int32_t
    //!           Return AC Quant Value in Lookup Table with Index and Offset
    //!           Multiplied by 101581 and Then Devided by 2^16 (About 1.55)
    //!
    int32_t Ac2Quant(int32_t qindex, int32_t delta);

    //!
    //! \brief    Get AC Quant Value from Lookup Table for UV
    //! \param    [in] qindex
    //!           Quant Index
    //! \param    [in] delta
    //!           Quant Index Offset
    //! \return   int32_t
    //!           Return AC Quant Value in Lookup Table with Index and Offset for UV
    //!
    int32_t AcUVQuant(int32_t qindex, int32_t delta);

    //!
    //! \brief    Initialize Quant Table for Y/UV in Frame Head
    //! \return   void
    //!
    void QuantInit();

    //!
    //! \brief    Initialize DC/AC DeltaQ Value in Frame Head for Y/UV
    //! \return   void
    //!
    void QuantSetup();

    const uint8_t     *pBufferEnd;              //!< Pointer to Data Buffer End
    const uint8_t     *pBuffer;                 //!< Pointer to Data Buffer
    int32_t            iCount;                  //!< Bits Count for Bitstream Buffer
    uint32_t           uiValue;                 //!< Entropy Value
    uint32_t           uiRange;                 //!< Entropy Range
};

using PVP8_ENTROPY_STATE = Vp8EntropyState*;


//!
//! \class CodechalDecodeVp8
//! \brief This class defines the member fields, functions etc used by VP8 decoder.
//!
class CodechalDecodeVp8 : public CodechalDecode
{
public:
    const int32_t CODECHAL_DECODE_VP8_COEFFPROB_TABLE_SIZE = 4 * 8 * 3 * 11;

    //!
    //! \brief  Constructor
    //! \param    [in] hwInterface
    //!           Hardware interface
    //! \param    [in] debugInterface
    //!           Debug interface
    //! \param    [in] standardInfo
    //!           The information of decode standard for this instance
    //!
    CodechalDecodeVp8(
        CodechalHwInterface   *hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Destructor
    //!
    ~CodechalDecodeVp8();

    //!
    //! \brief    Allocate and initialize VP8 decoder standard
    //! \param    [in] settings
    //!           Pointer to CODECHAL_SETTINGS
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  AllocateStandard(
        PCODECHAL_SETTINGS          settings) override;

    //!
    //! \brief  Set states for each frame to prepare for VP8 decode
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  SetFrameStates() override;

    //!
    //! \brief    VP8 decoder state level function
    //! \details  State level function for VP8 decoder
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  DecodeStateLevel() override;

    //!
    //! \brief    VP9 decoder primitive level function
    //! \details  Primitive level function for GEN specific VP8 decoder
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  DecodePrimitiveLevel() override;

    MOS_STATUS  InitMmcState() override;

    //!
    //! \brief    Allocate fixed sized resources
    //! \details  Allocate fixed sized resources VP8 decode driver
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  AllocateResourcesFixedSizes();

    //!
    //! \brief    Allocate variable sized resources
    //! \details  Allocate variable sized resources in VP8 decode driver
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  AllocateResourcesVariableSizes();

    // Parameters passed by application
    uint16_t                        u16PicWidthInMbLastMaxAlloced;              //!< Max Picture Width in MB used for buffer allocation in past frames
    uint16_t                        u16PicHeightInMbLastMaxAlloced;             //!< Max Picture Height in MB used for buffer allocation in past frames
    bool                            bShortFormatInUse;                          //!< Short Format Indicator
    uint32_t                        u32DataSize;                                //!< Data Size in Decode Params
    uint32_t                        u32DataOffset;                              //!< Data Offset in Decode Params
    PCODEC_VP8_PIC_PARAMS           pVp8PicParams;                              //!< Pointer to VP8 Pic Params
    PCODEC_VP8_IQ_MATRIX_PARAMS     pVp8IqMatrixParams;                         //!< Pointer to VP8 IQ Matrix Params
    MOS_SURFACE                     sDestSurface;                               //!< Pointer to MOS_SURFACE of render surface
    PMOS_RESOURCE                   presLastRefSurface;                         //!< Pointer to resource of Last Reference Surface
    PMOS_RESOURCE                   presGoldenRefSurface;                       //!< Pointer to resource of Golden Reference Surface
    PMOS_RESOURCE                   presAltRefSurface;                          //!< Pointer to resource of Alternate Reference Surface
    MOS_RESOURCE                    resDataBuffer;                              //!< Graphics resource of bitstream data surface
    MOS_RESOURCE                    resCoefProbBuffer;                          //!< Graphics resource of Coefficient Probability data surface

    // Track for several row store buffer's max picture width in MB used for buffer allocation in past frames
    uint16_t                        u16MfdDeblockingFilterRowStoreScratchBufferPicWidthInMb;
    uint16_t                        u16MfdIntraRowStoreScratchBufferPicWidthInMb;
    uint16_t                        u16BsdMpcRowStoreScratchBufferPicWidthInMb;

    // Internally maintained
    MOS_RESOURCE                    resTmpBitstreamBuffer;                          //!< Graphics resource of Bitstream data surface
    MOS_RESOURCE                    resMfdIntraRowStoreScratchBuffer;               //!< Graphics resource of MFD Intra Row Store Scratch data surface
    MOS_RESOURCE                    resMfdDeblockingFilterRowStoreScratchBuffer;    //!< Graphics resource of MFD Deblocking Filter Row Store Scratch data surface
    MOS_RESOURCE                    resBsdMpcRowStoreScratchBuffer;                 //!< Graphics resource of BSD/MPC Row Store Scratch data surface
    MOS_RESOURCE                    resMprRowStoreScratchBuffer;                    //!< Graphics resource of MPR Row Store Scratch data surface
    MOS_RESOURCE                    resSegmentationIdStreamBuffer;                  //!< Graphics resource of Segmentation ID Stream data surface
    PCODEC_REF_LIST                 pVp8RefList[CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP8]; //!< VP8 Reference List
    MOS_RESOURCE                    resSyncObject;                                  //!< Graphics resource of Sync Object
    MOS_RESOURCE                    resPrivateInputBuffer;                          //!< Graphics resource of private surface for bitstream and coeff prob table
    uint32_t                        u32PrivateInputBufferSize;                      //!< Size of private surface
    uint32_t                        u32CoeffProbTableOffset;                        //!< Coefficient Probability Table Offset

    bool                            bDeblockingEnabled;                             //!< VP8 Loop Filter Enable Indicator

    // VP8 Frame Head
    Vp8EntropyState               Vp8EntropyState;                                //!< VP8 Entropy State class to parse frame head
    CODECHAL_DECODE_VP8_FRAME_HEAD  Vp8FrameHead;                                   //!< VP8 Frame Head

    // HuC copy related
    bool                            bHuCCopyInUse;                                  //!< a sync flag used when huc copy and decoder run in the different VDBOX
    MOS_RESOURCE                    resSyncObjectWaContextInUse;                    //!< signals on the video WA context
    MOS_RESOURCE                    resSyncObjectVideoContextInUse;                 //!< signals on the video context

protected:
    //!
    //! \brief    Parse Frame Head
    //! \details  Parse Frame Head from Bitstream Buffer for VP8
    //!
    //! \param    [in] bitstreamBuffer
    //!           Pointer to input Bitstream Buffer
    //! \param    [in] bitstreamBufferSize
    //!           Size of Input Bitstream Buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ParseFrameHead(
        uint8_t* bitstreamBuffer,
        uint32_t bitstreamBufferSize);

    //!
    //! \brief    Copy Bitstream Buffer
    //! \details  Copy Bitstream Buffer from Source to Destiny
    //!
    //! \param    [in] srcBitstreamBuffer
    //!           Resource of Source Bitstream Buffer
    //! \param    [in] dstBitstreamBuffer
    //!           Pointer to Resource of Destiny Bitstream Buffer
    //! \param    [in] size
    //!           Size of Bitstream Buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CopyBitstreamBuffer(
        MOS_RESOURCE    srcBitstreamBuffer,
        PMOS_RESOURCE   dstBitstreamBuffer,
        uint32_t        size);

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS DumpPicParams(
        PCODEC_VP8_PIC_PARAMS picParams);

    MOS_STATUS DumpIQParams(
        PCODEC_VP8_IQ_MATRIX_PARAMS matrixData);
#endif
};

#endif  // __CODECHAL_DECODER_VP8_H__
