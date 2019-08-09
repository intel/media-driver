/*
* Copyright (c) 2013-2017, Intel Corporation
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
//! \file     mos_defs_specific.h
//! \brief    Defines common types and macros across different platform on Linux
//! \details  Defines common types and macros across different platform on Linux
//!
#ifndef __MOS_DEFS_SPECIFIC_H__
#define __MOS_DEFS_SPECIFIC_H__

#include <pthread.h>
#include <semaphore.h>

typedef pthread_mutex_t         MOS_MUTEX, *PMOS_MUTEX, MosMutex;         //!< mutex pointer
typedef sem_t                   MOS_SEMAPHORE, *PMOS_SEMAPHORE;           //!< semaphore pointer
typedef pthread_t               MOS_THREADHANDLE;                         //!< thread handle
typedef uint32_t                UFKEY, *PUFKEY;                           //!< Handle of user feature key

#define _T(x)     x
#define MAX_PATH  128

#include <va/va.h>                          // For VAStatus
typedef VAStatus                            MOS_OSRESULT;
#include <stdarg.h>
#define MOS_FUNC_EXPORT                     __attribute__((visibility("default")))
#define MOS_DATA_EXPORT                     __attribute__((visibility("default")))
#define MOS_FUNC_PRIVATE                    __attribute__((visibility("hidden")))
#define MOS_DATA_PRIVATE                    __attribute__((visibility("hidden")))
#define MOS_EXPORT_DECL

#ifndef __UFO_PORTABLE_DATATYPE_DEFINED__
typedef struct tagRECT
{
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;
} RECT, *PRECT, *LPRECT;

typedef union _LARGE_INTEGER {
    struct {
        uint32_t LowPart;
        int32_t HighPart;
    } DUMMYSTRUCTNAME;
    struct {
        uint32_t LowPart;
        int32_t HighPart;
    } u;
    int64_t QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

typedef void*       HANDLE;                     //!< Opaque handle comprehended only by the OS
typedef void**      PHANDLE;                    //!< Opaque handle comprehended only by the OS
typedef void*       HMODULE;                    //!< Opaque handle comprehended only by the OS

typedef uint32_t TP_WAIT_RESULT;
typedef struct _TP_WAIT TP_WAIT, *PTP_WAIT;
typedef struct _TP_CALLBACK_INSTANCE TP_CALLBACK_INSTANCE, *PTP_CALLBACK_INSTANCE;

#define INFINITE         0xFFFFFFFF

#endif

/* compile-time ASSERT */

#ifndef C_ASSERT
    #define __CONCATING( a1, a2 )   a1 ## a2
    #define __UNIQUENAME( a1, a2 )  __CONCATING( a1, a2 )
    #define UNIQUENAME( __text )    __UNIQUENAME( __text, __COUNTER__ )
    #define C_ASSERT(e) typedef char UNIQUENAME(STATIC_ASSERT_)[(e)?1:-1]
#endif

#if __GNUC__ < 4
    #error "Unsupported GCC version. Please use 4.0+"
#endif

#define __noop
#if defined __x86_64__
    #define __stdcall       // deprecated for x86-64
    #define __cdecl         // deprecated for x86-64
#else
    #define __cdecl         __attribute__((__cdecl__))
    #define __stdcall       __attribute__((__stdcall__))
#endif

#define __declspec(x)           __declspec_##x

#define __MEDIA_PORTABLE_DATAYPE_DEFINED__

#define _stprintf                                           sprintf
#define _sntprintf                                          snprintf

#define vsprintf_s(pBuffer, size, format, arg)              vsnprintf(pBuffer, size, format, arg)
#define sprintf_s(pBuffer, size, format, ...)               snprintf(pBuffer, size, format, ##__VA_ARGS__)

enum SYNC_HAZARD
{
    SYNC_RAW_HAZARD,
    SYNC_WAR_HAZARD,
    SYNC_WAW_HAZARD
};

#endif // __MOS_DEFS_SPECIFIC_H__
