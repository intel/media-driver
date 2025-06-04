/*
* Copyright (c) 2020, Intel Corporation
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
//! \file      cm_mem.cpp
//! \brief     Contains CM memory function implementations 
//!

#include "cm_mem.h"
#include "cm_mem_c_impl.h"

#if !defined(__loongarch64)
#include "cm_mem_sse2_impl.h"
#endif

typedef void(*t_CmFastMemCopy)( void* dst, const   void* src, const size_t bytes );
typedef void(*t_CmFastMemCopyWC)( void* dst,   const void* src, const size_t bytes );

#if !defined(__loongarch64)
#define CM_FAST_MEM_COPY_CPU_INIT_C(func)       (func ## _C)
#define CM_FAST_MEM_COPY_CPU_INIT_SSE2(func)    (func ## _SSE2)
#define CM_FAST_MEM_COPY_CPU_INIT(func)         (is_SSE2_available ? CM_FAST_MEM_COPY_CPU_INIT_SSE2(func) : CM_FAST_MEM_COPY_CPU_INIT_C(func))
#endif

void CmFastMemCopy( void* dst, const void* src, const size_t bytes )
{
#if defined(__loongarch64)
    CmFastMemCopy_C(dst, src, bytes);
#else
    static const bool is_SSE2_available = (GetCpuInstructionLevel() >= CPU_INSTRUCTION_LEVEL_SSE2);
    static const t_CmFastMemCopy CmFastMemCopy_impl = CM_FAST_MEM_COPY_CPU_INIT(CmFastMemCopy);

    CmFastMemCopy_impl(dst, src, bytes);
#endif
}

void CmFastMemCopyWC( void* dst, const void* src, const size_t bytes )
{
#if defined(__loongarch64)
    CmFastMemCopyWC_C(dst, src, bytes);
#else
    static const bool is_SSE2_available = (GetCpuInstructionLevel() >= CPU_INSTRUCTION_LEVEL_SSE2);
    static const t_CmFastMemCopyWC CmFastMemCopyWC_impl = CM_FAST_MEM_COPY_CPU_INIT(CmFastMemCopyWC);

    CmFastMemCopyWC_impl(dst, src, bytes);
#endif
}
