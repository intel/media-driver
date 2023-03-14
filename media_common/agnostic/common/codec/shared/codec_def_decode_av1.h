/*
* Copyright (c) 2017-2023, Intel Corporation
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
//! \file     codec_def_decode_av1.h
//! \brief    Defines decode AV1 types and macros shared by CodecHal, MHW, and DDI layer
//! \details  Applies to AV1 decode only. Should not contain any DDI specific code.
//!
#ifndef __CODEC_DEF_DECODE_AV1_H__
#define __CODEC_DEF_DECODE_AV1_H__

#include "codec_def_common_av1.h"

#define CODECHAL_MAX_DPB_NUM_AV1              127                                // Maximum number of uncompressed decoded buffers that driver supports for non-LST mode
#define CODEC_NUM_REF_AV1_TEMP_BUFFERS        8
#define CODEC_NUM_AV1_SECOND_BB               64
#define CODEC_NUM_AV1_TEMP_BUFFERS            (CODEC_NUM_REF_AV1_TEMP_BUFFERS + 1) //!< Temp buffers number
#define CODECHAL_MAX_DPB_NUM_LST_AV1          255                                // Maximum number of uncompressed decoded buffers that driver supports with LST support

// AV1 parameters definition

//!
//! \struct CodecAv1FilmGrainParams
//! \brief Define the film grain parameters for AV1
//!
struct CodecAv1FilmGrainParams
{
    union
    {
        struct
        {
            uint32_t    m_applyGrain                    : 1;            //!< apply Grain flag
            uint32_t    m_chromaScalingFromLuma         : 1;            //!< chroma scaling from luma
            uint32_t    m_grainScalingMinus8            : 2;            //!< Grain scaling minus 8
            uint32_t    m_arCoeffLag                    : 2;            //!< AR Coeff lag
            uint32_t    m_arCoeffShiftMinus6            : 2;            //!< AR coeff shift minus 6
            uint32_t    m_grainScaleShift               : 2;            //!< grain scale shift
            uint32_t    m_overlapFlag                   : 1;            //!< overlap flag
            uint32_t    m_clipToRestrictedRange         : 1;            //!< clip to restricted range flag
            uint32_t    m_reservedBits                  : 20;           //!< reserved bits
        } m_fields;
        uint32_t m_value;   //!< film grain info flag value
    } m_filmGrainInfoFlags;

    uint16_t    m_randomSeed;           //!< random seed
    uint8_t     m_numYPoints;           //!< num Y points, range [0..14]
    uint8_t     m_pointYValue[14];      //!< point Y value array
    uint8_t     m_pointYScaling[14];    //!< point Y scaling array
    uint8_t     m_numCbPoints;          //!< num Cb Points, range [0..10]
    uint8_t     m_pointCbValue[10];     //!< point Cb Value
    uint8_t     m_pointCbScaling[10];   //!< point Cb Scaling
    uint8_t     m_numCrPoints;          //!< num Cr Points, range [0..10]
    uint8_t     m_pointCrValue[10];     //!< point Cr Value
    uint8_t     m_pointCrScaling[10];   //!< point Cr Scaling
    int8_t      m_arCoeffsY[24];        //!< ar coeffs for Y, range [-128..127]
    int8_t      m_arCoeffsCb[25];       //!< ar coeffs for Cb, range [-128..127]
    int8_t      m_arCoeffsCr[25];       //!< ar coeffs for Cr, range [-128..127]
    uint8_t     m_cbMult;               //!< cb multipler
    uint8_t     m_cbLumaMult;           //!< cb Luma Multipler
    uint16_t    m_cbOffset;             //!< cb offset, range [0..512]
    uint8_t     m_crMult;               //!< Cr multipler
    uint8_t     m_crLumaMult;           //!< Cr Luma Multipler
    uint16_t    m_crOffset;             //!< Cr Offset, range [0..512]
    uint32_t    m_reservedDws[4];       //!< reserved DWs
};

//!
//! \struct CodecAv1PicParams
//! \brief Define AV1 picture parameters
//!
struct CodecAv1PicParams
{
    CODEC_PICTURE   m_currPic;
    CODEC_PICTURE   m_currDisplayPic;
    uint8_t         m_profile;        // [0..2]
    uint8_t         m_anchorFrameInsertion;
    uint8_t         m_anchorFrameNum;
    PMOS_SURFACE    m_anchorFrameList;

    // sequence info
    uint8_t         m_orderHintBitsMinus1;      // [0..7]
    uint8_t         m_bitDepthIdx;              // [0..2]
    uint8_t         m_matrixCoefficients;       // [0..14]
    uint8_t         m_reserved8b;

    union
    {
        struct
        {
            uint32_t    m_stillPicture              : 1;
            uint32_t    m_use128x128Superblock      : 1;
            uint32_t    m_enableFilterIntra         : 1;
            uint32_t    m_enableIntraEdgeFilter     : 1;

            // read_compound_tools
            uint32_t    m_enableInterintraCompound  : 1;      // [0..1]
            uint32_t    m_enableMaskedCompound      : 1;      // [0..1]

            uint32_t    m_enableDualFilter          : 1;
            uint32_t    m_enableOrderHint           : 1;
            uint32_t    m_enableJntComp             : 1;
            uint32_t    m_enableCdef                : 1;
            uint32_t    m_reserved3b                : 3;

            uint32_t    m_monoChrome                : 1;
            uint32_t    m_colorRange                : 1;
            uint32_t    m_subsamplingX              : 1;
            uint32_t    m_subsamplingY              : 1;
            uint32_t    m_chromaSamplePosition      : 1;
            uint32_t    m_filmGrainParamsPresent    : 1;
            uint32_t    m_reservedSeqInfoBits       : 13;
        } m_fields;
        uint32_t    m_value;
    } m_seqInfoFlags;

    // frame info
    union
    {
        struct
        {
            uint32_t    m_frameType                 : 2;
            uint32_t    m_showFrame                 : 1;
            uint32_t    m_showableFrame             : 1;
            uint32_t    m_errorResilientMode        : 1;
            uint32_t    m_disableCdfUpdate          : 1;
            uint32_t    m_allowScreenContentTools   : 1;  // [0..1]
            uint32_t    m_forceIntegerMv            : 1;  // [0..1]
            uint32_t    m_allowIntrabc              : 1;
            uint32_t    m_useSuperres               : 1;
            uint32_t    m_allowHighPrecisionMv      : 1;
            uint32_t    m_isMotionModeSwitchable    : 1;
            uint32_t    m_useRefFrameMvs            : 1;
            uint32_t    m_disableFrameEndUpdateCdf  : 1;
            uint32_t    m_uniformTileSpacingFlag    : 1;
            uint32_t    m_allowWarpedMotion         : 1;
            uint32_t    m_largeScaleTile            : 1;
            uint32_t    m_reservedPicInfoBits       : 15;
        } m_fields;
        uint32_t    m_value;
    } m_picInfoFlags;

    uint16_t        m_frameWidthMinus1;   // [0..65535] //!< Super-Res downscaled resolution
    uint16_t        m_frameHeightMinus1;  // [0..65535] //!< Super-Res downscaled resolution

    CODEC_PICTURE   m_refFrameMap[8];
    uint8_t         m_refFrameIdx[7];     // [0..7]
    uint8_t         m_primaryRefFrame;    // [0..7]

    uint16_t        m_outputFrameWidthInTilesMinus1;  // [0..65535]
    uint16_t        m_outputFrameHeightInTilesMinus1; // [0..65535]
    uint32_t        m_reserved32b2;

    // deblocking filter
    uint8_t         m_filterLevel[2];  // [0..63]
    uint8_t         m_filterLevelU;    // [0..63]
    uint8_t         m_filterLevelV;    // [0..63]
    union
    {
        struct
        {
            uint8_t     m_sharpnessLevel        : 3;  // [0..7]
            uint8_t     m_modeRefDeltaEnabled   : 1;
            uint8_t     m_modeRefDeltaUpdate    : 1;
            uint8_t     m_reservedField         : 3;  // [0]
        } m_fields;
        uint8_t m_value;
    } m_loopFilterInfoFlags;

    uint8_t         m_orderHint;
    uint8_t         m_superresScaleDenominator;   // [9..16]
    uint8_t         m_interpFilter;               // [0..9]

    int8_t          m_refDeltas[8];   // [-64..63]
    int8_t          m_modeDeltas[2];  // [-64..63]

    // quantization
    uint16_t        m_baseQindex;  // [0..255]
    int8_t          m_yDcDeltaQ;   // [-64..63]
    int8_t          m_uDcDeltaQ;   // [-64..63]
    int8_t          m_uAcDeltaQ;   // [-64..63]
    int8_t          m_vDcDeltaQ;   // [-64..63]
    int8_t          m_vAcDeltaQ;   // [-64..63]
    uint8_t         m_reserved8b2;

    // quantization_matrix
    union
    {
        struct
        {
            uint16_t    m_usingQmatrix     : 1;
            // valid only when using_qmatrix is 1.
            uint16_t    m_qmY              : 4;           // [0..15]
            uint16_t    m_qmU              : 4;           // [0..15]
            uint16_t    m_qmV              : 4;           // [0..15]
            uint16_t    m_reservedField    : 3;           // [0]
        } m_fields;
        uint16_t m_value;
    } m_qMatrixFlags;

    union
    {
        struct
        {
            // delta_q parameters
            uint32_t    m_deltaQPresentFlag     : 1;    // [0..1]
            uint32_t    m_log2DeltaQRes         : 2;    // [0..3]

            // delta_lf parameters
            uint32_t    m_deltaLfPresentFlag    : 1;    // [0..1]
            uint32_t    m_log2DeltaLfRes        : 2;    // [0..3]
            uint32_t    m_deltaLfMulti          : 1;    // [0..1]

            // read_tx_mode
            uint32_t    m_txMode                : 2;    // [0..3]

            // read_frame_reference_mode
            uint32_t    m_referenceMode         : 2;    // [0..3]  will be replaced by reference_select
            uint32_t    m_reducedTxSetUsed      : 1;    // [0..1]

            // tiles
            uint32_t    m_skipModePresent       : 1;    // [0..1]
            uint32_t    m_reservedField         : 19;   // [0]
        } m_fields;
        uint32_t    m_value;
    } m_modeControlFlags;

    CodecAv1SegmentsParams m_av1SegData;                               //!< segment data

    uint8_t         m_tileCols;
    uint16_t        m_widthInSbsMinus1[64]; //!< note: 64 not 63
    uint8_t         m_tileRows;
    uint16_t        m_heightInSbsMinus1[64];

    uint16_t        m_tileCountMinus1;
    uint16_t        m_contextUpdateTileId;

    // CDEF
    uint8_t         m_cdefDampingMinus3;        // [0..3]
    uint8_t         m_cdefBits;                 // [0..3]
    uint8_t         m_cdefYStrengths[8];        // [0..63]
    uint8_t         m_cdefUvStrengths[8];       // [0..63]

    union
    {
        struct
        {
            uint16_t    m_yframeRestorationType    : 2;    // [0..3]
            uint16_t    m_cbframeRestorationType   : 2;    // [0..3]
            uint16_t    m_crframeRestorationType   : 2;    // [0..3]
            uint16_t    m_lrUnitShift              : 2;    // [0..2]
            uint16_t    m_lrUvShift                : 1;    // [0..1]
            uint16_t    m_reservedField            : 7;    // [0]
        } m_fields;
        uint16_t    m_value;
    } m_loopRestorationFlags;

    // global motion
    CodecAv1WarpedMotionParams      m_wm[7];
    CodecAv1FilmGrainParams         m_filmGrainParams;

    uint32_t        m_bsBytesInBuffer;
    uint32_t        m_statusReportFeedbackNumber;

    // Below are parameters for driver internal use only, not corresponding to any DDI parameter
    bool            m_losslessMode;                     //!< frame lossless mode
    uint16_t        m_superResUpscaledWidthMinus1;      //!< Super-Res upscaled width, [0..65535] 
    uint16_t        m_superResUpscaledHeightMinus1;     //!< Super-Res upscaled height, [0..65535] 
    uint8_t         m_activeRefBitMaskMfmv[7];          //!< active reference bitmask for Motion Field Projection, [0]: LAST_FRAME, [6]: ALTREF
    uint8_t         m_refFrameSide[8];                  //!< ref_frame_side for each reference
};

struct CodecAv1TileParams
{
    uint32_t        m_bsTileDataLocation;
    uint32_t        m_bsTileBytesInBuffer;
    uint16_t        m_badBSBufferChopping;
    uint16_t        m_tileRow;
    uint16_t        m_tileColumn;
    uint16_t        m_tileIndex;
    uint16_t        m_reserved16b;
    uint16_t        m_startTileIdx;
    uint16_t        m_endTileIdx;
    uint16_t        m_tile_idx_in_tile_list;
    CODEC_PICTURE   m_anchorFrameIdx;
    uint32_t        m_bsTilePayloadSizeInBytes;
};


struct Av1SharedBuf
{
    PMOS_BUFFER buffer = nullptr;
    int         refCnt = 0;
};

struct Av1RefAssociatedBufs
{
    PMOS_BUFFER   mvBuf    = nullptr;
    Av1SharedBuf *segIdBuf = nullptr;
    Av1SharedBuf  segIdWriteBuf;
    Av1SharedBuf *initCdfBuf = nullptr;
    Av1SharedBuf  bwdAdaptCdfBuf;
    Av1SharedBuf  defaultCdfBuf;
    bool          disableFrmEndUpdateCdf = false;
};

enum
{
    av1ChromaFormatMonochrome = 0,
    av1ChromaFormatYuv420     = 1,
    av1ChromaFormatYuv422     = 2,
    av1ChromaFormatYuv444     = 3,
};

#endif  // __CODEC_DEF_DECODE_AV1_H__

