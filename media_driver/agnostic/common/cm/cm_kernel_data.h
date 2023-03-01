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
//! \file      cm_kernel_data.h 
//! \brief     Contains Class CmKernelData definitions 
//!

#ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMKERNELDATA_H_
#define MEDIADRIVER_AGNOSTIC_COMMON_CM_CMKERNELDATA_H_

#include "cm_array.h"
#include "cm_hal.h"

namespace CMRT_UMD
{
class CmKernelRT;

class CmKernelData : public CmDynamicArray
{
public:

    static int32_t Create( CmKernelRT* kernel, CmKernelData*& kernelData );
    static int32_t Destroy( CmKernelData* &kernelData );

    int32_t GetCmKernel( CmKernelRT*& kernel );
    int32_t SetKernelDataSize(int32_t value);
    int32_t GetKernelDataSize();
    uint32_t Acquire(void);
    uint32_t SafeRelease(void);

    virtual PCM_HAL_KERNEL_PARAM GetHalCmKernelData( );
    bool IsInUse( void );
    uint32_t GetKernelCurbeSize( void );
    int32_t ResetStatus( void );

protected:

    CmKernelData( CmKernelRT* kernel );
    ~CmKernelData( void );

    int32_t Initialize( void );

    uint32_t     m_kerneldatasize;
    CmKernelRT*  m_kernel;
    uint32_t     m_refCount;
    CM_HAL_KERNEL_PARAM m_halKernelParam;

    // if it is Ture, it means the task with this kernel is not flushed yet
    bool         m_isInUse;

private:
    CmKernelData (const CmKernelData& other);
    CmKernelData& operator= (const CmKernelData& other);
};
};//namespace

#endif  // #ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMKERNELDATA_H_
