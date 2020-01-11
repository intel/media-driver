/*
* Copyright (c) 2009-2017, Intel Corporation
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
//! \file      cm_innerdef_os.h 
//! \brief     Contains CM definitions (cm_innerdef_os.h) 
//!

#pragma once

#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"

// Use this Macro to distinguish open-source and close-source.
// Note that this will be moved to CMake Files in future
#define USE_EXTENSION_CODE 0

#include "mos_os.h"
#include "media_libva_common.h"
#include <sys/types.h>
#if defined(__linux__)
#include <sys/syscall.h>
#elif defined(__DragonFly__) || defined(__FreeBSD__)
#include <pthread_np.h>
#elif defined(__NetBSD__)
#include <lwp.h>
#elif defined(__sun)
#include <thread.h>
#endif
#include <unistd.h>


typedef void* UMD_RESOURCE;

#define CM_LINUX  1

typedef int  (__cdecl *pCallBackReleaseVaSurface)( void *VaDpy, void *pVaSurfID);

#define CM_INVALID_TAG              (LONG_LONG_MAX)

#define CM_TRACKER_ID_QWORD_PER_TASK     0

#define CM_ERROR_NULL_POINTER CM_NULL_POINTER

#ifndef _SYSTEMTIME_DEFINE_PROTECTOR_
#define _SYSTEMTIME_DEFINE_PROTECTOR_

typedef struct _SYSTEMTIME
{
    uint16_t wYear;
    uint16_t wMonth;
    uint16_t wDayOfWeek;
    uint16_t wDay;
    uint16_t wHour;
    uint16_t wMinute;
    uint16_t wSecond;
    uint16_t wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME;

inline void GetLocalTime(PSYSTEMTIME psystime)
{
    time_t temp;
    struct tm *ltime;
    time(&temp);
    ltime=localtime(&temp);
    psystime->wYear = ltime->tm_year;
    psystime->wMonth = ltime->tm_mon;
    psystime->wDayOfWeek = ltime->tm_wday;
    psystime->wDay = ltime->tm_mday;
    psystime->wHour = ltime->tm_hour;
    psystime->wMinute = ltime->tm_min;
    psystime->wSecond = ltime->tm_sec;
    psystime->wMilliseconds = 0;
}

#endif

#define CmGetCurProcessId() getpid()
#if defined(__linux__)
#define CmGetCurThreadId()  syscall(SYS_gettid)
#elif defined(__DragonFly__) || defined(__FreeBSD__)
#define CmGetCurThreadId()  pthread_getthreadid_np()
#elif defined(__NetBSD__)
#define CmGetCurThreadId()  _lwp_self()
#elif defined(__OpenBSD__)
#define CmGetCurThreadId()  getthrid()
#elif defined(__sun)
#define CmGetCurThreadId()  thr_self()
#else
#warning "Cannot get kernel thread identifier on this platform."
#define CmGetCurThreadId()  (uintptr_t)pthread_self()
#endif

