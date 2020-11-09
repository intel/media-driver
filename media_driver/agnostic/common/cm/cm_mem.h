/*
* Copyright (c) 2017-2020, Intel Corporation
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

void CmFastMemCopy( void* dst, const   void* src, const size_t bytes );
void CmFastMemCopyWC( void* dst,   const void* src, const size_t bytes );

inline void Prefetch( const void* ptr );

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
    uint32_t sizeInDwords = (uint32_t)(bytes >> 2); // divide by 4 byte to dword
    uint32_t *maxPtr = ptr + sizeInDwords;
    while(ptr < maxPtr)
        *ptr++ = data;
}

/*****************************************************************************\
Inline Function:
    CmSafeMemCopy

Description:
    Memory Copy
\*****************************************************************************/
inline void CmSafeMemCopy( void* dst, const void *src, const size_t bytes )
{
  uint8_t *cacheDst = (uint8_t*)dst;
  uint8_t *cacheSrc = (uint8_t*)src;

#if defined(_DEBUG)
    __try
#endif
    {
        MOS_SecureMemcpy( cacheDst, bytes, cacheSrc, bytes );
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
    int cpuInfo[4];
    memset( cpuInfo, 0, 4*sizeof(int) );

    GetCPUID(cpuInfo, 1);

    CPU_INSTRUCTION_LEVEL cpuInstructionLevel = CPU_INSTRUCTION_LEVEL_UNKNOWN;
    if( (cpuInfo[2] & BIT(19)) && TestSSE4_1() )
    {
        cpuInstructionLevel = CPU_INSTRUCTION_LEVEL_SSE4_1;
    }
    else if( cpuInfo[2] & BIT(1) )
    {
        cpuInstructionLevel = CPU_INSTRUCTION_LEVEL_SSE3;
    }
    else if( cpuInfo[3] & BIT(26) )
    {
        cpuInstructionLevel = CPU_INSTRUCTION_LEVEL_SSE2;
    }
    else if( cpuInfo[3] & BIT(25) )
    {
        cpuInstructionLevel = CPU_INSTRUCTION_LEVEL_SSE;
    }
    else if( cpuInfo[3] & BIT(23) )
    {
        cpuInstructionLevel = CPU_INSTRUCTION_LEVEL_MMX;
    }

    return cpuInstructionLevel;
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
