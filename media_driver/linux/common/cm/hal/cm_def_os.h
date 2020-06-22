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
//! \file      cm_def_os.h
//! \brief     Contains CM definitions
//!
#pragma once

#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <iostream>
#include <exception>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <cstring>
#include "dlfcn.h"
#include "media_libva_cm.h"

#ifndef SUCCEEDED
#define SUCCEEDED(hr)   (hr == VA_STATUS_SUCCESS)
#endif // !SUCCEEDED

#ifndef FAILED
#define FAILED(hr)      (hr != VA_STATUS_SUCCESS)
#endif // !FAILED

#define CM_DRIVER_EXPOSED __attribute__ ((visibility ("default")))

#ifdef __cplusplus
#define EXTERN_C     extern "C"
#else
#define EXTERN_C
#endif

static inline char *
strtok_s(char *strToken, const char *strDelimit, char **context)
{
    return strtok_r(strToken, strDelimit, context);
}

inline int memcpy_s(void *dst, size_t numberOfElements, const void *src, size_t count)
{
    if ((dst == nullptr) || (src == nullptr))
    {
        return EINVAL;
    }
    if (numberOfElements < count)
    {
        return ERANGE;
    }
    std::memcpy(dst, src, count);
    return 0;
}

#define CM_CONTEXT_DATA  CM_CONTEXT
#define PCM_CONTEXT_DATA PCM_CONTEXT

#define CM_MAX_SURFACE2D_FORMAT_COUNT   29
#define CM_MAX_SURFACE2D_FORMAT_COUNT_INTERNAL   (CM_MAX_SURFACE2D_FORMAT_COUNT-1)

// max resolution for surface 2D
#define CM_MAX_2D_SURF_WIDTH  16384
#define CM_MAX_2D_SURF_HEIGHT 16384

typedef enum _CM_TEXTURE_ADDRESS_TYPE
{
    CM_TEXTURE_ADDRESS_WRAP         = 1,
    CM_TEXTURE_ADDRESS_MIRROR       = 2,
    CM_TEXTURE_ADDRESS_CLAMP        = 3,
    CM_TEXTURE_ADDRESS_BORDER       = 4,
    CM_TEXTURE_ADDRESS_MIRRORONCE   = 5
} CM_TEXTURE_ADDRESS_TYPE;

typedef enum _CM_TEXTURE_FILTER_TYPE
{
    CM_TEXTURE_FILTER_TYPE_NONE             = 0,
    CM_TEXTURE_FILTER_TYPE_POINT            = 1,
    CM_TEXTURE_FILTER_TYPE_LINEAR           = 2,
    CM_TEXTURE_FILTER_TYPE_ANISOTROPIC      = 3,
    CM_TEXTURE_FILTER_TYPE_FLATCUBIC        = 4,
    CM_TEXTURE_FILTER_TYPE_GAUSSIANCUBIC    = 5,
    CM_TEXTURE_FILTER_TYPE_PYRAMIDALQUAD    = 6,
    CM_TEXTURE_FILTER_TYPE_GAUSSIANQUAD     = 7
} CM_TEXTURE_FILTER_TYPE;

// From Compiler
#define CM_NOINLINE __attribute__((noinline))

namespace CMRT_UMD
{
class SurfaceIndex
{
public:
    CM_NOINLINE SurfaceIndex() { index = 0; extraByte = 0; };
    CM_NOINLINE SurfaceIndex(const SurfaceIndex& src) { index = src.index; extraByte = src.extraByte; };
    CM_NOINLINE SurfaceIndex(const unsigned int& n) { index = n; extraByte = 0; };
    CM_NOINLINE SurfaceIndex& operator = (const unsigned int& n) { this->index = n; return *this; };
    CM_NOINLINE SurfaceIndex& operator + (const unsigned int& n) { this->index += n; return *this; };
    CM_NOINLINE SurfaceIndex& operator= (const SurfaceIndex& other) { this->index = other.index; return *this; };
    virtual unsigned int get_data(void) { return index; };

    //g++ warning: class has virtual functions but non-virtual destructor
    virtual ~SurfaceIndex() {};

private:
    unsigned int index;

    /*
     * Do not delete this line:
     * SurfaceIndex is commonly used as CM kernel function's parameter.
     * It has virutal table and has copy constructor, so GNU calling convention will pass the object's pointer to kernel function.
     * This is different from MSVC, which always copies the entire object transferred on the callee's stack.
     *
     * Depending on the special object size after adding below "extraByte",
     * SetKernelArg and SetThreadArg can recognize this object and follow GNU's convention to construct kernel function's stack.
     */
    unsigned char extraByte;
};

class SamplerIndex
{
public:
    CM_NOINLINE SamplerIndex() { index = 0; extraByte = 0;};
    CM_NOINLINE SamplerIndex(SamplerIndex& src) { index = src.get_data(); extraByte = src.extraByte; };
    CM_NOINLINE SamplerIndex(const unsigned int& n) { index = n; extraByte = 0; };
    CM_NOINLINE SamplerIndex& operator = (const unsigned int& n) { this->index = n; return *this; };
    virtual unsigned int get_data(void) { return index; };
    virtual ~SamplerIndex(){};

private:
    unsigned int index;

    /*
     * Do not delete this line:
     * Same reason as SurfaceIndex.
     */
    unsigned char extraByte;
    SamplerIndex& operator= (const SamplerIndex& other);
};
}

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

inline void GetLocalTime(PSYSTEMTIME sysTime)
{
    time_t temp;
    struct tm *localTime;
    time(&temp);
    localTime=localtime(&temp);
    sysTime->wYear = localTime->tm_year;
    sysTime->wMonth = localTime->tm_mon;
    sysTime->wDayOfWeek = localTime->tm_wday;
    sysTime->wDay = localTime->tm_mday;
    sysTime->wHour = localTime->tm_hour;
    sysTime->wMinute = localTime->tm_min;
    sysTime->wSecond = localTime->tm_sec;
    sysTime->wMilliseconds = 0;
}
#endif

