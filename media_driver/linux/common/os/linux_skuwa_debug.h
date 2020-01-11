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
//! \file        linux_skuwa_debug.h 
//! \brief 
//!

#ifndef __LINUX_SKUWA_DEBUG_H__
#define __LINUX_SKUWA_DEBUG_H__

#if defined(ANDROID)
#include <log/log.h>

#define DEVINFO_ASSERT(expr) ALOG_ASSERT(expr)

#define DEVINFO_WARNING(msg) ALOGW(msg)
#define DEVINFO_ERROR(msg) ALOGE(msg)

#elif !defined(_WIN32) // Linux libskuwa
#include <stdio.h>
#include <assert.h>

#define DEVINFO_ASSERT(expr) assert(expr)

#define DEVINFO_WARNING(msg) printf("Warning: " msg "\n")
#define DEVINFO_ERROR(msg) printf("Error:" msg "\n")

#else // No debug capability for other modules
#define DEVINFO_ASSERT(expr)
#define DEVINFO_WARNING(msg)
#define DEVINFO_ERROR(msg)
#endif

#endif //__LINUX_SKUWA_DEBUG_H__
