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
//! \file     codec_def_common_avc.h
//! \brief    Defines basic AVC types and macros shared by CodecHal, MHW, and DDI layer
//! \details  This is the base header for all codec_def AVC files. All codec_def AVC files should include this file which should not contain any DDI specific code.
//!
#ifndef __CODEC_DEF_COMMON_AVC_H__
#define __CODEC_DEF_COMMON_AVC_H__

#include "codec_def_common.h"

#define CODEC_AVC_MAX_NUM_REF_FRAME         16
#define CODEC_AVC_NUM_REF_LISTS             2
#define CODEC_AVC_NUM_REF_DMV_BUFFERS       (CODEC_AVC_MAX_NUM_REF_FRAME + 1) // Max 16 references + 1 for the current frame
#define CODEC_AVC_NUM_DMV_BUFFERS           (CODEC_AVC_NUM_REF_DMV_BUFFERS + 1)  // 1 for non-reference

#define CODEC_MAX_NUM_REF_FIELD             (CODEC_MAX_NUM_REF_FRAME * CODEC_NUM_FIELDS_PER_FRAME)

enum CODEC_AVC_WEIGHT_SCALE_SIZE
{
    CODEC_AVC_WEIGHT_SCALE_4x4 = 24,
    CODEC_AVC_WEIGHT_SCALE_8x8 = 32,
    CODEC_AVC_WEIGHT_SCALE     = 56
};

#endif  // __CODEC_DEF_COMMON_AVC_H__
