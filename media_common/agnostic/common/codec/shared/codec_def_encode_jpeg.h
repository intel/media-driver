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
//! \file     codec_def_encode_jpeg.h
//! \brief    Defines encode JPEG types and macros shared by CodecHal, MHW, and DDI layer
//! \details  Applies to JPEG encode only. Should not contain any DDI specific code.
//!

#ifndef __CODEC_DEF_ENCODE_JPEG_H__
#define __CODEC_DEF_ENCODE_JPEG_H__

#include "codec_def_common_jpeg.h"
#include "codec_def_common_encode.h"

#define JPEG_MAX_NUM_QUANT_TABLE_INDEX          3    // Max 3 quant tables are allowed for encode
#define JPEG_MAX_QUANT_TABLE                    3    // MAx Number of Quantization tables that can be sent by the application
#define JPEG_NUM_ENCODE_HUFF_BUFF               4    // Total number of Huffman tables that app can send for JPEG encode (2AC and 2DC tables allowed per frame)

// Max supported resolution for JPEG encode is 16K X 16K
#define ENCODE_JPEG_MAX_PIC_WIDTH     16384
#define ENCODE_JPEG_MAX_PIC_HEIGHT    16384

#define JPEG_MAX_NUM_HUFF_TABLES      2    // Max 2 sets of Huffman Tables are allowed (2AC and 2 DC)

//!
//! \struct CodecEncodeJpegQuantTable
//! \brief Define JPEG Quant Table
//!
struct CodecEncodeJpegQuantTable
{
    struct
    {
        uint32_t      m_tableID;                    //!< Table ID
        uint32_t      m_precision;                  //!< Precision
        uint16_t      m_qm[JPEG_NUM_QUANTMATRIX];   //!< Quant Matrix
    } m_quantTable[JPEG_MAX_NUM_QUANT_TABLE_INDEX];   //!< Quant table array
};

//!
//! \struct CodecEncodeJpegHuffData
//! \brief Define Huffman data for JPEG encode
//!
struct CodecEncodeJpegHuffData
{
    uint32_t   m_tableClass;                                //!< table class
    uint32_t   m_tableID;                                   //!< table ID
    uint8_t    m_bits[JPEG_NUM_HUFF_TABLE_AC_BITS];         //!< AC bits
    uint8_t    m_huffVal[JPEG_NUM_HUFF_TABLE_AC_HUFFVAL];   //!< AC Huffman value
};

//!
//! \struct CodecEncodeJpegHuffmanDataArray
//! \brief Define Huffman data array for JPEG encode
//!
struct CodecEncodeJpegHuffmanDataArray
{
    //!< huffmanData[0] --> Table for DC component of luma
    //!< huffmanData[1] --> Table for AC component of luma
    //!< huffmanData[2] --> Table for DC component of chroma
    //!< huffmanData[3] --> Table for AC component of chroma
    CodecEncodeJpegHuffData  m_huffmanData[JPEG_NUM_ENCODE_HUFF_BUFF];
};

//!
//! \enum CodecEncodeJpegInputSurfaceFormat
//! \brief matches up with InputSurfaceFormats
//! (converted from MOS format in ConvertMediaFormatToInputSurfaceFormat())
//! May want to unify enums instead of casting
//!
enum CodecEncodeJpegInputSurfaceFormat
{
    codechalJpegNV12    = 1,    //!< NV12 surface format
    codechalJpegUYVY    = 2,    //!< UYVY surface format
    codechalJpegYUY2    = 3,    //!< YUY2 surface format
    codechalJpegY8      = 4,    //!< Y8 surface format
    codechalJpegRGB     = 5     //!< RGB surface format
};

//!
//! \struct CodecEncodeJpegPictureParams
//! \brief Picture Parameter Set for JPEG Encode
//!
struct CodecEncodeJpegPictureParams
{
    uint32_t    m_profile      : 2;             //!< Profile. 0 -Baseline, 1 - Extended, 2 - Lossless, 3 - Hierarchical
    uint32_t    m_progressive  : 1;             //!< Progressive flag. 1- Progressive, 0 - Sequential
    uint32_t    m_huffman      : 1;             //!< Huffman flag. 1 - Huffman , 0 - Arithmetic
    uint32_t    m_interleaved  : 1;             //!< Interleaved flag. 1 - Interleaved, 0 - NonInterleaved
    uint32_t    m_differential : 1;             //!< Differential flag. 1 - Differential, 0 - NonDifferential

    uint32_t    m_picWidth;                     //!< Picture Width
    uint32_t    m_picHeight;                    //!< Picture Height

    uint32_t    m_inputSurfaceFormat;           //!< Input surface format
    uint32_t    m_sampleBitDepth;               //!< Sample bit depth

    uint32_t    m_numComponent;                 //!< Component Number
    uint8_t     m_componentID[4];               //!< Component ID
    uint8_t     m_quantTableSelector[4];        //!< Quant table selector

    uint32_t    m_quality;                      //!< Quality

    uint32_t    m_numScan;                      //!< Scan number
    uint32_t    m_numQuantTable;                //!< Quant table number
    uint32_t    m_numCodingTable;               //!< Coding table number

    uint32_t    m_statusReportFeedbackNumber;   //!< Status report feedback number

};

//!
//! \struct CodecEncodeJpegScanHeader
//! \brief Scan Header structure for JPEG Encode
//!
struct CodecEncodeJpegScanHeader
{
    uint32_t     m_restartInterval;             //!< Restart Interval

    uint32_t     m_numComponent;                //!< Component number
    uint8_t      m_componentSelector[4];        //!< Component selector
    uint8_t      m_dcCodingTblSelector[4];      //!< DC coding table selector
    uint8_t      m_acCodingTblSelector[4];      //!< AC coding table selector

    uint32_t     FirstDCTCoeff;
    uint32_t     LastDCTCoeff;
    uint32_t     Ah;
    uint32_t     Al;
};

// matrix required to read in the quantization matrix
static const uint8_t jpeg_qm_scan_8x8[64] =
{
    // Zig-Zag scan pattern
    0,   1,  8, 16,  9,  2,  3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};

#endif  // __CODEC_DEF_ENCODE_JPEG_H__
