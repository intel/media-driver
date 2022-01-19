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
//! \file     codec_def_decode_mpeg2.h
//! \brief    Defines decode MPEG2 types and macros shared by CodecHal, MHW, and DDI layer
//! \details  Applies to MPEG2 decode only. Should not contain any DDI specific code.
//!
#ifndef __CODEC_DEF_DECODE_MPEG2_H__
#define __CODEC_DEF_DECODE_MPEG2_H__

#include "codec_def_common_mpeg2.h"

#define CODEC_MPEG2_BATCH_BUFFERS_NUM         3        //! number of batch buffers
#define CODEC_MPEG2_BYTES_PER_MB              512      //! bitstream size per macroblock
#define CODEC_MPEG2_IDCTBLOCK_SIZE            64       //! IDCT block size

#define CODEC_MPEG2_MB_MOTION_FORWARD         2        //!< Bit 1
#define CODEC_MPEG2_MB_MOTION_BACKWARD        4        //!< Bit 2

//!
//! \struct CodecDecodeMpeg2PicParams
//! \brief  Mpeg2 picture parameter
//!
struct CodecDecodeMpeg2PicParams
{
    CODEC_PICTURE       m_currPic;                      //!< The current frame surface
    uint16_t            m_forwardRefIdx;                //!< Forward reference index
    uint16_t            m_backwardRefIdx;               //!< Backward reference index
    uint32_t            m_topFieldFirst;                //!< Indicate that the first field of the reconstructed frame is the top field
    bool                m_secondField;                  //!< Indicate the second field
    uint32_t            m_statusReportFeedbackNumber;   //!< The status report feedback data

    union
    {
        struct
        {
            uint16_t    m_reserved0               : 6;  //!< Reserved
            uint16_t    m_scanOrder               : 1;  //!< Indicate the scan order
            uint16_t    m_intraVlcFormat          : 1;  //!< Intra VLC format
            uint16_t    m_quantizerScaleType      : 1;  //!< Quantizer scan type
            uint16_t    m_concealmentMVFlag       : 1;  //!< Concealment MV flag
            uint16_t    m_frameDctPrediction      : 1;  //!< Frame DCT prediction
            uint16_t    m_topFieldFirst           : 1;  //!< First frame first
            uint16_t    m_reserved1               : 2;  //!< Reserved
            uint16_t    m_intraDCPrecision        : 2;  //!< Intra DC precision
        };
        struct
        {
            uint16_t    m_value;                        //!< word 0 value
        };
    } W0;
    union
    {
        struct
        {
            uint16_t    m_fcode11                : 4;   //!< Used for backward vertical motion vector prediction
            uint16_t    m_fcode10                : 4;   //!< Used for backward horizontal motion vector prediction
            uint16_t    m_fcode01                : 4;   //!< Used for forward vertical motion vector prediction
            uint16_t    m_fcode00                : 4;   //!< Used for forward horizontal motion vector prediction
        };
        struct
        {
            uint16_t    m_value;                        //!< word 1 value
        };
    } W1;

    uint16_t            m_horizontalSize;               //!< Picture horizontal size
    uint16_t            m_verticalSize;                 //!< Picture vertical size
    int32_t             m_pictureCodingType;            //!< Picture coding type
};

//!
//! \struct CodecDecodeMpeg2SliceParams
//! \brief  Mpeg2 slice parameter
//!
struct CodecDecodeMpeg2SliceParams
{
    uint32_t m_sliceDataSize;               //!< Number of bits for this slice
    uint32_t m_sliceDataOffset;             //!< Offset to the first byte of slice data
    uint32_t m_macroblockOffset;            //!< Offset to the first bit of MB from the first byte of slice data
    uint32_t m_sliceHorizontalPosition;     //!< The horizontal position of the first macroblock in the slice
    uint32_t m_sliceVerticalPosition;       //!< The vertical position of the first macroblock in the slice
    int32_t  m_quantiserScaleCode;          //!< Quantiser scale code
    uint32_t m_numMbsForSlice;              //!<  The number of macroblocks in the slice
    bool     m_numMbsForSliceOverflow;      //!< NumMbsForSlice > max allowable Mbs
    uint8_t  m_reservedBits;                //!< for WAs
    uint8_t  m_startCodeBitOffset;          //!< Start code bit offset
};

#endif  // __CODEC_DEF_DECODE_MPEG2_H__

