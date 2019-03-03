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

#ifndef CMRTLIB_AGNOSTIC_SHARE_CM_MEM_H_
#define CMRTLIB_AGNOSTIC_SHARE_CM_MEM_H_

#include <cstring>
#include <new>
#include "cm_debug.h"

#define CmSafeDeleteArray(_ptr) {if(_ptr) {delete[] (_ptr); (_ptr)=0;}}
#define CmSafeRelease(_ptr)     {if(_ptr) {delete (_ptr); (_ptr)=0;}}

/*****************************************************************************\
Inline Function:
    CmSafeMemCopy

Description:
    Exception Handler Memory Copy function
\*****************************************************************************/
inline void CmSafeMemCopy( void* dst, const void* src, const size_t bytes )
{
#ifdef _DEBUG
    __try
#endif
    {
        memcpy( dst, src, bytes );
    }
#ifdef _DEBUG
    // catch exceptions here so they are easily debugged
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        CmAssert(0);
    }
#endif
}

/*****************************************************************************\
Inline Function:
    CmSafeMemSet

Description:
    Memory set
\*****************************************************************************/
inline void CmSafeMemSet( void* dst, const int data, const size_t bytes )
{
#ifdef _DEBUG
    __try
#endif
    {
        ::memset( dst, data, bytes );
    }
#ifdef _DEBUG
    // catch exceptions here so they are easily debugged
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        CmAssert(0);
    }
#endif
}

inline void CmDwordMemSet( void* dst, const uint32_t data, const size_t bytes )
{
    uint32_t *ptr = reinterpret_cast<uint32_t*>( dst );
    uint32_t size = (uint32_t)(bytes >> 2); // divide by 4 byte to dword
    uint32_t *maxPtr = ptr + size;
    while(ptr < maxPtr)
        *ptr++ = data;
}

/*****************************************************************************\
Inline Function:
CmSafeMemCompare

Description:
Exception Handler Memory Compare function
\*****************************************************************************/
inline int CmSafeMemCompare(const void* dst, const void* src, const size_t bytes)
{
#ifdef _DEBUG
    __try
#endif
    {
        return ::memcmp(dst, src, bytes);
    }
#ifdef _DEBUG
    // catch exceptions here so they are easily debugged
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        CmAssert(0);
        return 0x7FFFFFFF;  //  An unreasonably large value indicating errors.
    }
#endif
}

#endif  // #ifndef CMRTLIB_AGNOSTIC_SHARE_CM_MEM_H_
