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
//! \file     codec_def_common_jpeg.h
//! \brief    Defines basic JPEG types and macros shared by CodecHal, MHW, and DDI layer
//! \details  This is the base header for all codec_def JPEG files. All codec_def JPEG files should include this file which should not contain any DDI specific code.
//!
#ifndef __CODEC_DEF_COMMON_JPEG_H__
#define __CODEC_DEF_COMMON_JPEG_H__

#include "codec_def_common.h"

#define JPEG_MAX_NUM_HUFF_TABLE_INDEX           2    // For baseline only allowed 2, else could have 4.
#define JPEG_NUM_QUANTMATRIX                    64   // Elements of 8x8 matrix in zig-zag scan order.
#define JPEG_MAX_NUM_OF_QUANTMATRIX             4    // JPEG decoders can store up to 4 different quantization matrix

#define JPEG_NUM_HUFF_TABLE_DC_BITS             12   // Huffman Table DC BITS
#define JPEG_NUM_HUFF_TABLE_DC_HUFFVAL          12   // Huffman Table DC HUFFVAL
#define JPEG_NUM_HUFF_TABLE_AC_BITS             16   // Huffman Table AC BITS
#define JPEG_NUM_HUFF_TABLE_AC_HUFFVAL          162  // Huffman Table AC HUFFVAL

//!
//! \enum CodecJpegComponents
//! \brief JPEG Component Types
//!
enum CodecJpegComponents
{
    jpegComponentY      = 0,    //!< Component Y
    jpegComponentU      = 1,    //!< Component U
    jpegComponentV      = 2,    //!< Component V
    jpegNumComponent    = 3,    //!< Component number
};

//!
//! \struct CodecJpegQuantMatrix
//! \brief JPEG Quantization Matrix
//!
struct CodecJpegQuantMatrix
{
    uint32_t m_jpegQMTableType[JPEG_MAX_NUM_OF_QUANTMATRIX];                    //!< Quant Matrix table type
    uint8_t  m_quantMatrix[JPEG_MAX_NUM_OF_QUANTMATRIX][JPEG_NUM_QUANTMATRIX];  //!< Quant Matrix
};

#endif  // __CODEC_DEF_COMMON_JPEG_H__
