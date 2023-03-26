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
//! \file     codec_def_common_mpeg2.h
//! \brief    Defines basic MPEG2 types and macros shared by CodecHal, MHW, and DDI layer
//! \details  This is the base header for all codec_def MPEG2 files. All codec_def MPEG2 files should include this file which should not contain any DDI specific code.
//!
#ifndef __CODEC_DEF_COMMON_MPEG2_H__
#define __CODEC_DEF_COMMON_MPEG2_H__

#include "codec_def_common.h"

//!
//! \struct CodecMpeg2IqMatrix
//! \brief Inverse Quantization Matrix Buffer
//!
struct CodecMpeg2IqMatrix
{
    int32_t m_loadIntraQuantiserMatrix;         //!< Indicate if intra Quantiser Matrix is available
    int32_t m_loadNonIntraQuantiserMatrix;      //!< Indicate if non intra Quantiser Matrix is available
    int32_t m_loadChromaIntraQuantiserMatrix;   //!< Indicate if chroma intra Quantiser Matrix is available
    int32_t m_loadChromaNonIntraQuantiserMatrix;//!< Indicate if chroma non intra Quantiser Matrix is available
    uint8_t m_intraQuantiserMatrix[64];         //!< Intra Quantiser Matrix
    uint8_t m_nonIntraQuantiserMatrix[64];      //!< Non intra Quantiser Matrix
    uint8_t m_chromaIntraQuantiserMatrix[64];   //!< Chroma intra Quantiser Matrix
    uint8_t m_chromaNonIntraQuantiserMatrix[64];//!< Chroma non intra Quantiser Matrix
};

//!
//! \struct CodecDecodeMpeg2MbParams
//! \brief  Mpeg2 MB parameter
//!
struct CodecDecodeMpeg2MbParams
{
    int32_t m_mbAddr;  //!< Macroblock address
    union
    {
        struct
        {
            uint16_t m_intraMb : 1;           //!< Intra macroblock
            uint16_t m_motionFwd : 1;         //!< Microblock forward motion
            uint16_t m_motionBwd : 1;         //!< Microblock backward motion
            uint16_t m_motion4mv : 1;         //!< Microblock 4MV motion
            uint16_t m_h261Lpfilter : 1;      //!< low pass filter
            uint16_t m_fieldResidual : 1;     //!< DCT type
            uint16_t m_mbScanMethod : 2;      //!< 0: zigzag scan, 1:alternative-vertical, 2: alternative-horizontal
            uint16_t m_motionType : 2;        //!< Invalid for I pic, for P/B pic
            uint16_t m_hostResidualDiff : 1;  //!< Host residual difference
            uint16_t m_reserved : 1;          //!< Reserved
            uint16_t m_mvertFieldSel : 4;     //!< Motion vertical field select
        };
        uint16_t m_value;
    } MBType;
    uint16_t m_mbSkipFollowing;                   //!< The number of skipped macroblocks to be generated following the current macroblock.
    uint32_t m_mbDataLoc;                         //!< Used as an index into residual difference block data buffer.
    uint16_t m_codedBlockPattern;                 //!< Coded block pattern
    uint8_t  m_numCoeff[CODEC_NUM_BLOCK_PER_MB];  //!< Indicates the number of coefficients in the residual difference data buffer for each block i of the macroblock.
    int16_t  m_motionVectors[8];                  //!< Motion vector
};

#endif  // __CODEC_DEF_COMMON_MPEG2_H__
