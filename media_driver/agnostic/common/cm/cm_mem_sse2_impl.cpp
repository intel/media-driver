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
//! \file      cm_mem_sse2_impl.cpp
//! \brief     Contains CM memory function implementations
//!

#include "cm_mem.h"
#include "cm_mem_sse2_impl.h"

#if defined(__SSE2__) || !(defined(LINUX) || defined(ANDROID))

#include <mmintrin.h>

// GCC>=11 gives a warning when dividing sizeof(array) with sizeof(another_type)
// in case division was mistyped and meant to calculate array size.
// As that's not the case here, use extra divisor parenthesis to suppress
// the warning.
#define DQWORD_PER_PREFETCH(P) ( sizeof(P)/(sizeof(DQWORD)) )

void FastMemCopy_SSE2_movntdq_movdqa(
    void* dst,
    void* src,
    const size_t doubleQuadWords )
{
    CM_ASSERT( IsAligned( dst, sizeof(DQWORD) ) );
    CM_ASSERT( IsAligned( src, sizeof(DQWORD) ) );

    
    const size_t doubleQuadWordsPerPrefetch = DQWORD_PER_PREFETCH(PREFETCH);

    // Prefetch the src data
    Prefetch( (uint8_t*)src );
    Prefetch( (uint8_t*)src + sizeof(PREFETCH) );

    // Convert to SSE2 registers
    __m128i* dst128i = (__m128i*)dst;
    __m128i* src128i = (__m128i*)src;

    size_t count = doubleQuadWords;

    // Copies a cacheline per loop iteration
    while( count >= doubleQuadWordsPerPrefetch )
    {
        Prefetch( (uint8_t*)src128i + 2 * sizeof(PREFETCH) );

        count -= doubleQuadWordsPerPrefetch;

        // Copy cacheline of data
        for( size_t i = 0; i < doubleQuadWordsPerPrefetch; i++ )
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

void FastMemCopy_SSE2_movdqu_movdqa(
    void* dst,
    void* src,
    const size_t doubleQuadWords )
{
    CM_ASSERT( IsAligned( src, sizeof(DQWORD) ) );

    const size_t doubleQuadWordsPerPrefetch = DQWORD_PER_PREFETCH(PREFETCH);

    // Prefetch the src data
    Prefetch( (uint8_t*)src );
    Prefetch( (uint8_t*)src + sizeof(PREFETCH) );

    // Convert to SSE2 registers
    __m128i* dst128i = (__m128i*)dst;
    __m128i* src128i = (__m128i*)src;

    size_t count = doubleQuadWords;

    // Copies a cacheline per loop iteration
    while( count >= doubleQuadWordsPerPrefetch )
    {
        Prefetch( (uint8_t*)src128i + 2 * sizeof(PREFETCH) );

        count -= doubleQuadWordsPerPrefetch;

        // Copy cacheline of data
        for( size_t i = 0; i < doubleQuadWordsPerPrefetch; i++ )
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

void FastMemCopy_SSE2_movntdq_movdqu(
    void* dst,
    const void* src,
    const size_t doubleQuadWords )
{
    CM_ASSERT( IsAligned( dst, sizeof(DQWORD) ) );

    const size_t doubleQuadWordsPerPrefetch = DQWORD_PER_PREFETCH(PREFETCH);

    // Prefetch the src data
    Prefetch( (uint8_t*)src );
    Prefetch( (uint8_t*)src + sizeof(PREFETCH) );

    // Convert to SSE2 registers
    __m128i* dst128i = (__m128i*)dst;
    __m128i* src128i = (__m128i*)src;

    size_t count = doubleQuadWords;

    // Copies a cacheline per loop iteration
    while( count >= doubleQuadWordsPerPrefetch )
    {
        Prefetch( (uint8_t*)src128i + 2 * sizeof(PREFETCH) );

        count -= doubleQuadWordsPerPrefetch;

        // Copy cacheline of data
        for( size_t i = 0; i < doubleQuadWordsPerPrefetch; i++ )
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

void FastMemCopy_SSE2_movdqu_movdqu(
    void* dst,
    const void* src,
    const size_t doubleQuadWords )
{
    const size_t doubleQuadWordsPerPrefetch = DQWORD_PER_PREFETCH(PREFETCH);

    // Prefetch the src data
    Prefetch( (uint8_t*)src );
    Prefetch( (uint8_t*)src + sizeof(PREFETCH) );

    // Convert to SSE2 registers
    __m128i* dst128i = (__m128i*)dst;
    __m128i* src128i = (__m128i*)src;

    size_t count = doubleQuadWords;

    // Copies a cacheline per loop iteration
    while( count >= doubleQuadWordsPerPrefetch )
    {
        Prefetch( (uint8_t*)src128i + 2 * sizeof(PREFETCH) );

        count -= doubleQuadWordsPerPrefetch;

        // Copy cacheline of data
        for( size_t i = 0; i < doubleQuadWordsPerPrefetch; i++ )
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

void FastMemCopy_SSE2(
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

void CmFastMemCopy_SSE2( void* dst, const void* src, const size_t bytes )
{
    // Cache pointers to memory
    uint8_t *cacheDst = (uint8_t*)dst;
    uint8_t *cacheSrc = (uint8_t*)src;

    size_t count = bytes;

    // Get the number of DQWORDs to be copied
    const size_t doubleQuadWords = count / sizeof(DQWORD);

    if( count >= CM_CPU_FASTCOPY_THRESHOLD && doubleQuadWords )
    {
        FastMemCopy_SSE2( cacheDst, cacheSrc, doubleQuadWords );

        cacheDst += doubleQuadWords * sizeof(DQWORD);
        cacheSrc += doubleQuadWords * sizeof(DQWORD);
        count -= doubleQuadWords * sizeof(DQWORD);
    }

    // Copy remaining uint8_t(s)
    if( count )
    {
        MOS_SecureMemcpy( cacheDst, count, cacheSrc, count );
    }
}

void CmFastMemCopyWC_SSE2( void* dst, const void* src, const size_t bytes )
{
  // Cache pointers to memory
  uint8_t *cacheDst = (uint8_t*)dst;
  uint8_t *cacheSrc = (uint8_t*)src;

  size_t count = bytes;

  if( count >= CM_CPU_FASTCOPY_THRESHOLD )
  {
    const size_t doubleQuadwordAlignBytes =
      GetAlignmentOffset( cacheDst, sizeof(DQWORD) );

    // The destination pointer should be 128-bit aligned
    if( doubleQuadwordAlignBytes )
    {
      MOS_SecureMemcpy( cacheDst, doubleQuadwordAlignBytes,cacheSrc, doubleQuadwordAlignBytes );

      cacheDst += doubleQuadwordAlignBytes;
      cacheSrc += doubleQuadwordAlignBytes;
      count -= doubleQuadwordAlignBytes;
    }

    // Get the number of DQWORDs to be copied
    const size_t doubleQuadWords = count / sizeof(DQWORD);

    if( doubleQuadWords && count >= sizeof(PREFETCH))
    {
      // Determine if the source and destination addresses are
      // 128-bit aligned
      CM_ASSERT( IsAligned( cacheDst, sizeof(DQWORD) ) );

      const bool isSrcDoubleQuadWordAligned =
        IsAligned( cacheSrc, sizeof(DQWORD) );

      if( isSrcDoubleQuadWordAligned )
      {
        FastMemCopy_SSE2_movntdq_movdqa( cacheDst, cacheSrc,
          doubleQuadWords );
      }
      else
      {
        FastMemCopy_SSE2_movntdq_movdqu( cacheDst, cacheSrc,
          doubleQuadWords );
      }

      cacheDst += doubleQuadWords * sizeof(DQWORD);
      cacheSrc += doubleQuadWords * sizeof(DQWORD);
      count -= doubleQuadWords * sizeof(DQWORD);
    }
  }

  // Copy remaining uint8_t(s)
  if( count )
  {
    MOS_SecureMemcpy( cacheDst, count, cacheSrc, count );
  }
}

#endif // __SSE2__ || !(LINUX || ANDROID)
