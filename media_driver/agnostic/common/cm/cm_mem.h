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
//! \file      cm_mem.h 
//! \brief     Contains CM memory function definitions 
//!
#pragma once

#include <mmintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include "cm_debug.h"
#include "mos_utilities.h"

enum CPU_INSTRUCTION_LEVEL
{
    CPU_INSTRUCTION_LEVEL_UNKNOWN,
    CPU_INSTRUCTION_LEVEL_MMX,
    CPU_INSTRUCTION_LEVEL_SSE,
    CPU_INSTRUCTION_LEVEL_SSE2,
    CPU_INSTRUCTION_LEVEL_SSE3,
    CPU_INSTRUCTION_LEVEL_SSE4,
    CPU_INSTRUCTION_LEVEL_SSE4_1,
    NUM_CPU_INSTRUCTION_LEVELS
};

typedef __m128              DQWORD;         // 128-bits,   16-bytes
typedef uint32_t            PREFETCH[8];    //             32-bytes
typedef uint32_t            CACHELINE[8];   //             32-bytes
typedef uint16_t            DHWORD[32];     // 512-bits,   64-bytes

#define CmSafeDeleteArray(_ptr) {if(_ptr != nullptr) {delete[] (_ptr); (_ptr)=nullptr;}}
#define CmSafeDelete(_ptr)      {if(_ptr != nullptr) {delete (_ptr);(_ptr)=nullptr;}}
#define MosSafeDeleteArray(_ptr) {MOS_DeleteArray(_ptr); (_ptr)=nullptr;}
#define MosSafeDelete(_ptr)      {MOS_Delete(_ptr); (_ptr)=nullptr;}
inline void CmSafeMemSet( void* dst, const int data, const size_t bytes );
inline void CmDwordMemSet( void* dst, const uint32_t data, const size_t bytes );
inline void CmSafeMemCopy( void* pdst, const void *psrc, const size_t bytes );
inline int  CmSafeMemCompare( const void*, const void*, const size_t );

inline bool IsPowerOfTwo( const size_t number );
inline void* Align( void* const ptr, const size_t alignment );
inline size_t GetAlignmentOffset( void* const ptr, const size_t alignSize );
inline bool IsAligned( void * ptr, const size_t alignSize );
inline size_t Round( const size_t value, const size_t size );

inline void CmFastMemCopy( void* dst, const   void* src, const size_t bytes );
inline void CmFastMemCopyWC( void* dst,   const void* src, const size_t bytes );
inline void CmFastMemCopyFromWC( void* dst, const void* src, const size_t bytes, CPU_INSTRUCTION_LEVEL cpuInstructionLevel );

inline void Prefetch( const void* ptr );
inline void FastMemCopy_SSE2( void* dst,  void* src, const size_t doubleQuadWords );

inline void FastMemCopy_SSE2_movntdq_movdqa( void* dst,  void* src,  const size_t doubleQuadWords );
inline void FastMemCopy_SSE2_movdqu_movdqa( void* dst,   void* src,  const size_t doubleQuadWords );
inline void FastMemCopy_SSE2_movntdq_movdqu( void* dst, const void* src, const size_t doubleQuadWords);

/*****************************************************************************\
MACROS:
    EMIT_R_MR
    Example:  movntdqa xmm1, xmmword ptr [eax]

    EMIT_R_MR_OFFSET
    Example: movntdqa xmm1, xmmword ptr [eax + 0x10]

Description:
    Used to encode SSE4.1 instructions with parametrs
\*****************************************************************************/
#define EMIT_R_MR(OPCODE, X, Y )   \
    OPCODE                         \
    __asm _emit (0x00 + X*8 + Y)

#define EMIT_R_MR_OFFSET(OPCODE, X, Y, OFFSET)  \
    OPCODE                                      \
    __asm _emit (0x80 + X*8 + Y)                \
    __asm _emit (OFFSET&0xFF)                   \
    __asm _emit ((OFFSET>>8)&0xFF)              \
    __asm _emit ((OFFSET>>16)&0xFF)             \
    __asm _emit ((OFFSET>>24)&0xFF)

/*****************************************************************************\
MACROS:
    MOVNTDQA_OP
    MOVNTDQA_R_MR
    MOVNTDQA_R_MRB

Description:
    Used to emit SSE4_1 movntdqa (streaming load) instructions
        SRC - XMM Register, destination data is to be stored
        DST - General Purpose Register containing source address
        OFFSET - Offset to be added to the source address
\*****************************************************************************/
#define MOVNTDQA_OP     \
    _asm _emit 0x66     \
    _asm _emit 0x0F     \
    _asm _emit 0x38     \
    _asm _emit 0x2A

#define MOVNTDQA_R_MR(DST, SRC)                 \
    EMIT_R_MR(MOVNTDQA_OP, DST, SRC)

#define MOVNTDQA_R_MR_OFFSET(DST, SRC, OFFSET)  \
    EMIT_R_MR_OFFSET(MOVNTDQA_OP, DST, SRC, OFFSET)

#ifndef BIT
#define BIT( n )    ( 1 << (n) )
#endif

#include "cm_mem_os.h"

/*****************************************************************************\
Inline Function:
    CmSafeMemSet

Description:
    Memory set
\*****************************************************************************/
inline void CmSafeMemSet( void* dst, const int data, const size_t bytes )
{
#if defined(_DEBUG)
    __try
#endif
    {
        ::memset( dst, data, bytes );
    }
#if defined(_DEBUG)
    // catch exceptions here so they are easily debugged
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        CM_ASSERTMESSAGE("Error:  Memory Set failure.");
    }
#endif
}

/*****************************************************************************\
Inline Function:
    CmDwordMemSet

Description:
    uint32_t memory set
\*****************************************************************************/
inline void CmDwordMemSet( void* dst, const uint32_t data, const size_t bytes )
{
    uint32_t *ptr = reinterpret_cast<uint32_t*>( dst );
    uint32_t size_in_dwords = (uint32_t)(bytes >> 2); // divide by 4 byte to dword
    uint32_t *maxPtr = ptr + size_in_dwords;
    while(ptr < maxPtr)
        *ptr++ = data;
}

/*****************************************************************************\
Inline Function:
    CmSafeMemCopy

Description:
    Memory Copy
\*****************************************************************************/
inline void CmSafeMemCopy( void* pdst, const void *psrc, const size_t bytes )
{
  uint8_t *p_dst = (uint8_t*)pdst;
  uint8_t *p_src = (uint8_t*)psrc;

#if defined(_DEBUG)
    __try
#endif
    {
        MOS_SecureMemcpy( p_dst, bytes, p_src, bytes );
    }
#if defined(_DEBUG)
    // catch exceptions here so they are easily debugged
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        CM_ASSERTMESSAGE("Error:  Memory Copy failure.");
    }
#endif
}

/*****************************************************************************\
Inline Function:
    CmSafeMemCompare

Description:
    Exception Handler Memory Compare function
\*****************************************************************************/
inline int CmSafeMemCompare( const void* dst, const void* src, const size_t bytes )
{
#if defined(_DEBUG)
    __try
#endif
    {
        return ::memcmp( dst, src, bytes );
    }
#if defined(_DEBUG)
    // catch exceptions here so they are easily debugged
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        CM_ASSERTMESSAGE("Error:  Memory Compare failure.");
        return -1;
    }
#endif
}

/*****************************************************************************\
Inline Function:
    GetCpuInstructionLevel

Description:
    Returns the highest level of IA32 intruction extensions supported by the CPU
    ( i.e. SSE, SSE2, SSE4, etc )

Output:
    CPU_INSTRUCTION_LEVEL - highest level of IA32 instruction extension(s) supported
    by CPU
\*****************************************************************************/
inline CPU_INSTRUCTION_LEVEL GetCpuInstructionLevel( void )
{
    int CpuInfo[4];
    memset( CpuInfo, 0, 4*sizeof(int) );

    GetCPUID(CpuInfo, 1);

    CPU_INSTRUCTION_LEVEL CpuInstructionLevel = CPU_INSTRUCTION_LEVEL_UNKNOWN;
    if( (CpuInfo[2] & BIT(19)) && TestSSE4_1() )
    {
        CpuInstructionLevel = CPU_INSTRUCTION_LEVEL_SSE4_1;
    }
    else if( CpuInfo[2] & BIT(1) )
    {
        CpuInstructionLevel = CPU_INSTRUCTION_LEVEL_SSE3;
    }
    else if( CpuInfo[3] & BIT(26) )
    {
        CpuInstructionLevel = CPU_INSTRUCTION_LEVEL_SSE2;
    }
    else if( CpuInfo[3] & BIT(25) )
    {
        CpuInstructionLevel = CPU_INSTRUCTION_LEVEL_SSE;
    }
    else if( CpuInfo[3] & BIT(23) )
    {
        CpuInstructionLevel = CPU_INSTRUCTION_LEVEL_MMX;
    }

    return CpuInstructionLevel;
}

/*****************************************************************************\
Inline Function:
    Round

Description:
    Rounds an unsigned integer to the next multiple of (power-2) size
\*****************************************************************************/
inline size_t Round( const size_t value, const size_t size )
{

    CM_ASSERT( IsPowerOfTwo(size) );
    size_t mask = size - 1;
    size_t roundedValue = ( value + mask ) & ~( mask );
    return roundedValue;

}

/*****************************************************************************\
Inline Template Function:
    Max

Description:
    Returns the max of the two values
\*****************************************************************************/
__inline size_t Max( size_t var0, size_t var1 )
{
    return ( var0 >= var1 ) ? var0 : var1;
}
/*****************************************************************************\
Inline Function:
    IsAligned

Description:
    Determines if the given size is aligned to the given size
\*****************************************************************************/
inline bool IsAligned( void * ptr, const size_t alignSize )
{
    return ( ( (size_t)ptr % alignSize ) == 0 );
}

/*****************************************************************************\
Inline Function:
    IsPowerOfTwo

Description:
    Determines if the given value is a power of two.
\*****************************************************************************/
inline bool IsPowerOfTwo( const size_t number )
{
    return ( ( number & ( number - 1 ) ) == 0 );
}

/*****************************************************************************\
Inline Function:
    GetAlignmentOffset

Description:
    Returns the size in bytes needed to align the given size to the
    given alignment size
\*****************************************************************************/
inline size_t GetAlignmentOffset( void* const ptr, const size_t alignSize )
{
    CM_ASSERT( alignSize );

    uint32_t offset = 0;

    if( IsPowerOfTwo(alignSize) )
    {   // can recast 'ptr' to uint32_t, since offset is uint32_t
        offset = uint32_t( uintptr_t( Align(ptr, alignSize) ) - (uintptr_t)(ptr) );
    }
    else
    {
        const uint32_t modulo = (uint32_t)(uintptr_t(ptr) % alignSize);

        if( modulo )
        {
            offset = (uint32_t)alignSize - modulo;
        }
    }

    return offset;
}

/*****************************************************************************\
Inline Function:
    Align

Description:
    Type-safe (power-2) alignment of a pointer.
\*****************************************************************************/
inline void* Align( void* const ptr, const size_t alignment )
{
    CM_ASSERT( IsPowerOfTwo(alignment) );

    return (void*)( ( ((size_t)ptr) + alignment-1 ) & ~( alignment-1 ) );
}

/*****************************************************************************\
Inline Function:
    FastMemCopy_SSE2_movntdq_movdqa

Description:
    Intel C++ Compiler Memory Copy function using Streaming SIMD Extensions 2

Input:
    dst - 16-byte aligned pointer to destination buffer
    src - 16-byte aligned pointer to source buffer
    doubleQuadWords - number of DoubleQuadWords to copy
\*****************************************************************************/
inline void FastMemCopy_SSE2_movntdq_movdqa(
    void* dst,
    void* src,
    const size_t doubleQuadWords )
{
    CM_ASSERT( IsAligned( dst, sizeof(DQWORD) ) );
    CM_ASSERT( IsAligned( src, sizeof(DQWORD) ) );

    const size_t DoubleQuadWordsPerPrefetch = sizeof(PREFETCH) / sizeof(DQWORD);

    // Prefetch the src data
    Prefetch( (uint8_t*)src );
    Prefetch( (uint8_t*)src + sizeof(PREFETCH) );

    // Convert to SSE2 registers
    __m128i* dst128i = (__m128i*)dst;
    __m128i* src128i = (__m128i*)src;

    size_t count = doubleQuadWords;

    // Copies a cacheline per loop iteration
    while( count >= DoubleQuadWordsPerPrefetch )
    {
        Prefetch( (uint8_t*)src128i + 2 * sizeof(PREFETCH) );

        count -= DoubleQuadWordsPerPrefetch;

        // Copy cacheline of data
        for( size_t i = 0; i < DoubleQuadWordsPerPrefetch; i++ )
        {
            _mm_stream_si128( dst128i++,
                _mm_load_si128( src128i++ ) );
        }
    }

    // Copy DQWORD if not cacheline multiple
    while( count-- )
    {
        _mm_stream_si128( dst128i++,
            _mm_load_si128( src128i++ ) );
    }
}

/*****************************************************************************\
Inline Function:
    FastMemCopy_SSE2_movdqu_movdqa

Description:
    Intel C++ Compiler Memory Copy function using Streaming SIMD Extensions 2

Input:
    dst - pointer to destination buffer
    src - 16-byte aligned pointer to source buffer
    doubleQuadWords - number of DoubleQuadWords to copy
\*****************************************************************************/
inline void FastMemCopy_SSE2_movdqu_movdqa(
    void* dst,
    void* src,
    const size_t doubleQuadWords )
{
    CM_ASSERT( IsAligned( src, sizeof(DQWORD) ) );

    const size_t DoubleQuadWordsPerPrefetch = sizeof(PREFETCH) / sizeof(DQWORD);

    // Prefetch the src data
    Prefetch( (uint8_t*)src );
    Prefetch( (uint8_t*)src + sizeof(PREFETCH) );

    // Convert to SSE2 registers
    __m128i* dst128i = (__m128i*)dst;
    __m128i* src128i = (__m128i*)src;

    size_t count = doubleQuadWords;

    // Copies a cacheline per loop iteration
    while( count >= DoubleQuadWordsPerPrefetch )
    {
        Prefetch( (uint8_t*)src128i + 2 * sizeof(PREFETCH) );

        count -= DoubleQuadWordsPerPrefetch;

        // Copy cacheline of data
        for( size_t i = 0; i < DoubleQuadWordsPerPrefetch; i++ )
        {
            _mm_storeu_si128( dst128i++,
                _mm_load_si128( src128i++ ) );
        }
    }

    // Copy DQWORD if not cacheline multiple
    while( count-- )
    {
        _mm_storeu_si128( dst128i++,
            _mm_load_si128( src128i++ ) );
    }
}

/*****************************************************************************\
Inline Function:
    FastMemCopy_SSE2_movntdq_movdqu

Description:
    Intel C++ Compiler Memory Copy function using Streaming SIMD Extensions 2

Input:
    dst - 16-byte aligned pointer to destination buffer
    src - pointer to source buffer
    doubleQuadWords - number of DoubleQuadWords to copy
\*****************************************************************************/
inline void FastMemCopy_SSE2_movntdq_movdqu(
    void* dst,
    const void* src,
    const size_t doubleQuadWords )
{
    CM_ASSERT( IsAligned( dst, sizeof(DQWORD) ) );

    const size_t DoubleQuadWordsPerPrefetch = sizeof(PREFETCH) / sizeof(DQWORD);

    // Prefetch the src data
    Prefetch( (uint8_t*)src );
    Prefetch( (uint8_t*)src + sizeof(PREFETCH) );

    // Convert to SSE2 registers
    __m128i* dst128i = (__m128i*)dst;
    __m128i* src128i = (__m128i*)src;

    size_t count = doubleQuadWords;

    // Copies a cacheline per loop iteration
    while( count >= DoubleQuadWordsPerPrefetch )
    {
        Prefetch( (uint8_t*)src128i + 2 * sizeof(PREFETCH) );

        count -= DoubleQuadWordsPerPrefetch;

        // Copy cacheline of data
        for( size_t i = 0; i < DoubleQuadWordsPerPrefetch; i++ )
        {
            _mm_stream_si128( dst128i++,
                _mm_loadu_si128( src128i++ ) );
        }
    }

    // Copy DQWORD if not cacheline multiple
    while( count-- )
    {
        _mm_stream_si128( dst128i++,
            _mm_loadu_si128( src128i++ ) );
    }
}

/*****************************************************************************\
Inline Function:
    FastMemCopy_SSE2_movdqu_movdqu

Description:
    Intel C++ Compiler Memory Copy function using Streaming SIMD Extensions 2

Input:
    dst - pointer to destination buffer
    src - pointer to source buffer
    doubleQuadWords - number of DoubleQuadWords to copy
\*****************************************************************************/
inline void FastMemCopy_SSE2_movdqu_movdqu(
    void* dst,
    const void* src,
    const size_t doubleQuadWords )
{
    const size_t DoubleQuadWordsPerPrefetch = sizeof(PREFETCH) / sizeof(DQWORD);

    // Prefetch the src data
    Prefetch( (uint8_t*)src );
    Prefetch( (uint8_t*)src + sizeof(PREFETCH) );

    // Convert to SSE2 registers
    __m128i* dst128i = (__m128i*)dst;
    __m128i* src128i = (__m128i*)src;

    size_t count = doubleQuadWords;

    // Copies a cacheline per loop iteration
    while( count >= DoubleQuadWordsPerPrefetch )
    {
        Prefetch( (uint8_t*)src128i + 2 * sizeof(PREFETCH) );

        count -= DoubleQuadWordsPerPrefetch;

        // Copy cacheline of data
        for( size_t i = 0; i < DoubleQuadWordsPerPrefetch; i++ )
        {
            _mm_storeu_si128( dst128i++,
                _mm_loadu_si128( src128i++ ) );
        }
    }

    // Copy DQWORD if not cacheline multiple
    while( count-- )
    {
        _mm_storeu_si128( dst128i++,
            _mm_loadu_si128( src128i++ ) );
    }
}

/*****************************************************************************\
Inline Function:
    FastMemCopy_SSE2

Description:
    Intel C++ Compiler Memory Copy function using Streaming SIMD Extensions 2

Input:
    dst - pointer to destination buffer
    src - pointer to source buffer
    doubleQuadWords - number of DoubleQuadWords to copy
\*****************************************************************************/
inline void FastMemCopy_SSE2(
    void* dst,
    void* src,
    const size_t doubleQuadWords )
{
    // Determine if the source and destination addresses are 128-bit aligned
    const bool isDstDoubleQuadWordAligned = IsAligned( dst, sizeof(DQWORD) );
    const bool isSrcDoubleQuadWordAligned = IsAligned( src, sizeof(DQWORD) );

    if( isSrcDoubleQuadWordAligned && isDstDoubleQuadWordAligned )
    {
        FastMemCopy_SSE2_movntdq_movdqa( dst, src, doubleQuadWords );
    }
    else if( isDstDoubleQuadWordAligned )
    {
        FastMemCopy_SSE2_movntdq_movdqu( dst, src, doubleQuadWords );
    }
    else if( isSrcDoubleQuadWordAligned )
    {
        FastMemCopy_SSE2_movdqu_movdqa( dst, src, doubleQuadWords );
    }
    else // if( !isSrcDoubleQuadWordAligned && !isDstDoubleQuadWordAligned )
    {
        FastMemCopy_SSE2_movdqu_movdqu( dst, src, doubleQuadWords );
    }
}

/*****************************************************************************\
Inline Function:
    CmFastMemCopy

Description:
    Intel C++ Compiler Memory Copy function for large amounts of data

Input:
    dst - pointer to destination buffer
    src - pointer to source buffer
    bytes - number of bytes to copy
\*****************************************************************************/
inline void CmFastMemCopy( void* dst, const   void* src, const size_t bytes )
{

    // Cache pointers to memory
    uint8_t *p_dst = (uint8_t*)dst;
    uint8_t *p_src = (uint8_t*)src;

    size_t count = bytes;

    // Get the number of DQWORDs to be copied
    const size_t doubleQuadWords = count / sizeof(DQWORD);

    if( doubleQuadWords )
    {
        FastMemCopy_SSE2( p_dst, p_src, doubleQuadWords );

        p_dst += doubleQuadWords * sizeof(DQWORD);
        p_src += doubleQuadWords * sizeof(DQWORD);
        count -= doubleQuadWords * sizeof(DQWORD);
    }

    // Copy remaining uint8_t(s)
    if( count )
    {
        MOS_SecureMemcpy( p_dst, count, p_src, count );
    }
}

/*****************************************************************************\
Inline Function:
CmFastMemCopyWC

Description:
Intel C++ Compiler Memory Copy function for large amounts of data, just now prefetch
compared with FastMemCopyWC. It is the same as the FastMemCopyWC_NoPf in CMRT@APP.

Input:
dst - pointer to write-combined destination buffer
src - pointer to source buffer
bytes - number of bytes to copy
\*****************************************************************************/
inline void CmFastMemCopyWC( void* dst,   const void* src, const size_t bytes )
{
  // Cache pointers to memory
  uint8_t *p_dst = (uint8_t*)dst;
  uint8_t *p_src = (uint8_t*)src;

  size_t count = bytes;

  if( count >= sizeof(DQWORD) )
  {
    const size_t doubleQuadwordAlignBytes =
      GetAlignmentOffset( p_dst, sizeof(DQWORD) );

    // The destination pointer should be 128-bit aligned
    if( doubleQuadwordAlignBytes )
    {
      MOS_SecureMemcpy( p_dst, doubleQuadwordAlignBytes,p_src, doubleQuadwordAlignBytes );

      p_dst += doubleQuadwordAlignBytes;
      p_src += doubleQuadwordAlignBytes;
      count -= doubleQuadwordAlignBytes;
    }

    // Get the number of DQWORDs to be copied
    const size_t doubleQuadWords = count / sizeof(DQWORD);

    if( doubleQuadWords )
    {
      // Determine if the source and destination addresses are
      // 128-bit aligned
      CM_ASSERT( IsAligned( p_dst, sizeof(DQWORD) ) );

      const bool isSrcDoubleQuadWordAligned =
        IsAligned( p_src, sizeof(DQWORD) );

      if( isSrcDoubleQuadWordAligned )
      {
        FastMemCopy_SSE2_movntdq_movdqa( p_dst, p_src,
          doubleQuadWords );
      }
      else
      {
        FastMemCopy_SSE2_movntdq_movdqu( p_dst, p_src,
          doubleQuadWords );
      }

      p_dst += doubleQuadWords * sizeof(DQWORD);
      p_src += doubleQuadWords * sizeof(DQWORD);
      count -= doubleQuadWords * sizeof(DQWORD);
    }
  }

  // Copy remaining uint8_t(s)
  if( count )
  {
    MOS_SecureMemcpy( p_dst, count, p_src, count );
  }
}

