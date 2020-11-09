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
//! \file      cm_mem_sse2_impl.h 
//! \brief     Contains CM memory function definitions 
//!
#pragma once

/*****************************************************************************\
Function:
    FastMemCopy_SSE2_movntdq_movdqa

Description:
    Intel C++ Compiler Memory Copy function using Streaming SIMD Extensions 2

Input:
    dst - 16-byte aligned pointer to destination buffer
    src - 16-byte aligned pointer to source buffer
    doubleQuadWords - number of DoubleQuadWords to copy
\*****************************************************************************/
void FastMemCopy_SSE2_movntdq_movdqa(
    void* dst,
    void* src,
    const size_t doubleQuadWords );

/*****************************************************************************\
Function:
    FastMemCopy_SSE2_movdqu_movdqa

Description:
    Intel C++ Compiler Memory Copy function using Streaming SIMD Extensions 2

Input:
    dst - pointer to destination buffer
    src - 16-byte aligned pointer to source buffer
    doubleQuadWords - number of DoubleQuadWords to copy
\*****************************************************************************/
void FastMemCopy_SSE2_movdqu_movdqa(
    void* dst,
    void* src,
    const size_t doubleQuadWords );

/*****************************************************************************\
Function:
    FastMemCopy_SSE2_movntdq_movdqu

Description:
    Intel C++ Compiler Memory Copy function using Streaming SIMD Extensions 2

Input:
    dst - 16-byte aligned pointer to destination buffer
    src - pointer to source buffer
    doubleQuadWords - number of DoubleQuadWords to copy
\*****************************************************************************/
void FastMemCopy_SSE2_movntdq_movdqu(
    void* dst,
    const void* src,
    const size_t doubleQuadWords );

/*****************************************************************************\
Function:
    FastMemCopy_SSE2_movdqu_movdqu

Description:
    Intel C++ Compiler Memory Copy function using Streaming SIMD Extensions 2

Input:
    dst - pointer to destination buffer
    src - pointer to source buffer
    doubleQuadWords - number of DoubleQuadWords to copy
\*****************************************************************************/
void FastMemCopy_SSE2_movdqu_movdqu(
    void* dst,
    const void* src,
    const size_t doubleQuadWords );

/*****************************************************************************\
Function:
    FastMemCopy_SSE2

Description:
    Intel C++ Compiler Memory Copy function using Streaming SIMD Extensions 2

Input:
    dst - pointer to destination buffer
    src - pointer to source buffer
    doubleQuadWords - number of DoubleQuadWords to copy
\*****************************************************************************/
void FastMemCopy_SSE2(
    void* dst,
    void* src,
    const size_t doubleQuadWords );

/*****************************************************************************\
Function:
    CmFastMemCopy

Description:
    Intel C++ Compiler Memory Copy function for large amounts of data

Input:
    dst - pointer to destination buffer
    src - pointer to source buffer
    bytes - number of bytes to copy
\*****************************************************************************/
void CmFastMemCopy_SSE2( void* dst, const   void* src, const size_t bytes );

/*****************************************************************************\
Function:
CmFastMemCopyWC

Description:
Intel C++ Compiler Memory Copy function for large amounts of data, just now prefetch
compared with FastMemCopyWC. It is the same as the FastMemCopyWC_NoPf in CMRT@APP.

Input:
dst - pointer to write-combined destination buffer
src - pointer to source buffer
bytes - number of bytes to copy
\*****************************************************************************/
void CmFastMemCopyWC_SSE2( void* dst,   const void* src, const size_t bytes );
