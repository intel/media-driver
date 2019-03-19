/*
* Copyright (c) 2012-2019, Intel Corporation
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
//! \file     mos_sw_swizzle.h
//! \brief    Mesa ISL (Intel Surface Layout) API for sw-based tiling/detiling
//! \details  Mesa ISL (Intel Surface Layout) API for sw-based tiling/detiling
//!
#ifndef __MOS_SW_SWIZZLE_H__
#define __MOS_SW_SWIZZLE_H__

#include <stdint.h>
#include "mos_defs.h"
#include "mos_util_debug.h"

    enum mos_memcpy_type
{
  MOS_MEMCPY = 0,
  MOS_MEMCPY_STREAMING_LOAD,
  MOS_MEMCPY_INVALID,
};

#ifdef __cplusplus
extern "C" {
#endif

void
mos_memcpy_tiled_to_linear(uint32_t xt1, uint32_t xt2,
                           uint32_t yt1, uint32_t yt2,
                           char *dst, const char *src,
                           int32_t dst_pitch, uint32_t src_pitch,
                           bool has_swizzling,
                           uint32_t tiling,
                           mos_memcpy_type copy_type);
void
mos_memcpy_linear_to_tiled(uint32_t xt1, uint32_t xt2,
                           uint32_t yt1, uint32_t yt2,
                           char *dst, const char *src,
                           uint32_t dst_pitch, int32_t src_pitch,
                           bool has_swizzling,
                           uint32_t tiling,
                           mos_memcpy_type copy_type);
#ifdef __cplusplus
}
#endif

#endif // __MOS_SW_SWIZZLE_H__
