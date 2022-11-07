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
//! \file     media_libvpx_vp9_next.h
//! \brief    Defines the VP9 structure/function that is from libvpx
//!

/*
 * This file declares some vp9 function, and
 * they are ported from libvpx (https://github.com/webmproject/libvpx/).
 * The original copyright and licence statement as below.
 */

/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the media_libvpx.LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file media_libvpx.PATENTS.  All contributing project authors may
 *  be found in the media_libvpx.AUTHORS file in the root of the source tree.
 */

#ifndef _MEDIA_LIBVPX_VP9_NEXT_H
#define _MEDIA_LIBVPX_VP9_NEXT_H

#include <stdbool.h>
#include <stdint.h>
#include "ddi_libva_encoder_specific.h"

#define VP9_PROFILE_0 0
#define VP9_PROFILE_1 1
#define VP9_PROFILE_2 2
#define VP9_PROFILE_3 3

typedef struct _vp9_header_bitoffset_ {
    uint32_t    bit_offset_ref_lf_delta;
    uint32_t    bit_offset_mode_lf_delta;
    uint32_t    bit_offset_lf_level;
    uint32_t    bit_offset_qindex;
    uint32_t    bit_offset_first_partition_size;
    uint32_t    bit_offset_segmentation;
    uint32_t    bit_size_segmentation;
} vp9_header_bitoffset;

bool Vp9WriteUncompressHeader(encode::DDI_ENCODE_CONTEXT *ddiContext,
                                       uint32_t codecProfile,
                                       uint8_t  *headerData,
                                       uint32_t *headerLen,
                                       vp9_header_bitoffset *headerBitoffset);

#endif /*_MEDIA_LIBVPX_VP9_NEXT_H */
