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
//! \file     codec_def_decode_jpeg.h
//! \brief    Defines decode JPEG types and macros shared by CodecHal, MHW, and DDI layer
//! \details  Applies to JPEG decode only. Should not contain any DDI specific code.
//!
#ifndef __CODEC_DEF_DECODE_JPEG_H__
#define __CODEC_DEF_DECODE_JPEG_H__

#include "codec_def_common_jpeg.h"
#include "media_defs.h"
#define CODEC_DECODE_JPEG_BLOCK_SIZE 8
//!
//! \enum CodecDecodeJpegRotationType
//! \brief JPEG Rotation Types
//!
enum CodecDecodeJpegRotationType
{
    jpegRotation0       = 0,    //!< Rotation of 0 digree
    jpegRotation90      = 1,    //!< Rotation of 90 digrees
    jpegRotation270     = 2,    //!< Rotation of 270 digrees
    jpegRotation180     = 3,    //!< Rotation of 180 digrees
};

//!
//! \struct CodecDecodeJpegImageLayout
//! \brief Information pertaining to the output of the decode render target
//!
struct CodecDecodeJpegImageLayout
{
    uint32_t       m_componentDataOffset[jpegNumComponent];       //!< The offset within the decode render target for YUV components (Y/U/V/A)
    uint32_t       m_pitch;                                       //!< Pitch of the decode render target
};

//!
//! JPEG Scan Parameter
//! \brief JPEG Scan Parameter
//! Note: Some DDIs treat scans as slices
//!
struct CodecDecodeJpegScanParameter
{
    struct
    {
        uint16_t              NumComponents;                         //!<  Number of components
        uint8_t               ComponentSelector[jpegNumComponent];   //!<  Component selector
        uint8_t               DcHuffTblSelector[jpegNumComponent];   //!<  DC Huffman table selector
        uint8_t               AcHuffTblSelector[jpegNumComponent];   //!<  AC Huffman table selector
        uint16_t              RestartInterval;                       //!<  Indicate restart interval
        uint32_t              MCUCount;                              //!<  MCU count
        uint16_t              ScanHoriPosition;                      //!<  Scan horizontal position
        uint16_t              ScanVertPosition;                      //!<  Scan vertical position
        uint32_t              DataOffset;                            //!<  Data offset
        uint32_t              DataLength;                            //!<  Data length
    }ScanHeader[jpegNumComponent];

    uint16_t              NumScans;
};

typedef struct tagJPEG_DECODE_QUERY_STATUS
{
    uint32_t StatusReportFeedbackNumber;
    uint8_t  bStatus;
    uint8_t  reserved8bits;
    uint16_t reserved16bits;
} JPEG_DECODE_QUERY_STATUS, *PJPEG_DECODE_QUERY_STATUS;
#endif  // __CODEC_DEF_DECODE_JPEG_H__

