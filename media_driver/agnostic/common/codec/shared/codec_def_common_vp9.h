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
//! \file     codec_def_common_vp9.h
//! \brief    Defines decode VP9 types and macros shared by CodecHal, MHW, and DDI layer
//! \details  Applies to VP9 codec  only. Should not contain any DDI specific code.
//!
#ifndef __CODEC_DEF_COMMON_VP9_H__
#define __CODEC_DEF_COMMON_VP9_H__

#include "mos_os.h"

#define CODECHAL_VP9_SUPER_BLOCK_WIDTH          64
#define CODECHAL_VP9_SUPER_BLOCK_HEIGHT         64
#define CODECHAL_VP9_MIN_BLOCK_WIDTH            8
#define CODECHAL_VP9_MIN_BLOCK_HEIGHT           8

typedef struct _CODEC_VP9_SEG_PARAMS
{
    union
    {
        struct
        {
            uint16_t      SegmentReferenceEnabled         : 1;        // [0..1]
            uint16_t      SegmentReference                : 2;        // [0..3]
            uint16_t      SegmentReferenceSkipped         : 1;        // [0..1]
            uint16_t      ReservedField3                  : 12;       // [0]
        } fields;
        uint32_t            value;
    } SegmentFlags;

    uint8_t               FilterLevel[4][2];                          // [0..63]
    uint16_t              LumaACQuantScale;                           //
    uint16_t              LumaDCQuantScale;                           //
    uint16_t              ChromaACQuantScale;                         //
    uint16_t              ChromaDCQuantScale;                         //
} CODEC_VP9_SEG_PARAMS, *PCODEC_VP9_SEG_PARAMS;

#endif

