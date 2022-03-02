/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     media_ddi_encode_const.h
//! \brief    Add some const string definition for media_libva_encoder
//!

// The defined const string is used as the Key for supported encoding codec list. And
// it is included by each encoding codec.
// And it is also included in media_libva_ecoder.c. The corresponding string is used
// as the key to search and create one instance from the supported encoding list.

#ifndef _MEDIA_LIBVA_ENCODE_CONST_H_
#define _MEDIA_LIBVA_ENCODE_CONST_H_

#define ENCODE_ID_NONE      "VIDEO_ENCODE_NONE"
#define ENCODE_ID_AVC       "VIDEO_ENCODE_AVC"
#define ENCODE_ID_AVCFEI    "VIDEO_ENCODE_AVCFEI"
#define ENCODE_ID_HEVC      "VIDEO_ENCODE_HEVC"
#define ENCODE_ID_HEVCFEI   "VIDEO_ENCODE_HEVCFEI"
#define ENCODE_ID_MPEG2     "VIDEO_ENCODE_MPEG2"
#define ENCODE_ID_JPEG      "VIDEO_ENCODE_JPEG"
#define ENCODE_ID_VP8       "VIDEO_ENCODE_VP8"
#define ENCODE_ID_VP9       "VIDEO_ENCODE_VP9"
#define ENCODE_ID_AV1       "VIDEO_ENCODE_AV1"

#endif /*  _MEDIA_LIBVA_ENCODE_CONST_H_ */
