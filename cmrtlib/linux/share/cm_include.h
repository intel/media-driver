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
// cm_include.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#ifndef CMRTLIB_LINUX_SHARE_CM_INCLUDE_H_
#define CMRTLIB_LINUX_SHARE_CM_INCLUDE_H_

#include <dlfcn.h>

#ifndef ANDROID
#include "va/va_x11.h"
#include "va/va.h"
#else
#include <va/va_android.h>
#define Display unsigned int
#define ANDROID_DISPLAY 0x18c34078
#endif

#define sprintf_s snprintf

#ifdef CM_RT_EXPORTS
#define CM_RT_API __attribute__((visibility("default")))
#else
#define CM_RT_API 
#endif

#ifndef CM_NOINLINE
  #define CM_NOINLINE __attribute__((noinline)) 
#endif

#define __cdecl
#define __stdcall  __attribute__((__stdcall__))

#ifdef __try
#undef __try
#endif
#define __try try

#ifdef __except
#undef __except
#endif
#define __except(e)  catch(e)

#define  EXCEPTION_EXECUTE_HANDLER std::exception const& e

#if NO_EXCEPTION_HANDLING || ANDROID
    #define try         if (true)
    #define catch(x)    if (false)
    #define throw(...)
#endif

typedef union _LARGE_INTEGER
{
    struct
    {
        uint32_t lowPart;
        int32_t highPart;
    } u;
    int64_t quadPart;
} LARGE_INTEGER;

typedef int HANDLE;

extern "C" int32_t QueryPerformanceFrequency(LARGE_INTEGER *frequency);
extern "C" int32_t QueryPerformanceCounter(LARGE_INTEGER *performanceCount);

#endif  // #ifndef CMRTLIB_LINUX_SHARE_CM_INCLUDE_H_
