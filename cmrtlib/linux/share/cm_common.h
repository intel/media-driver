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

#ifndef CMRTLIB_LINUX_SHARE_CM_COMMON_H_
#define CMRTLIB_LINUX_SHARE_CM_COMMON_H_

#include "cm_include.h"

#ifndef __SURFACE_SAMPLER_INDEX_DEFINED__
#define __SURFACE_SAMPLER_INDEX_DEFINED__

class SurfaceIndex
{
public:
    CM_NOINLINE SurfaceIndex() { m_index = 0; };
    CM_NOINLINE SurfaceIndex(const SurfaceIndex& src) { m_index = src.m_index; };
    CM_NOINLINE SurfaceIndex(const unsigned int& n) { m_index = n; };
    CM_NOINLINE SurfaceIndex& operator = (const unsigned int& n) { this->m_index = n; return *this; };
    CM_NOINLINE SurfaceIndex& operator + (const unsigned int& n) { this->m_index += n; return *this; };
    virtual unsigned int get_data(void) { return m_index; };

private:
    unsigned int m_index;
    /*
     * Do not delete this line:
     * SurfaceIndex is commonly used as CM kernel function's parameter.
     * It has virutal table and has copy constructor, so GNU calling convetion will pass the object's pointer to kernel function.
     * This is different from MSVC, which always copies the entire object transferred on the callee's stack.
     *
     * Depending on the special object size after adding below "extra_byte",
     * SetKernelArg and SetThreadArg can recognize this object and follow GNU's convention to construct kernel function's stack.
     */
    unsigned char m_extraByte;
};

class SamplerIndex
{
public:
    CM_NOINLINE SamplerIndex() { m_index = 0; };
    CM_NOINLINE SamplerIndex(SamplerIndex& src) { m_index = src.get_data(); };
    CM_NOINLINE SamplerIndex(const unsigned int& n) { m_index = n; };
    CM_NOINLINE SamplerIndex& operator = (const unsigned int& n) { this->m_index = n; return *this; };
    virtual unsigned int get_data(void) { return m_index; };

private:
    unsigned int m_index;
    /*
     * Do not delete this line:
     * Same reason as SurfaceIndex.
     */
    unsigned char m_extraByte;
};

#endif /* __SURFACE_SAMPLER_INDEX_DEFINED__ */

#ifndef __VME_INDEX_DEFINED__
#define __VME_INDEX_DEFINED__

class VmeIndex

{
public:
    CM_NOINLINE VmeIndex () { m_index = 0; };
    CM_NOINLINE VmeIndex (VmeIndex& src) { m_index = src.get_data(); };
    CM_NOINLINE VmeIndex (const unsigned int& n) { m_index = n; };
    CM_NOINLINE VmeIndex & operator = (const unsigned int& n) { this->m_index = n; return *this; };
    virtual unsigned int get_data(void) { return m_index; };
private:
    unsigned int m_index;
    /*
     * Do not delete this line:
     * Same reason as SurfaceIndex.
     */
    unsigned char m_extraByte;
};

#endif /* __VME_INDEX_DEFINED__ */

#endif  // #ifndef CMRTLIB_LINUX_SHARE_CM_COMMON_H_
