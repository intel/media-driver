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
//! \file      cm_type.h 
//! \brief     Contains CM related type definitions 
//!

#include <assert.h>
#include <iostream>
#include <string>
#include <string.h>
#include <stddef.h> //offsetof()
#include <limits>
#include <limits.h>
#include <stdlib.h>

#ifdef CM_DEBUG
#include <sstream>
#endif /* CM_DEBUG */

#define OBJ_COUNTER 0

#ifdef CM_GENX
#define OFFSET ushort
#else
#define OFFSET uint
#endif /* CM_GENX */

#ifndef CM_TYPES_H
#define CM_TYPES_H

#define CM_KERNEL_FUNCTION(...) CM_KERNEL_FUNCTION2(__VA_ARGS__)
#define CM_KERNEL_FUNCTION2(...) #__VA_ARGS__

namespace CMRT_UMD {

typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;

#ifndef __int8
typedef char __int8;
#endif

#ifndef __int16
typedef short __int16; 
#endif

#ifndef __int32
typedef int __int32;
#endif

#ifndef __int64
typedef long long __int64;
#endif

#ifndef __uint64
typedef unsigned long long __uint64; 
#endif

#if !defined(__CLANG_CM) 
#include <stdint.h>
#else
typedef signed   __int8   int8_t;
typedef signed   __int16  int16_t;
typedef signed   __int32  int32_t;
typedef signed   __int64  int64_t;
typedef unsigned __int8   uint8_t;
typedef unsigned __int16  uint16_t;
typedef unsigned __int32  uint32_t;
typedef unsigned __int64  uint64_t;
#endif


#define SAT 1

typedef enum _CmAtomicOpType_
{
    ATOMIC_ADD                     = 0x0,
    ATOMIC_SUB                     = 0x1,
    ATOMIC_INC                     = 0x2,
    ATOMIC_DEC                     = 0x3,
    ATOMIC_MIN                     = 0x4,
    ATOMIC_MAX                     = 0x5,
    ATOMIC_XCHG                    = 0x6,
    ATOMIC_CMPXCHG                 = 0x7,
    ATOMIC_AND                     = 0x8,
    ATOMIC_OR                      = 0x9,
    ATOMIC_XOR                     = 0xa,
    ATOMIC_MINSINT                 = 0xb,
    ATOMIC_MAXSINT                 = 0xc
} CmAtomicOpType;


typedef enum _ChannelMaskType_
{   CM_R_ENABLE         = 1,
    CM_G_ENABLE         = 2,
    CM_GR_ENABLE        = 3,
    CM_B_ENABLE         = 4,
    CM_BR_ENABLE        = 5,
    CM_BG_ENABLE        = 6,
    CM_BGR_ENABLE       = 7,
    CM_A_ENABLE         = 8,
    CM_AR_ENABLE        = 9,
    CM_AG_ENABLE        = 10,
    CM_AGR_ENABLE       = 11,
    CM_AB_ENABLE        = 12,
    CM_ABR_ENABLE       = 13,
    CM_ABG_ENABLE       = 14,
    CM_ABGR_ENABLE      = 15
} ChannelMaskType;

// Moved from cm_sampler8x8.h as now used in cm_sampler.h as well
typedef enum _OutputFormatControl_
{   CM_16_FULL        = 0,
    CM_16_DOWN_SAMPLE = 1,
    CM_8_FULL         = 2,
    CM_8_DOWN_SAMPLE  = 3
} OutputFormatControl;

typedef enum _CmRoundingMode_
{
    // These values are stored pre-shifted to remove the need to shift the values when applying to
    // the control word
    CM_RTE                = 0,       // Round to nearest or even
    CM_RTP                = 1 << 4,  // Round towards +ve inf
    CM_RTN                = 2 << 4,  // Round towards -ve inf
    CM_RTZ                = 3 << 4   // Round towards zero
} CmRoundingMode;

typedef enum _CmFPMode_
{
    CM_IEEE               = 0,
    CM_ALT                = 1
} CmFPMode;


namespace CmEmulSys
{

static long long
abs(long long a)
{
    if (a < 0) {
        return -a;
    } else {
        return a;
    }
}

template<typename RT>
struct satur {
template<typename T> static RT
saturate(const T val, const int flags) {
    if ((flags & SAT) == 0) {
        return (RT) val;
    }

#pragma push_macro("max")
#pragma push_macro("min")

#ifdef max
#undef max
#undef min
#endif
    const RT t_max = std::numeric_limits<RT>::max();
    const RT t_min = std::numeric_limits<RT>::min();

    
    if (val > t_max) {
        return t_max;
    } else if ((val >= 0 ) && (t_min < 0)) {
        // RT is "signed" if t_min < 0
        // when comparing a signed and a unsigned variable, the signed one cast to unsigned first.
        return (RT) val;
    } else if (val < t_min) {
        return t_min;
    } else {
        return (RT) val;
    }

#pragma pop_macro("min")
#pragma pop_macro("max")
}
};


template<>
struct satur<float> {
template<typename T> static float
saturate(const T val, const int flags) {
    if ((flags & SAT) == 0) {
        return (float) val;
    } 

    if (val < 0.) {
        return 0;
    } else if (val > 1.) {
        return 1.;
    } else {
        return (float) val; 
    }
}
};

template<>
struct satur<double> {
template<typename T> static double
saturate(const T val, const int flags) {
    if ((flags & SAT) == 0) {
        return (double) val;
    } 

    if (val < 0.) {
        return 0;
    } else if (val > 1.) {
        return 1.;
    } else {
        return (double) val; 
    }
}
};

template<typename T1, bool B> struct _SetSatur {
    static uint SetSatur() {
        return 0;
    }
};

template <> struct _SetSatur<float, true> {
    static uint SetSatur() {
        return SAT;
    }
};

template <> struct _SetSatur<double, true> {
    static uint SetSatur() {
        return SAT;
    }
};

} /* ::CmEmulSys */

template <typename T1, typename T2> struct restype {
private:
    restype();
};

//#if defined(CM_NOCONV) || defined(CM_NONSTRICT)
#if defined(CM_NOCONV)

template <> struct restype<char, char>            { typedef short  type; };
template <> struct restype<char, unsigned char>   { typedef short  type; };
template <> struct restype<char, short>           { typedef short type; };
template <> struct restype<char, unsigned short>  { typedef short type; };
template <> struct restype<char, int>             { typedef int   type; };
template <> struct restype<char, unsigned int>    { typedef int   type; };
template <> struct restype<char, float>           { typedef float type; };
template <> struct restype<char, double>          { typedef double type; };
template <> struct restype<char, long long>       { typedef long long type; };
template <> struct restype<char, unsigned long long>           { typedef long long type; };

template <> struct restype<unsigned char, char>            { typedef short  type; };
template <> struct restype<unsigned char, unsigned char>   { typedef short  type; };
template <> struct restype<unsigned char, short>           { typedef short type; };
template <> struct restype<unsigned char, unsigned short>  { typedef short type; };
template <> struct restype<unsigned char, int>             { typedef int   type; };
template <> struct restype<unsigned char, unsigned int>    { typedef int   type; };
template <> struct restype<unsigned char, float>           { typedef float type; };
template <> struct restype<unsigned char, double>          { typedef double type; };
template <> struct restype<unsigned char, long long>       { typedef long long type; };
template <> struct restype<unsigned char, unsigned long long>           { typedef long long type; };

template <> struct restype<short, char>            { typedef short type; };
template <> struct restype<short, unsigned char>   { typedef short type; };
template <> struct restype<short, short>           { typedef short type; };
template <> struct restype<short, unsigned short>  { typedef short type; };
template <> struct restype<short, int>             { typedef int   type; };
template <> struct restype<short, unsigned int>    { typedef int   type; };
template <> struct restype<short, float>           { typedef float type; };
template <> struct restype<short, double>          { typedef double type; };
template <> struct restype<short, long long>       { typedef long long type; };
template <> struct restype<short, unsigned long long>           { typedef long long type; };

template <> struct restype<unsigned short, char>            { typedef short type; };
template <> struct restype<unsigned short, unsigned char>   { typedef short type; };
template <> struct restype<unsigned short, short>           { typedef short type; };
template <> struct restype<unsigned short, unsigned short>  { typedef short type; };
template <> struct restype<unsigned short, int>             { typedef int type; };
template <> struct restype<unsigned short, unsigned int>    { typedef int type; };
template <> struct restype<unsigned short, float>           { typedef float type; };
template <> struct restype<unsigned short, double>          { typedef double type; };
template <> struct restype<unsigned short, long long>       { typedef long long type; };
template <> struct restype<unsigned short, unsigned long long>           { typedef long long type; };

template <> struct restype<int, char>            { typedef int type; };
template <> struct restype<int, unsigned char>   { typedef int type; };
template <> struct restype<int, short>           { typedef int type; };
template <> struct restype<int, unsigned short>  { typedef int type; };
template <> struct restype<int, int>             { typedef int type; };
template <> struct restype<int, unsigned int>    { typedef int type; };
template <> struct restype<int, float>           { typedef float type; };
template <> struct restype<int, double>          { typedef double type; };
template <> struct restype<int, long long>       { typedef long long type; };
template <> struct restype<int, unsigned long long>           { typedef long long type; };

template <> struct restype<unsigned int, char>            { typedef int type; };
template <> struct restype<unsigned int, unsigned char>   { typedef int type; };
template <> struct restype<unsigned int, short>           { typedef int type; };
template <> struct restype<unsigned int, unsigned short>  { typedef int type; };
template <> struct restype<unsigned int, int>             { typedef int type; };
template <> struct restype<unsigned int, unsigned int>    { typedef int type; };
template <> struct restype<unsigned int, float>           { typedef float type; };
template <> struct restype<unsigned int, double>          { typedef double type; };
template <> struct restype<unsigned int, long long>       { typedef long long type; };
template <> struct restype<unsigned int, unsigned long long>           { typedef long long type; };

template <> struct restype<float, char>            { typedef float type; };
template <> struct restype<float, unsigned char>   { typedef float type; };
template <> struct restype<float, short>           { typedef float type; };
template <> struct restype<float, unsigned short>  { typedef float type; };
template <> struct restype<float, int>             { typedef float type; };
template <> struct restype<float, unsigned int>    { typedef float type; };
template <> struct restype<float, float>           { typedef float type; };
template <> struct restype<float, double>          { typedef double type; };
template <> struct restype<float, long long>       { typedef float type; };
template <> struct restype<float, unsigned long long>           { typedef float type; };

template <> struct restype<double, char>            { typedef double type; };
template <> struct restype<double, unsigned char>   { typedef double type; };
template <> struct restype<double, short>           { typedef double type; };
template <> struct restype<double, unsigned short>  { typedef double type; };
template <> struct restype<double, int>             { typedef double type; };
template <> struct restype<double, unsigned int>    { typedef double type; };
template <> struct restype<double, float>           { typedef double type; };
template <> struct restype<double, double>          { typedef double type; };
template <> struct restype<double, long long>       { typedef double type; };
template <> struct restype<double, unsigned long long>           { typedef double type; };

template <> struct restype<long long, char>            { typedef long long type; };
template <> struct restype<long long, unsigned char>   { typedef long long type; };
template <> struct restype<long long, short>           { typedef long long type; };
template <> struct restype<long long, unsigned short>  { typedef long long type; };
template <> struct restype<long long, int>             { typedef long long type; };
template <> struct restype<long long, unsigned int>    { typedef long long type; };
template <> struct restype<long long, float>           { typedef float type; };
template <> struct restype<long long, double>          { typedef double type; };
template <> struct restype<long long, long long>       { typedef long long type; };
template <> struct restype<long long, unsigned long long>           { typedef long long type; };

template <> struct restype<unsigned long long, char>             { typedef long long type; };
template <> struct restype<unsigned long long, unsigned char>    { typedef long long type; };
template <> struct restype<unsigned long long, short>            { typedef long long type; };
template <> struct restype<unsigned long long, unsigned short>   { typedef long long type; };
template <> struct restype<unsigned long long, int>              { typedef long long type; };
template <> struct restype<unsigned long long, unsigned int>     { typedef long long type; };
template <> struct restype<unsigned long long, float>            { typedef float type; };
template <> struct restype<unsigned long long, double>           { typedef double type; };
template <> struct restype<unsigned long long, long long>        { typedef long long type; };
template <> struct restype<unsigned long long, unsigned long long>            { typedef long long type; };

#else

template <> struct restype<char, char>            { typedef int  type; };
template <> struct restype<char, unsigned char>   { typedef int  type; };
template <> struct restype<char, short>           { typedef int type; };
template <> struct restype<char, unsigned short>  { typedef int type; };
template <> struct restype<char, int>             { typedef int   type; };
template <> struct restype<char, unsigned int>    { typedef unsigned int   type; };
template <> struct restype<char, float>           { typedef float type; };
template <> struct restype<char, double>          { typedef double type; };
template <> struct restype<char, long long>       { typedef long long type; };
template <> struct restype<char, unsigned long long>           { typedef unsigned long long type; };

template <> struct restype<unsigned char, char>            { typedef int  type; };
template <> struct restype<unsigned char, unsigned char>   { typedef int  type; };
template <> struct restype<unsigned char, short>           { typedef int type; };
template <> struct restype<unsigned char, unsigned short>  { typedef int type; };
template <> struct restype<unsigned char, int>             { typedef int   type; };
template <> struct restype<unsigned char, unsigned int>    { typedef unsigned int   type; };
template <> struct restype<unsigned char, float>           { typedef float type; };
template <> struct restype<unsigned char, double>          { typedef double type; };
template <> struct restype<unsigned char, long long>       { typedef long long type; };
template <> struct restype<unsigned char, unsigned long long>           { typedef unsigned long long type; };

template <> struct restype<short, char>            { typedef int type; };
template <> struct restype<short, unsigned char>   { typedef int type; };
template <> struct restype<short, short>           { typedef int type; };
template <> struct restype<short, unsigned short>  { typedef int type; };
template <> struct restype<short, int>             { typedef int   type; };
template <> struct restype<short, unsigned int>    { typedef unsigned int   type; };
template <> struct restype<short, float>           { typedef float type; };
template <> struct restype<short, double>          { typedef double type; };
template <> struct restype<short, long long>       { typedef long long type; };
template <> struct restype<short, unsigned long long>           { typedef unsigned long long type; };

template <> struct restype<unsigned short, char>            { typedef int type; };
template <> struct restype<unsigned short, unsigned char>   { typedef int type; };
template <> struct restype<unsigned short, short>           { typedef int type; };
template <> struct restype<unsigned short, unsigned short>  { typedef int type; };
template <> struct restype<unsigned short, int>             { typedef int type; };
template <> struct restype<unsigned short, unsigned int>    { typedef unsigned int type; };
template <> struct restype<unsigned short, float>           { typedef float type; };
template <> struct restype<unsigned short, double>          { typedef double type; };
template <> struct restype<unsigned short, long long>       { typedef long long type; };
template <> struct restype<unsigned short, unsigned long long>           { typedef unsigned long long type; };

template <> struct restype<int, char>            { typedef int type; };
template <> struct restype<int, unsigned char>   { typedef int type; };
template <> struct restype<int, short>           { typedef int type; };
template <> struct restype<int, unsigned short>  { typedef int type; };
template <> struct restype<int, int>             { typedef int type; };
template <> struct restype<int, unsigned int>    { typedef unsigned int type; };
template <> struct restype<int, float>           { typedef float type; };
template <> struct restype<int, double>          { typedef double type; };
template <> struct restype<int, long long>       { typedef long long type; };
template <> struct restype<int, unsigned long long>           { typedef unsigned long long type; };

template <> struct restype<unsigned int, char>            { typedef unsigned int type; };
template <> struct restype<unsigned int, unsigned char>   { typedef unsigned int type; };
template <> struct restype<unsigned int, short>           { typedef unsigned int type; };
template <> struct restype<unsigned int, unsigned short>  { typedef unsigned int type; };
template <> struct restype<unsigned int, int>             { typedef unsigned int type; };
template <> struct restype<unsigned int, unsigned int>    { typedef unsigned int type; };
template <> struct restype<unsigned int, float>           { typedef float type; };
template <> struct restype<unsigned int, double>          { typedef double type; };
template <> struct restype<unsigned int, long long>       { typedef long long type; };
template <> struct restype<unsigned int, unsigned long long>           { typedef unsigned long long type; };

template <> struct restype<float, char>            { typedef float type; };
template <> struct restype<float, unsigned char>   { typedef float type; };
template <> struct restype<float, short>           { typedef float type; };
template <> struct restype<float, unsigned short>  { typedef float type; };
template <> struct restype<float, int>             { typedef float type; };
template <> struct restype<float, unsigned int>    { typedef float type; };
template <> struct restype<float, float>           { typedef float type; };
template <> struct restype<float, double>          { typedef double type; };
template <> struct restype<float, long long>       { typedef float type; };
template <> struct restype<float, unsigned long long>           { typedef float type; };

template <> struct restype<double, char>            { typedef double type; };
template <> struct restype<double, unsigned char>   { typedef double type; };
template <> struct restype<double, short>           { typedef double type; };
template <> struct restype<double, unsigned short>  { typedef double type; };
template <> struct restype<double, int>             { typedef double type; };
template <> struct restype<double, unsigned int>    { typedef double type; };
template <> struct restype<double, float>           { typedef double type; };
template <> struct restype<double, double>          { typedef double type; };
template <> struct restype<double, long long>       { typedef double type; };
template <> struct restype<double, unsigned long long>           { typedef double type; };

template <> struct restype<unsigned long long, char>            { typedef unsigned long long type; };
template <> struct restype<unsigned long long, unsigned char>   { typedef unsigned long long type; };
template <> struct restype<unsigned long long, short>           { typedef unsigned long long type; };
template <> struct restype<unsigned long long, unsigned short>  { typedef unsigned long long type; };
template <> struct restype<unsigned long long, int>             { typedef unsigned long long type; };
template <> struct restype<unsigned long long, unsigned int>    { typedef unsigned long long type; };
template <> struct restype<unsigned long long, float>           { typedef float type; };
template <> struct restype<unsigned long long, double>          { typedef double type; };
template <> struct restype<unsigned long long, long long>       { typedef unsigned long long type; };
template <> struct restype<unsigned long long, unsigned long long>           { typedef unsigned long long type; };

template <> struct restype<long long, char>            { typedef long long type; };
template <> struct restype<long long, unsigned char>   { typedef long long type; };
template <> struct restype<long long, short>           { typedef long long type; };
template <> struct restype<long long, unsigned short>  { typedef long long type; };
template <> struct restype<long long, int>             { typedef long long type; };
template <> struct restype<long long, unsigned int>    { typedef long long type; };
template <> struct restype<long long, float>           { typedef float type; };
template <> struct restype<long long, double>          { typedef double type; };
template <> struct restype<long long, long long>       { typedef long long type; };
template <> struct restype<long long, unsigned long long>           { typedef unsigned long long type; };

#endif 

template <typename T1, typename T2> struct bitwise_restype {
private:
    bitwise_restype();
};

#if defined(CM_NOCONV)

template <> struct bitwise_restype<char, char>            { typedef char  type; };
template <> struct bitwise_restype<char, unsigned char>   { typedef short  type; };
template <> struct bitwise_restype<char, short>           { typedef short type; };
template <> struct bitwise_restype<char, unsigned short>  { typedef short type; };
template <> struct bitwise_restype<char, int>             { typedef int   type; };
template <> struct bitwise_restype<char, unsigned int>    { typedef int   type; };
template <> struct bitwise_restype<char, float>           { typedef float type; };
template <> struct bitwise_restype<char, double>          { typedef double type; };
template <> struct bitwise_restype<char, long long>       { typedef long long type; };
template <> struct bitwise_restype<char, unsigned long long>           { typedef long long type; };

template <> struct bitwise_restype<unsigned char, char>            { typedef char  type; }; 
template <> struct bitwise_restype<unsigned char, unsigned char>   { typedef char  type; };
template <> struct bitwise_restype<unsigned char, short>           { typedef short type; };
template <> struct bitwise_restype<unsigned char, unsigned short>  { typedef short type; };
template <> struct bitwise_restype<unsigned char, int>             { typedef int   type; };
template <> struct bitwise_restype<unsigned char, unsigned int>    { typedef int   type; };
template <> struct bitwise_restype<unsigned char, float>           { typedef float type; };
template <> struct bitwise_restype<unsigned char, double>          { typedef double type; };
template <> struct bitwise_restype<unsigned char, long long>       { typedef long long type; };
template <> struct bitwise_restype<unsigned char, unsigned long long>           { typedef long long type; };

template <> struct bitwise_restype<short, char>            { typedef short type; };
template <> struct bitwise_restype<short, unsigned char>   { typedef short type; };
template <> struct bitwise_restype<short, short>           { typedef short type; };
template <> struct bitwise_restype<short, unsigned short>  { typedef short type; };  
template <> struct bitwise_restype<short, int>             { typedef int   type; };
template <> struct bitwise_restype<short, unsigned int>    { typedef int   type; };
template <> struct bitwise_restype<short, float>           { typedef float type; };
template <> struct bitwise_restype<short, double>          { typedef double type; };
template <> struct bitwise_restype<short, long long>       { typedef long long type; };
template <> struct bitwise_restype<short, unsigned long long>           { typedef long long type; };

template <> struct bitwise_restype<unsigned short, char>            { typedef short type; };
template <> struct bitwise_restype<unsigned short, unsigned char>   { typedef short type; };
template <> struct bitwise_restype<unsigned short, short>           { typedef short type; };
template <> struct bitwise_restype<unsigned short, unsigned short>  { typedef short type; };  
template <> struct bitwise_restype<unsigned short, int>             { typedef int type; };
template <> struct bitwise_restype<unsigned short, unsigned int>    { typedef int type; };
template <> struct bitwise_restype<unsigned short, float>           { typedef float type; };
template <> struct bitwise_restype<unsigned short, double>          { typedef double type; };
template <> struct bitwise_restype<unsigned short, long long>       { typedef long long type; };
template <> struct bitwise_restype<unsigned short, unsigned long long>           { typedef long long type; };

template <> struct bitwise_restype<int, char>            { typedef int type; };
template <> struct bitwise_restype<int, unsigned char>   { typedef int type; };
template <> struct bitwise_restype<int, short>           { typedef int type; };
template <> struct bitwise_restype<int, unsigned short>  { typedef int type; };
template <> struct bitwise_restype<int, int>             { typedef int type; };
template <> struct bitwise_restype<int, unsigned int>    { typedef int type; };
template <> struct bitwise_restype<int, float>           { typedef float type; };
template <> struct bitwise_restype<int, double>          { typedef double type; };
template <> struct bitwise_restype<int, long long>       { typedef long long type; };
template <> struct bitwise_restype<int, unsigned long long>           { typedef long long type; };

template <> struct bitwise_restype<unsigned int, char>            { typedef int type; };
template <> struct bitwise_restype<unsigned int, unsigned char>   { typedef int type; };
template <> struct bitwise_restype<unsigned int, short>           { typedef int type; };
template <> struct bitwise_restype<unsigned int, unsigned short>  { typedef int type; };
template <> struct bitwise_restype<unsigned int, int>             { typedef int type; };
template <> struct bitwise_restype<unsigned int, unsigned int>    { typedef int type; };  
template <> struct bitwise_restype<unsigned int, float>           { typedef float type; };
template <> struct bitwise_restype<unsigned int, double>          { typedef double type; };
template <> struct bitwise_restype<unsigned int, long long>       { typedef long long type; };
template <> struct bitwise_restype<unsigned int, unsigned long long>           { typedef long long type; };

template <> struct bitwise_restype<float, char>            { typedef float type; };
template <> struct bitwise_restype<float, unsigned char>   { typedef float type; };
template <> struct bitwise_restype<float, short>           { typedef float type; };
template <> struct bitwise_restype<float, unsigned short>  { typedef float type; };
template <> struct bitwise_restype<float, int>             { typedef float type; };
template <> struct bitwise_restype<float, unsigned int>    { typedef float type; };
template <> struct bitwise_restype<float, float>           { typedef float type; };
template <> struct bitwise_restype<float, double>          { typedef double type; };
template <> struct bitwise_restype<float, long long>       { typedef float type; };
template <> struct bitwise_restype<float, unsigned long long>           { typedef float type; };

template <> struct bitwise_restype<double, char>            { typedef double type; };
template <> struct bitwise_restype<double, unsigned char>   { typedef double type; };
template <> struct bitwise_restype<double, short>           { typedef double type; };
template <> struct bitwise_restype<double, unsigned short>  { typedef double type; };
template <> struct bitwise_restype<double, int>             { typedef double type; };
template <> struct bitwise_restype<double, unsigned int>    { typedef double type; };
template <> struct bitwise_restype<double, float>           { typedef double type; };
template <> struct bitwise_restype<double, double>          { typedef double type; };
template <> struct bitwise_restype<double, long long>       { typedef double type; };
template <> struct bitwise_restype<double, unsigned long long>           { typedef double type; };

template <> struct bitwise_restype<long long, char>            { typedef long long type; };
template <> struct bitwise_restype<long long, unsigned char>   { typedef long long type; };
template <> struct bitwise_restype<long long, short>           { typedef long long type; };
template <> struct bitwise_restype<long long, unsigned short>  { typedef long long type; };
template <> struct bitwise_restype<long long, int>             { typedef long long type; };
template <> struct bitwise_restype<long long, unsigned int>    { typedef long long type; };
template <> struct bitwise_restype<long long, float>           { typedef float type; };
template <> struct bitwise_restype<long long, double>          { typedef double type; };
template <> struct bitwise_restype<long long, long long>       { typedef long long type; };
template <> struct bitwise_restype<long long, unsigned long long>           { typedef long long type; };

template <> struct bitwise_restype<unsigned long long, char>            { typedef long long type; };
template <> struct bitwise_restype<unsigned long long, unsigned char>   { typedef long long type; };
template <> struct bitwise_restype<unsigned long long, short>           { typedef long long type; };
template <> struct bitwise_restype<unsigned long long, unsigned short>  { typedef long long type; };
template <> struct bitwise_restype<unsigned long long, int>             { typedef long long type; };
template <> struct bitwise_restype<unsigned long long, unsigned int>    { typedef long long type; };
template <> struct bitwise_restype<unsigned long long, float>           { typedef float type; };
template <> struct bitwise_restype<unsigned long long, double>          { typedef double type; };
template <> struct bitwise_restype<unsigned long long, long long>       { typedef long long type; };
template <> struct bitwise_restype<unsigned long long, unsigned long long>           { typedef long long type; };
#else

template <> struct bitwise_restype<char, char>            { typedef char  type; };
template <> struct bitwise_restype<char, unsigned char>   { typedef unsigned char  type; };  
template <> struct bitwise_restype<char, short>           { typedef short type; };
template <> struct bitwise_restype<char, unsigned short>  { typedef unsigned short type; };   
template <> struct bitwise_restype<char, int>             { typedef int   type; };
template <> struct bitwise_restype<char, unsigned int>    { typedef unsigned int   type; };
template <> struct bitwise_restype<char, float>           { typedef float type; };
template <> struct bitwise_restype<char, double>          { typedef double type; };
template <> struct bitwise_restype<char, long long>       { typedef long long type; };
template <> struct bitwise_restype<char, unsigned long long>           { typedef unsigned long long type; };

template <> struct bitwise_restype<unsigned char, char>            { typedef char type; };
template <> struct bitwise_restype<unsigned char, unsigned char>   { typedef unsigned char type; }; 
template <> struct bitwise_restype<unsigned char, short>           { typedef short type; };
template <> struct bitwise_restype<unsigned char, unsigned short>  { typedef unsigned short type; };
template <> struct bitwise_restype<unsigned char, int>             { typedef int   type; };
template <> struct bitwise_restype<unsigned char, unsigned int>    { typedef unsigned int   type; };
template <> struct bitwise_restype<unsigned char, float>           { typedef float type; };
template <> struct bitwise_restype<unsigned char, double>          { typedef double type; };
template <> struct bitwise_restype<unsigned char, long long>       { typedef long long type; };
template <> struct bitwise_restype<unsigned char, unsigned long long>           { typedef unsigned long long type; };

template <> struct bitwise_restype<short, char>            { typedef short type; };
template <> struct bitwise_restype<short, unsigned char>   { typedef short type; };
template <> struct bitwise_restype<short, short>           { typedef short type; };
template <> struct bitwise_restype<short, unsigned short>  { typedef unsigned short type; };
template <> struct bitwise_restype<short, int>             { typedef int   type; };
template <> struct bitwise_restype<short, unsigned int>    { typedef unsigned int   type; };
template <> struct bitwise_restype<short, float>           { typedef float type; };
template <> struct bitwise_restype<short, double>          { typedef double type; };
template <> struct bitwise_restype<short, long long>       { typedef long long type; };
template <> struct bitwise_restype<short, unsigned long long>           { typedef unsigned long long type; };

template <> struct bitwise_restype<unsigned short, char>            { typedef unsigned short type; };
template <> struct bitwise_restype<unsigned short, unsigned char>   { typedef unsigned short type; };
template <> struct bitwise_restype<unsigned short, short>           { typedef unsigned short type; };
template <> struct bitwise_restype<unsigned short, unsigned short>  { typedef unsigned short type; };
template <> struct bitwise_restype<unsigned short, int>             { typedef int type; };
template <> struct bitwise_restype<unsigned short, unsigned int>    { typedef unsigned int type; };
template <> struct bitwise_restype<unsigned short, float>           { typedef float type; };
template <> struct bitwise_restype<unsigned short, double>          { typedef double type; };
template <> struct bitwise_restype<unsigned short, long long>       { typedef long long type; };
template <> struct bitwise_restype<unsigned short, unsigned long long>           { typedef unsigned long long type; };

template <> struct bitwise_restype<int, char>            { typedef int type; };
template <> struct bitwise_restype<int, unsigned char>   { typedef int type; };
template <> struct bitwise_restype<int, short>           { typedef int type; };
template <> struct bitwise_restype<int, unsigned short>  { typedef int type; };
template <> struct bitwise_restype<int, int>             { typedef int type; };
template <> struct bitwise_restype<int, unsigned int>    { typedef unsigned int type; };
template <> struct bitwise_restype<int, float>           { typedef float type; };
template <> struct bitwise_restype<int, double>          { typedef double type; };
template <> struct bitwise_restype<int, long long>       { typedef long long type; };
template <> struct bitwise_restype<int, unsigned long long>           { typedef unsigned long long type; };

template <> struct bitwise_restype<unsigned int, char>            { typedef unsigned int type; };
template <> struct bitwise_restype<unsigned int, unsigned char>   { typedef unsigned int type; };
template <> struct bitwise_restype<unsigned int, short>           { typedef unsigned int type; };
template <> struct bitwise_restype<unsigned int, unsigned short>  { typedef unsigned int type; };
template <> struct bitwise_restype<unsigned int, int>             { typedef unsigned int type; };
template <> struct bitwise_restype<unsigned int, unsigned int>    { typedef unsigned int type; };
template <> struct bitwise_restype<unsigned int, float>           { typedef float type; };
template <> struct bitwise_restype<unsigned int, double>          { typedef double type; };
template <> struct bitwise_restype<unsigned int, long long>       { typedef long long type; };
template <> struct bitwise_restype<unsigned int, unsigned long long>           { typedef unsigned long long type; };

template <> struct bitwise_restype<float, char>            { typedef float type; };
template <> struct bitwise_restype<float, unsigned char>   { typedef float type; };
template <> struct bitwise_restype<float, short>           { typedef float type; };
template <> struct bitwise_restype<float, unsigned short>  { typedef float type; };
template <> struct bitwise_restype<float, int>             { typedef float type; };
template <> struct bitwise_restype<float, unsigned int>    { typedef float type; };
template <> struct bitwise_restype<float, float>           { typedef float type; };
template <> struct bitwise_restype<float, double>          { typedef double type; };
template <> struct bitwise_restype<float, long long>       { typedef float type; };
template <> struct bitwise_restype<float, unsigned long long>           { typedef float type; };

template <> struct bitwise_restype<double, char>            { typedef double type; };
template <> struct bitwise_restype<double, unsigned char>   { typedef double type; };
template <> struct bitwise_restype<double, short>           { typedef double type; };
template <> struct bitwise_restype<double, unsigned short>  { typedef double type; };
template <> struct bitwise_restype<double, int>             { typedef double type; };
template <> struct bitwise_restype<double, unsigned int>    { typedef double type; };
template <> struct bitwise_restype<double, float>           { typedef double type; };
template <> struct bitwise_restype<double, double>          { typedef double type; };
template <> struct bitwise_restype<double, long long>       { typedef double type; };
template <> struct bitwise_restype<double, unsigned long long>           { typedef double type; };

template <> struct bitwise_restype<long long, char>            { typedef long long type; };
template <> struct bitwise_restype<long long, unsigned char>   { typedef long long type; };
template <> struct bitwise_restype<long long, short>           { typedef long long type; };
template <> struct bitwise_restype<long long, unsigned short>  { typedef long long type; };
template <> struct bitwise_restype<long long, int>             { typedef long long type; };
template <> struct bitwise_restype<long long, unsigned int>    { typedef long long type; };
template <> struct bitwise_restype<long long, float>           { typedef float type; };
template <> struct bitwise_restype<long long, double>          { typedef double type; };
template <> struct bitwise_restype<long long, long long>       { typedef long long type; };
template <> struct bitwise_restype<long long, unsigned long long>           { typedef unsigned long long type; };

template <> struct bitwise_restype<unsigned long long, char>            { typedef unsigned long long type; };
template <> struct bitwise_restype<unsigned long long, unsigned char>   { typedef unsigned long long type; };
template <> struct bitwise_restype<unsigned long long, short>           { typedef unsigned long long type; };
template <> struct bitwise_restype<unsigned long long, unsigned short>  { typedef unsigned long long type; };
template <> struct bitwise_restype<unsigned long long, int>             { typedef unsigned long long type; };
template <> struct bitwise_restype<unsigned long long, unsigned int>    { typedef unsigned long long type; };
template <> struct bitwise_restype<unsigned long long, float>           { typedef float type; };
template <> struct bitwise_restype<unsigned long long, double>          { typedef double type; };
template <> struct bitwise_restype<unsigned long long, long long>       { typedef unsigned long long type; };
template <> struct bitwise_restype<unsigned long long, unsigned long long>           { typedef unsigned long long type; };

#endif 

template <typename T1, typename T2> struct restype_ex {
private:
    restype_ex();
};
//#ifdef CM_NONSTRICT
#if 0
template <> struct restype_ex<char, char>            { typedef short  type; };
template <> struct restype_ex<char, unsigned char>   { typedef short  type; };
template <> struct restype_ex<char, short>           { typedef short type; };
template <> struct restype_ex<char, unsigned short>  { typedef short type; };
template <> struct restype_ex<char, int>             { typedef int   type; };
template <> struct restype_ex<char, unsigned int>    { typedef int   type; };
template <> struct restype_ex<char, float>           { typedef float type; };
template <> struct restype_ex<char, double>          { typedef double type; };

template <> struct restype_ex<unsigned char, char>            { typedef short  type; };
template <> struct restype_ex<unsigned char, unsigned char>   { typedef short  type; };
template <> struct restype_ex<unsigned char, short>           { typedef short type; };
template <> struct restype_ex<unsigned char, unsigned short>  { typedef short type; };
template <> struct restype_ex<unsigned char, int>             { typedef int   type; };
template <> struct restype_ex<unsigned char, unsigned int>    { typedef int   type; };
template <> struct restype_ex<unsigned char, float>           { typedef float type; };
template <> struct restype_ex<unsigned char, double>          { typedef double type; };

template <> struct restype_ex<short, char>            { typedef short type; };
template <> struct restype_ex<short, unsigned char>   { typedef short type; };
template <> struct restype_ex<short, short>           { typedef short type; };
template <> struct restype_ex<short, unsigned short>  { typedef short type; };
template <> struct restype_ex<short, int>             { typedef int   type; };
template <> struct restype_ex<short, unsigned int>    { typedef int   type; };
template <> struct restype_ex<short, float>           { typedef float type; };
template <> struct restype_ex<short, double>          { typedef double type; };

template <> struct restype_ex<unsigned short, char>            { typedef short type; };
template <> struct restype_ex<unsigned short, unsigned char>   { typedef short type; };
template <> struct restype_ex<unsigned short, short>           { typedef short type; };
template <> struct restype_ex<unsigned short, unsigned short>  { typedef short type; };
template <> struct restype_ex<unsigned short, int>             { typedef int type; };
template <> struct restype_ex<unsigned short, unsigned int>    { typedef int type; };
template <> struct restype_ex<unsigned short, float>           { typedef float type; };
template <> struct restype_ex<unsigned short, double>          { typedef double type; };

template <> struct restype_ex<int, char>            { typedef int type; };
template <> struct restype_ex<int, unsigned char>   { typedef int type; };
template <> struct restype_ex<int, short>           { typedef int type; };
template <> struct restype_ex<int, unsigned short>  { typedef int type; };
template <> struct restype_ex<int, int>             { typedef int type; };
template <> struct restype_ex<int, unsigned int>    { typedef int type; };
template <> struct restype_ex<int, float>           { typedef float type; };
template <> struct restype_ex<int, double>          { typedef double type; };

template <> struct restype_ex<unsigned int, char>            { typedef int type; };
template <> struct restype_ex<unsigned int, unsigned char>   { typedef int type; };
template <> struct restype_ex<unsigned int, short>           { typedef int type; };
template <> struct restype_ex<unsigned int, unsigned short>  { typedef int type; };
template <> struct restype_ex<unsigned int, int>             { typedef int type; };
template <> struct restype_ex<unsigned int, unsigned int>    { typedef int type; };
template <> struct restype_ex<unsigned int, float>           { typedef float type; };
template <> struct restype_ex<unsigned int, double>          { typedef double type; };

template <> struct restype_ex<float, char>            { typedef float type; };
template <> struct restype_ex<float, unsigned char>   { typedef float type; };
template <> struct restype_ex<float, short>           { typedef float type; };
template <> struct restype_ex<float, unsigned short>  { typedef float type; };
template <> struct restype_ex<float, int>             { typedef float type; };
template <> struct restype_ex<float, unsigned int>    { typedef float type; };
template <> struct restype_ex<float, float>           { typedef float type; };
template <> struct restype_ex<float, double>          { typedef double type; };

template <> struct restype_ex<double, char>            { typedef double type; };
template <> struct restype_ex<double, unsigned char>   { typedef double type; };
template <> struct restype_ex<double, short>           { typedef double type; };
template <> struct restype_ex<double, unsigned short>  { typedef double type; };
template <> struct restype_ex<double, int>             { typedef double type; };
template <> struct restype_ex<double, unsigned int>    { typedef double type; };
template <> struct restype_ex<double, float>           { typedef double type; };
template <> struct restype_ex<double, double>          { typedef double type; };

template <> struct restype_ex<long long, char>            { typedef long long type; };
template <> struct restype_ex<long long, unsigned char>   { typedef long long type; };
template <> struct restype_ex<long long, short>           { typedef long long type; };
template <> struct restype_ex<long long, unsigned short>  { typedef long long type; };
template <> struct restype_ex<long long, int>             { typedef long long type; };
template <> struct restype_ex<long long, unsigned int>    { typedef long long type; };
template <> struct restype_ex<long long, float>           { typedef float type; };
template <> struct restype_ex<long long, double>          { typedef double type; };
template <> struct restype_ex<long long, long long>       { typedef long long type; };
template <> struct restype_ex<long long, unsigned long long>           { typedef long long type; };

template <> struct restype_ex<unsigned long long, char>            { typedef long long type; };
template <> struct restype_ex<unsigned long long, unsigned char>   { typedef long long type; };
template <> struct restype_ex<unsigned long long, short>           { typedef long long type; };
template <> struct restype_ex<unsigned long long, unsigned short>  { typedef long long type; };
template <> struct restype_ex<unsigned long long, int>             { typedef long long type; };
template <> struct restype_ex<unsigned long long, unsigned int>    { typedef long long type; };
template <> struct restype_ex<unsigned long long, float>           { typedef float type; };
template <> struct restype_ex<unsigned long long, double>          { typedef double type; };
template <> struct restype_ex<unsigned long long, long long>       { typedef long long type; };
template <> struct restype_ex<unsigned long long, unsigned long long>           { typedef long long type; };

template <typename T> struct maxtype;
template<> struct maxtype<float> { typedef float type; };
template<> struct maxtype<char> { typedef char type; };
template<> struct maxtype<short> { typedef short type; };
template<> struct maxtype<int> { typedef int type; };
template<> struct maxtype<uchar> { typedef uchar type; };
template<> struct maxtype<ushort> { typedef ushort type; };
template<> struct maxtype<uint> { typedef uint type; };
template<> struct maxtype<double> { typedef double type; };
template<> struct maxtype<long long> { typedef long long type; };
template<> struct maxtype<unsigned long long> { typedef unsigned long long type; };

#else

template <> struct restype_ex<char, char>            { typedef int  type; };
template <> struct restype_ex<char, unsigned char>   { typedef int  type; };
template <> struct restype_ex<char, short>           { typedef int type; };
template <> struct restype_ex<char, unsigned short>  { typedef int type; };
template <> struct restype_ex<char, int>             { typedef long long   type; };
template <> struct restype_ex<char, unsigned int>    { typedef long long   type; };
template <> struct restype_ex<char, float>           { typedef float type; };
template <> struct restype_ex<char, double>           { typedef double type; };

template <> struct restype_ex<unsigned char, char>            { typedef int  type; };
template <> struct restype_ex<unsigned char, unsigned char>   { typedef int  type; };
template <> struct restype_ex<unsigned char, short>           { typedef int type; };
template <> struct restype_ex<unsigned char, unsigned short>  { typedef int type; };
template <> struct restype_ex<unsigned char, int>             { typedef long long   type; };
template <> struct restype_ex<unsigned char, unsigned int>    { typedef long long   type; };
template <> struct restype_ex<unsigned char, float>           { typedef float type; };
template <> struct restype_ex<unsigned char, double>          { typedef double type; };
template <> struct restype_ex<unsigned char, long long>       { typedef long long type; };
template <> struct restype_ex<unsigned char, unsigned long long>           { typedef long long type; };

template <> struct restype_ex<short, char>            { typedef int type; };
template <> struct restype_ex<short, unsigned char>   { typedef int type; };
template <> struct restype_ex<short, short>           { typedef int type; };
template <> struct restype_ex<short, unsigned short>  { typedef int type; };
template <> struct restype_ex<short, int>             { typedef long long   type; };
template <> struct restype_ex<short, unsigned int>    { typedef long long   type; };
template <> struct restype_ex<short, float>           { typedef float type; };
template <> struct restype_ex<short, double>          { typedef double type; };
template <> struct restype_ex<short, long long>       { typedef long long type; };
template <> struct restype_ex<short, unsigned long long>           { typedef long long type; };

template <> struct restype_ex<unsigned short, char>            { typedef int type; };
template <> struct restype_ex<unsigned short, unsigned char>   { typedef int type; };
template <> struct restype_ex<unsigned short, short>           { typedef int type; };
template <> struct restype_ex<unsigned short, unsigned short>  { typedef int type; };
template <> struct restype_ex<unsigned short, int>             { typedef long long type; };
template <> struct restype_ex<unsigned short, unsigned int>    { typedef long long type; };
template <> struct restype_ex<unsigned short, float>           { typedef float type; };
template <> struct restype_ex<unsigned short, double>          { typedef double type; };
template <> struct restype_ex<unsigned short, long long>       { typedef long long type; };
template <> struct restype_ex<unsigned short, unsigned long long>           { typedef long long type; };

template <> struct restype_ex<int, char>            { typedef long long type; };
template <> struct restype_ex<int, unsigned char>   { typedef long long type; };
template <> struct restype_ex<int, short>           { typedef long long type; };
template <> struct restype_ex<int, unsigned short>  { typedef long long type; };
template <> struct restype_ex<int, int>             { typedef long long type; };
template <> struct restype_ex<int, unsigned int>    { typedef long long type; };
template <> struct restype_ex<int, float>           { typedef float type; };
template <> struct restype_ex<int, double>          { typedef double type; };
template <> struct restype_ex<int, long long>       { typedef long long type; };
template <> struct restype_ex<int, unsigned long long>           { typedef long long type; };

template <> struct restype_ex<unsigned int, char>            { typedef long long type; };
template <> struct restype_ex<unsigned int, unsigned char>   { typedef long long type; };
template <> struct restype_ex<unsigned int, short>           { typedef long long type; };
template <> struct restype_ex<unsigned int, unsigned short>  { typedef long long type; };
template <> struct restype_ex<unsigned int, int>             { typedef long long type; };
template <> struct restype_ex<unsigned int, unsigned int>    { typedef long long type; };
template <> struct restype_ex<unsigned int, float>           { typedef float type; };
template <> struct restype_ex<unsigned int, double>          { typedef double type; };
template <> struct restype_ex<unsigned int, long long>       { typedef long long type; };
template <> struct restype_ex<unsigned int, unsigned long long>           { typedef long long type; };

template <> struct restype_ex<float, char>            { typedef float type; };
template <> struct restype_ex<float, unsigned char>   { typedef float type; };
template <> struct restype_ex<float, short>           { typedef float type; };
template <> struct restype_ex<float, unsigned short>  { typedef float type; };
template <> struct restype_ex<float, int>             { typedef float type; };
template <> struct restype_ex<float, unsigned int>    { typedef float type; };
template <> struct restype_ex<float, float>           { typedef float type; };
template <> struct restype_ex<float, double>          { typedef double type; };
template <> struct restype_ex<float, long long>       { typedef float type; };
template <> struct restype_ex<float, unsigned long long>           { typedef float type; };

template <> struct restype_ex<double, char>            { typedef double type; };
template <> struct restype_ex<double, unsigned char>   { typedef double type; };
template <> struct restype_ex<double, short>           { typedef double type; };
template <> struct restype_ex<double, unsigned short>  { typedef double type; };
template <> struct restype_ex<double, int>             { typedef double type; };
template <> struct restype_ex<double, unsigned int>    { typedef double type; };
template <> struct restype_ex<double, float>           { typedef double type; };
template <> struct restype_ex<double, double>          { typedef double type; };
template <> struct restype_ex<double, long long>       { typedef double type; };
template <> struct restype_ex<double, unsigned long long>           { typedef double type; };

template <> struct restype_ex<long long, char>            { typedef long long type; };
template <> struct restype_ex<long long, unsigned char>   { typedef long long type; };
template <> struct restype_ex<long long, short>           { typedef long long type; };
template <> struct restype_ex<long long, unsigned short>  { typedef long long type; };
template <> struct restype_ex<long long, int>             { typedef long long type; };
template <> struct restype_ex<long long, unsigned int>    { typedef long long type; };
template <> struct restype_ex<long long, float>           { typedef float type; };
template <> struct restype_ex<long long, double>          { typedef double type; };
template <> struct restype_ex<long long, long long>       { typedef long long type; };
template <> struct restype_ex<long long, unsigned long long>           { typedef long long type; };

template <> struct restype_ex<unsigned long long, char>            { typedef long long type; };
template <> struct restype_ex<unsigned long long, unsigned char>   { typedef long long type; };
template <> struct restype_ex<unsigned long long, short>           { typedef long long type; };
template <> struct restype_ex<unsigned long long, unsigned short>  { typedef long long type; };
template <> struct restype_ex<unsigned long long, int>             { typedef long long type; };
template <> struct restype_ex<unsigned long long, unsigned int>    { typedef long long type; };
template <> struct restype_ex<unsigned long long, float>           { typedef float type; };
template <> struct restype_ex<unsigned long long, double>          { typedef double type; };
template <> struct restype_ex<unsigned long long, long long>       { typedef long long type; };
template <> struct restype_ex<unsigned long long, unsigned long long>           { typedef long long type; };

template <typename T> struct maxtype;
template<> struct maxtype<float>       { typedef float type; };
template<> struct maxtype<char>        { typedef int type; };
template<> struct maxtype<short>       { typedef int type; };
template<> struct maxtype<int>         { typedef int type; };
template<> struct maxtype<uchar>       { typedef uint type; };
template<> struct maxtype<ushort>      { typedef uint type; };
template<> struct maxtype<uint>        { typedef uint type; };
template<> struct maxtype<double>      { typedef double type; };
template<> struct maxtype<long long>   { typedef long long type; };
template<> struct maxtype<unsigned long long>       { typedef unsigned long long type; };

#endif

template <typename T1, typename T2> struct uchar_type {
private:
        uchar_type();
};
template <> struct uchar_type<char, char>            { typedef uchar type; };
template <> struct uchar_type<char, unsigned char>   { typedef uchar type; };
template <> struct uchar_type<char, short>           { typedef uchar type; };
template <> struct uchar_type<char, unsigned short>  { typedef uchar type; };
template <> struct uchar_type<char, int>             { typedef uchar type; };
template <> struct uchar_type<char, unsigned int>    { typedef uchar type; };
template <> struct uchar_type<char, float>           { typedef uchar type; };
template <> struct uchar_type<char, double>          { typedef uchar type; };
template <> struct uchar_type<char, long long>       { typedef uchar type; };
template <> struct uchar_type<char, unsigned long long>           { typedef uchar type; };

template <> struct uchar_type<unsigned char, char>            { typedef uchar type; };
template <> struct uchar_type<unsigned char, unsigned char>   { typedef uchar type; };
template <> struct uchar_type<unsigned char, short>           { typedef uchar type; };
template <> struct uchar_type<unsigned char, unsigned short>  { typedef uchar type; };
template <> struct uchar_type<unsigned char, int>             { typedef uchar type; };
template <> struct uchar_type<unsigned char, unsigned int>    { typedef uchar type; };
template <> struct uchar_type<unsigned char, float>           { typedef uchar type; };
template <> struct uchar_type<unsigned char, double>          { typedef uchar type; };
template <> struct uchar_type<unsigned char, long long>       { typedef uchar type; };
template <> struct uchar_type<unsigned char, unsigned long long>           { typedef uchar type; };

template <> struct uchar_type<short, char>            { typedef uchar type; };
template <> struct uchar_type<short, unsigned char>   { typedef uchar type; };
template <> struct uchar_type<short, short>           { typedef uchar type; };
template <> struct uchar_type<short, unsigned short>  { typedef uchar type; };
template <> struct uchar_type<short, int>             { typedef uchar type; };
template <> struct uchar_type<short, unsigned int>    { typedef uchar type; };
template <> struct uchar_type<short, float>           { typedef uchar type; };
template <> struct uchar_type<short, double>          { typedef uchar type; };
template <> struct uchar_type<short, long long>       { typedef uchar type; };
template <> struct uchar_type<short, unsigned long long>           { typedef uchar type; };

template <> struct uchar_type<unsigned short, char>            { typedef uchar type; };
template <> struct uchar_type<unsigned short, unsigned char>   { typedef uchar type; };
template <> struct uchar_type<unsigned short, short>           { typedef uchar type; };
template <> struct uchar_type<unsigned short, unsigned short>  { typedef uchar type; };
template <> struct uchar_type<unsigned short, int>             { typedef uchar type; };
template <> struct uchar_type<unsigned short, unsigned int>    { typedef uchar type; };
template <> struct uchar_type<unsigned short, float>           { typedef uchar type; };
template <> struct uchar_type<unsigned short, double>          { typedef uchar type; };
template <> struct uchar_type<unsigned short, long long>       { typedef uchar type; };
template <> struct uchar_type<unsigned short, unsigned long long>           { typedef uchar type; };

template <> struct uchar_type<int, char>            { typedef uchar type; };
template <> struct uchar_type<int, unsigned char>   { typedef uchar type; };
template <> struct uchar_type<int, short>           { typedef uchar type; };
template <> struct uchar_type<int, unsigned short>  { typedef uchar type; };
template <> struct uchar_type<int, int>             { typedef uchar type; };
template <> struct uchar_type<int, unsigned int>    { typedef uchar type; };
template <> struct uchar_type<int, float>           { typedef uchar type; };
template <> struct uchar_type<int, double>          { typedef uchar type; };
template <> struct uchar_type<int, long long>       { typedef uchar type; };
template <> struct uchar_type<int, unsigned long long>           { typedef uchar type; };

template <> struct uchar_type<unsigned int, char>            { typedef uchar type; };
template <> struct uchar_type<unsigned int, unsigned char>   { typedef uchar type; };
template <> struct uchar_type<unsigned int, short>           { typedef uchar type; };
template <> struct uchar_type<unsigned int, unsigned short>  { typedef uchar type; };
template <> struct uchar_type<unsigned int, int>             { typedef uchar type; };
template <> struct uchar_type<unsigned int, unsigned int>    { typedef uchar type; };
template <> struct uchar_type<unsigned int, float>           { typedef uchar type; };
template <> struct uchar_type<unsigned int, double>          { typedef uchar type; };
template <> struct uchar_type<unsigned int, long long>       { typedef uchar type; };
template <> struct uchar_type<unsigned int, unsigned long long>           { typedef uchar type; };

template <> struct uchar_type<float, char>            { typedef uchar type; };
template <> struct uchar_type<float, unsigned char>   { typedef uchar type; };
template <> struct uchar_type<float, short>           { typedef uchar type; };
template <> struct uchar_type<float, unsigned short>  { typedef uchar type; };
template <> struct uchar_type<float, int>             { typedef uchar type; };
template <> struct uchar_type<float, unsigned int>    { typedef uchar type; };
template <> struct uchar_type<float, float>           { typedef uchar type; };
template <> struct uchar_type<float, double>          { typedef uchar type; };
template <> struct uchar_type<float, long long>       { typedef uchar type; };
template <> struct uchar_type<float, unsigned long long>           { typedef uchar type; };

template <> struct uchar_type<double, char>            { typedef uchar type; };
template <> struct uchar_type<double, unsigned char>   { typedef uchar type; };
template <> struct uchar_type<double, short>           { typedef uchar type; };
template <> struct uchar_type<double, unsigned short>  { typedef uchar type; };
template <> struct uchar_type<double, int>             { typedef uchar type; };
template <> struct uchar_type<double, unsigned int>    { typedef uchar type; };
template <> struct uchar_type<double, float>           { typedef uchar type; };
template <> struct uchar_type<double, double>          { typedef uchar type; };
template <> struct uchar_type<double, long long>       { typedef uchar type; };
template <> struct uchar_type<double, unsigned long long>           { typedef uchar type; };

template <> struct uchar_type<long long, char>            { typedef uchar type; };
template <> struct uchar_type<long long, unsigned char>   { typedef uchar type; };
template <> struct uchar_type<long long, short>           { typedef uchar type; };
template <> struct uchar_type<long long, unsigned short>  { typedef uchar type; };
template <> struct uchar_type<long long, int>             { typedef uchar type; };
template <> struct uchar_type<long long, unsigned int>    { typedef uchar type; };
template <> struct uchar_type<long long, float>           { typedef uchar type; };
template <> struct uchar_type<long long, double>          { typedef uchar type; };
template <> struct uchar_type<long long, long long>       { typedef uchar type; };
template <> struct uchar_type<long long, unsigned long long>           { typedef uchar type; };

template <> struct uchar_type<unsigned long long, char>            { typedef uchar type; };
template <> struct uchar_type<unsigned long long, unsigned char>   { typedef uchar type; };
template <> struct uchar_type<unsigned long long, short>           { typedef uchar type; };
template <> struct uchar_type<unsigned long long, unsigned short>  { typedef uchar type; };
template <> struct uchar_type<unsigned long long, int>             { typedef uchar type; };
template <> struct uchar_type<unsigned long long, unsigned int>    { typedef uchar type; };
template <> struct uchar_type<unsigned long long, float>           { typedef uchar type; };
template <> struct uchar_type<unsigned long long, double>          { typedef uchar type; };
template <> struct uchar_type<unsigned long long, long long>       { typedef uchar type; };
template <> struct uchar_type<unsigned long long, unsigned long long>           { typedef uchar type; };

template <typename T1, typename T2> struct ushort_type {
private:
        ushort_type();
};
template <> struct ushort_type<char, char>            { typedef ushort type; };
template <> struct ushort_type<char, unsigned char>   { typedef ushort type; };
template <> struct ushort_type<char, short>           { typedef ushort type; };
template <> struct ushort_type<char, unsigned short>  { typedef ushort type; };
template <> struct ushort_type<char, int>             { typedef ushort type; };
template <> struct ushort_type<char, unsigned int>    { typedef ushort type; };
template <> struct ushort_type<char, float>           { typedef ushort type; };
template <> struct ushort_type<char, double>          { typedef ushort type; };
template <> struct ushort_type<char, long long>       { typedef ushort type; };
template <> struct ushort_type<char, unsigned long long>           { typedef ushort type; };

template <> struct ushort_type<unsigned char, char>            { typedef ushort type; };
template <> struct ushort_type<unsigned char, unsigned char>   { typedef ushort type; };
template <> struct ushort_type<unsigned char, short>           { typedef ushort type; };
template <> struct ushort_type<unsigned char, unsigned short>  { typedef ushort type; };
template <> struct ushort_type<unsigned char, int>             { typedef ushort type; };
template <> struct ushort_type<unsigned char, unsigned int>    { typedef ushort type; };
template <> struct ushort_type<unsigned char, float>           { typedef ushort type; };
template <> struct ushort_type<unsigned char, double>          { typedef ushort type; };
template <> struct ushort_type<unsigned char, long long>       { typedef ushort type; };
template <> struct ushort_type<unsigned char, unsigned long long>           { typedef ushort type; };

template <> struct ushort_type<short, char>            { typedef ushort type; };
template <> struct ushort_type<short, unsigned char>   { typedef ushort type; };
template <> struct ushort_type<short, short>           { typedef ushort type; };
template <> struct ushort_type<short, unsigned short>  { typedef ushort type; };
template <> struct ushort_type<short, int>             { typedef ushort type; };
template <> struct ushort_type<short, unsigned int>    { typedef ushort type; };
template <> struct ushort_type<short, float>           { typedef ushort type; };
template <> struct ushort_type<short, double>          { typedef ushort type; };
template <> struct ushort_type<short, long long>       { typedef ushort type; };
template <> struct ushort_type<short, unsigned long long>           { typedef ushort type; };

template <> struct ushort_type<unsigned short, char>            { typedef ushort type; };
template <> struct ushort_type<unsigned short, unsigned char>   { typedef ushort type; };
template <> struct ushort_type<unsigned short, short>           { typedef ushort type; };
template <> struct ushort_type<unsigned short, unsigned short>  { typedef ushort type; };
template <> struct ushort_type<unsigned short, int>             { typedef ushort type; };
template <> struct ushort_type<unsigned short, unsigned int>    { typedef ushort type; };
template <> struct ushort_type<unsigned short, float>           { typedef ushort type; };
template <> struct ushort_type<unsigned short, double>          { typedef ushort type; };
template <> struct ushort_type<unsigned short, long long>       { typedef ushort type; };
template <> struct ushort_type<unsigned short, unsigned long long>           { typedef ushort type; };

template <> struct ushort_type<int, char>            { typedef ushort type; };
template <> struct ushort_type<int, unsigned char>   { typedef ushort type; };
template <> struct ushort_type<int, short>           { typedef ushort type; };
template <> struct ushort_type<int, unsigned short>  { typedef ushort type; };
template <> struct ushort_type<int, int>             { typedef ushort type; };
template <> struct ushort_type<int, unsigned int>    { typedef ushort type; };
template <> struct ushort_type<int, float>           { typedef ushort type; };
template <> struct ushort_type<int, double>          { typedef ushort type; };
template <> struct ushort_type<int, long long>       { typedef ushort type; };
template <> struct ushort_type<int, unsigned long long>           { typedef ushort type; };

template <> struct ushort_type<unsigned int, char>            { typedef ushort type; };
template <> struct ushort_type<unsigned int, unsigned char>   { typedef ushort type; };
template <> struct ushort_type<unsigned int, short>           { typedef ushort type; };
template <> struct ushort_type<unsigned int, unsigned short>  { typedef ushort type; };
template <> struct ushort_type<unsigned int, int>             { typedef ushort type; };
template <> struct ushort_type<unsigned int, unsigned int>    { typedef ushort type; };
template <> struct ushort_type<unsigned int, float>           { typedef ushort type; };
template <> struct ushort_type<unsigned int, double>          { typedef ushort type; };
template <> struct ushort_type<unsigned int, long long>       { typedef ushort type; };
template <> struct ushort_type<unsigned int, unsigned long long>           { typedef ushort type; };

template <> struct ushort_type<float, char>            { typedef ushort type; };
template <> struct ushort_type<float, unsigned char>   { typedef ushort type; };
template <> struct ushort_type<float, short>           { typedef ushort type; };
template <> struct ushort_type<float, unsigned short>  { typedef ushort type; };
template <> struct ushort_type<float, int>             { typedef ushort type; };
template <> struct ushort_type<float, unsigned int>    { typedef ushort type; };
template <> struct ushort_type<float, float>           { typedef ushort type; };
template <> struct ushort_type<float, double>          { typedef ushort type; };
template <> struct ushort_type<float, long long>       { typedef ushort type; };
template <> struct ushort_type<float, unsigned long long>           { typedef ushort type; };

template <> struct ushort_type<double, char>            { typedef ushort type; };
template <> struct ushort_type<double, unsigned char>   { typedef ushort type; };
template <> struct ushort_type<double, short>           { typedef ushort type; };
template <> struct ushort_type<double, unsigned short>  { typedef ushort type; };
template <> struct ushort_type<double, int>             { typedef ushort type; };
template <> struct ushort_type<double, unsigned int>    { typedef ushort type; };
template <> struct ushort_type<double, float>           { typedef ushort type; };
template <> struct ushort_type<double, double>          { typedef ushort type; };
template <> struct ushort_type<double, long long>       { typedef ushort type; };
template <> struct ushort_type<double, unsigned long long>           { typedef ushort type; };

template <> struct ushort_type<long long, char>            { typedef ushort type; };
template <> struct ushort_type<long long, unsigned char>   { typedef ushort type; };
template <> struct ushort_type<long long, short>           { typedef ushort type; };
template <> struct ushort_type<long long, unsigned short>  { typedef ushort type; };
template <> struct ushort_type<long long, int>             { typedef ushort type; };
template <> struct ushort_type<long long, unsigned int>    { typedef ushort type; };
template <> struct ushort_type<long long, float>           { typedef ushort type; };
template <> struct ushort_type<long long, double>          { typedef ushort type; };
template <> struct ushort_type<long long, long long>       { typedef ushort type; };
template <> struct ushort_type<long long, unsigned long long>           { typedef ushort type; };

template <> struct ushort_type<unsigned long long, char>            { typedef ushort type; };
template <> struct ushort_type<unsigned long long, unsigned char>   { typedef ushort type; };
template <> struct ushort_type<unsigned long long, short>           { typedef ushort type; };
template <> struct ushort_type<unsigned long long, unsigned short>  { typedef ushort type; };
template <> struct ushort_type<unsigned long long, int>             { typedef ushort type; };
template <> struct ushort_type<unsigned long long, unsigned int>    { typedef ushort type; };
template <> struct ushort_type<unsigned long long, float>           { typedef ushort type; };
template <> struct ushort_type<unsigned long long, double>          { typedef ushort type; };
template <> struct ushort_type<unsigned long long, long long>       { typedef ushort type; };
template <> struct ushort_type<unsigned long long, unsigned long long>           { typedef ushort type; };

template <typename T1, typename T2> struct uint_type {
private:
        uint_type();
};
template <> struct uint_type<char, char>            { typedef uint type; };
template <> struct uint_type<char, unsigned char>   { typedef uint type; };
template <> struct uint_type<char, short>           { typedef uint type; };
template <> struct uint_type<char, unsigned short>  { typedef uint type; };
template <> struct uint_type<char, int>             { typedef uint type; };
template <> struct uint_type<char, unsigned int>    { typedef uint type; };
template <> struct uint_type<char, float>           { typedef uint type; };
template <> struct uint_type<char, double>          { typedef uint type; };
template <> struct uint_type<char, long long>       { typedef uint type; };
template <> struct uint_type<char, unsigned long long>           { typedef uint type; };

template <> struct uint_type<unsigned char, char>            { typedef uint type; };
template <> struct uint_type<unsigned char, unsigned char>   { typedef uint type; };
template <> struct uint_type<unsigned char, short>           { typedef uint type; };
template <> struct uint_type<unsigned char, unsigned short>  { typedef uint type; };
template <> struct uint_type<unsigned char, int>             { typedef uint type; };
template <> struct uint_type<unsigned char, unsigned int>    { typedef uint type; };
template <> struct uint_type<unsigned char, float>           { typedef uint type; };
template <> struct uint_type<unsigned char, double>          { typedef uint type; };
template <> struct uint_type<unsigned char, long long>       { typedef uint type; };
template <> struct uint_type<unsigned char, unsigned long long>           { typedef uint type; };

template <> struct uint_type<short, char>            { typedef uint type; };
template <> struct uint_type<short, unsigned char>   { typedef uint type; };
template <> struct uint_type<short, short>           { typedef uint type; };
template <> struct uint_type<short, unsigned short>  { typedef uint type; };
template <> struct uint_type<short, int>             { typedef uint type; };
template <> struct uint_type<short, unsigned int>    { typedef uint type; };
template <> struct uint_type<short, float>           { typedef uint type; };
template <> struct uint_type<short, double>          { typedef uint type; };
template <> struct uint_type<short, long long>       { typedef uint type; };
template <> struct uint_type<short, unsigned long long>           { typedef uint type; };

template <> struct uint_type<unsigned short, char>            { typedef uint type; };
template <> struct uint_type<unsigned short, unsigned char>   { typedef uint type; };
template <> struct uint_type<unsigned short, short>           { typedef uint type; };
template <> struct uint_type<unsigned short, unsigned short>  { typedef uint type; };
template <> struct uint_type<unsigned short, int>             { typedef uint type; };
template <> struct uint_type<unsigned short, unsigned int>    { typedef uint type; };
template <> struct uint_type<unsigned short, float>           { typedef uint type; };
template <> struct uint_type<unsigned short, double>          { typedef uint type; };
template <> struct uint_type<unsigned short, long long>       { typedef uint type; };
template <> struct uint_type<unsigned short, unsigned long long>           { typedef uint type; };

template <> struct uint_type<int, char>            { typedef uint type; };
template <> struct uint_type<int, unsigned char>   { typedef uint type; };
template <> struct uint_type<int, short>           { typedef uint type; };
template <> struct uint_type<int, unsigned short>  { typedef uint type; };
template <> struct uint_type<int, int>             { typedef uint type; };
template <> struct uint_type<int, unsigned int>    { typedef uint type; };
template <> struct uint_type<int, float>           { typedef uint type; };
template <> struct uint_type<int, double>          { typedef uint type; };
template <> struct uint_type<int, long long>       { typedef uint type; };
template <> struct uint_type<int, unsigned long long>           { typedef uint type; };

template <> struct uint_type<unsigned int, char>            { typedef uint type; };
template <> struct uint_type<unsigned int, unsigned char>   { typedef uint type; };
template <> struct uint_type<unsigned int, short>           { typedef uint type; };
template <> struct uint_type<unsigned int, unsigned short>  { typedef uint type; };
template <> struct uint_type<unsigned int, int>             { typedef uint type; };
template <> struct uint_type<unsigned int, unsigned int>    { typedef uint type; };
template <> struct uint_type<unsigned int, float>           { typedef uint type; };
template <> struct uint_type<unsigned int, double>          { typedef uint type; };
template <> struct uint_type<unsigned int, long long>       { typedef uint type; };
template <> struct uint_type<unsigned int, unsigned long long>           { typedef uint type; };

template <> struct uint_type<float, char>            { typedef uint type; };
template <> struct uint_type<float, unsigned char>   { typedef uint type; };
template <> struct uint_type<float, short>           { typedef uint type; };
template <> struct uint_type<float, unsigned short>  { typedef uint type; };
template <> struct uint_type<float, int>             { typedef uint type; };
template <> struct uint_type<float, unsigned int>    { typedef uint type; };
template <> struct uint_type<float, float>           { typedef uint type; };
template <> struct uint_type<float, double>          { typedef uint type; };
template <> struct uint_type<float, long long>       { typedef uint type; };
template <> struct uint_type<float, unsigned long long>           { typedef uint type; };

template <> struct uint_type<double, char>            { typedef uint type; };
template <> struct uint_type<double, unsigned char>   { typedef uint type; };
template <> struct uint_type<double, short>           { typedef uint type; };
template <> struct uint_type<double, unsigned short>  { typedef uint type; };
template <> struct uint_type<double, int>             { typedef uint type; };
template <> struct uint_type<double, unsigned int>    { typedef uint type; };
template <> struct uint_type<double, float>           { typedef uint type; };
template <> struct uint_type<double, double>          { typedef uint type; };
template <> struct uint_type<double, long long>       { typedef uint type; };
template <> struct uint_type<double, unsigned long long>           { typedef uint type; };

template <> struct uint_type<long long, char>            { typedef uint type; };
template <> struct uint_type<long long, unsigned char>   { typedef uint type; };
template <> struct uint_type<long long, short>           { typedef uint type; };
template <> struct uint_type<long long, unsigned short>  { typedef uint type; };
template <> struct uint_type<long long, int>             { typedef uint type; };
template <> struct uint_type<long long, unsigned int>    { typedef uint type; };
template <> struct uint_type<long long, float>           { typedef uint type; };
template <> struct uint_type<long long, double>          { typedef uint type; };
template <> struct uint_type<long long, long long>       { typedef uint type; };
template <> struct uint_type<long long, unsigned long long>           { typedef uint type; };

template <> struct uint_type<unsigned long long, char>            { typedef uint type; };
template <> struct uint_type<unsigned long long, unsigned char>   { typedef uint type; };
template <> struct uint_type<unsigned long long, short>           { typedef uint type; };
template <> struct uint_type<unsigned long long, unsigned short>  { typedef uint type; };
template <> struct uint_type<unsigned long long, int>             { typedef uint type; };
template <> struct uint_type<unsigned long long, unsigned int>    { typedef uint type; };
template <> struct uint_type<unsigned long long, float>           { typedef uint type; };
template <> struct uint_type<unsigned long long, double>          { typedef uint type; };
template <> struct uint_type<unsigned long long, long long>       { typedef uint type; };
template <> struct uint_type<unsigned long long, unsigned long long>           { typedef uint type; };

template <typename T1> struct int_uint_type {
private:
        int_uint_type();
};
template <> struct int_uint_type<char>             { typedef int type; };
template <> struct int_uint_type<short>             { typedef int type; };
template <> struct int_uint_type<int>               { typedef int type; };
template <> struct int_uint_type<long long>               { typedef long long type; };
template <> struct int_uint_type<unsigned char>             { typedef unsigned int type; };
template <> struct int_uint_type<unsigned short>             { typedef unsigned int type; };
template <> struct int_uint_type<unsigned int>             { typedef unsigned int type; };
template <> struct int_uint_type<unsigned long long>             { typedef unsigned long long type; };

template <typename T1, typename T2> struct restype_sat {
private:
    restype_sat();
};

template <> struct restype_sat<char, char>            { typedef int  type; };
template <> struct restype_sat<char, unsigned char>   { typedef int  type; };
template <> struct restype_sat<char, short>           { typedef int type; };
template <> struct restype_sat<char, unsigned short>  { typedef int type; };
template <> struct restype_sat<char, int>             { typedef long long   type; };
template <> struct restype_sat<char, unsigned int>    { typedef long long   type; };
template <> struct restype_sat<char, float>           { typedef float type; };
template <> struct restype_sat<char, double>          { typedef double type; };
template <> struct restype_sat<char, long long>       { typedef long long type; };
template <> struct restype_sat<char, unsigned long long>           { typedef long long type; };

template <> struct restype_sat<unsigned char, char>            { typedef int  type; };
template <> struct restype_sat<unsigned char, unsigned char>   { typedef int  type; };
template <> struct restype_sat<unsigned char, short>           { typedef int type; };
template <> struct restype_sat<unsigned char, unsigned short>  { typedef int type; };
template <> struct restype_sat<unsigned char, int>             { typedef long long   type; };
template <> struct restype_sat<unsigned char, unsigned int>    { typedef long long   type; };
template <> struct restype_sat<unsigned char, float>           { typedef float type; };
template <> struct restype_sat<unsigned char, double>          { typedef double type; };
template <> struct restype_sat<unsigned char, long long>       { typedef long long type; };
template <> struct restype_sat<unsigned char, unsigned long long>           { typedef long long type; };

template <> struct restype_sat<short, char>            { typedef int type; };
template <> struct restype_sat<short, unsigned char>   { typedef int type; };
template <> struct restype_sat<short, short>           { typedef int type; };
template <> struct restype_sat<short, unsigned short>  { typedef int type; };
template <> struct restype_sat<short, int>             { typedef long long   type; };
template <> struct restype_sat<short, unsigned int>    { typedef long long   type; };
template <> struct restype_sat<short, float>           { typedef float type; };
template <> struct restype_sat<short, double>          { typedef double type; };
template <> struct restype_sat<short, long long>       { typedef long long type; };
template <> struct restype_sat<short, unsigned long long>           { typedef long long type; };

template <> struct restype_sat<unsigned short, char>            { typedef int type; };
template <> struct restype_sat<unsigned short, unsigned char>   { typedef int type; };
template <> struct restype_sat<unsigned short, short>           { typedef int type; };
template <> struct restype_sat<unsigned short, unsigned short>  { typedef unsigned int type; };
template <> struct restype_sat<unsigned short, int>             { typedef long long type; };
template <> struct restype_sat<unsigned short, unsigned int>    { typedef long long type; };
template <> struct restype_sat<unsigned short, float>           { typedef float type; };
template <> struct restype_sat<unsigned short, double>          { typedef double type; };
template <> struct restype_sat<unsigned short, long long>       { typedef long long type; };
template <> struct restype_sat<unsigned short, unsigned long long>           { typedef long long type; };

template <> struct restype_sat<int, char>            { typedef long long type; };
template <> struct restype_sat<int, unsigned char>   { typedef long long type; };
template <> struct restype_sat<int, short>           { typedef long long type; };
template <> struct restype_sat<int, unsigned short>  { typedef long long type; };
template <> struct restype_sat<int, int>             { typedef long long type; };
template <> struct restype_sat<int, unsigned int>    { typedef long long type; };
template <> struct restype_sat<int, float>           { typedef float type; };
template <> struct restype_sat<int, double>          { typedef double type; };
template <> struct restype_sat<int, long long>       { typedef long long type; };
template <> struct restype_sat<int, unsigned long long>           { typedef long long type; };

template <> struct restype_sat<unsigned int, char>            { typedef long long type; };
template <> struct restype_sat<unsigned int, unsigned char>   { typedef long long type; };
template <> struct restype_sat<unsigned int, short>           { typedef long long type; };
template <> struct restype_sat<unsigned int, unsigned short>  { typedef long long type; };
template <> struct restype_sat<unsigned int, int>             { typedef long long type; };
template <> struct restype_sat<unsigned int, unsigned int>    { typedef long long type; };
template <> struct restype_sat<unsigned int, float>           { typedef float type; };
template <> struct restype_sat<unsigned int, double>          { typedef double type; };
template <> struct restype_sat<unsigned int, long long>       { typedef long long type; };
template <> struct restype_sat<unsigned int, unsigned long long>           { typedef long long type; };

template <> struct restype_sat<float, char>            { typedef float type; };
template <> struct restype_sat<float, unsigned char>   { typedef float type; };
template <> struct restype_sat<float, short>           { typedef float type; };
template <> struct restype_sat<float, unsigned short>  { typedef float type; };
template <> struct restype_sat<float, int>             { typedef float type; };
template <> struct restype_sat<float, unsigned int>    { typedef float type; };
template <> struct restype_sat<float, float>           { typedef float type; };
template <> struct restype_sat<float, double>           { typedef double type; };

template <> struct restype_sat<double, char>            { typedef double type; };
template <> struct restype_sat<double, unsigned char>   { typedef double type; };
template <> struct restype_sat<double, short>           { typedef double type; };
template <> struct restype_sat<double, unsigned short>  { typedef double type; };
template <> struct restype_sat<double, int>             { typedef double type; };
template <> struct restype_sat<double, unsigned int>    { typedef double type; };
template <> struct restype_sat<double, float>           { typedef double type; };
template <> struct restype_sat<double, double>          { typedef double type; };
template <> struct restype_sat<double, long long>       { typedef double type; };
template <> struct restype_sat<double, unsigned long long>           { typedef double type; };

template <typename T> struct abstype;
template<> struct abstype<float> { typedef float type; };
template<> struct abstype<char> { typedef uchar type; };
template<> struct abstype<short> { typedef ushort type; };
template<> struct abstype<int> { typedef int type; };
template<> struct abstype<uchar> { typedef uchar type; };
template<> struct abstype<ushort> { typedef ushort type; };
template<> struct abstype<uint> { typedef uint type; };
template<> struct abstype<double> { typedef double type; };
template<> struct abstype<long long> { typedef unsigned long long type; };
template<> struct abstype<unsigned long long> { typedef unsigned long long type; };

template <typename T>
struct to_int {
    typedef T Int;
};
template<> struct to_int<float> { typedef int Int; };
template<> struct to_int<double> { typedef int Int; };

template <bool VALUE> struct check_true{
    static const bool value = false;
};
template <> struct check_true<true> {
    static const bool value = true;
};

template <typename T> struct inttype;
template <> struct inttype<char> {
    static const bool value = true;
};
template <> struct inttype<unsigned char> {
    static const bool value = true;
};
template <> struct inttype<short> {
    static const bool value = true;
};
template <> struct inttype<unsigned short> {
    static const bool value = true;
};
template <> struct inttype<int> {
    static const bool value = true;
};
template <> struct inttype<unsigned int> {
    static const bool value = true;
};
template <> struct inttype<long long> {
    static const bool value = true;
};
template <> struct inttype<unsigned long long> {
    static const bool value = true;
};

template <typename T> struct is_inttype {
    static const bool value = false;
};
template <> struct is_inttype<char> {
    static const bool value = true;
};
template <> struct is_inttype<unsigned char> {
    static const bool value = true;
};
template <> struct is_inttype<short> {
    static const bool value = true;
};
template <> struct is_inttype<unsigned short> {
    static const bool value = true;
};
template <> struct is_inttype<int> {
    static const bool value = true;
};
template <> struct is_inttype<unsigned int> {
    static const bool value = true;
};
template <> struct is_inttype<long long> {
    static const bool value = true;
};
template <> struct is_inttype<unsigned long long> {
    static const bool value = true;
};

template <typename T> struct is_byte_type {
    static const bool value = false;
};
template <> struct is_byte_type<char> {
    static const bool value = true;
};
template <> struct is_byte_type<uchar> {
    static const bool value = true;
};

template <typename T> struct is_word_type {
    static const bool value = false;
};
template <> struct is_word_type<short> {
    static const bool value = true;
};
template <> struct is_word_type<ushort> {
    static const bool value = true;
};

template <typename T> struct is_dword_type {
    static const bool value = false;
};
template <> struct is_dword_type<int> {
    static const bool value = true;
};
template <> struct is_dword_type<uint> {
    static const bool value = true;
};

template <typename T> struct is_hf_type {
    static const bool value = false;
};

template <typename T> struct is_fp_type {
    static const bool value = false;
};
template <> struct is_fp_type<float> {
    static const bool value = true;
};

template <typename T> struct is_df_type {
    static const bool value = false;
};
template <> struct is_df_type<double> {
    static const bool value = true;
};

template <typename T> struct is_fp_or_dword_type {
    static const bool value = false;
};
template <> struct is_fp_or_dword_type<int> {
    static const bool value = true;
};
template <> struct is_fp_or_dword_type<uint> {
    static const bool value = true;
};
template <> struct is_fp_or_dword_type<float> {
    static const bool value = true;
};
// The check is only used for dataport APIs,
// which also support df data type.
template <> struct is_fp_or_dword_type<double> {
    static const bool value = true;
};

template <typename T> struct is_ushort_type {
    static const bool value = false;
};
template <> struct is_ushort_type<ushort> {
    static const bool value = true;
};

template <typename T1, typename T2> struct is_float_dword {
    static const bool value = false;
};
template <> struct is_float_dword<float, int> {
    static const bool value = true;
};
template <> struct is_float_dword<float, uint> {
    static const bool value = true;
};
template <> struct is_float_dword<int, float> {
    static const bool value = true;
};
template <> struct is_float_dword<uint, float> {
    static const bool value = true;
};

template <typename T> struct hftype {
    static const bool value = false;
};

template <typename T> struct fptype {
    static const bool value = false;
};
template <> struct fptype<float> {
    static const bool value = true;
};

template <typename T> struct dftype {
    static const bool value = false;
};
template <> struct dftype<double> {
    static const bool value = true;
};

template <typename T> struct cmtype;
template <> struct cmtype<char> {
    static const bool value = true;
};

template <> struct cmtype<signed char> {
    static const bool value = true;
};

template <> struct cmtype<unsigned char> {
    static const bool value = true;
};

template <> struct cmtype<short> {
    static const bool value = true;
};

template <> struct cmtype<unsigned short> {
    static const bool value = true;
};
template <> struct cmtype<int> {
    static const bool value = true;
};

template <> struct cmtype<unsigned int> {
    static const bool value = true;
};

template <> struct cmtype<unsigned long> {
    static const bool value = true;
};

template <> struct cmtype<float> {
    static const bool value = true;
};

template <> struct cmtype<double> {
    static const bool value = true;
};

template <> struct cmtype<long long> {
    static const bool value = true;
};

template <> struct cmtype<unsigned long long> {
    static const bool value = true;
};

template <> struct cmtype<CMRT_UMD::SurfaceIndex> {
    static const bool value = true;
};

template<typename T> struct bytetype;
template<> struct bytetype<char> {
    static const bool value = true;
};
template<> struct bytetype<uchar> {
    static const bool value = true;
};

template<typename T> struct wordtype;
template<> struct wordtype<short> {
    static const bool value = true;
};
template<> struct wordtype<ushort> {
    static const bool value = true;
};

template<typename T> struct dwordtype;
template<> struct dwordtype<int> {
    static const bool value = true;
};
template<> struct dwordtype<uint> {
    static const bool value = true;
};

template<typename T> struct unsignedtype{
    static const bool value = false;
};
template<> struct unsignedtype<uint> {
    static const bool value = true;
};
template<> struct unsignedtype<ushort> {
    static const bool value = true;
};
template<> struct unsignedtype<uchar> {
    static const bool value = true;
};
template<> struct unsignedtype<unsigned long long> {
    static const bool value = true;
};

template<typename T> struct uinttype;
template<> struct uinttype<uint> {
    static const bool value = true;
};

template <uint N1, uint N2> struct ressize {
    static const uint size = (N1 > N2)?N1:N2;
    static const bool conformable = check_true<N1%size == 0 && N2%size == 0>::value;
};

    // used to control generation of compile time assertions
#if defined(__CLANG_CM) && _MSC_VER >= 1600
    // CM_STATIC_ERROR uses the static_assert mechanism
#include <assert.h>
// type traits are often used in CM_STATIC_ERROR conditions
#include <type_traits>
#define CM_STATIC_ERROR(C,M)   static_assert((C),"CM:e:" M)
#define CM_STATIC_WARNING(C,M) static_assert((C),"CM:w:" M)
#else
#define CM_STATIC_ERROR(C,M)
#define CM_STATIC_WARNING(C,M)
#endif

#ifndef CM_COMMON_MACROS_H
#define CM_COMMON_MACROS_H

#ifndef NEW_CM_RT
#define NEW_CM_RT  // Defined for new CM Runtime APIs
#endif

#ifndef AUTO_CM_MODE_SET
#if defined(__CM)
    /// Defined these macros for the Intel CM Compiler.
#define _GENX_MAIN_ __attribute__((genx_main))
#define _GENX_ __attribute__((genx))
#define _GENX_STACKCALL_ __attribute__((genx_stackcall))
#define _CM_INPUT_ __attribute__((cm_input))
#define _CM_OUTPUT_ __attribute__((cm_output))
#define _CM_INPUT_OUTPUT_ __attribute__(cm_input_output)
#define _CM_OUTPUT_INPUT_ __attribute__(cm_input_output)
#define _CM_CALLABLE_ __attribute__((cm_callable))
#else
    /// Defined these macros for MSVC and GCC.
#define CM_GENX
#define _GENX_MAIN_
#define _GENX_
#define _GENX_STACKCALL_
#define _CM_INPUT_
#define _CM_OUTPUT_
#define _CM_INPUT_OUTPUT_
#define _CM_OUTPUT_INPUT_
#define _CM_CALLABLE_
#endif /* __CM */
#define AUTO_CM_MODE_SET
#endif /* AUTO_CM_MODE_SET */

#ifndef CM_NOINLINE
#define CM_NOINLINE __attribute__((noinline)) 
#endif

#ifndef CM_INLINE
#define CM_INLINE inline __attribute__((always_inline))
#endif

#ifndef CM_API
#if defined(NEW_CM_RT) && defined(LIBCM_TEST_EXPORTS)
#define CM_API __attribute__((visibility("default")))
#elif defined(NEW_CM_RT)
#define CM_API
#else
#define CM_API
#endif 
#endif /* CM_API */

#ifndef CMRT_LIBCM_API
#if defined(NEW_CM_RT) && defined(LIBCM_TEST_EXPORTS)
#define CMRT_LIBCM_API __attribute__((visibility("default")))
#elif defined(NEW_CM_RT)
#define CMRT_LIBCM_API __attribute__((visibility("default")))
#else
#define CMRT_LIBCM_API
#endif 
#endif /* CMRT_LIBCM_API */

#define CM_CHK_RESULT(cm_call)                                  \
do {                                                            \
    int result = cm_call;                                       \
    if (result != CM_SUCCESS) {                                 \
        fprintf(stderr, "Invalid CM call at %s:%d. Error %d: ", \
        __FILE__, __LINE__, result);                            \
        fprintf(stderr, ".\n");                                 \
        exit(EXIT_FAILURE);                                     \
    }                                                           \
} while(false)


#endif

template <typename T, uint R, uint C> class matrix;
template <typename T, uint R, uint C> class matrix_ref;
template <typename T, uint SZ> class vector;
template <typename T, uint SZ> class vector_ref;

namespace cm {
  template<typename ty>
  struct pointer_traits
  {
    enum { dim = 0 };
    typedef ty tail_pointee_type;
  };

  template<typename ty, int n>
  struct pointer_traits<ty [n]>
  {
    enum { dim = pointer_traits<ty>::dim + 1 };
    typedef typename pointer_traits<ty>::tail_pointee_type tail_pointee_type;
  };

  template<typename ty>
  struct pointer_traits<ty *>
  {
    enum { dim = pointer_traits<ty>::dim + 1 };
    typedef typename pointer_traits<ty>::tail_pointee_type tail_pointee_type;
  };

};

/* Basic stream. Non template class. */
class basic_stream {
public:
    virtual int extract_data(void *buf, uint size = 0xffffffff) = 0;  /* extract datas to CmEmulSys::iobuffer */
    virtual void* get_addr(uint i) = 0;                         /* return address of data element */
    virtual void* get_addr_data() = 0;
    virtual void* get_addr_obj() = 0;                         /* return address of this */
    virtual uint get_size_of_element() const = 0;         /* return size of one element */
    virtual uint get_number_of_elements() const = 0;      /* return number of elements */
    virtual uint get_size_data() const = 0;
    virtual uint get_size_object() const =0;
};

// stream
template <typename T, uint SZ>
class stream: public basic_stream {
        static const bool type_conformable = cmtype<T>::value;
public:
        typedef  T _Type;

        CM_INLINE int n_elems() const { return SZ; }

        virtual T get(uint i) const = 0; // call to this virtual function won't appear in IL0
        virtual T& getref(uint i) = 0; // call to this virtual function won't appear in IL0
        virtual void* get_addr(uint i) = 0; // call to this virtual function won't appear in IL0
        virtual void* get_addr_data() = 0;
        virtual void* get_addr_obj() =0;
        int extract_data(void *buf,uint size = 0xffffffff);
        virtual uint get_size_of_element() const { return sizeof(T);};
        virtual uint get_number_of_elements() const {return SZ;};
        virtual uint get_size_data() const = 0;
        virtual uint get_size_object() const =0;

        /* merge */
        CM_NOINLINE void merge(const T x, const uint c);
        template <typename T1>  void CM_NOINLINE merge(const stream<T1,SZ> &x, const uint c);
        template <typename T1, typename T2> CM_NOINLINE void merge(const stream<T1,SZ> &x, const stream<T2,SZ> &c);
        template <typename T1> CM_NOINLINE void merge(const T x, const stream<T1,SZ> &c);
        template <typename T1, typename T2> CM_NOINLINE void merge(const stream<T1,SZ>& x, const stream<T2,SZ>& y, const uint c);
        template <typename T1> CM_NOINLINE void merge(const T x,              const stream<T1,SZ>& y, const uint c);
        template <typename T1> CM_NOINLINE  void merge(const stream<T1,SZ>& x, const T y,              const uint c);
        CM_NOINLINE void merge(const T x,              const T y,              const uint c);
        template <typename T1, typename T2, typename T3> CM_NOINLINE void merge(const stream<T1,SZ>& x, const stream<T2,SZ>& y, const stream<T3,SZ>& c);
        template <typename T1, typename T2>  CM_NOINLINE void merge(const T x,              const stream<T1,SZ>& y, const stream<T2,SZ>& c);
        template <typename T1, typename T2> CM_NOINLINE void merge(const stream<T1,SZ>& x, const T y,              const stream<T2,SZ>& c);
        template <typename T1> CM_NOINLINE void merge(const T x,              const T y,              const stream<T1,SZ>& c);

        // any,all
        CM_NOINLINE ushort any( void ) const;
        CM_NOINLINE ushort all( void ) const;

        // for debug
#ifdef CM_DEBUG
        virtual std::string type_name() const = 0;
        virtual std::string obj_name() const = 0;
#endif /* CM_DEBUG */
};

// matrix
template <typename T, uint R, uint C>
class matrix : public stream<T,R*C> {
public:
        template <typename T1, uint R1, uint C1> friend class matrix;
        template <typename T1, uint R1, uint C1> friend class matrix_ref;
        template <typename T1, uint SZ1> friend class vector;
        template <typename T1, uint SZ1> friend class vector_ref;
    
        enum { SZ = R*C };
        enum { ROWS=R, COLS=C, ELEMS=R*C };

        CM_INLINE int n_rows() const { return R; }
        CM_INLINE int n_cols() const { return C; }

        template <uint REP> CM_INLINE
        const vector<T, R*C*REP> replicate(OFFSET ioff=0, OFFSET joff=0)
        { return genx_select<REP,0,R*C,1>(ioff, joff); };
        template <uint REP, uint W> CM_INLINE
        const vector<T, W*REP> replicate(OFFSET ioff=0, OFFSET joff=0)
        { return genx_select<REP,0,W,1>(ioff, joff); };
        template <uint REP, uint VS, uint W> CM_INLINE
        const vector<T, W*REP> replicate(OFFSET ioff=0, OFFSET joff=0)
        { return genx_select<REP,VS,W,1>(ioff, joff); };
        template <uint REP, uint VS, uint W, uint HS> CM_INLINE
        const vector<T, W*REP> replicate(OFFSET ioff=0, OFFSET joff=0)
        { return genx_select<REP,VS,W,HS>(ioff, joff); };
        
        virtual T get(uint i) const { return data[i]; }
        virtual T& getref(uint i) { return data[i]; }
        virtual void* get_addr(uint i) { return &data[i]; }
        virtual void* get_addr_data() { 
                return &data[0]; 
        }
        virtual void* get_addr_obj() { return this; }
        virtual uint get_size_data() const { 
                return sizeof(data); 
        }
        virtual uint get_size_object() const { return sizeof(*this); }
        
        CM_NOINLINE T operator () (OFFSET i, OFFSET j) const { 
            assert(i < R && j < C);
            return get(i*C+j);
        }

        CM_NOINLINE T& operator () (OFFSET i, OFFSET j) {
            assert(i < R && j < C);
            return data[i*C+j];
        }

        template <typename T1, uint R1, uint C1>
        class inner_hack {
            matrix* m_t;
            OFFSET _i;
        public:
            inner_hack(matrix* m, OFFSET i):m_t(m){_i=i;}
            CM_NOINLINE const T1 operator[](OFFSET j) const{return (*m_t)(_i,j);}
            CM_NOINLINE T1& operator[](OFFSET j){return (*m_t)(_i,j);}
        
        };

        CM_NOINLINE inner_hack<T,R,C> operator [] (OFFSET i) const { 
            return inner_hack<T,R,C>(this,i);
        }

        CM_NOINLINE inner_hack<T,R,C> operator [] (OFFSET i) {
            return inner_hack<T,R,C>(this,i);
        }

        // constructors
        CM_NOINLINE matrix();
        // CM_NOINLINE matrix(void *ptr);                /* constructor for preload datas from URB */
        template <typename T2> CM_NOINLINE matrix(const T2 initArray[]); // constructor with array initializer
        CM_NOINLINE matrix(const matrix<T,R,C>& src); // copy constructor
        template <typename T2> CM_NOINLINE matrix(const T2& src);
        template <typename T2, uint R2, uint C2> CM_NOINLINE matrix(const matrix<T2,R2,C2>& src, const uint sat = 0 ); 
        template <typename T2, uint R2, uint C2> CM_NOINLINE matrix(const matrix_ref<T2,R2,C2>& src, const uint sat = 0);
        template <typename T2> CM_NOINLINE matrix(const vector<T2,R*C>& src) 
        { new (this) matrix<T,R,C>((matrix<T2,1,R*C>&)src); }

        template <typename T2> CM_NOINLINE matrix(const vector_ref<T2,R*C>& src) 
        { new (this) matrix<T,R,C>((matrix_ref<T2,1,R*C>&)src); }

        //operator =
        CM_NOINLINE matrix<T,R,C>& operator = (const matrix<T,R,C>& src); // assignment operator
        template <typename T2> CM_NOINLINE matrix<T,R,C>& operator = (const T2 src);
        template <typename T2, uint R2, uint C2> CM_NOINLINE matrix<T,R,C>& operator = (const matrix<T2,R2,C2>& src);
        template <typename T2, uint R2, uint C2> CM_NOINLINE matrix<T,R,C>& operator = (const matrix_ref<T2,R2,C2>& src);
        template <typename T2> CM_NOINLINE matrix<T,R,C>& operator = (const vector<T2,R*C>& src) { return this->operator=((const matrix<T2,1,R*C>&)src); };
        template <typename T2> CM_NOINLINE matrix<T,R,C>& operator = (const vector_ref<T2,R*C>& src) { return this->operator=((const matrix_ref<T2,1,R*C>&)src); };

        //selects
        template <typename T2> CM_NOINLINE vector_ref<T2,R*C*sizeof(T)/sizeof(T2)> format(); // to vector 
        template <typename T2, uint R2, uint C2> CM_NOINLINE matrix_ref<T2,R2,C2> format();    // to matrix R2xC2
        template <typename T2> CM_NOINLINE const vector_ref<T2,R*C*sizeof(T)/sizeof(T2)> format() const; // to vector 
        template <typename T2, uint R2, uint C2> CM_NOINLINE const matrix_ref<T2,R2,C2> format() const;    // to matrix R2xC2
        vector_ref<T, C> CM_NOINLINE row(OFFSET i);
        matrix_ref<T,R,1> CM_NOINLINE column(OFFSET i);
        template <uint R2, uint RS, uint C2, uint CS> CM_NOINLINE matrix_ref<T,R2,C2> select(OFFSET ioff=0, OFFSET joff=0);
        template <uint R2, uint RS, uint C2, uint CS> CM_NOINLINE const matrix_ref<T,R2,C2> select(OFFSET ioff=0, OFFSET joff=0) const;

        //1D iselect
        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector<T2,WD>& index);
        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector_ref<T2,WD>& index);
#if _MSC_VER >= 1700
        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector<T2,WD>& index, std::true_type);
        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector<T2,WD>& index, std::false_type);
        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector_ref<T2,WD>& index, std::true_type);
        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector_ref<T2,WD>& index, std::false_type);
#endif

        //2D iselect
        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector<T2,WD>& index_x, const vector<T2,WD>& index_y);

        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector<T2,WD>& index_x, const vector_ref<T2,WD>& index_y);

        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector_ref<T2,WD>& index_x, const vector<T2,WD>& index_y);

        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector_ref<T2,WD>& index_x, const vector_ref<T2,WD>& index_y);
        //end of iselect

        template <uint R2, uint VS, uint WD, uint HS> CM_NOINLINE const vector<T, R2*WD> genx_select(OFFSET ioff=0, OFFSET joff=0);
        
        matrix_ref<T, R, C> CM_NOINLINE select_all();
        const matrix_ref<T, R, C> CM_NOINLINE select_all() const;
     
        // operators +=, -=, ...
        #define declare_operation(OP) \
        template <typename T2> CM_NOINLINE matrix<T,R,C>& operator OP (const T2 x);\
        template <typename T2, uint R2, uint C2> CM_NOINLINE matrix<T,R,C>& operator OP (const matrix<T2,R2,C2>& x);\
        template <typename T2, uint R2, uint C2> CM_NOINLINE matrix<T,R,C>& operator OP (const matrix_ref<T2,R2,C2>& x);\
        template <typename T2> CM_NOINLINE matrix<T,R,C>& operator OP (const vector<T2,SZ>& x);\
        template <typename T2> CM_NOINLINE matrix<T,R,C>& operator OP (const vector_ref<T2,SZ>& x);\

        declare_operation(+=)     // +=
        declare_operation(-=)     // -=
        declare_operation(*=)     // *= 
        declare_operation(/=)     // /=
        declare_operation(%=)     // %=
        declare_operation(&=)     // &=
        declare_operation(|=)     // |=
        declare_operation(^=)     // ^=
        declare_operation(>>=)     // >>=
        declare_operation(<<=)     // <<=
        #undef declare_operation

        // for debug
        uint id() const { return number; }

#ifdef CM_DEBUG
        virtual std::string type_name() const {std::stringstream ss; ss << "M<" << typeid(T).name() << "," << R << "," << C << ">"; return ss.str();}
        virtual std::string obj_name() const {std::stringstream ss; ss << typeid(T).name() << "[" << /*id()*/SZ << "]"; return ss.str();}
#endif /* CM_DEBUG */

private: 
  
        T data[SZ];
        CM_NOINLINE T operator () (uint i) const { 
            assert(i < SZ);
            return get(i);
        }

        CM_NOINLINE T& operator () (uint i) { 
            assert(i < SZ);
            return data[i];
        }
/*
        CM_NOINLINE T operator [] (uint i) const { 
            assert(i < SZ);
            return get(i);
        }

        CM_NOINLINE T& operator [] (uint i) { 
            assert(i < SZ);
            return data[i];
        }
*/
        // for debug
        uint number;          
};

// matrix_ref
template <typename T, uint R, uint C>
class matrix_ref : public stream<T,R*C> {
public:
        template <typename T1, uint R1, uint C1> friend class matrix;
        template <typename T1, uint R1, uint C1> friend class matrix_ref;
        template <typename T1, uint SZ1> friend class vector;
        template <typename T1, uint SZ1> friend class vector_ref;
    
        enum { SZ = R*C };
        enum { ROWS=R, COLS=C, ELEMS=R*C };

        CM_INLINE int n_rows() const { return R; }
        CM_INLINE int n_cols() const { return C; }

        template <uint REP> CM_INLINE
        const vector<T, R*C*REP> replicate(OFFSET ioff=0, OFFSET joff=0)
        { return genx_select<REP,0,R*C,1>(ioff, joff); };
        template <uint REP, uint W> CM_INLINE
        const vector<T, W*REP> replicate(OFFSET ioff=0, OFFSET joff=0)
        { return genx_select<REP,0,W,1>(ioff, joff); };
        template <uint REP, uint VS, uint W> CM_INLINE
        const vector<T, W*REP> replicate(OFFSET ioff=0, OFFSET joff=0)
        { return genx_select<REP,VS,W,1>(ioff, joff); };
        template <uint REP, uint VS, uint W, uint HS> CM_INLINE
        const vector<T, W*REP> replicate(OFFSET ioff=0, OFFSET joff=0)
        { return genx_select<REP,VS,W,HS>(ioff, joff); };
        
        virtual T get(uint i) const { return *data[i]; }
        virtual T& getref(uint i) { return *data[i]; }

        virtual void* get_addr(uint i) { return data[i]; }
        virtual void* get_addr_data() { 
                return &data[0]; 
        }
        virtual void* get_addr_obj() { return this; }
        void set_elem_ref(uint i, T* ptr) { data[i] = ptr; }
        virtual uint get_size_data() const { 
                return sizeof(*data); 
        }
        virtual uint get_size_object() const { return sizeof(*this); }

        CM_NOINLINE T operator () (OFFSET i, OFFSET j) const  {
            assert(i < R && j < C);
            return get(i*C+j);
        }

        CM_NOINLINE T& operator () (OFFSET i, OFFSET j) { 
            assert(i < R && j < C);
            return *data[i*C+j];
        }

        template <typename T1, uint R1, uint C1>
        class inner_hack {
            matrix_ref* m_t;
            OFFSET _i;
        public:
            inner_hack(matrix_ref* m, OFFSET i):m_t(m){_i=i;}
            CM_NOINLINE const T1 operator[](OFFSET j) const{return (*m_t)(_i,j);}
            CM_NOINLINE T1& operator[](OFFSET j){return (*m_t)(_i,j);}
        
        };

        CM_NOINLINE inner_hack<T,R,C> operator [] (OFFSET i) const { 
            return inner_hack<T,R,C>(this,i);
        }

        CM_NOINLINE inner_hack<T,R,C> operator [] (OFFSET i) {
            return inner_hack<T,R,C>(this,i);
        }

        // constructors
        CM_NOINLINE matrix_ref(const matrix_ref<T,R,C>& src); // copy constructor
        CM_NOINLINE matrix_ref(matrix<T,R,C>& src); // Point reference on matrix 
#if defined(__CLANG_CM)
        CM_NOINLINE matrix_ref(const vector<T,R*C>& src); 
        CM_NOINLINE matrix_ref(const vector_ref<T,R*C>& src); 
#endif

        //operator =
        CM_NOINLINE matrix_ref<T,R,C>& operator = (const matrix<T,R,C>& src); // assignment operator
        CM_NOINLINE matrix_ref<T,R,C>& operator = (const matrix_ref<T,R,C>& src); 
        template <typename T2> CM_NOINLINE matrix_ref<T,R,C>& operator = (const T2 src);
        template <typename T2, uint R2, uint C2> CM_NOINLINE matrix_ref<T,R,C>& operator = (const matrix<T2,R2,C2>& src);
        template <typename T2, uint R2, uint C2> CM_NOINLINE matrix_ref<T,R,C>& operator = (const matrix_ref<T2,R2,C2>& src);
        template <typename T2> CM_NOINLINE matrix_ref<T,R,C>& operator = (const vector<T2,R*C>& src) { return this->operator=((const matrix<T2,1,R*C>&)src); };
        template <typename T2> CM_NOINLINE matrix_ref<T,R,C>& operator = (const vector_ref<T2,R*C>& src) { return this->operator=((const matrix_ref<T2,1,R*C>&)src); };

        // operators +=, -=, ...
        #define declare_operation(OP) \
        template <typename T2> CM_NOINLINE matrix_ref<T,R,C>& operator OP (const T2 x);\
        template <typename T2, uint R2, uint C2> CM_NOINLINE matrix_ref<T,R,C>& operator OP (const matrix<T2,R2,C2>& x);\
        template <typename T2, uint R2, uint C2> CM_NOINLINE matrix_ref<T,R,C>& operator OP (const matrix_ref<T2,R2,C2>& x);\
        template <typename T2> CM_NOINLINE matrix_ref<T,R,C>& operator OP (const vector<T2,SZ>& x);\
        template <typename T2> CM_NOINLINE matrix_ref<T,R,C>& operator OP (const vector_ref<T2,SZ>& x);\

        declare_operation(+=)     // +=
        declare_operation(-=)     // -=
        declare_operation(*=)     // *= 
        declare_operation(/=)     // /=
        declare_operation(%=)     // %=
        declare_operation(&=)     // &=
        declare_operation(|=)     // |=
        declare_operation(^=)     // ^=
        declare_operation(>>=)     // >>=
        declare_operation(<<=)     // <<=
        #undef declare_operation

        bool is_contiguous() const;
        bool is_contiguous(const uint start, const uint end) const;
        //selects
        template <typename T2> CM_NOINLINE vector_ref<T2,R*C*sizeof(T)/sizeof(T2)> format(); // to vector 
        template <typename T2, uint R2, uint C2> CM_NOINLINE matrix_ref<T2,R2,C2> format();    // to matrix R2xC2
        template <typename T2> CM_NOINLINE const vector_ref<T2,R*C*sizeof(T)/sizeof(T2)> format() const; // to vector 
        template <typename T2, uint R2, uint C2> CM_NOINLINE const matrix_ref<T2,R2,C2> format() const;    // to matrix R2xC2
        vector_ref<T, C> CM_NOINLINE row(OFFSET i);
        matrix_ref<T,R,1> CM_NOINLINE column(OFFSET i);
        template <uint R2, uint RS, uint C2, uint CS> CM_NOINLINE matrix_ref<T,R2,C2> select(OFFSET ioff=0, OFFSET joff=0);
        template <uint R2, uint RS, uint C2, uint CS> CM_NOINLINE const matrix_ref<T,R2,C2> select(OFFSET ioff=0, OFFSET joff=0) const;
        template <uint R2, uint VS, uint WD, uint HS> CM_NOINLINE const vector<T, R2*WD> genx_select(OFFSET ioff=0, OFFSET joff=0);

        //1D iselect
        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector<T2,WD>& index);
        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector_ref<T2,WD>& index);
#if _MSC_VER >= 1700
        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector<T2,WD>& index, std::true_type);
        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector<T2,WD>& index, std::false_type);
        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector_ref<T2,WD>& index, std::true_type);
        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector_ref<T2,WD>& index, std::false_type);
#endif
        

        //2D iselect
        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector<T2,WD>& index_x, const vector<T2,WD>& index_y);

        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector<T2,WD>& index_x, const vector_ref<T2,WD>& index_y);

        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector_ref<T2,WD>& index_x, const vector<T2,WD>& index_y);

        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector_ref<T2,WD>& index_x, const vector_ref<T2,WD>& index_y);
        //end of 2D iselect

        // for debug
        uint id() const { return number; }

        T* dummy() { return &_dummy; }

#ifdef CM_DEBUG
        virtual std::string type_name() const {std::stringstream ss; ss << "M<" << typeid(T).name() << "," << R << "," << C << ">"; return ss.str();}
        virtual std::string obj_name() const {std::stringstream ss; ss << typeid(T).name() << "[" << id() << "]"; return ss.str();}
#endif /* CM_DEBUG */

private:
        matrix_ref(const uint id) : number(id) {  } // id for debug 
        T* data[SZ];

        T _dummy;

        CM_NOINLINE T operator () (uint i) const {
            assert(i < SZ);
            return get(i);
        }

        CM_NOINLINE T& operator () (uint i) {
            assert(i < SZ);
            return *data[i];
        }
/*
        CM_NOINLINE T operator [] (uint i) const {
            assert(i < SZ);
            return get(i);
        }

        CM_NOINLINE T& operator [] (uint i) {
            assert(i < SZ);
            return *data[i];
        }
*/
        // for debug
        uint number;
};

// vector
template <typename T, uint SZ>
class vector : public matrix<T,1,SZ> {
        void assign(const stream<T, SZ> &src); //special member-function, not for CM language
public:
        template <typename T1, uint R1, uint C1> friend class matrix;
        template <typename T1, uint R1, uint C1> friend class matrix_ref;
        template <typename T1, uint SZ1> friend class vector_ref;
        template <typename T1, uint SZ1> friend class stream;

        template <uint REP> CM_INLINE
        vector<T,SZ*REP> replicate(OFFSET joff=0)
        {return ((matrix<T,1,SZ> *)this)->template replicate<REP>(0, joff);};
        template <uint REP, uint W> CM_INLINE
        vector<T,W*REP> replicate(OFFSET joff=0)
        {return ((matrix<T,1,SZ> *)this)->template replicate<REP,W>(0, joff);};
        template <uint REP, uint VS, uint W> CM_INLINE
        vector<T,W*REP> replicate(OFFSET joff=0)
        {return ((matrix<T,1,SZ> *)this)->template replicate<REP,VS,W>(0, joff);};
        template <uint REP, uint VS, uint W, uint HS> CM_INLINE
        vector<T,W*REP> replicate(OFFSET joff=0)
        {return ((matrix<T,1,SZ> *)this)->template replicate<REP,VS,W,HS>(0, joff);};    

        // constructors: call base versions of constructors
        CM_NOINLINE vector() : CMRT_UMD::matrix<T,1,SZ>() {}
        template <typename T2> CM_NOINLINE vector(const T2 initArray[]) : CMRT_UMD::matrix<T,1,SZ>(initArray) {
            // constructor with array initializer
            CM_STATIC_ERROR(!std::is_floating_point<T2>::value, "floating point array initialization values are not supported");
        } 
        CM_NOINLINE vector(const vector<T,SZ>& src) : CMRT_UMD::matrix<T,1,SZ>((const matrix<T,1,SZ>&)src) {} // copy constructor
        template <typename T2> CM_NOINLINE vector(const T2& src) : CMRT_UMD::matrix<T,1,SZ>(src) {} 
        template <typename T2, uint R2, uint C2> CM_NOINLINE vector(const matrix<T2,R2,C2>& src, uint sat = 0) : CMRT_UMD::matrix<T,1,SZ>(src, sat) {}
        template <typename T2, uint R2, uint C2> CM_NOINLINE vector(const matrix_ref<T2,R2,C2>& src, uint sat = 0) : CMRT_UMD::matrix<T,1,SZ>(src, sat) {}
        template <typename T2> CM_NOINLINE vector(const vector<T2,SZ>& src) : CMRT_UMD::matrix<T,1,SZ>(src) {}
        template <typename T2> CM_NOINLINE vector(const vector_ref<T2,SZ>& src) : CMRT_UMD::matrix<T,1,SZ>(src) {}

        
        CM_NOINLINE T operator () (OFFSET i) const { 
            assert(i < SZ);
            return matrix<T,1,SZ>::get(i);
        }

        CM_NOINLINE T& operator () (OFFSET i) { 
            assert(i < SZ);
            return matrix<T,1,SZ>::data[i];
        }

        CM_NOINLINE T operator [] (OFFSET i) const { 
            assert(i < SZ);
            return matrix<T,1,SZ>::get(i);
        }

        CM_NOINLINE T& operator [] (OFFSET i) { 
            assert(i < SZ);
            return matrix<T,1,SZ>::data[i];
        }

        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> operator () (const vector<T2,WD>& index) {
            return matrix<T,1,SZ>::template iselect<T2,WD>(index);
        }

        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> operator () (const vector_ref<T2,WD>& index) {
            return matrix<T,1,SZ>::template iselect<T2,WD>(index);
        }

        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> operator [] (const vector<T2,WD>& index) {
            return matrix<T,1,SZ>::template iselect<T2,WD>(index);
        }

        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> operator [] (const vector_ref<T2,WD>& index) {
            return matrix<T,1,SZ>::template iselect<T2,WD>(index);
        }

        //1D iselect only
        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector<T2,WD>& index) {
            CM_STATIC_WARNING((std::is_unsigned<T2>::value), "iselect index vector element type must be unsigned");
            return matrix<T,1,SZ>::template iselect<T2,WD>(index);
        }

        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector_ref<T2,WD>& index) {
            CM_STATIC_WARNING((std::is_unsigned<T2>::value), "iselect index vector element type must be unsigned");
            return matrix<T,1,SZ>::template iselect<T2,WD>(index);
        }
        //end of iselect

        //operator =: call base versions of operator =
        CM_NOINLINE vector<T,SZ>& operator = (const vector<T,SZ>& src) { ((matrix<T,1,SZ>*)this)->operator=(src); return *this; } // assignment operator
        template <typename T2> CM_NOINLINE vector<T,SZ>& operator = (const T2 src) {
            ((matrix<T,1,SZ>*)this)->operator=(src); return *this;
        }
        template <typename T2, uint R2, uint C2> CM_NOINLINE vector<T,SZ>& operator = (const matrix<T2,R2,C2>& src) { ((matrix<T,1,SZ>*)this)->operator=(src); return *this; }
        template <typename T2, uint R2, uint C2> CM_NOINLINE vector<T,SZ>& operator = (const matrix_ref<T2,R2,C2>& src) { ((matrix<T,1,SZ>*)this)->operator=(src); return *this; }
        template <typename T2> CM_NOINLINE vector<T,SZ>& operator = (const vector<T2,SZ>& src) { ((matrix<T,1,SZ>*)this)->operator=(src); return *this; }
        template <typename T2> CM_NOINLINE vector<T,SZ>& operator = (const vector_ref<T2,SZ>& src) { ((matrix<T,1,SZ>*)this)->operator=(src); return *this; }

        //vector select
        template <uint C, uint CS> CM_NOINLINE const vector_ref<T,C> select(OFFSET joff=0) const {
            CM_STATIC_ERROR(((SZ) >= (C)), "select size is greater than source vector size");
            CM_STATIC_ERROR(((SZ) >= ((C-1)*(CS))+1) || (CS == 1), "select range exceeds source vector bounds");
            return ((matrix<T,1,SZ>*)this)->template select<1,1,C,CS>(0,joff);
        }
        template <uint C, uint CS> CM_NOINLINE vector_ref<T,C> select(OFFSET joff=0) {
            CM_STATIC_ERROR(((SZ) >= (C)), "select size is greater than source vector size");
            CM_STATIC_ERROR(((SZ) >= ((C-1)*(CS))+1) || (CS == 1), "select range exceeds source vector bounds");
            return ((matrix<T,1,SZ>*)this)->template select<1,1,C,CS>(0,joff);
        }

        //vector genx_select
        template <uint R, uint VS, uint WD, uint HS> CM_NOINLINE const vector<T, R*WD> genx_select(OFFSET joff=0) {
            CM_STATIC_ERROR((!std::is_same<T, double>::value), "genx_select is not supported for vectors with element type 'double'");
            CM_STATIC_ERROR(((SZ) >= (WD)), "genx_select width is greater than source vector size");
            return ((matrix<T,1,SZ>*)this)->template genx_select<R,VS,WD,HS>(0, joff);
        }
};

// vector_ref
template <typename T, uint SZ>
class vector_ref : public matrix_ref<T,1,SZ> {
public:
        template <typename T1, uint R1, uint C1> friend class matrix;
        template <typename T1, uint R1, uint C1> friend class matrix_ref;
        template <typename T1, uint SZ1> friend class vector;
        template <typename T1, uint SZ1> friend class vector_ref;

        template <uint REP> CM_INLINE
        vector<T,SZ*REP> replicate(OFFSET joff = 0)
        {return ((matrix_ref<T,1,SZ> *)this)->template replicate<REP>(0, joff);};
        template <uint REP, uint W> CM_INLINE
        vector<T,W*REP> replicate(OFFSET joff = 0)
        {return ((matrix_ref<T,1,SZ> *)this)->template replicate<REP,W>(0, joff);};
        template <uint REP, uint VS, uint W> CM_INLINE
        vector<T,W*REP> replicate(OFFSET joff = 0)
        {return ((matrix_ref<T,1,SZ> *)this)->template replicate<REP,VS,W>(0, joff);};
        template <uint REP, uint VS, uint W, uint HS> CM_INLINE
        vector<T,W*REP> replicate(OFFSET joff = 0)
        {return ((matrix_ref<T,1,SZ> *)this)->template replicate<REP,VS,W,HS>(0, joff);};    

        // constructors
        CM_NOINLINE vector_ref(const vector_ref<T,SZ>& src) : CMRT_UMD::matrix_ref<T,1,SZ>((const matrix_ref<T,1,SZ>&)src) {} // copy constructor
        CM_NOINLINE vector_ref(vector<T,SZ>& src) : CMRT_UMD::matrix_ref<T,1,SZ>((matrix<T,1,SZ>&)src) {} //assign vector_ref to vector
        CM_NOINLINE vector_ref(const matrix_ref<T,1,SZ>& src) : CMRT_UMD::matrix_ref<T,1,SZ>(src) {}
        CM_NOINLINE vector_ref(matrix<T,1,SZ>& src) : CMRT_UMD::matrix_ref<T,1,SZ>(src) {}
#if defined(__CLANG_CM)
        CM_NOINLINE vector_ref(matrix<T,1,SZ> src) : CMRT_UMD::matrix_ref<T,1,SZ>(src) {}
        CM_NOINLINE vector_ref(matrix_ref<T,1,SZ>& src) : CMRT_UMD::matrix_ref<T,1,SZ>(src) {}
#endif

        CM_NOINLINE T operator () (OFFSET i) const {
            assert(i < SZ);
            return matrix_ref<T,1,SZ>::get(i);
        }

        CM_NOINLINE T& operator () (OFFSET i) {
            assert(i < SZ);
            return *matrix_ref<T,1,SZ>::data[i];
        }

        CM_NOINLINE T operator [] (OFFSET i) const {
            assert(i < SZ);
            return matrix_ref<T,1,SZ>::get(i);
        }

        CM_NOINLINE T& operator [] (OFFSET i) {
            assert(i < SZ);
            return *matrix_ref<T,1,SZ>::data[i];
        }

        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> operator () (const vector<T2,WD>& index) const{
            return matrix_ref<T,1,SZ>::template iselect<T,WD>(index);
        }

        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> operator () (const vector_ref<T2,WD>& index) const{
            return matrix_ref<T,1,SZ>::template iselect<T,WD>(index);
        }

        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> operator [] (const vector<T2,WD>& index) const{
            return matrix_ref<T,1,SZ>::template iselect<T,WD>(index);
        }

        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> operator [] (const vector_ref<T2,WD>& index) const{
            return matrix_ref<T,1,SZ>::template iselect<T,WD>(index);
        }

        //1D iselect only
        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector<T2,WD >& index) {
            CM_STATIC_WARNING((std::is_unsigned<T2>::value), "iselect index vector element type must be unsigned");
            return matrix_ref<T,1,SZ>::template iselect<T2,WD>(index);
        }

        template <typename T2, uint WD> CM_NOINLINE vector<T,WD> iselect(const vector_ref<T2,WD>& index) {
            CM_STATIC_WARNING((std::is_unsigned<T2>::value), "iselect index vector element type must be unsigned");
            return matrix_ref<T,1,SZ>::template iselect<T2,WD>(index);
        }
        //end of iselect


        //operator =: call base versions of operator =
        CM_NOINLINE vector_ref<T,SZ>& operator = (const vector_ref<T,SZ>& src) { ((matrix_ref<T,1,SZ>*)this)->operator=(src); return *this; } //assignment operator
        template <typename T2> CM_NOINLINE vector_ref<T,SZ>& operator = (const T2 src) { ((matrix_ref<T,1,SZ>*)this)->operator=(src); return *this; }
        template <typename T2, uint R2, uint C2> CM_NOINLINE vector_ref<T,SZ>& operator = (const matrix<T2,R2,C2>& src){ ((matrix_ref<T,1,SZ>*)this)->operator=(src); return *this; }
        template <typename T2, uint R2, uint C2> CM_NOINLINE vector_ref<T,SZ>& operator = (const matrix_ref<T2,R2,C2>& src){ ((matrix_ref<T,1,SZ>*)this)->operator=(src); return *this; }
        template <typename T2> CM_NOINLINE vector_ref<T,SZ>& operator = (const vector<T2,SZ>& src) { ((matrix_ref<T,1,SZ>*)this)->operator=(src); return *this; }
        template <typename T2> CM_NOINLINE vector_ref<T,SZ>& operator = (const vector_ref<T2,SZ>& src) { ((matrix_ref<T,1,SZ>*)this)->operator=(src); return *this; }

        //vector_ref select
        template <uint C, uint CS> CM_NOINLINE vector_ref<T,C> select(OFFSET joff=0) {return ((matrix_ref<T,1,SZ>*)this)->template select<1,1,C,CS>(0,joff);} 
        template <uint C, uint CS> CM_NOINLINE const vector_ref<T,C> select(OFFSET joff=0) const {return ((matrix_ref<T,1,SZ>*)this)->template select<1,1,C,CS>(0,joff);} 

        //vector_ref genx_select
        template <uint R, uint VS, uint WD, uint HS> CM_NOINLINE const vector<T, R*WD> genx_select(OFFSET joff=0) {
            CM_STATIC_WARNING((!std::is_same<T, double>::value), "genx_select is not supported for vectors with element type of 'double'");
            CM_STATIC_ERROR(((SZ) >= (WD)), "genx_select width is greater than source vector size");
            return ((matrix_ref<T,1,SZ>*)this)->template genx_select<R,VS,WD,HS>(0, joff);
        };

private:
        CM_NOINLINE vector_ref(const uint id) : CMRT_UMD::matrix_ref<T,1,SZ>(id) {}
};

#ifdef CM_DEBUG
template <typename T, uint R, uint C>
std::ostream& operator << (std::ostream &out, const matrix<T,R,C>& m)
{
        out << "---" << m.obj_name() << ":" << std::endl; 
        for (uint i=0; i<R; ++i) {
                for (uint j=0; j<C; ++j) {
                        out << m(i,j) << " ";
                }
                out << std::endl;
        }
        return out;
}
template <uint R, uint C>
std::ostream& operator << (std::ostream &out, const matrix<char,R,C>& m)
{
        out << "---" << m.obj_name() << ":" << std::endl; 
        for (uint i=0; i<R; ++i) {
                for (uint j=0; j<C; ++j) {
                        out << (int)m(i,j) << " ";
                }
                out << std::endl;
        }
        return out;
}
#endif /* CM_DEBUG */

/*******************************************************************
/
/                         stream
/
*******************************************************************/

template <typename T, uint SZ>
int stream<T,SZ>::extract_data(void *buf, uint size)
{
    uint i;
  
    assert(SZ*sizeof(T) <= size);

    for (i=0; i< SZ; i++) {
        ((T*)buf)[i] = get(i);
    }

    return SZ*sizeof(T);
}


#define SIMDCF_WRAPPER(X, SZ, i) X

#define SIMDCF_ELEMENT_SKIP(i)


/*
 *  merge
 */
template <typename T, uint SZ>
void stream<T,SZ>::merge(const T x, const uint c)
{
    T* p; 
    for (uint i=0; i<SZ; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            if (((c >> i) & 1) != 0) {
                p = (T*)this->get_addr(i);
                *p = x;
            }
    }
}

template <typename T, uint SZ>
template <typename T1>
void stream<T,SZ>::merge(const stream<T1,SZ> &x, const uint c)
{
    vector<T1, SZ> in_x; in_x.assign(x);
    T* p; 
    for (uint i=0; i<SZ; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            if (((c >> i) & 1) != 0) {
                p = (T*)this->get_addr(i);
                *p = in_x.get(i);
            }
    }
}


template <typename T, uint SZ>
template <typename T1, typename T2>
void stream<T,SZ>::merge(const stream<T1,SZ> &x, const stream<T2,SZ> &c) 
{
    vector<T1, SZ> in_x; in_x.assign(x);
    vector<T2, SZ> in_c; in_c.assign(c);
    T* p; 
    for (uint i=0; i<SZ; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            if ((in_c.get(i) & 1) != 0) {
                p = (T*)this->get_addr(i);
                *p = (T) in_x.get(i);
            }
    }
}

template <typename T, uint SZ>
template <typename T1>
void stream<T,SZ>::merge(const T x, const stream<T1,SZ> &c) 
{
    vector<T1, SZ> in_c; in_c.assign(c);
    T* p; 
    for (uint i=0; i<SZ; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            if ((in_c.get(i) & 1) != 0) {
                p = (T*)this->get_addr(i);
                *p = x;
            }
    }
}

template <typename T, uint SZ>
template <typename T1, typename T2>
void stream<T,SZ>::merge(const stream<T1,SZ>& x, const stream<T2,SZ>& y,
                         const uint c)
{
    vector<T1, SZ> in_x; in_x.assign(x);
    vector<T2, SZ> in_y; in_y.assign(y);
    T* p; 
    for (uint i=0; i<SZ; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            p = (T*)this->get_addr(i);
            if (((c >> i) & 1) != 0) {
                *p = in_x.get(i);
            } else {
                *p = in_y.get(i);
            }
    }
}

template <typename T, uint SZ>
template <typename T1>
void stream<T,SZ>::merge(const T x, const stream<T1,SZ>& y, const uint c)
{
    vector<T1, SZ> in_y; in_y.assign(y);
    T* p; 
    for (uint i=0; i<SZ; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            p = (T*)this->get_addr(i);
            if (((c >> i) & 1) != 0) {
                *p = x;
            } else {
                *p = in_y.get(i);
            }
    }
}

template <typename T, uint SZ>
template <typename T1>
void stream<T,SZ>::merge(const stream<T1,SZ>& x, const T y, const uint c) 
{
    vector<T1, SZ> in_x; in_x.assign(x); 
    T* p; 
    for (uint i=0; i<SZ; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            p = (T*)this->get_addr(i);
            if (((c >> i) & 1) != 0) {
                *p = in_x.get(i);
            } else {
                *p = y;
            }
    }
}

template <typename T, uint SZ>
void stream<T,SZ>::merge(const T x, const T y, const uint c) 
{
    T* p; 
    for (uint i=0; i<SZ; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            p = (T*)this->get_addr(i);
            if (((c >> i) & 1) != 0) {
                *p = x;
            } else {
                *p = y;
            }
    }
}

template <typename T, uint SZ>
template <typename T1, typename T2, typename T3>
void stream<T,SZ>::merge(const stream<T1,SZ>& x, const stream<T2,SZ>& y, const stream<T3,SZ>& c)
{
    vector<T1, SZ> in_x; in_x.assign(x);
    vector<T2, SZ> in_y; in_y.assign(y);
    vector<T3, SZ> in_c; in_c.assign(c);

    T* p; 
    for (uint i=0; i<SZ; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            p = (T*)this->get_addr(i);
            if ((in_c.get(i) & 1) != 0) {
                *p = in_x.get(i);
            } else
            {
                *p = in_y.get(i);
            }
    }
}

template <typename T, uint SZ>
template <typename T1, typename T2>
void stream<T,SZ>::merge(const T x, const stream<T1,SZ>& y, const stream<T2,SZ>& c)
{
    vector<T1, SZ> in_y; in_y.assign(y);
    vector<T2, SZ> in_c; in_c.assign(c);
    T* p; 
    for (uint i=0; i<SZ; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            p = (T*)this->get_addr(i);
            if ((in_c.get(i) & 1) != 0) {
                *p = x;
            } else
            {
                *p = in_y.get(i);
            }
    }
}

template <typename T, uint SZ>
template <typename T1, typename T2>
void stream<T,SZ>::merge(const stream<T1,SZ>& x, const T y,
                         const stream<T2,SZ>& c)
{
    vector<T1, SZ> in_x; in_x.assign(x);
    vector<T2, SZ> in_c; in_c.assign(c);
    T* p; 
    for (uint i=0; i<SZ; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            p = (T*)this->get_addr(i);
            if ((in_c.get(i) & 1) != 0) {
                *p = in_x.get(i);
            } else
            {
                *p = y;
            }
    }
}

template <typename T, uint SZ>
template <typename T1>
void stream<T,SZ>::merge(const T x, const T y, const stream<T1,SZ>& c)
{
    vector<T1, SZ> in_c; in_c.assign(c);
    T* p; 
    for (uint i=0; i<SZ; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            p = (T*)this->get_addr(i);
            if ((in_c.get(i) & 1) != 0) {
                *p = x;
            } else
            {
                *p = y;
            }
    }
}


/*******************************************************************
/
/                         matrix
/
*******************************************************************/

/*
 * matrix constructors
 */

// Matrix Initialization with array
template <typename T, uint R, uint C>
template <typename T2>
matrix<T,R,C>::matrix(const T2 initArray[])
{
    CM_STATIC_ERROR(!std::is_floating_point<T2>::value, "floating point array initialization values are not supported");
    //int i;

    //for (i = 0; i < SZ; i++) {
    //    // data[i] = (T)initArray[i];
    //    data[i] = *((T *)((char *)initArray + i*sizeof(T)));
    //}
  typedef typename cm::pointer_traits<T2>::tail_pointee_type tail_pointee_type;
  for (int i = 0; i < SZ; i++) {   
    SIMDCF_WRAPPER(data[i] = (T)((tail_pointee_type *)initArray)[i], SZ, i);
  }
}

template <typename T, uint R, uint C>
matrix<T,R,C>::matrix()
{
    //number = OBJ_COUNTER ? ++CmEmulSys::_count : 0;
}

//copy constructor
template <typename T, uint R, uint C>
matrix<T,R,C>::matrix(const matrix<T,R,C>& src)
{
        //number = OBJ_COUNTER ? ++CmEmulSys::_count : 0;
        (*this) = src;
}
template <typename T, uint R, uint C>
template <typename T2>
matrix<T,R,C>::matrix(const T2& src)
{
        //number = OBJ_COUNTER ? ++CmEmulSys::_count : 0;
        (*this) = src;
}
template <typename T, uint R, uint C>
template <typename T2, uint R2, uint C2>   
matrix<T,R,C>::matrix(const matrix<T2,R2,C2>& src, const uint sat)
{
        //number = OBJ_COUNTER ? ++CmEmulSys::_count : 0;
        static const bool conformable = check_true<R*C == R2*C2>::value;
        assert(R*C == R2*C2);

        uint sat1 = 0; 
        //uint sat1 = CmEmulSys::_SetSatur<T2, is_inttype<T>::value>::SetSatur();
        vector<T2, SZ> in_src; in_src.assign(src);

        for (uint i=0; i < SZ; i++) {
            SIMDCF_WRAPPER((*this)(i) = CmEmulSys::satur<T>::saturate(in_src(i), sat | sat1), SZ, i);
        }
}
template <typename T, uint R, uint C>
template <typename T2, uint R2, uint C2>
matrix<T,R,C>::matrix(const matrix_ref<T2,R2,C2>& src, const uint sat)
{
        //number = OBJ_COUNTER ? ++CmEmulSys::_count : 0;
        static const bool conformable = check_true<R*C == R2*C2>::value;
        assert(R*C == R2*C2);

        uint sat1 = 0; 
        //uint sat1 = CmEmulSys::_SetSatur<T2, is_inttype<T>::value>::SetSatur();
        vector<T2, SZ> in_src; in_src.assign(src);

        for (uint i=0; i < SZ; i++) {
            SIMDCF_WRAPPER((*this)(i) = CmEmulSys::satur<T>::saturate(in_src(i), sat), SZ, i);
        }
}
//
// matrix operator =
//
template <typename T, uint R, uint C>
matrix<T,R,C>& matrix<T,R,C>::operator = (const matrix<T,R,C>& src)
{
        vector<T, SZ> in_src; in_src.assign(src);
        for (uint i=0; i<SZ; ++i) {
            SIMDCF_WRAPPER(this->getref(i) = in_src(i), SZ, i); 
        }
        return *this;
}

template <typename T, uint R, uint C>
template <typename T2>
matrix<T,R,C>& matrix<T,R,C>::operator = (const T2 src)
{
        uint sat1 = 0;
        //uint sat1 = CmEmulSys::_SetSatur<T2, is_inttype<T>::value>::SetSatur();

        for (uint i=0; i < SZ; i++) {
            SIMDCF_WRAPPER(this->getref(i) = CmEmulSys::satur<T>::saturate(src, sat1), SZ, i);
        }
    
        return *this;
}
template <typename T, uint R, uint C>
template <typename T2, uint R2, uint C2>
matrix<T,R,C>& matrix<T,R,C>::operator = (const matrix<T2,R2,C2>& src)
{
        CM_STATIC_ERROR(R*C == R2*C2, "matrices have different dimensions"); \
        static const bool conformable = check_true<R*C == R2*C2>::value;
        assert(R*C == R2*C2);

        uint sat1 = 0;
        //uint sat1 = CmEmulSys::_SetSatur<T2, is_inttype<T>::value>::SetSatur();
        vector<T2, SZ> in_src; in_src.assign(src);

        for (uint i=0; i < SZ; i++) {
            SIMDCF_WRAPPER(this->getref(i) = CmEmulSys::satur<T>::saturate(in_src(i), sat1), SZ, i);
        }

        return *this;
}
template <typename T, uint R, uint C>
template <typename T2, uint R2, uint C2>
matrix<T,R,C>& matrix<T,R,C>::operator = (const matrix_ref<T2,R2,C2>& src)
{
        CM_STATIC_ERROR(R*C == R2*C2, "matrices have different dimensions"); \
        static const bool conformable = check_true<R*C == R2*C2>::value;
        assert(R*C == R2*C2);

        uint sat1 = 0;
        //uint sat1 = CmEmulSys::_SetSatur<T2, is_inttype<T>::value>::SetSatur();
        vector<T2, SZ> in_src; in_src.assign(src);

        for (uint i=0; i < SZ; i++) {
            SIMDCF_WRAPPER(this->getref(i) = CmEmulSys::satur<T>::saturate(in_src(i), sat1), SZ, i);
        }

        return *this;
}


//Should be inserted for GenX style of float->integer conversions
//sat1 = CmEmulSys::_SetSatur<T2, is_inttype<T>::value>::SetSatur();
#define matrix_operation(OP) \
\
template <typename T, uint R, uint C> \
template <typename T2> \
matrix<T,R,C>& matrix<T,R,C>::operator OP##= (const T2 x) \
{ \
        static const bool type_conformable = cmtype<T2>::value; \
        uint sat1 = 0; \
        for (uint i=0; i < SZ; i++) { \
            SIMDCF_WRAPPER(this->getref(i) = CmEmulSys::satur<T>::saturate((*this).get(i) OP x, sat1), SZ, i); \
        } \
        return *this; \
} \
template <typename T, uint R, uint C> \
template <typename T2, uint R2, uint C2> \
matrix<T,R,C>& matrix<T,R,C>::operator OP##= (const matrix<T2,R2,C2>& x) \
{ \
        CM_STATIC_ERROR(R*C == R2*C2, "matrices have different dimensions"); \
        static const bool conformable = check_true<R*C == R2*C2>::value; \
        assert(R*C == R2*C2); \
        uint sat1 = 0; \
        vector<T2, /*SZ*/R*C> in_x; in_x.assign(x); \
        for (uint i=0; i < SZ; i++) { \
            SIMDCF_WRAPPER(this->getref(i) = CmEmulSys::satur<T>::saturate((*this).get(i) OP in_x(i), sat1), SZ, i);\
        } \
        return *this; \
} \
template <typename T, uint R, uint C> \
template <typename T2, uint R2, uint C2> \
matrix<T,R,C>& matrix<T,R,C>::operator OP##= (const matrix_ref<T2,R2,C2>& x) \
{ \
        CM_STATIC_ERROR(R*C == R2*C2, "matrices have different dimensions"); \
        static const bool conformable = check_true<R*C == R2*C2>::value; \
        assert(R*C == R2*C2); \
        uint sat1 = 0; \
        vector<T2, /*SZ*/R*C> in_x; in_x.assign(x); \
        for (uint i=0; i < SZ; i++) { \
            SIMDCF_WRAPPER(this->getref(i) = CmEmulSys::satur<T>::saturate((*this).get(i) OP in_x(i), sat1), SZ, i);\
        } \
        return *this; \
} \
template <typename T, uint R, uint C> \
template <typename T2> \
matrix<T,R,C>& matrix<T,R,C>::operator OP##= (const vector<T2,SZ>& x) \
{ \
        uint sat1 = 0; \
        vector<T2, SZ> in_x; in_x.assign(x); \
        for (uint i=0; i < SZ; i++) { \
            SIMDCF_WRAPPER(this->getref(i) = CmEmulSys::satur<T>::saturate((*this).get(i) OP in_x(i), sat1), SZ, i);\
        } \
        return *this; \
} \
template <typename T, uint R, uint C> \
template <typename T2> \
matrix<T,R,C>& matrix<T,R,C>::operator OP##= (const vector_ref<T2,SZ>& x) \
{ \
        uint sat1 = 0; \
        vector<T2, SZ> in_x; in_x.assign(x); \
        for (uint i=0; i < SZ; i++) { \
            SIMDCF_WRAPPER(this->getref(i) = CmEmulSys::satur<T>::saturate((*this).get(i) OP in_x(i), sat1), SZ, i);\
        } \
        return *this; \
} \


matrix_operation(+)     // +=
matrix_operation(-)     // -=
matrix_operation(*)     // *= 
matrix_operation(/)     // /=
matrix_operation(%)     // %=
matrix_operation(&)     // &=
matrix_operation(|)     // |=
matrix_operation(^)     // ^=
matrix_operation(>>)     // >>=
matrix_operation(<<)     // <<=
#undef matrix_operation


//
// matrix selects
//
template <typename T, uint R, uint C>
template <typename T2>
vector_ref<T2,R*C*sizeof(T)/sizeof(T2)> matrix<T,R,C>::format()
{
        CM_STATIC_ERROR(R>0, "format row size is zero");
        CM_STATIC_ERROR(C>0, "format column size is zero");
        CM_STATIC_WARNING(((R*C*sizeof(T)%sizeof(T2)) == 0), "source matrix size is not exactly divisible by format type size");
        const uint N = R*C*sizeof(T)/sizeof(T2);
        static const bool conformable = check_true<(R*C*sizeof(T))%sizeof(T2) == 0>::value;
        assert((R*C*sizeof(T))%sizeof(T2) == 0);
        vector_ref<T2,N> ret(id());
        for (uint i=0; i<N; ++i) {
            SIMDCF_WRAPPER(ret.set_elem_ref(i, ((T2*)data) + i), N, i);
        }

        return ret;
}
template <typename T, uint R, uint C>
template <typename T2, uint R2, uint C2>
matrix_ref<T2,R2,C2> matrix<T,R,C>::format()
{
        CM_STATIC_ERROR(R2>0, "format row size is zero");
        CM_STATIC_ERROR(C2>0, "format column size is zero");
        CM_STATIC_ERROR((R2 == 0) || (C2 == 0) || (sizeof(T)*R*C >= sizeof(T2)*R2*C2), "format result size is larger than source size");
        CM_STATIC_WARNING((R2 == 0) || (C2 == 0) || (sizeof(T)*R*C <= sizeof(T2)*R2*C2), "format result size is smaller than source size");
        static const bool conformable1 = check_true<(R2 >= 0)>::value;
        static const bool conformable2 = check_true<(C2 >= 0)>::value;
        static const bool conformable = check_true<sizeof(T)*R*C == sizeof(T2)*R2*C2>::value;
        assert(sizeof(T)*R*C == sizeof(T2)*R2*C2);
        matrix_ref<T2,R2,C2> ret(id());
        for (uint i=0; i<R2*C2; ++i) {
            SIMDCF_WRAPPER(ret.set_elem_ref(i, ((T2*)data) + i), R2*C2, i);
        }

        return ret;
}
template <typename T, uint R, uint C>
template <typename T2>
const vector_ref<T2,R*C*sizeof(T)/sizeof(T2)> matrix<T,R,C>::format() const
{
        CM_STATIC_ERROR(R>0, "format row size is zero");
        CM_STATIC_ERROR(C>0, "format column size is zero");
        CM_STATIC_WARNING(((R*C*sizeof(T)%sizeof(T2)) == 0), "source matrix size is not exactly divisible by format type size");
        const uint N = R*C*sizeof(T)/sizeof(T2);
        static const bool conformable = check_true<(R*C*sizeof(T))%sizeof(T2) == 0>::value;
        assert((R*C*sizeof(T))%sizeof(T2) == 0);
        vector_ref<T2,N> ret(id());
        for (uint i=0; i<N; ++i) {
            SIMDCF_WRAPPER(ret.set_elem_ref(i, ((T2*)data) + i), N, i);
        }

        return ret;
}
template <typename T, uint R, uint C>
template <typename T2, uint R2, uint C2>
const matrix_ref<T2,R2,C2> matrix<T,R,C>::format() const
{
        CM_STATIC_ERROR(R2>0, "format row size is zero");
        CM_STATIC_ERROR(C2>0, "format column size is zero");
        CM_STATIC_ERROR((R2 == 0) || (C2 == 0) || (sizeof(T)*R*C >= sizeof(T2)*R2*C2), "format result size is larger than source size");
        CM_STATIC_WARNING((R2 == 0) || (C2 == 0) || (sizeof(T)*R*C <= sizeof(T2)*R2*C2), "format result size is smaller than source size");
        static const bool conformable1 = check_true<(R2 >= 0)>::value;
        static const bool conformable2 = check_true<(C2 >= 0)>::value;
        static const bool conformable = check_true<sizeof(T)*R*C == sizeof(T2)*R2*C2>::value;
        assert(sizeof(T)*R*C == sizeof(T2)*R2*C2);
        matrix_ref<T2,R2,C2> ret(id());
        for (uint i=0; i<R2*C2; ++i) {
            SIMDCF_WRAPPER(ret.set_elem_ref(i, ((T2*)data) + i), R2*C2, i);
        }

        return ret;
}
template <typename T, uint R, uint C>
vector_ref<T, C> matrix<T,R,C>::row(OFFSET index)
{
        CM_STATIC_ERROR(R>0, "row size is zero");
        CM_STATIC_ERROR(C>0, "column size is zero");
        assert(index < R);

        vector_ref<T, C> ret(id());
        for (uint i=0; i<C; ++i) {
            SIMDCF_WRAPPER(ret.set_elem_ref(i, data + C*index + i), C, i);
        }
        return ret;
}
template <typename T, uint R, uint C>
matrix_ref<T,R,1> matrix<T,R,C>::column(OFFSET index)
{
        CM_STATIC_ERROR(R>0, "row size is zero");
        CM_STATIC_ERROR(C>0, "column size is zero");
        assert(index < C);

        matrix_ref<T,R,1> ret(id());
        for (uint i=0; i<R; ++i) {
            SIMDCF_WRAPPER(ret.set_elem_ref(i, data + C*i + index), R, i);
        }
        return ret;
}
template <typename T, uint R, uint C>
template <uint R2, uint RS, uint C2, uint CS>
matrix_ref<T,R2,C2> matrix<T,R,C>::select(OFFSET ioff, OFFSET joff)
{
        CM_STATIC_ERROR((RS > 0), "select does not support a row stride of 0");
        CM_STATIC_ERROR((CS > 0), "select does not support a column stride of 0");
        CM_STATIC_WARNING(!(R2 == 1 && RS != 1), "when row size is 1 the row stride must also be 1");
        CM_STATIC_WARNING(!(C2 == 1 && CS != 1), "when column size is 1 the column stride must also be 1");
        CM_STATIC_WARNING(((C2 - 1) * CS + 1 <= C), "new row must fit inside the source row (new row out of bounds wrt original)");
        CM_STATIC_WARNING(((R2 - 1) * RS + 1 <= R), "new matrix must fit inside the source matrix (new matrix out of bounds wrt original)");

        static const bool conformable1 = check_true<((R2 - 1) * RS < R)>::value;
        static const bool conformable2 = check_true<((C2 - 1) * CS < C)>::value;
        static const bool conformable3 = check_true<(RS > 0)>::value;
        static const bool conformable4 = check_true<(CS > 0)>::value;
        static const bool conformable5 = check_true<!(R2 == 1 && RS != 1)>::value;
        static const bool conformable6 = check_true<!(C2 == 1 && CS != 1)>::value;

        assert(ioff  < R - (R2 - 1) * RS);
        assert(joff  < C - (C2 - 1) * CS);

        matrix_ref<T,R2,C2> ret(id());
        for (uint i=0; i<R2; i++) {
            for (uint j=0; j<C2; j++) {
                if ((CS*j + joff) >= C) {
                    // We go off the end of the source row
                    // Fire an assert in debug mode
#ifdef CM_DEBUG
                    assert( 0 && "select statement access is out-of-bounds on source matrix");
#endif
                    SIMDCF_WRAPPER(ret.set_elem_ref(C2*i + j, ret.dummy()), R2 * C2, i * C2 + j);
                } else if ((C*(RS*i + ioff) + (CS*j) + joff) >= (C * R)) {
                    // We go off the end of the source matrix
                    // Fire an assert in debug mode
#ifdef CM_DEBUG
                    assert( 0 && "select statement access is out-of-bounds on source matrix");
#endif
                    SIMDCF_WRAPPER(ret.set_elem_ref(C2*i + j, ret.dummy()), R2 * C2, i * C2 + j );
                } else {
                    // Everything is within bounds
                    SIMDCF_WRAPPER(ret.set_elem_ref(C2*i + j, ((T*)data) + C*(RS*i + ioff) + (CS*j) + joff), R2 * C2, i * C2 + j);
                }
            }
        }
        return ret;
}

template <typename T, uint R, uint C>
template <uint R2, uint RS, uint C2, uint CS>
const matrix_ref<T,R2,C2> matrix<T,R,C>::select(OFFSET ioff, OFFSET joff) const
{
        CM_STATIC_ERROR((RS > 0), "select does not support a row stride of 0");
        CM_STATIC_ERROR((CS > 0), "select does not support a column stride of 0");
        CM_STATIC_WARNING(!(R2 == 1 && RS != 1), "when row size is 1 the row stride must also be 1");
        CM_STATIC_WARNING(!(C2 == 1 && CS != 1), "when column size is 1 the column stride must also be 1");
        CM_STATIC_WARNING(((C2 - 1) * CS + 1 <= C), "new row must fit inside the source row (new row out of bounds wrt original)");
        CM_STATIC_WARNING(((R2 - 1) * RS + 1 <= R), "new matrix must fit inside the source matrix (new matrix out of bounds wrt original)");

        static const bool conformable1 = check_true<((R2 - 1) * RS < R)>::value;
        static const bool conformable2 = check_true<((C2 - 1) * CS < C)>::value;
        static const bool conformable3 = check_true<(RS > 0)>::value;
        static const bool conformable4 = check_true<(CS > 0)>::value;
        static const bool conformable5 = check_true<!(R2 == 1 && RS != 1)>::value;
        static const bool conformable6 = check_true<!(C2 == 1 && CS != 1)>::value;

        assert(ioff  < R - (R2 - 1) * RS);
        assert(joff  < C - (C2 - 1) * CS);

        matrix_ref<T,R2,C2> ret(id());
        for (uint i=0; i<R2; i++) {
            for (uint j=0; j<C2; j++) {
                if ((CS*j + joff) >= C) {
                    // We go off the end of the source row
                    // Fire an assert in debug mode
#ifdef CM_DEBUG
                    assert( 0 && "select statement access is out-of-bounds on source matrix");
#endif
                    SIMDCF_WRAPPER(ret.set_elem_ref(C2*i + j, ret.dummy()), R2 * C2, i * C2 + j);
                } else if ((C*(RS*i + ioff) + (CS*j) + joff) >= (C * R)) {
                    // We go off the end of the source matrix
                    // Fire an assert in debug mode
#ifdef CM_DEBUG
                    assert( 0 && "select statement access is out-of-bounds on source matrix");
#endif
                    SIMDCF_WRAPPER(ret.set_elem_ref(C2*i + j, ret.dummy()), R2 * C2, i * C2 + j );
                } else {
                    // Everything is within bounds
                    SIMDCF_WRAPPER(ret.set_elem_ref(C2*i + j, ((T*)data) + C*(RS*i + ioff) + (CS*j) + joff), R2 * C2, i * C2 + j);
                }
            }
        }
        return ret;
}

template <typename T, uint R, uint C>
template <uint R2, uint VS, uint WD, uint HS>
const vector<T, R2*WD> matrix<T,R,C>::genx_select(OFFSET ioff, OFFSET joff)
{
        CM_STATIC_ERROR((!std::is_same<T, double>::value), "genx_select is not supported for matrices with element type of 'double'");
        static const bool conformable1 = check_true<(R2 > 0)>::value;
        static const bool conformable2 = check_true<(VS >= 0)>::value;
        static const bool conformable3 = check_true<(WD > 0)>::value;
        static const bool conformable4 = check_true<(HS >= 0)>::value;
        assert(R2>=0 && VS>=0 && WD>=0 && HS >=0);

        assert(ioff < R);
        assert(joff < C);

        vector<T,R2*WD> ret(id());
        for (uint i=0; i < R2*WD; i++) {
            SIMDCF_WRAPPER(ret(i) = data[C*ioff + joff + (i/WD)*VS + (i%WD)*HS], R2*WD, i);
        }
        return ret;
}

//1D iselect for matrix
#if _MSC_VER >= 1700
template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix<T,R,C>::iselect(const vector<T2,WD>& index)
{
        return iselect(index, std::is_integral<T2>());
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix<T,R,C>::iselect(const vector<T2,WD>& index, std::true_type)
{
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index.get(i) < SZ), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(ret(i) = data[index.get(i)], WD, i);
        }
        return ret;
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix<T,R,C>::iselect(const vector<T2,WD>& index, std::false_type)
{
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index.get(i) < SZ), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            // in this case index doesn't have integral type elements
            // so can't be used - we will have already generated an error,
            // so just use 0 to allow compilation to continue (in case there
            // are more errors to find...)  
            SIMDCF_WRAPPER(ret(i) = data[0], WD, i);
        }
        return ret;
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix<T,R,C>::iselect(const vector_ref<T2,WD>& index)
{
        return iselect(index, std::is_integral<T2>());
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix<T,R,C>::iselect(const vector_ref<T2,WD>& index, std::true_type)
{
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index.get(i) < SZ), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(ret(i) = data[index.get(i)], WD, i);
        }
        return ret;
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix<T,R,C>::iselect(const vector_ref<T2,WD>& index, std::false_type)
{
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index.get(i) < SZ), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            // in this case index doesn't have integral type elements
            // so can't be used - we will have already generated an error,
            // so just use 0 to allow compilation to continue (in case there
            // are more errors to find...)  
            SIMDCF_WRAPPER(ret(i) = data[0], WD, i);
        }
        return ret;
}
#else
template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix<T,R,C>::iselect(const vector<T2,WD>& index)
{
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index.get(i) < SZ), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(ret(i) = data[index.get(i)], WD, i);
        }
        return ret;
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix<T,R,C>::iselect(const vector_ref<T2,WD>& index)
{        
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
          SIMDCF_WRAPPER(assert(index.get(i) < SZ), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(ret(i) = data[index.get(i)], WD, i);
        }
        return ret;
}
#endif

//below are 2D iselect for matrix
template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix<T,R,C>::iselect(const vector<T2,WD>& index_x, const vector<T2,WD>& index_y)
{
        CM_STATIC_WARNING((std::is_unsigned<T2>::value), "iselect index vector element type must be unsigned");
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index_x.get(i) < R), WD, i);
            SIMDCF_WRAPPER(assert(index_y.get(i) < C), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(ret(i) = data[index_x.get(i)*C + index_y.get(i)], WD, i);
        }
        return ret;
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix<T,R,C>::iselect(const vector_ref<T2,WD>& index_x, const vector<T2,WD>& index_y)
{
        CM_STATIC_WARNING((std::is_unsigned<T2>::value), "iselect index vector element type must be unsigned");
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
          SIMDCF_WRAPPER(assert(index_x.get(i) < R), WD, i);
          SIMDCF_WRAPPER(assert(index_y.get(i) < C), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(ret(i) = data[index_x.get(i)*C + index_y.get(i)], WD, i);
        }
        return ret;
}


template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix<T,R,C>::iselect(const vector<T2,WD>& index_x, const vector_ref<T2,WD>& index_y)
{
        CM_STATIC_WARNING((std::is_unsigned<T2>::value), "iselect index vector element type must be unsigned");
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index_x.get(i) < R), WD, i);
            SIMDCF_WRAPPER(assert(index_y.get(i) < C), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(ret(i) = data[index_x.get(i)*C + index_y.get(i)], WD, i);
        }
        return ret;
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix<T,R,C>::iselect(const vector_ref<T2,WD>& index_x, const vector_ref<T2,WD>& index_y)
{
        CM_STATIC_WARNING((std::is_unsigned<T2>::value), "iselect index vector element type must be unsigned");
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
          SIMDCF_WRAPPER(assert(index_x.get(i) < R), WD, i);
          SIMDCF_WRAPPER(assert(index_y.get(i) < C), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(ret(i) = data[index_x.get(i)*C + index_y.get(i)], WD, i);
        }
        return ret;
}
// end of iselect for 2D matrix

template <typename T, uint R, uint C>
matrix_ref<T, R, C> CM_NOINLINE matrix<T,R,C>::select_all()
{
        matrix_ref<T,R,C> ret = this->select<R, 1, C, 1>();
        return ret;
}

template <typename T, uint R, uint C>
const matrix_ref<T, R, C> CM_NOINLINE matrix<T,R,C>::select_all() const
{
        const matrix_ref<T,R,C> ret = this->select<R, 1, C, 1>();
        return ret;
}

/*******************************************************************
/
/                         matrix_ref
/
*******************************************************************/
template <typename T, uint R, uint C>
bool matrix_ref<T,R,C>::is_contiguous() const
{
        if (SZ == 1)
                return true;
        
        for (uint i=0; i<SZ-1; ++i) {
                if (data[i+1]-data[i] != 1)
                        return false;
        }
        return true;
}

template <typename T, uint R, uint C>
bool matrix_ref<T,R,C>::is_contiguous(const uint start, const uint end) const
{
        if (start == end)
            return true;

        if (SZ == 1)
            return true;
        
        for (uint i=start; i != end; ++i) {
                if (data[i+1]-data[i] != 1)
                        return false;
        }
        return true;
}

//
// matrix_ref copy constructor
//
template <typename T, uint R, uint C>
matrix_ref<T,R,C>::matrix_ref(const matrix_ref<T,R,C>& src)
{
        number = src.number;

        matrix_ref<T,R,C> in_src(id());
        memcpy(in_src.data, src.data, sizeof(T*) * SZ);

        memcpy(data, in_src.data,sizeof(T*)*SZ);
}

template <typename T, uint R, uint C>   
matrix_ref<T,R,C>::matrix_ref(matrix<T,R,C>& src)
{
        number = src.id();
        for (uint i = 0; i < ROWS; i++)
            for (uint j = 0; j < COLS; j++) {
                SIMDCF_WRAPPER(
                        set_elem_ref(i * COLS + j, (T*)(src.get_addr(i * COLS + j))),
                       SZ, j);
            } 
}
//
// matrix_ref assignment operator
//
template <typename T, uint R, uint C>
matrix_ref<T,R,C>& matrix_ref<T,R,C>::operator = (const matrix<T,R,C>& src)
{
        vector<T, SZ> in_src; in_src.assign(src);
        for (uint i=0; i<SZ; ++i) {
            SIMDCF_WRAPPER((*this)(i) = in_src(i), SZ, i);
        }
        return *this;
}

//
// matrix_ref operator =
//
template <typename T, uint R, uint C>
template <typename T2>
matrix_ref<T,R,C>& matrix_ref<T,R,C>::operator = (const T2 src)
{
        uint sat1 = 0; 
        //uint sat1 = CmEmulSys::_SetSatur<T2, is_inttype<T>::value>::SetSatur();
        for (uint i=0; i < SZ; i++) {
            SIMDCF_WRAPPER(this->getref(i) = CmEmulSys::satur<T>::saturate(src, sat1), SZ, i);
        }
    
        return *this;
}
template <typename T, uint R, uint C>
template <typename T2, uint R2, uint C2>
matrix_ref<T,R,C>& matrix_ref<T,R,C>::operator = (const matrix<T2,R2,C2>& src)
{
        CM_STATIC_ERROR(R*C == R2*C2, "matrices have different dimensions"); \
        static const bool conformable = check_true<R*C == R2*C2>::value;
        assert(R*C == R2*C2);

        uint sat1 = 0;
        //uint sat1 = CmEmulSys::_SetSatur<T2, is_inttype<T>::value>::SetSatur();
        vector<T2, SZ> in_src; in_src.assign(src);
        for (uint i=0; i < SZ; i++) {
            SIMDCF_WRAPPER(this->getref(i) = CmEmulSys::satur<T>::saturate(in_src(i), sat1), SZ, i);
        }

        return *this;
}

template <typename T, uint R, uint C>
matrix_ref<T,R,C>& matrix_ref<T,R,C>::operator = (const matrix_ref<T,R,C>& src)
{
        vector<T, SZ> in_src; in_src.assign(src);
        for (uint i=0; i<SZ; ++i) {
            SIMDCF_WRAPPER(this->getref(i) = T(in_src(i)), SZ, i);
        }
        return *this;
}

template <typename T, uint R, uint C>
template <typename T2, uint R2, uint C2>
matrix_ref<T,R,C>& matrix_ref<T,R,C>::operator = (const matrix_ref<T2,R2,C2>& src)
{
        CM_STATIC_ERROR(R*C == R2*C2, "matrices have different dimensions"); \
        static const bool conformable = check_true<R*C == R2*C2>::value;
        assert(R*C == R2*C2);

        uint sat1 = 0;
        //uint sat1 = CmEmulSys::_SetSatur<T2, is_inttype<T>::value>::SetSatur();
        vector<T2, SZ> in_src; in_src.assign(src);
        for (uint i=0; i < SZ; i++) {
            SIMDCF_WRAPPER(this->getref(i) = CmEmulSys::satur<T>::saturate(in_src(i), sat1), SZ, i);
        }

        return *this;
}

//Should be inserted for GenX style of float->integer conversions
//sat1 = CmEmulSys::_SetSatur<T2, is_inttype<T>::value>::SetSatur();
#define matrix_ref_operation(OP) \
\
template <typename T, uint R, uint C> \
template <typename T2> \
matrix_ref<T,R,C>& matrix_ref<T,R,C>::operator OP##= (const T2 x) \
{ \
        static const bool type_conformable = cmtype<T2>::value; \
        uint sat1 = 0; \
        for (uint i=0; i < SZ; i++) { \
            SIMDCF_WRAPPER(this->getref(i) = CmEmulSys::satur<T>::saturate((*this).get(i) OP x, sat1), SZ, i);\
        } \
        return *this; \
} \
template <typename T, uint R, uint C> \
template <typename T2, uint R2, uint C2> \
matrix_ref<T,R,C>& matrix_ref<T,R,C>::operator OP##= (const matrix<T2,R2,C2>& x) \
{ \
        CM_STATIC_ERROR(R*C == R2*C2, "matrices have different dimensions"); \
        static const bool conformable = check_true<R*C == R2*C2>::value; \
        assert(R*C == R2*C2); \
        uint sat1 = 0; \
        vector<T2, SZ> in_x; in_x.assign(x); \
        for (uint i=0; i < SZ; i++) { \
            SIMDCF_WRAPPER(this->getref(i) = CmEmulSys::satur<T>::saturate((*this).get(i) OP in_x(i), sat1), SZ, i);\
        } \
        return *this; \
} \
template <typename T, uint R, uint C> \
template <typename T2, uint R2, uint C2> \
matrix_ref<T,R,C>& matrix_ref<T,R,C>::operator OP##= (const matrix_ref<T2,R2,C2>& x) \
{ \
        CM_STATIC_ERROR(R*C == R2*C2, "matrices have different dimensions"); \
        static const bool conformable = check_true<R*C == R2*C2>::value; \
        assert(R*C == R2*C2); \
        uint sat1 = 0; \
        vector<T2, SZ> in_x; in_x.assign(x); \
        for (uint i=0; i < SZ; i++) { \
            SIMDCF_WRAPPER(this->getref(i) = CmEmulSys::satur<T>::saturate((*this).get(i) OP in_x(i), sat1), SZ, i);\
        } \
        return *this; \
} \
template <typename T, uint R, uint C> \
template <typename T2> \
matrix_ref<T,R,C>& matrix_ref<T,R,C>::operator OP##= (const vector<T2,SZ>& x) \
{ \
        CM_STATIC_ERROR(R*C == SZ, "matrix and vector have a different number of elements"); \
        uint sat1 = 0; \
        vector<T2, SZ> in_x; in_x.assign(x); \
        for (uint i=0; i < SZ; i++) { \
            SIMDCF_WRAPPER(this->getref(i) = CmEmulSys::satur<T>::saturate((*this).get(i) OP in_x(i), sat1), SZ, i);\
        } \
        return *this; \
} \
template <typename T, uint R, uint C> \
template <typename T2> \
matrix_ref<T,R,C>& matrix_ref<T,R,C>::operator OP##= (const vector_ref<T2,SZ>& x) \
{ \
        uint sat1 = 0; \
        vector<T2, SZ> in_x; in_x.assign(x); \
        for (uint i=0; i < SZ; i++) { \
            SIMDCF_WRAPPER(this->getref(i) = CmEmulSys::satur<T>::saturate((*this).get(i) OP in_x(i), sat1), SZ, i); \
        } \
        return *this; \
} \

matrix_ref_operation(+)     // +=
matrix_ref_operation(-)     // -=
matrix_ref_operation(*)     // *= 
matrix_ref_operation(/)     // /=
matrix_ref_operation(%)     // %=
matrix_ref_operation(&)     // &=
matrix_ref_operation(|)     // |=
matrix_ref_operation(^)     // ^=
matrix_ref_operation(>>)     // >>=
matrix_ref_operation(<<)     // <<=
#undef matrix_operation

//
// matrix_ref selects
//
template <typename T, uint R, uint C>
template <typename T2>
vector_ref<T2,R*C*sizeof(T)/sizeof(T2)> matrix_ref<T,R,C>::format()
{
        CM_STATIC_ERROR(R>0, "format row size is zero");
        CM_STATIC_ERROR(C>0, "format column size is zero");
        CM_STATIC_WARNING(((R*C*sizeof(T)%sizeof(T2)) == 0), "source matrix size is not exactly divisible by format type size");

//        assert(is_contiguous());

        const uint N = R*C*sizeof(T)/sizeof(T2);
        static const bool conformable = check_true<(R*C*sizeof(T))%sizeof(T2) == 0>::value;
        assert((R*C*sizeof(T))%sizeof(T2) == 0);
        vector_ref<T2,N> ret(id());

        if (sizeof(T2) < sizeof(T))
        {
            uint ratio = sizeof(T) / sizeof(T2);
            for (uint i = 0; i < R*C; ++i)
            {
                for (uint j = 0; j < ratio; j++)
                {
                    SIMDCF_WRAPPER(ret.set_elem_ref(ratio* i + j, ((T2*)data[i]) + j), N, ratio* i + j);
                }
            }
        }
        else
        {

            for (uint i = 0; i < N; ++i) {
                SIMDCF_WRAPPER(ret.set_elem_ref(i, (T2*)(data[i * sizeof(T2) / sizeof(T)])), N, i);
            }
        }
        return ret;
}
template <typename T, uint R, uint C>
template <typename T2, uint R2, uint C2>
matrix_ref<T2,R2,C2> matrix_ref<T,R,C>::format()
{
        CM_STATIC_ERROR(R2>0, "format row size is zero");
        CM_STATIC_ERROR(C2>0, "format column size is zero");
        CM_STATIC_ERROR((R2 == 0) || (C2 == 0) || (sizeof(T)*R*C >= sizeof(T2)*R2*C2), "format result size is larger than source size");
        CM_STATIC_WARNING((R2 == 0) || (C2 == 0) || (sizeof(T)*R*C <= sizeof(T2)*R2*C2), "format result size is smaller than source size");
        static const bool conformable1 = check_true<(R2 >= 0)>::value;
        static const bool conformable2 = check_true<(C2 >= 0)>::value;
        static const bool conformable = check_true<sizeof(T)*R*C == sizeof(T2)*R2*C2>::value;

//        assert(is_contiguous());

        assert(sizeof(T)*R*C == sizeof(T2)*R2*C2);
        matrix_ref<T2,R2,C2> ret(id());
        if (sizeof(T2) < sizeof(T))
        {
            uint ratio = sizeof(T) / sizeof(T2);
            for (uint i = 0; i < R*C; ++i)
            {
                for (uint j = 0; j < ratio; j++)
                {
                    SIMDCF_WRAPPER(ret.set_elem_ref(ratio* i + j, ((T2*)data[i]) + j), R2*C2, ratio* i + j);
                }
            }
        }
        else
        {
            for (uint i = 0; i<R2*C2; ++i) {
                SIMDCF_WRAPPER(ret.set_elem_ref(i, (T2*)(data[i * sizeof(T2) / sizeof(T)])), R2*C2, i);
            }
        }

        return ret;
}
template <typename T, uint R, uint C>
template <typename T2>
const vector_ref<T2,R*C*sizeof(T)/sizeof(T2)> matrix_ref<T,R,C>::format() const
{
        CM_STATIC_ERROR(R>0, "format row size is zero");
        CM_STATIC_ERROR(C>0, "format column size is zero");
        CM_STATIC_WARNING(((R*C*sizeof(T)%sizeof(T2)) == 0), "source matrix size is not exactly divisible by format type size");

//        assert(is_contiguous());

        const uint N = R*C*sizeof(T)/sizeof(T2);
        static const bool conformable = check_true<(R*C*sizeof(T))%sizeof(T2) == 0>::value;
        assert((R*C*sizeof(T))%sizeof(T2) == 0);
        vector_ref<T2,N> ret(id());
        if (sizeof(T2) < sizeof(T))
        {
            uint ratio = sizeof(T) / sizeof(T2);
            for (uint i = 0; i < R*C; ++i)
            {
                for (uint j = 0; j < ratio; j++)
                {
                    SIMDCF_WRAPPER(ret.set_elem_ref(ratio* i + j, ((T2*)data[i]) + j), N, ratio* i + j);
                }
            }
        }
        else
        {

            for (uint i = 0; i < N; ++i) {
                SIMDCF_WRAPPER(ret.set_elem_ref(i, (T2*)(data[i * sizeof(T2) / sizeof(T)])), N, i);
            }
        }
        return ret;
}
template <typename T, uint R, uint C>
template <typename T2, uint R2, uint C2>
const matrix_ref<T2,R2,C2> matrix_ref<T,R,C>::format() const
{
        CM_STATIC_ERROR(R2>0, "format row size is zero");
        CM_STATIC_ERROR(C2>0, "format column size is zero");
        CM_STATIC_ERROR((R2 == 0) || (C2 == 0) || (sizeof(T)*R*C >= sizeof(T2)*R2*C2), "format result size is larger than source size");
        CM_STATIC_WARNING((R2 == 0) || (C2 == 0) || (sizeof(T)*R*C <= sizeof(T2)*R2*C2), "format result size is smaller than source size");
        static const bool conformable1 = check_true<(R2 >= 0)>::value;
        static const bool conformable2 = check_true<(C2 >= 0)>::value;
        static const bool conformable = check_true<sizeof(T)*R*C == sizeof(T2)*R2*C2>::value;

//        assert(is_contiguous());

        assert(sizeof(T)*R*C == sizeof(T2)*R2*C2);
        matrix_ref<T2,R2,C2> ret(id());
        if (sizeof(T2) < sizeof(T))
        {
            uint ratio = sizeof(T) / sizeof(T2);
            for (uint i = 0; i < R*C; ++i)
            {
                for (uint j = 0; j < ratio; j++)
                {
                    SIMDCF_WRAPPER(ret.set_elem_ref(ratio* i + j, ((T2*)data[i]) + j), R2*C2, ratio* i + j);
                }
            }
        }
        else
        {
            for (uint i = 0; i<R2*C2; ++i) {
                SIMDCF_WRAPPER(ret.set_elem_ref(i, (T2*)(data[i * sizeof(T2) / sizeof(T)])), R2*C2, i);
            }
        }

        return ret;
}
template <typename T, uint R, uint C>
vector_ref<T, C> matrix_ref<T,R,C>::row(OFFSET index)
{
        assert(index < R);

#ifdef CM_V1
        assert(is_contiguous());
#endif

        vector_ref<T, C> ret(id());
        for (uint i=0; i<C; ++i) {
                SIMDCF_WRAPPER(ret.set_elem_ref(i, *(data + C*index + i)), C, i);
        }
        return ret;
}
template <typename T, uint R, uint C>
matrix_ref<T,R,1> matrix_ref<T,R,C>::column(OFFSET index)
{
        assert(index < C);

#ifdef CM_V1
        assert(is_contiguous());
#endif

        matrix_ref<T,R,1> ret(id());
        for (uint i=0; i<R; ++i) {
                SIMDCF_WRAPPER(ret.set_elem_ref(i, data[C*i + index]), R, i); 
        }
        return ret;
}
template <typename T, uint R, uint C>
template <uint R2, uint RS, uint C2, uint CS>
matrix_ref<T,R2,C2> matrix_ref<T,R,C>::select(OFFSET ioff, OFFSET joff)
{
        CM_STATIC_ERROR((RS > 0), "select does not support a row stride of 0");
        CM_STATIC_ERROR((CS > 0), "select does not support a column stride of 0");
        CM_STATIC_WARNING(!(R2 == 1 && RS != 1), "when row size is 1 the row stride must also be 1");
        CM_STATIC_WARNING(!(C2 == 1 && CS != 1), "when column size is 1 the column stride must also be 1");
        CM_STATIC_WARNING(((C2 - 1) * CS + 1 <= C), "new row must fit inside the source row (new row out of bounds wrt original)");
        CM_STATIC_WARNING(((R2 - 1) * RS + 1 <= R), "new matrix must fit inside the source matrix (new matrix out of bounds wrt original)");

        static const bool conformable1 = check_true<((R2 - 1) * RS < R)>::value;
        static const bool conformable2 = check_true<((C2 - 1) * CS < C)>::value;
        static const bool conformable3 = check_true<(RS > 0)>::value;
        static const bool conformable4 = check_true<(CS > 0)>::value;
        static const bool conformable5 = check_true<!(R2 == 1 && RS != 1)>::value;
        static const bool conformable6 = check_true<!(C2 == 1 && CS != 1)>::value;

        assert(ioff  < R - (R2 - 1) * RS);
        assert(joff  < C - (C2 - 1) * CS);

#ifdef CM_V1
        assert(is_contiguous()); 
#endif

        matrix_ref<T,R2,C2> ret(id());
        for (uint i=0; i<R2; i++) {
            for (uint j=0; j<C2; j++) {
                if ((CS*j + joff) >= C) {
                    // We go off the end of the source row
                    // Fire an assert in debug mode
#ifdef CM_DEBUG
                    assert(0 && "select statement access is out-of-bounds on source matrix_ref");
#endif
                    ret.set_elem_ref(C2*i + j, ret.dummy());
                } else if ((C*(RS*i + ioff) + (CS*j) + joff) >= (C * R)) {
                    // We go off the end of the source matrix
                    // Fire an assert in debug mode
#ifdef CM_DEBUG
                    assert(0 && "select statement access is out-of-bounds on source matrix_ref");
#endif
                    ret.set_elem_ref(C2*i + j, ret.dummy());
                } else {
                    // Everything is within bounds
                    ret.set_elem_ref(C2*i + j, data[C*(RS*i + ioff) + (CS*j) + joff]); 
                }
            }
        }
        return ret;
}

template <typename T, uint R, uint C>
template <uint R2, uint RS, uint C2, uint CS>
const matrix_ref<T,R2,C2> matrix_ref<T,R,C>::select(OFFSET ioff, OFFSET joff) const
{
        CM_STATIC_ERROR((RS > 0), "select does not support a row stride of 0");
        CM_STATIC_ERROR((CS > 0), "select does not support a column stride of 0");
        CM_STATIC_WARNING(!(R2 == 1 && RS != 1), "when row size is 1 the row stride must also be 1");
        CM_STATIC_WARNING(!(C2 == 1 && CS != 1), "when column size is 1 the column stride must also be 1");
        CM_STATIC_WARNING(((C2 - 1) * CS + 1 <= C), "new row must fit inside the source row (new row out of bounds wrt original)");
        CM_STATIC_WARNING(((R2 - 1) * RS + 1 <= R), "new matrix must fit inside the source matrix (new matrix out of bounds wrt original)");

        static const bool conformable1 = check_true<((R2 - 1) * RS < R)>::value;
        static const bool conformable2 = check_true<((C2 - 1) * CS < C)>::value;
        static const bool conformable3 = check_true<(RS > 0)>::value;
        static const bool conformable4 = check_true<(CS > 0)>::value;
        static const bool conformable5 = check_true<!(R2 == 1 && RS != 1)>::value;
        static const bool conformable6 = check_true<!(C2 == 1 && CS != 1)>::value;

        assert(ioff  < R - (R2 - 1) * RS);
        assert(joff  < C - (C2 - 1) * CS);

#ifdef CM_V1
        assert(is_contiguous()); 
#endif

        matrix_ref<T,R2,C2> ret(id());
        for (uint i=0; i<R2; i++) {
            for (uint j=0; j<C2; j++) {
                if ((CS*j + joff) >= C) {
                    // We go off the end of the source row
                    // Fire an assert in debug mode
#ifdef CM_DEBUG
                    assert(0 && "select statement access is out-of-bounds on source matrix_ref");
#endif
                    ret.set_elem_ref(C2*i + j, ret.dummy());
                } else if ((C*(RS*i + ioff) + (CS*j) + joff) >= (C * R)) {
                    // We go off the end of the source matrix
                    // Fire an assert in debug mode
#ifdef CM_DEBUG
                    assert(0 && "select statement access is out-of-bounds on source matrix_ref");
#endif
                    ret.set_elem_ref(C2*i + j, ret.dummy());
                } else {
                    // Everything is within bounds
                    ret.set_elem_ref(C2*i + j, data[C*(RS*i + ioff) + (CS*j) + joff]); 
                }
            }
        }
        return ret;
}

template <typename T, uint R, uint C>
template <uint R2, uint VS, uint WD, uint HS>
const vector<T, R2*WD> matrix_ref<T,R,C>::genx_select(OFFSET ioff, OFFSET joff)
{
        static const bool conformable1 = check_true<(R2 > 0)>::value;
        static const bool conformable2 = check_true<(VS >= 0)>::value;
        static const bool conformable3 = check_true<(WD > 0)>::value;
        static const bool conformable4 = check_true<(HS >= 0)>::value;
        assert(R2>=0 && VS>=0 && WD>=0 && HS >=0);

        assert(ioff < R);
        assert(joff < C);

        vector<T,R2*WD> ret(id());
        for (uint i=0; i < R2*WD; i++) {
            SIMDCF_WRAPPER(ret(i) = *data[C*ioff + joff + (i/WD)*VS + (i%WD)*HS], R2*WD, i);
        }
        return ret;
}

//below are 1D iselect for matrix_ref
#if _MSC_VER >= 1700
template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix_ref<T,R,C>::iselect(const vector<T2,WD>& index)
{
        return iselect(index, std::is_integral<T2>());
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix_ref<T,R,C>::iselect(const vector<T2,WD>& index, std::true_type)
{
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index.get(i) < SZ), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(ret(i) = *data[index.get(i)], WD, i);
        }
        return ret;
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix_ref<T,R,C>::iselect(const vector<T2,WD>& index, std::false_type)
{
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index.get(i) < SZ), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            // in this case index doesn't have integral type elements
            // so can't be used - we will have already generated an error,
            // so just use 0 to allow compilation to continue (in case there
            // are more errors to find...)  
            SIMDCF_WRAPPER(ret(i) = *data[0], WD, i);
        }
        return ret;
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix_ref<T,R,C>::iselect(const vector_ref<T2,WD>& index)
{
        return iselect(index, std::is_integral<T2>());
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix_ref<T,R,C>::iselect(const vector_ref<T2,WD>& index, std::true_type)
{
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index.get(i) < SZ), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(ret(i) = *data[index.get(i)], WD, i);
        }
        return ret;
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix_ref<T,R,C>::iselect(const vector_ref<T2,WD>& index, std::false_type)
{
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index.get(i) < SZ), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            // in this case index doesn't have integral type elements
            // so can't be used - we will have already generated an error,
            // so just use 0 to allow compilation to continue (in case there
            // are more errors to find...)  
            SIMDCF_WRAPPER(ret(i) = *data[0], WD, i);
        }
        return ret;
}
#else
template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix_ref<T,R,C>::iselect(const vector<T2,WD>& index)
{
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index.get(i) < SZ), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(ret(i) = *data[index.get(i)], WD, i);
        }
        return ret;
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix_ref<T,R,C>::iselect(const vector_ref<T2,WD>& index)
{
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index.get(i) < SZ), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(ret(i) = *data[index.get(i)], WD, i);
        }
        return ret;
}
#endif

//below are 2D iselect for matrix_ref
template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix_ref<T,R,C>::iselect(const vector<T2,WD>& index_x,
                                            const vector<T2,WD>& index_y)
{
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index_x.get(i) < R), WD, i);
            SIMDCF_WRAPPER(assert(index_y.get(i) < C), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(ret(i) = *data[index_x.get(i)*C+index_y.get(i)], WD, i);
        }
        return ret;
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix_ref<T,R,C>::iselect(const vector_ref<T2,WD>& index_x,
                                            const vector<T2,WD>& index_y)
{
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index_x.get(i) < R), WD, i);
            SIMDCF_WRAPPER(assert(index_y.get(i) < C), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(ret(i) = *data[index_x.get(i)*C + index_y.get(i)], WD, i);
        }
        return ret;
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix_ref<T,R,C>::iselect(const vector<T2,WD>& index_x,
                                            const vector_ref<T2,WD>& index_y)
{
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index_x.get(i) < R), WD, i);
            SIMDCF_WRAPPER(assert(index_y.get(i) < C), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(ret(i) = *data[index_x.get(i)*C + index_y.get(i)], WD, i);
        }
        return ret;
}

template <typename T, uint R, uint C>
template <typename T2, uint WD>
vector<T,WD> matrix_ref<T,R,C>::iselect(const vector_ref<T2,WD>& index_x,
                                            const vector_ref<T2,WD>& index_y)
{
        static const bool conformable1 = check_true<(WD > 0)>::value;
        static const bool type_conformable = is_inttype<T2>::value;
        assert(WD>=0 && R>=0 && C>=0);

        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(assert(index_x.get(i) < R), WD, i);
            SIMDCF_WRAPPER(assert(index_y.get(i) < C), WD, i);
        }

        vector<T,WD> ret(id());
        for (uint i=0; i < WD; i++) {
            SIMDCF_WRAPPER(ret(i) = *data[index_x.get(i)*C + index_y.get(i)], WD, i);
        }
        return ret;
}
//end of 2D iselect for matrix_ref

/*******************************************************************
/
/                         vector
/
*******************************************************************/
template <typename T, uint SZ>
void vector<T, SZ>::assign(const stream<T, SZ> &src) {
    uint i;
    for (i = 0; i < SZ; i++) {
        SIMDCF_WRAPPER((*this)(i) = src.get(i), SZ, i);
    }
}


/*******************************************************************
/
/                  global functions/operators
/
*******************************************************************/
template <typename T, uint SZ>
CM_NOINLINE vector<typename restype<T,int>::type,SZ> operator + (const stream<T,SZ> &x) {
    vector<typename restype<T,int>::type, SZ> ret;

    for (uint i=0; i<SZ; ++i) {
        SIMDCF_WRAPPER(ret(i) =  x.get(i), SZ, i);
    }

    return ret;
}

template <typename T, uint SZ>
CM_NOINLINE vector<typename restype<T,int>::type, SZ> operator - (const stream<T,SZ>& x) {
    vector<typename restype<T,int>::type, SZ> ret;
    
    for (uint i=0; i<SZ; ++i) {
        SIMDCF_WRAPPER(ret(i) = - x.get(i), SZ, i);
    }

    return ret;
}

template <typename T, uint SZ>
CM_NOINLINE vector<typename restype<T,int>::type, SZ> operator ~ (const stream<T,SZ>& x) {
    vector<typename restype<T,int>::type, SZ> ret;
    
    for (uint i=0; i<SZ; ++i) {
        SIMDCF_WRAPPER(ret(i) = ~ x.get(i), SZ, i);
    }

    return ret;
}

template <typename T, uint SZ>
CM_NOINLINE vector<ushort, SZ> operator ! (const stream<T,SZ>& x) {
    vector<ushort, SZ> ret;
    
    for (uint i=0; i<SZ; ++i) {
        SIMDCF_WRAPPER(ret(i) = ! x.get(i), SZ, i);
    }

    return ret;
}

#if 0

#define binary_arith_op(OP) \
\
template<typename T1, typename T2, uint SZ>\
CM_NOINLINE vector<typename restype<T1,T2>::type,SZ> operator OP (const stream<T1,SZ>& x, const stream<T2,SZ>& y)\
{\
        typedef typename restype<T1,T2>::type RT;\
        vector<RT,SZ> ret;\
        for (uint i=0; i<SZ; ++i) {\
                ret(i) = RT(x.get(i) OP y.get(i));\
        }\
        return ret;\
}\
\
template<typename T1, typename T2, uint SZ>\
CM_NOINLINE vector<typename restype<T1,T2>::type,SZ> operator OP (const stream<T1,SZ>& x, const T2 y)\
{\
        typedef typename restype<T1,T2>::type RT;\
        vector<RT,SZ> ret;\
        for (uint i=0; i<SZ; ++i) {\
                ret(i) = x.get(i) OP y;\
        }\
        return ret;\
}\
\
template<typename T1, typename T2, uint SZ>\
CM_NOINLINE vector<typename restype<T1,T2>::type,SZ> operator OP (const T1 x, const stream<T2,SZ>& y)\
{\
        typedef typename restype<T1,T2>::type RT;\
        vector<RT,SZ> ret;\
        for (uint i=0; i<SZ; ++i) {\
                ret(i) = x OP y.get(i);\
        }\
        return ret;\
}\

binary_arith_op(+)
binary_arith_op(-)
binary_arith_op(*)
binary_arith_op(/)
binary_arith_op(%)
binary_arith_op(&)
binary_arith_op(|)
binary_arith_op(^)
#undef binary_arith_op

#else

#define binary_arith_op(OP) \
\
template<typename T1, typename T2, uint SZ>\
CM_NOINLINE vector<typename restype<T1,T2>::type,SZ> operator OP (const stream<T1,SZ>& x, const stream<T2,SZ>& y)\
{\
        typedef typename restype<T1,T2>::type RT;\
        vector<RT,SZ> ret;\
        for (uint i=0; i<SZ; ++i) {\
            SIMDCF_WRAPPER(ret(i) = RT(x.get(i) OP y.get(i)), SZ, i);\
        }\
        return ret;\
}\
\
template<typename T1, typename T2, uint SZ>\
CM_NOINLINE vector<typename restype<T1,T2>::type,SZ> operator OP (const stream<T1,SZ>& x, const T2 y)\
{\
        typedef typename restype<T1,T2>::type RT;\
        vector<RT,SZ> ret;\
        RT _y = y; \
        for (uint i=0; i<SZ; ++i) {\
            SIMDCF_WRAPPER(ret(i) = x.get(i) OP _y, SZ, i);\
        }\
        return ret;\
}\
\
template<typename T1, typename T2, uint SZ>\
CM_NOINLINE vector<typename restype<T1,T2>::type,SZ> operator OP (const T1 x, const stream<T2,SZ>& y)\
{\
        typedef typename restype<T1,T2>::type RT;\
        vector<RT,SZ> ret;\
        RT _x (x); \
        for (uint i=0; i<SZ; ++i) {\
            SIMDCF_WRAPPER(ret(i) = _x OP y.get(i), SZ, i);\
        }\
        return ret;\
}\

binary_arith_op(+)
binary_arith_op(-)
binary_arith_op(*)
binary_arith_op(/)
binary_arith_op(%)
#undef binary_arith_op

#define binary_bitwise_op(OP) \
\
template<typename T1, typename T2, uint SZ>\
CM_NOINLINE vector<typename bitwise_restype<T1,T2>::type,SZ> operator OP (const stream<T1,SZ>& x, const stream<T2,SZ>& y)\
{\
        typedef typename bitwise_restype<T1,T2>::type RT;\
        static const bool type_conformable = \
            check_true<is_inttype<T1>::value && is_inttype<T2>::value>::value; \
        vector<RT,SZ> ret;\
        for (uint i=0; i<SZ; ++i) {\
            SIMDCF_WRAPPER(ret(i) = RT(x.get(i) OP y.get(i)), SZ, i);\
        }\
        return ret;\
}\
\
template<typename T1, typename T2, uint SZ>\
CM_NOINLINE vector<typename bitwise_restype<T1,T2>::type,SZ> operator OP (const stream<T1,SZ>& x, const T2 y)\
{\
        typedef typename bitwise_restype<T1,T2>::type RT;\
        static const bool type_conformable = \
            check_true<is_inttype<T1>::value && is_inttype<T2>::value>::value; \
        vector<RT,SZ> ret;\
        for (uint i=0; i<SZ; ++i) {\
            SIMDCF_WRAPPER(ret(i) = x.get(i) OP y, SZ, i);\
        }\
        return ret;\
}\
\
template<typename T1, typename T2, uint SZ>\
CM_NOINLINE vector<typename bitwise_restype<T1,T2>::type,SZ> operator OP (const T1 x, const stream<T2,SZ>& y)\
{\
        typedef typename bitwise_restype<T1,T2>::type RT;\
        static const bool type_conformable = \
            check_true<is_inttype<T1>::value && is_inttype<T2>::value>::value; \
        vector<RT,SZ> ret;\
        for (uint i=0; i<SZ; ++i) {\
            SIMDCF_WRAPPER(ret(i) = x OP y.get(i), SZ, i); \
        }\
        return ret;\
}\

binary_bitwise_op(&)
binary_bitwise_op(|)
binary_bitwise_op(^)
#undef binary_bitwise_op

#endif

template <bool, class T = void> struct cm_enable_if {};
template <class T> struct cm_enable_if<true, T> {
    typedef T type;
};

template <typename T, T v>
struct cm_integral_constant {
    typedef T value_type;
    static const value_type value = v;
    typedef cm_integral_constant<T, v> type;
    operator value_type() { return value; }
};

typedef cm_integral_constant<bool, true> cm_true_type;
typedef cm_integral_constant<bool, false> cm_false_type;

template<typename T, typename U> struct cm_is_same : public cm_false_type {};
template<typename T>             struct cm_is_same<T, T> : public cm_true_type{};

template <typename T> struct cm_remove_const          { typedef T type; };
template <typename T> struct cm_remove_const<const T> { typedef T type; };

template <typename T>
struct is_cm_scalar : cm_integral_constant <
    bool,
    cm_is_same<        float, typename cm_remove_const<T>::type>::value ||
    cm_is_same<       double, typename cm_remove_const<T>::type>::value ||
    cm_is_same<         char, typename cm_remove_const<T>::type>::value ||
    cm_is_same<  signed char, typename cm_remove_const<T>::type>::value ||
    cm_is_same<unsigned char, typename cm_remove_const<T>::type>::value ||
    cm_is_same<         short, typename cm_remove_const<T>::type>::value ||
    cm_is_same<unsigned short, typename cm_remove_const<T>::type>::value ||
    cm_is_same<         int, typename cm_remove_const<T>::type>::value ||
    cm_is_same<unsigned int, typename cm_remove_const<T>::type>::value ||
    cm_is_same<         long, typename cm_remove_const<T>::type>::value ||
    cm_is_same<unsigned long, typename cm_remove_const<T>::type>::value ||
    cm_is_same<         long long, typename cm_remove_const<T>::type>::value ||
    cm_is_same<unsigned long long, typename cm_remove_const<T>::type>::value >
{};


#define binary_shift_op(OP) \
\
template<typename T1, typename T2, uint SZ>\
CM_NOINLINE vector<typename int_uint_type<T1>::type,SZ> operator OP (const stream<T1,SZ>& x, const stream<T2,SZ>& y)\
{\
        typedef typename int_uint_type<T1>::type RT;\
        vector<RT,SZ> ret;\
        for (uint i=0; i<SZ; ++i) {\
            SIMDCF_WRAPPER(ret(i) = RT(x.get(i) OP y.get(i)), SZ, i);\
        }\
        return ret;\
}\
\
template<typename T1, typename T2, uint SZ>\
CM_NOINLINE typename cm_enable_if<is_cm_scalar<T2>::value, vector<typename int_uint_type<T1>::type,SZ> >::type operator OP (const stream<T1,SZ>& x, const T2 y)\
{\
        typedef typename int_uint_type<T1>::type RT;\
        vector<RT,SZ> ret;\
        for (uint i=0; i<SZ; ++i) {\
            SIMDCF_WRAPPER(ret(i) = x.get(i) OP y, SZ, i);\
        }\
        return ret;\
}\
\
template<typename T1, typename T2, uint SZ>\
CM_NOINLINE vector<typename int_uint_type<T1>::type,SZ> operator OP (const T1 x, const stream<T2,SZ>& y)\
{\
        typedef typename int_uint_type<T1>::type RT;\
        vector<RT,SZ> ret;\
        for (uint i=0; i<SZ; ++i) {\
            SIMDCF_WRAPPER(ret(i) = x OP y.get(i), SZ, i);\
        }\
        return ret;\
}\

binary_shift_op(>>)
binary_shift_op(<<)
#undef binary_shift_op


#define binary_compare_op(OP) \
\
template<typename T1, uint SZ, typename T2>\
CM_NOINLINE vector<typename ushort_type<T1,T2>::type, SZ> operator OP (const stream<T1, SZ>& x, const T2 y)\
{\
        static const bool type_conformable = cmtype<T2>::value; \
        vector<ushort, SZ> ret((ushort)0);\
        for (int i=0; i<SZ; i++) {\
            ret(i) = 0; \
            SIMDCF_ELEMENT_SKIP(i);\
            if (x.get(i) OP y) {\
                ret(i) = 1;\
            }\
        }\
        return ret;\
}\
\
template<typename T1, uint SZ, typename T2>\
CM_NOINLINE vector<typename ushort_type<T1,T2>::type, SZ> operator OP (const T1 x, const stream<T2, SZ>& y)\
{\
        static const bool type_conformable = cmtype<T1>::value; \
        vector<ushort, SZ> ret((ushort)0);\
        for (int i=0; i<SZ; i++) {\
            SIMDCF_ELEMENT_SKIP(i);\
            if (x OP y.get(i)) {\
                ret(i) = 1;\
            }\
        }\
        return ret;\
}\
\
template<typename T1, uint SZ, typename T2>\
CM_NOINLINE vector<ushort, SZ> operator OP (const stream<T1, SZ>& x, const stream<T2, SZ>& y)\
{\
        vector<ushort, SZ> ret((ushort)0);\
        for (int i=0; i<SZ; i++) {\
            SIMDCF_ELEMENT_SKIP(i);\
            if (x.get(i) OP y.get(i)) {\
                ret(i) = 1;\
            }\
        }\
        return ret;\
}\
\

binary_compare_op(<)
binary_compare_op(<=)
binary_compare_op(>)
binary_compare_op(>=)
binary_compare_op(==)
binary_compare_op(!=)

#define reduce_boolean_op(OP,ReduceOP,initValue)    \
\
template<typename T, uint SZ>\
CM_NOINLINE ushort stream<T, SZ>::OP ( void ) const    \
{\
        static const bool type_conformable = cmtype<T>::value; \
        ushort ret((ushort)initValue);\
        for (int i=0; i<SZ; i++) {\
            SIMDCF_WRAPPER(ret = (get(i) ReduceOP ret), SZ, i);     \
            if ( ret!=initValue ) { return ret; } \
        }\
        return ret;\
}\
\

//SIMDCF_WRAPPER(ret = (get(i) ReduceOP ret), i);     


reduce_boolean_op(any,||,0)
reduce_boolean_op(all,&&,1)

#endif /* CM_TYPES_H */

};