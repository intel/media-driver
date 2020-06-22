/*
* Copyright (c) 2007-2017, Intel Corporation
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
//! \file      cm_mem_os.h 
//! \brief     Contains CM memory function definitions 
//!
#pragma once

#include <iostream>
#include "cpuid.h"
#include <smmintrin.h>

typedef uintptr_t           UINT_PTR;
#define __fastcall
#define __noop

#ifdef __try
    #undef __try
#endif
#define __try           try

#ifdef __except
    #undef __except
#endif

#define  EXCEPTION_EXECUTE_HANDLER std::exception const& e

#define __except(e)     catch(e)

#ifndef __EXCEPTIONS
// If -fno-exceptions, transform error handling code to work without it
#define NO_EXCEPTION_HANDLING  1
#endif

#if NO_EXCEPTION_HANDLING || ANDROID
    #define try         if (true)

    #ifndef catch
    #define catch(x)    if (false)
    #endif

    #define throw(...)
#endif

#define CM_CPU_FASTCOPY_THRESHOLD 1024

/*****************************************************************************\
Inline Function:
    Prefetch

Description:
    executes __asm prefetchnta
\*****************************************************************************/
inline void Prefetch( const void* ptr )
{
    __asm__("prefetchnta 0(%0)"::"r"(ptr));
}

inline bool TestSSE4_1( void )
{
    bool success = true;

#ifndef NO_EXCEPTION_HANDLING
    __try
    {
#endif //NO_EXCEPTION_HANDLING

    if ( sizeof(void *) == 4)  //32-bit Linux
    {
        __asm__ __volatile__ (".byte 0x66");
        __asm__ __volatile__ (".byte 0x0f");
        __asm__ __volatile__ (".byte 0x38");
        __asm__ __volatile__ (".byte 0x17");
        __asm__ __volatile__ (".byte 0xc1");
    }
    else //64-bit Linux
    {
        success = true;
    }

#ifndef NO_EXCEPTION_HANDLING
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        success = false;
    }
#endif // NO_EXCEPTION_HANDLING
    return success;
}

/*****************************************************************************\
Inline Function:
    GetCPUID

Description:
    Retrieves cpu information and capabilities supported
Input:
    int infoType - type of information requested
Output:
    int cpuInfo[4] - requested info
\*****************************************************************************/
inline void GetCPUID(int cpuInfo[4], int infoType)
{
#ifndef NO_EXCEPTION_HANDLING
    __try
    {
#endif //NO_EXCEPTION_HANDLING

    __get_cpuid(infoType, (unsigned int*)cpuInfo, (unsigned int*)cpuInfo + 1, (unsigned int*)cpuInfo + 2, (unsigned int*)cpuInfo + 3);

#ifndef NO_EXCEPTION_HANDLING
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        // cpuid failed!
        return;
    }
#endif  //NO_EXCEPTION_HANDLING
}

/*****************************************************************************\
Inline Function:
    CmFastMemCopyFromWC
Description:
    Intel C++ Compiler Memory Copy function for large amounts of data
Input:
    dst - pointer to write-combined destination buffer
    src - pointer to source buffer
    bytes - number of bytes to copy
\*****************************************************************************/
inline void CmFastMemCopyFromWC( void* dst, const void* src, const size_t bytes, CPU_INSTRUCTION_LEVEL cpuInstructionLevel )
{
    if( cpuInstructionLevel >= CPU_INSTRUCTION_LEVEL_SSE4_1 )
    {
        // Cache pointers to memory
        uint8_t *tempDst = (uint8_t*)dst;
        uint8_t *tempSrc = (uint8_t*)src;

        size_t count = bytes;

        if( count >= CM_CPU_FASTCOPY_THRESHOLD )
        {
            //Streaming Load must be 16-byte aligned but should
            //be 64-byte aligned for optimal performance
            const size_t doubleHexWordAlignBytes =
                GetAlignmentOffset( tempSrc, sizeof(DHWORD) );

            // Copy portion of the source memory that is not aligned
            if( doubleHexWordAlignBytes )
            {
                CmSafeMemCopy( tempDst, tempSrc, doubleHexWordAlignBytes );

                tempDst += doubleHexWordAlignBytes;
                tempSrc += doubleHexWordAlignBytes;
                count -= doubleHexWordAlignBytes;
            }

            CM_ASSERT( IsAligned( tempSrc, sizeof(DHWORD) ) == true );

            // Get the number of bytes to be copied (rounded down to nearets DHWORD)
            const size_t doubleHexWordsToCopy = count / sizeof(DHWORD);

            if( doubleHexWordsToCopy )
            {
                // Determine if the destination address is aligned
                const bool isDstDoubleQuadWordAligned =
                    IsAligned( tempDst, sizeof(DQWORD) );

                __m128i* mmSrc = (__m128i*)(tempSrc);
                __m128i* mmDst = reinterpret_cast<__m128i*>(tempDst);
                __m128i  xmm0, xmm1, xmm2, xmm3;

                if( isDstDoubleQuadWordAligned )
                {
                    for( size_t i=0; i<doubleHexWordsToCopy; i++ )
                    {
                        // Sync the WC memory data before issuing the MOVNTDQA instruction.
                        _mm_mfence();
                        xmm0 = _mm_stream_load_si128(mmSrc);
                        xmm1 = _mm_stream_load_si128(mmSrc + 1);
                        xmm2 = _mm_stream_load_si128(mmSrc + 2);
                        xmm3 = _mm_stream_load_si128(mmSrc + 3);
                        mmSrc += 4;

                        _mm_store_si128(mmDst, xmm0);
                        _mm_store_si128(mmDst + 1, xmm1);
                        _mm_store_si128(mmDst + 2, xmm2);
                        _mm_store_si128(mmDst + 3, xmm3);
                        mmDst += 4;

                        tempDst += sizeof(DHWORD);
                        tempSrc += sizeof(DHWORD);
                        count -= sizeof(DHWORD);
                    }
                }
                else
                {
                    for( size_t i=0; i<doubleHexWordsToCopy; i++ )
                    {
                        // Sync the WC memory data before issuing the MOVNTDQA instruction.
                        _mm_mfence();
                        xmm0 = _mm_stream_load_si128(mmSrc);
                        xmm1 = _mm_stream_load_si128(mmSrc + 1);
                        xmm2 = _mm_stream_load_si128(mmSrc + 2);
                        xmm3 = _mm_stream_load_si128(mmSrc + 3);
                        mmSrc += 4;

                        _mm_storeu_si128(mmDst, xmm0);
                        _mm_storeu_si128(mmDst + 1, xmm1);
                        _mm_storeu_si128(mmDst + 2, xmm2);
                        _mm_storeu_si128(mmDst + 3, xmm3);
                        mmDst += 4;
                        tempDst += sizeof(DHWORD);
                        tempSrc += sizeof(DHWORD);
                        count -= sizeof(DHWORD);
                    }
                }
            }
        }

        // Copy remaining uint8_t(s)
        if( count )
        {
            CmSafeMemCopy( tempDst, tempSrc, count );
        }
    }
    else
    {
        CmFastMemCopy( dst, src, bytes );
    }
}

