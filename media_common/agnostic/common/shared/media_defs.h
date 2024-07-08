/*
* Copyright (c) 2022-2024, Intel Corporation
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
//! \file     media_defs.h
//! \brief    Define for media usages
//! \details
//!
#ifndef __MEDIA_DEFS_H__
#define __MEDIA_DEFS_H__

//!
//! \enum CodecDecodeJpegChromaType
//! \brief JPEG Chroma Types
//!
enum CodecDecodeJpegChromaType
{
    jpegYUV400      = 0,    //!< YUV400
    jpegYUV420      = 1,    //!< YUV420
    jpegYUV422H2Y   = 2,    //!< YUV422H_2Y
    jpegYUV444      = 3,    //!< YUV444
    jpegYUV411      = 4,    //!< YUV411, not supported on JPEG encode
    jpegYUV422V2Y   = 5,    //!< YUV422V_2Y, not supported on JPEG encode
    jpegYUV422H4Y   = 6,    //!< YUV422H_4Y, not supported on JPEG encode
    jpegYUV422V4Y   = 7,    //!< YUV422V_4Y, not supported on JPEG encode
    jpegRGB         = 8,    //!< RGB, not supported on JPEG encode
    jpegBGR         = 9     //!< BGR, not supported on JPEG encode
};

//!
//! \enum     CODECHAL_STANDARD 
//! \brief    Codec standard
//!
enum CODECHAL_STANDARD
{
    // MFX/MFL pipeline
    CODECHAL_MPEG2      = 0,
    CODECHAL_VC1        = 1,
    CODECHAL_AVC        = 2,
    CODECHAL_JPEG       = 3,
    CODECHAL_RESERVED   = 4,    //formerly SVC
    CODECHAL_VP8        = 5,
    CODECHAL_UNDEFINED  = 9,

    // Cenc Decode
    CODECHAL_CENC       = 63,

    // HCP pipeline
    CODECHAL_HCP_BASE   = 64,
    CODECHAL_HEVC       = CODECHAL_HCP_BASE,
    CODECHAL_VP9        = CODECHAL_HCP_BASE + 1,

    //AVP pipeline
    CODECHAL_AVP_BASE   = CODECHAL_HCP_BASE + 2,
    CODECHAL_AV1        = CODECHAL_AVP_BASE,

    CODECHAL_VVC,
    CODECHAL_RESERVED2,
    CODECHAL_RESERVED3,
    CODECHAL_RESERVED4,
    CODECHAL_STANDARD_MAX
};

typedef enum _CODECHAL_SCALING_MODE
{
    CODECHAL_SCALING_NEAREST = 0,
    CODECHAL_SCALING_BILINEAR,
    CODECHAL_SCALING_AVS,
    CODECHAL_SCALING_ADV_QUALITY        // !< Advance Perf mode
} CODECHAL_SCALING_MODE;

#endif  // __MEDIA_DEFS_H__

