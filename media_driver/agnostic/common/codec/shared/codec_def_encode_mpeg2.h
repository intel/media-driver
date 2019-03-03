/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     codec_def_encode_mpeg2.h
//! \brief    Defines encode MPEG2 types and macros shared by CodecHal, MHW, and DDI layer
//! \details  Applies to MPEG2 encode only. Should not contain any DDI specific code.
//!

#ifndef __CODEC_DEF_ENCODE_MPEG2_H__
#define __CODEC_DEF_ENCODE_MPEG2_H__

#include "codec_def_common_encode.h"
#include "codec_def_common_mpeg2.h"

#define CODEC_ENCODE_MPEG2_BRC_PIC_HEADER_SURFACE_SIZE       1024
#define CODEC_ENCODE_MPEG2_VBV_BUFFER_SIZE_UNITS             (16 * 1024) //!< 16 K bits
#define CODEC_ENCODE_MPEG2_MAX_NAL_TYPE                      0x1f

//!
//! \enum     CodecEncodeMpeg2ExtensionStartCode
//! \brief    Codec encode MPEG2 extension start code
//!
enum CodecEncodeMpeg2ExtensionStartCode
{
    // 0x00 - Reserved
    Mpeg2sequenceExtension                   = 0x01,
    Mpeg2sequenceDisplayExtension            = 0x02,
    Mpeg2quantMatrixExtension                = 0x03,
    Mpeg2copyrightExtension                  = 0x04,
    Mpeg2sequnceScalableExtension            = 0x05,
    // 0x06 - Reserved
    Mpeg2pictureDisplayExtension             = 0x07,
    Mpeg2pictureCodingExtension              = 0x08,
    Mpeg2pictureSpatialScalableExtension     = 0x09,
    Mpeg2pictureTemporalScalableExtension    = 0x0A
    // 0x0B .. 0x0F - Reserved
} ;

//!
//! \struct CodecEncodeMpeg2SequenceParams
//! \brief  MPEG2 Sequence Parameter Set
//!
struct CodecEncodeMpeg2SequenceParams
{
    uint16_t  m_frameWidth;                     //!< Width of picture in unit of pixels
    uint16_t  m_frameHeight;                    //!< Height of picture in unit pixels
    uint8_t   m_profile;                        //!< Profile
    uint8_t   m_level;                          //!< Level
    uint8_t   m_chromaFormat;                   //!< Color sampling formats
    uint8_t   m_targetUsage;                    //!< Target usage number, indicates trade-offs between quality and speed

    // ENC + PAK related parameters
    union
    {
        uint16_t  m_aratioFrate;                    //!< Aspect ratio and frame rate
        struct
        {
            uint16_t  m_aspectRatio           : 4;  //!< Aspect ratio
            uint16_t  m_frameRateCode         : 4;  //!< Frame rate Code
            uint16_t  m_frameRateExtN         : 3;  //!< Frame rate extend numerator
            uint16_t  m_frameRateExtD         : 5;  //!< Frame rate extend denominator
        };
    };

    uint32_t    m_bitrate;                          //!<  Bit rate bits per second
    uint32_t    m_vbvBufferSize;                    //!<  VBV buffer size in bits

    uint8_t   m_progressiveSequence           : 1;  //!< Indicate progressive sequence
    uint8_t   m_lowDelay                      : 1;  //!< Indicate low delay
    uint8_t   m_resetBRC                      : 1;  //!< Indicate if a BRC reset is desired to set a new bit rate or frame rate
    uint8_t   m_noAcceleratorSPSInsertion     : 1;  //!< Indicates if current SPS is just a BRC parameter update, not a SPS change to be inserted into the bitstream.
    uint8_t   m_forcePanicModeControl         : 1;  // Force to control the panic mode through DDI.
    uint8_t   m_panicModeDisable              : 1;  // Disable the panic mode
    uint8_t   m_reserved0                     : 2;  //!<  Reserved
    uint8_t   m_rateControlMethod;                  //!< rate control method, CBR = 1, VBR = 2, AVBR = 4, CQP = 3
    uint16_t  m_reserved1;                          //!< Reserved
    uint32_t  m_maxBitRate;                         //!< Maximum bit rate, bits/sec
    uint32_t  m_minBitRate;                         //!< Minimum bit rate, bits/sec
    uint32_t  m_userMaxFrameSize;                   //!< Maximum frame size by user
    uint32_t  m_initVBVBufferFullnessInBit;         //!< Initial VBV buffer fullness size in bits
    uint16_t  m_reserved2;                          //!< Reserved
    uint16_t  m_reserved3;                          //!< Reserved
} ;

//!
//! \struct CodecEncodeMpeg2PictureParams
//! \brief  MPEG2 Picture Parameter Set
//!
struct CodecEncodeMpeg2PictureParams
{
    CODEC_PICTURE       m_currOriginalPic;               //!< The current uncompressed original frame surface for encoding
    CODEC_PICTURE       m_currReconstructedPic;          //!< The uncompressed frame surface for the current reconstructed picture.
    uint8_t             m_pictureCodingType;             //!< Coding Type
    uint8_t             m_fieldCodingFlag           : 1; //!< Indication of field mode coding when set to 1.
    uint8_t             m_fieldFrameCodingFlag      : 1; //!< Indication interlaced frame coding
    uint8_t             m_reserved0                 : 2; //!< Reserved
    uint8_t             m_interleavedFieldBFF       : 1; //!< Indication of input picture layout has top field and bottom field interleaved together
                                                         //!< with bottom field first when set to 1; otherwise (when set to 0) it is 
                                                         //!< interleaved with top field first.
    uint8_t             m_progressiveField          : 1; //!< Indication of input picture layout has only one field picture (half of a frame) stored progressively
    uint8_t             m_reserved1                 : 2; //!< Reserved

    uint8_t             m_numSlice;                      //!< Number of slices per frame; number of slices per field in field coding
    uint8_t             m_picBackwardPrediction;         //!< Indicates whether any macroblocks of the current picture might include backward prediction
    uint8_t             m_bidirectionalAveragingMode;    //!< Indicates the rounding method for combining prediction planes in bidirectional motion compensation
    uint8_t             m_pic4MVallowed;                 //!< Picture 4 MV allowed
    CODEC_PICTURE       m_refFrameList[2];               //!< List of reference frame buffers
    bool                m_useRawPicForRef;               //!< Setting to 1 may improve performance at the cost of image quality
    uint32_t            m_statusReportFeedbackNumber;    //!< The status report feedback data

    uint32_t            m_alternateScan             : 1; //!< Indicate the Inverse Scan method
    uint32_t            m_intraVlcFormat            : 1; //!< Intra VLC format
    uint32_t            m_qscaleType                : 1; //!< Quantizer Scale Type
    uint32_t            m_concealmentMotionVectors  : 1; //!< Indicates if the concealment motion vectors are coded in intra macroblocks
    uint32_t            m_framePredFrameDCT         : 1; //!< Frame Prediction Frame DCT
    uint32_t            m_disableMismatchControl    : 1; //!< Disable mismatch control
    uint32_t            m_intraDCprecision          : 2; //!< Intra DC Precision
    uint32_t            m_fcode00                   : 4; //!< Used for forward horizontal motion vector prediction
    uint32_t            m_fcode01                   : 4; //!< Used for forward vertical motion vector prediction
    uint32_t            m_fcode10                   : 4; //!< Used for backward horizontal motion vector prediction
    uint32_t            m_fcode11                   : 4; //!< Used for backward vertical motion vector prediction
    uint32_t            m_reserved2                 : 8; //!< Reserved

    // ENC + PAK related parameters
    bool                m_lastPicInStream;               //!< Indicate the last picture of the stream
    bool                m_newGop;                        //!< Indicates that a new GOP will start with this picture

    uint16_t            m_gopPicSize;                    //!< Number of pictures within the current GOP
    uint8_t             m_gopRefDist;                    //!< Distance between I- or P (or GPB) - key frames
    uint8_t             m_gopOptFlag                : 2; //!< Indicate the additional flags for the GOP specification
    uint8_t             m_reserved3                 : 6; //!< Reserved

    uint32_t            m_timeCode                  : 25;//!< Time code
    uint32_t            m_reserved4                 : 7; //!< Reserved

    uint16_t            m_temporalReference         : 10;//!< Temporal reference
    uint16_t            m_reserved5                 : 6; //!< Reserved

    uint16_t            m_vbvDelay;

    uint32_t            m_repeatFirstField          : 1; //!< Repeat first field
    uint32_t            m_compositeDisplayFlag      : 1; //!< Composite display flag
    uint32_t            m_vaxis                     : 1; //!< Vaxis
    uint32_t            m_fieldSequence             : 3; //!< Field sequence
    uint32_t            m_subCarrier                : 1; //!< Sub carrier
    uint32_t            m_burstAmplitude            : 7; //!< Burst Amplitude
    uint32_t            m_subCarrierPhase           : 8; //!< Sub carrier phase
    uint32_t            m_reserved6                 : 10;//!< Reserved

    // Parameters for Skip Frames
    uint8_t             m_skipFrameFlag;                 //!< Skip frame flag
    uint8_t             m_numSkipFrames;                 //!< only reserved for BRC case
    uint32_t            m_sizeSkipFrames;                //!< only reserved for BRC case
};

//!
//! \struct CodecEncodeMpeg2SliceParmas
//! \brief  MPEG2 Slice Parameters
//!
struct CodecEncodeMpeg2SliceParmas
{
    uint16_t      m_numMbsForSlice;                 //!< Number of macroblocks per slice
    uint16_t      m_firstMbX;                       //!< Specifies the horizontal position of the first macroblock of the slice expressed in units of macroblocks
    uint16_t      m_firstMbY;                       //!< Specifies the vertical position of the first macroblock of the slice expressed in units of macroblocks
    uint16_t      m_intraSlice;                     //!< Indicates slices coded as Intra Slice
    uint8_t       m_quantiserScaleCode;             //!< Quantier scale code
};

//!
//! \struct CodecEncodeMpeg2VuiParams
//! \brief  MPEG2 VUI Parameters
//!
struct CodecEncodeMpeg2VuiParams
{
    uint32_t    m_videoFormat               : 3; //!< Indicate the representation of the pictures
    uint32_t    m_reserved0                 : 4; //!< Reserved
    uint32_t    m_colourDescription         : 1; //!< Indicate the colour description is presented
    uint32_t    m_colourPrimaries           : 8; //!< The chromaticity coordinates of the source primaries
    uint32_t    m_transferCharacteristics   : 8; //!< The opto-electronic transfer characteristic of the source picture
    uint32_t    m_matrixCoefficients        : 8; //!< The matrix coefficients used in deriving luminance and chrominance signals

    uint32_t    m_displayHorizontalSize     : 14;//!< The horizontal size of the display active region
    uint32_t    m_reserved1                 : 2; //!< Reserved
    uint32_t    m_displayVerticalSize       : 14;//!< The vertical size of the display active region
    uint32_t    m_reserved2                 : 2; //!< Reserved
};

//!
//! \struct CodecEncodeMpeg2QmatixParams
//! \brief  MPEG2 QMATRIX Parameters
//!
struct CodecEncodeMpeg2QmatixParams
{
    uint8_t   m_newQmatrix[4];          //!< 0 - intra Y, 1 - inter Y, 2 - intra chroma, 3 - inter chroma
    uint16_t  m_qmatrix[4][64];         //!< Quantiser matrix
};

//!
//! \struct CodecEncodeMpeg2UserDataList
//! \brief  Linked List for MPEG-2 User Data
//!         User data may be provided in several pieces.
//!         So a linked list is implemented to keep track of them.
//!
struct CodecEncodeMpeg2UserDataList
{
    void                            *m_userData;
    uint32_t                        m_userDataSize;
    CodecEncodeMpeg2UserDataList    *m_nextItem;
};

#endif  // __CODEC_DEF_ENCODE_MPEG2_H__
