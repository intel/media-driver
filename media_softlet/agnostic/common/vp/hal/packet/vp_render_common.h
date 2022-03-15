/*
* Copyright (c) 2021-2022, Intel Corporation
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
#ifndef __VP_RENDER_COMMON_H__
#define __VP_RENDER_COMMON_H__

#include <stdint.h>

namespace vp
{
#define CM_MAX_KERNEL_NAME_SIZE          256
#define READ_FIELD_FROM_BUF( dst, type ) \
    dst = *((type *) &pBuf[bytePos]); \
    bytePos += sizeof(type);

#define CISA_MAGIC_NUMBER                   0x41534943      //"CISA"
#define GENX_TGLLP_CISAID                   12
#define CM_MAX_ARGS_PER_KERNEL              255   // compiler only supports up to 255 arguments
#define CM_PAYLOAD_OFFSET                   32    // CM Compiler generates offset by 32 bytes. This need to be subtracted from kernel data offset.
#define CM_MAX_THREAD_PAYLOAD_SIZE          2016  // 63 GRF
#define CM_KERNEL_BINARY_PADDING_SIZE       128                                 // Padding after kernel binary to WA page fault issue.
#define SURFACE_MASK                        0x7

struct KERNEL_THREAD_SPACE
{
    uint32_t uWidth;
    uint32_t uHeight;
};

// Need to consistant with compiler
enum KRN_ARG_KIND
{
    // compiler-defined kind
    ARG_KIND_GENERAL = 0x0,
    ARG_KIND_SAMPLER = 0x1,

    ARG_KIND_SURFACE = 0x8, //basic surface value
    // using 3 LSB to classify surface further
    ARG_KIND_SURFACE_2D = 0x9,
    ARG_KIND_SURFACE_1D = 0xa,
    ARG_KIND_SURFACE_SAMPLER8X8_AVS = 0xb,
    ARG_KIND_SURFACE_SAMPLER = 0xc
};

struct KRN_ARG
{
    uint32_t               uIndex;
    uint32_t               uOffsetInPayload; // offset relative to R0 in payload
    void*                  pData;
    uint32_t               uSize;            // size of arg in byte
    KRN_ARG_KIND           eArgKind;
};

using SurfaceIndex = uint32_t;
using SamplerIndex = uint32_t;
using KernelIndex  = uint32_t;              // index of current kernel in KERNEL_PARAMS_LIST

enum KERNEL_SUBMISSION_MODE
{
    MULTI_KERNELS_WITH_MULTI_MEDIA_STATES = 0,
    MULTI_KERNELS_WITH_ONE_MEDIA_STATE
};

enum KERNEL_BINDINGTABLE_MODE
{
    MULTI_KERNELS_WITH_MULTI_BINDINGTABLES = 0,
    MULTI_KERNELS_WITH_ONE_BINDINGTABLE
};

typedef struct _VP_RENDER_CACHE_CNTL
{
    // Input
    bool                        bDnDi;
    bool                        bLace;
    bool                        bCompositing;

    // Output
    VPHAL_DNDI_CACHE_CNTL        DnDi;
    VPHAL_LACE_CACHE_CNTL        Lace;
    VPHAL_COMPOSITE_CACHE_CNTL   Composite;
} VP_RENDER_CACHE_CNTL, *PVP_RENDER_CACHE_CNTL;

}
#endif // !__VP_SFC_COMMON_H__
