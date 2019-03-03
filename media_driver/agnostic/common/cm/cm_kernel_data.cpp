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
//! \file      cm_kernel_data.cpp 
//! \brief     Contains Class CmKernelData definitions 
//!

#include "cm_kernel_data.h"

#include "cm_device.h"
#include "cm_mem.h"

#define minimum(a,b) (((a)<(b))?(a):(b))

namespace CMRT_UMD
{
//*-----------------------------------------------------------------------------
//| Purpose:    Create Kernel Data
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelData::Create( CmKernelRT* kernel, CmKernelData*& kernelData )
{
    if(!kernel)
    {
        CM_ASSERTMESSAGE("Error: Invalid CmKernel.");
        return CM_NULL_POINTER;
    }

    int32_t result = CM_SUCCESS;
    kernelData = new (std::nothrow) CmKernelData( kernel );
    if( kernelData )
    {
        kernelData->Acquire();
        result = kernelData->Initialize();
        if( result != CM_SUCCESS )
        {
            CmKernelData::Destroy( kernelData );
        }
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to create CmKernelData due to out of system memory.");
        result = CM_OUT_OF_HOST_MEMORY;
    }
    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destroy CM Kernel Data
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelData::Destroy( CmKernelData* &kernelData )
{
    if(kernelData)
    {
        uint32_t refCount;
        refCount = kernelData->SafeRelease();
        if (refCount == 0)
        {
            kernelData = nullptr;
        }
    }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    CM Kernel Data constructor
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmKernelData::CmKernelData(  CmKernelRT* kernel ):
    m_kerneldatasize( 0 ),
    m_refCount(0),
    m_kernel(kernel),
    m_isInUse(true)
{
   CmSafeMemSet(&m_halKernelParam, 0, sizeof(CM_HAL_KERNEL_PARAM));
   m_halKernelParam.samplerHeap = MOS_New( std::list<SamplerParam> );
}

//*-----------------------------------------------------------------------------
//| Purpose:    CM Kernel Data Destructor
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmKernelData::~CmKernelData( void )
{
    //Free memory space for kernel arguments
    for(uint32_t i = 0; i< m_halKernelParam.numArgs; i++)
    {
        MosSafeDeleteArray(m_halKernelParam.argParams[i].firstValue);
    }
    for(uint32_t i = m_halKernelParam.numArgs; i< minimum(m_halKernelParam.numArgs+6, CM_MAX_ARGS_PER_KERNEL); i++)
    {
        MosSafeDeleteArray(m_halKernelParam.argParams[i].firstValue);
    }

    //Free memory for indirect data
    MosSafeDeleteArray(m_halKernelParam.indirectDataParam.indirectData);
    MosSafeDeleteArray(m_halKernelParam.indirectDataParam.surfaceInfo);

    //Free memory for thread space param
    MosSafeDeleteArray(m_halKernelParam.kernelThreadSpaceParam.dispatchInfo.numThreadsInWave);
    MosSafeDeleteArray(m_halKernelParam.kernelThreadSpaceParam.threadCoordinates);

    // Free memory for move instructions
    MosSafeDeleteArray(m_halKernelParam.movInsData);

    //Frees memory for sampler heap
    MosSafeDelete(m_halKernelParam.samplerHeap);
}

//*-----------------------------------------------------------------------------
//| Purpose:    Do nothing in initialization
//| Returns:    CM_SUCCESS
//*-----------------------------------------------------------------------------
int32_t CmKernelData::Initialize( void )
{

   return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get Kernel Data pointer
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelData::GetCmKernel( CmKernelRT*& kernel )
{
    kernel = m_kernel;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    SET Kernel Data pointer
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelData::SetKernelDataSize(int32_t value)
{
    m_kerneldatasize = value;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get Kernel Data pointer
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelData::GetKernelDataSize()
{
    return m_kerneldatasize;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Increase Reference count
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
uint32_t CmKernelData::Acquire( void )
{
    ++m_refCount;

    m_isInUse  = true; // reused or created

    return m_refCount;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destroy of Cm kernel data
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
uint32_t CmKernelData::SafeRelease( )
{

    --m_refCount;
    if( m_refCount == 0 )
    {
        delete this;
        return 0;
    }
    else
    {
        return m_refCount;
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get HalCmKernelData
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
PCM_HAL_KERNEL_PARAM CmKernelData::GetHalCmKernelData( )
{
    return &m_halKernelParam;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Whether the kernel data is in use
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
bool CmKernelData::IsInUse()
{
    return m_isInUse;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get Curbe Size from kernel data
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
uint32_t CmKernelData::GetKernelCurbeSize( void )
{
    return m_halKernelParam.totalCurbeSize;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Reset Kernel data status to IDLE. It will be called once kernel data flushed.
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmKernelData::ResetStatus( void )
{
    m_isInUse = false;

    return CM_SUCCESS;
}
}  // namespace
