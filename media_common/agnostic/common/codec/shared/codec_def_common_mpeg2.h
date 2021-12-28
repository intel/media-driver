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

#endif  // __CODEC_DEF_COMMON_MPEG2_H__
