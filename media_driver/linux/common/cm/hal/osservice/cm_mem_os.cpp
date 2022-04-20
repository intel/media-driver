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
//! \file      cm_mem_os.cpp
//! \brief     Contains CM memory function implementations 
//!

#include "cm_mem.h"
#include "cm_mem_os.h"
#include "cm_mem_os_c_impl.h"
#include "cm_mem_os_sse4_impl.h"

typedef void(*t_CmFastMemCopyFromWC)( void* dst, const void* src, const size_t bytes );

#define CM_FAST_MEM_COPY_CPU_INIT_C(func)       (func ## _C)
#define CM_FAST_MEM_COPY_CPU_INIT_SSE4(func)    (func ## _SSE4)
#define CM_FAST_MEM_COPY_CPU_INIT(func)         (is_SSE4_available ? CM_FAST_MEM_COPY_CPU_INIT_SSE4(func) : CM_FAST_MEM_COPY_CPU_INIT_C(func))

void CmFastMemCopyFromWC( void* dst, const void* src, const size_t bytes, CPU_INSTRUCTION_LEVEL cpuInstructionLevel )
{
    static const bool is_SSE4_available = (cpuInstructionLevel >= CPU_INSTRUCTION_LEVEL_SSE4_1);
    static const t_CmFastMemCopyFromWC CmFastMemCopyFromWC_impl = CM_FAST_MEM_COPY_CPU_INIT(CmFastMemCopyFromWC);

    CmFastMemCopyFromWC_impl(dst, src, bytes);
}
