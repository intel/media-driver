/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     decode_vp8_entropy_state.h
//! \brief    Defines entropy state list related for vp8 decode
//!
#ifndef __DECODE_VP8_ENTROPY_STATE_H__
#define __DECODE_VP8_ENTROPY_STATE_H__

#include "codec_def_decode_vp8.h" 
#include "mhw_vdbox.h"
#include "decode_allocator.h"
#include "codec_def_vp8_probs.h"


namespace decode
{

//!
//! \enum VP8_MB_LVL_FEATURES
//! VP8 MB Level Features
//!
typedef enum
{
    VP8_MB_LVL_ALT_Q  = 0,
    VP8_MB_LVL_ALT_LF = 1,
    VP8_MB_LVL_MAX    = 2
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
//! \enum VP8_MV_REFERENCE_FRAME
//! VP8 MV Reference Frame
//!
typedef enum
{
    VP8_NONE           = -1,
    VP8_INTRA_FRAME    = 0,
    VP8_LAST_FRAME     = 1,
    VP8_GOLDEN_FRAME   = 2,
    VP8_ALTREF_FRAME   = 3,
    VP8_MAX_REF_FRAMES = 4
} VP8_MV_REFERENCE_FRAME;

//!
//! \enum VP8_TOKEN_PARTITION
//! VP8 Token Partition
//!
typedef enum
{
    VP8_ONE_PARTITION   = 0,
    VP8_TWO_PARTITION   = 1,
    VP8_FOUR_PARTITION  = 2,
    VP8_EIGHT_PARTITION = 3
} VP8_TOKEN_PARTITION;


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

typedef struct _CODECHAL_DECODE_VP8_FRAME_HEAD CODECHAL_DECODE_VP8_FRAME_HEAD, *PCODECHAL_DECODE_VP8_FRAME_HEAD;

//!
//! \class Vp8EntropyState
//! \brief This class defines the member fields, functions etc used by VP8 entropy decoder to parse frame head.
//!
class Vp8EntropyState
{
public:
    const uint8_t  m_keyFrame    = 0;                                        //!< VP8 Key Frame Flag
    const uint8_t  m_interFrame  = 1;                                        //!< VP8 Inter Frame Flag
    const uint32_t m_bdValueSize = ((uint32_t)sizeof(uint32_t) * CHAR_BIT);  // VP8 BD Value Size
    const uint32_t m_lotsOfBits  = 0x40000000;                               //!< Offset for parsing frame head
    const uint8_t  m_probHalf    = 128;                                      //!< VP8 Half Probability

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

    PCODECHAL_DECODE_VP8_FRAME_HEAD m_frameHead           = nullptr;  //!< Pointer to VP8 Frame Head
    uint8_t *                       m_bitstreamBuffer     = nullptr;  //!< Pointer to Bitstream Buffer
    uint32_t                        m_bitstreamBufferSize = 0;        //!< Size of Bitstream Buffer
    uint8_t *                       m_dataBuffer          = nullptr;  //!< Pointer to Data Buffer
    uint8_t *                       m_dataBufferEnd       = nullptr;  //<! Pointer to Data Buffer End

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

    const uint8_t *m_bufferEnd = nullptr;  //!< Pointer to Data Buffer End
    const uint8_t *m_buffer = nullptr;     //!< Pointer to Data Buffer
    int32_t        m_count = 0;      //!< Bits Count for Bitstream Buffer
    uint32_t       m_value = 0;      //!< Entropy Value
    uint32_t       m_range = 0;      //!< Entropy Range

MEDIA_CLASS_DEFINE_END(decode__Vp8EntropyState)
};

}  // namespace decode

#endif  // !__DECODE_VP8_REFERENCE_FRAMES_H__
