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
//! \file      cm_task_rt.cpp
//! \brief     Contains OS-agnostic CmTaskRT member functions.
//!

#include "cm_task_rt.h"

#include "cm_kernel_rt.h"
#include "cm_mem.h"
#include "cm_thread_space_rt.h"
#include "cm_device_rt.h"
#include "cm_surface_manager.h"
#include "cm_buffer_rt.h"

namespace CMRT_UMD
{
//*-----------------------------------------------------------------------------
//| Purpose:    Create CmTask
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskRT::Create(CmDeviceRT *device,
                         uint32_t index,
                         uint32_t maxKernelCount,
                         CmTaskRT* &kernelArray)
{
    int32_t result = CM_SUCCESS;
    kernelArray = new (std::nothrow) CmTaskRT( device, index, maxKernelCount );
    if( kernelArray )
    {
        result = kernelArray->Initialize();
        if( result != CM_SUCCESS )
        {
            CmTaskRT::Destroy( kernelArray);
        }
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to create CmTask due to out of system memory.");
        result = CM_OUT_OF_HOST_MEMORY;
    }
    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destroy CmTask
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskRT::Destroy( CmTaskRT* &kernelArray )
{
    if( kernelArray )
    {
        delete kernelArray;
        kernelArray = nullptr;
    }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Constructor of  CmTask
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmTaskRT::CmTaskRT(CmDeviceRT *device,
                   uint32_t index,
                   uint32_t maxKernelCount):
    m_kernelArray( nullptr ),
    m_kernelCount(0),
    m_maxKernelCount( maxKernelCount ),
    m_indexTaskArray(index),
    m_syncBitmap( 0 ),
    m_conditionalEndBitmap( 0 ),
    m_device( device )
{
    CmSafeMemSet( &m_powerOption, 0, sizeof( m_powerOption ) );
    CmSafeMemSet(&m_conditionalEndInfo, 0, sizeof(m_conditionalEndInfo));
    CmSafeMemSet(&m_taskConfig, 0, sizeof(m_taskConfig));
    m_taskConfig.turboBoostFlag = CM_TURBO_BOOST_DEFAULT;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destructor of  CmTask
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmTaskRT::~CmTaskRT( void )
{
    MosSafeDeleteArray(m_kernelArray );
}

//*-----------------------------------------------------------------------------
//| Purpose:    Initialize CmTask
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskRT::Initialize( )
{
    m_kernelArray = MOS_NewArray(CmKernelRT*, m_maxKernelCount);
    if(m_kernelArray)
    {
        CmSafeMemSet( m_kernelArray, 0, sizeof(CmKernelRT*) * m_maxKernelCount );
        return CM_SUCCESS;
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to initialize CmTask due to out of system memory.");
        return CM_OUT_OF_HOST_MEMORY;
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    Add Kernel to task
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmTaskRT::AddKernel( CmKernel *kernel )
{
    INSERT_API_CALL_LOG();

    // already reached max kernel count
    if(m_maxKernelCount <= m_kernelCount)
    {
        return CM_EXCEED_MAX_KERNEL_PER_ENQUEUE;
    }
    // passed in nullptr pointer
    if(kernel == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to kernel is null.");
        return CM_INVALID_ARG_VALUE;
    }

    CmKernelRT *kernelRT = static_cast<CmKernelRT *>(kernel);
    m_kernelArray[m_kernelCount] = kernelRT;
    kernelRT->SetIndexInTask(m_kernelCount);

    m_kernelCount++;

#if USE_EXTENSION_CODE
    AddKernelForGTPin(kernel);
#endif
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Reset task and clear all the kernel
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmTaskRT::Reset( void )
{
    INSERT_API_CALL_LOG();

    m_kernelCount = 0;
    m_syncBitmap = 0;
    m_conditionalEndBitmap = 0;
    CmSafeMemSet(&m_conditionalEndInfo, 0, sizeof(m_conditionalEndInfo));
    CmSafeMemSet(&m_taskConfig, 0, sizeof(m_taskConfig));
    m_taskConfig.turboBoostFlag = CM_TURBO_BOOST_DEFAULT;
    if(m_kernelArray)
    {
        CmSafeMemSet( m_kernelArray, 0, sizeof(CmKernelRT*) * m_maxKernelCount );
        return CM_SUCCESS;
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Pointer to kernel array is null.");
        return CM_NULL_POINTER;
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the kernel count
//| Returns:    The count of kernels
//*-----------------------------------------------------------------------------
uint32_t CmTaskRT::GetKernelCount()
{
    return m_kernelCount;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the kernel pointer by its index
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmKernelRT* CmTaskRT::GetKernelPointer(uint32_t index)
{
    if(index >= m_kernelCount)
    {
        CM_ASSERTMESSAGE("Error: The kernel index exceeds the kernel count.");
        return nullptr;
    }
    return m_kernelArray[index];
}

uint32_t CmTaskRT::GetIndexInTaskArray()
{
    return m_indexTaskArray;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Check the integrity of kernel threadspaces within a task
//| Returns:    True if all kernel threadspaces are valid, False otherwise
//*-----------------------------------------------------------------------------
bool CmTaskRT::IntegrityCheckKernelThreadspace( void )
{
    int32_t               hr                  = CM_SUCCESS;
    uint32_t              kernelCount         = 0;
    uint32_t              i                   = 0;
    uint32_t              j                   = 0;
    CmKernelRT*             kernelRT          = nullptr;
    CmKernelRT*             kernelTmp            = nullptr;
    uint32_t              threadCount         = 0;
    CmThreadSpaceRT*        kernelThreadSpace           = nullptr;
    uint32_t              width               = 0;
    uint32_t              height              = 0;
    byte**                threadSpaceMapping          = nullptr;
    byte*                 kernelInScoreboard = nullptr;
    CM_THREAD_SPACE_UNIT* threadSpaceUnit    = nullptr;
    uint32_t              kernelIndex         = 0;
    uint32_t              unassociated        = 0;

    kernelCount = this->GetKernelCount();

    threadSpaceMapping = MOS_NewArray(byte*, kernelCount);
    kernelInScoreboard = MOS_NewArray(byte, kernelCount);

    CM_CHK_NULL_GOTOFINISH(threadSpaceMapping, CM_OUT_OF_HOST_MEMORY);
    CM_CHK_NULL_GOTOFINISH(kernelInScoreboard, CM_OUT_OF_HOST_MEMORY);

    CmSafeMemSet(threadSpaceMapping, 0, kernelCount*sizeof(byte *));
    CmSafeMemSet(kernelInScoreboard, 0, kernelCount*sizeof(byte));

    for( i = 0; i < kernelCount; ++i )
    {
        kernelRT = this->GetKernelPointer(i);
        CM_CHK_NULL_GOTOFINISH_CMERROR(kernelRT);

        CM_CHK_CMSTATUS_GOTOFINISH(kernelRT->GetThreadSpace(kernelThreadSpace));
        CM_CHK_NULL_GOTOFINISH(kernelThreadSpace, CM_KERNEL_THREADSPACE_NOT_SET);

        CM_CHK_CMSTATUS_GOTOFINISH(kernelThreadSpace->GetThreadSpaceSize(width, height));
        CM_CHK_CMSTATUS_GOTOFINISH(kernelRT->GetThreadCount(threadCount));
        if (threadCount == 0)
        {
            threadCount = width * height;
        }

        if( kernelThreadSpace->IsThreadAssociated() )
        {
            threadSpaceMapping[i] = MOS_NewArray(byte, threadCount);
            CM_CHK_NULL_GOTOFINISH(threadSpaceMapping[i], CM_OUT_OF_HOST_MEMORY);
            CmSafeMemSet(threadSpaceMapping[i], 0, threadCount * sizeof(byte));
            kernelInScoreboard[i] = false;

            hr = kernelThreadSpace->GetThreadSpaceUnit(threadSpaceUnit);
            if( hr != CM_SUCCESS  || threadSpaceUnit == nullptr )
            {
                CM_ASSERTMESSAGE("Error: Invalid thread space unit");
                MosSafeDeleteArray(threadSpaceMapping[i]);
                hr = CM_FAILURE;
                goto finish;
            }

            for( j = 0; j < width * height; ++j )
            {
                kernelTmp = static_cast<CmKernelRT*>(threadSpaceUnit[j].kernel);
                if( kernelTmp == nullptr )
                {
                    if (kernelThreadSpace->GetNeedSetKernelPointer())
                    {
                        kernelTmp = kernelThreadSpace->GetKernelPointer();
                    }
                    if (kernelTmp == nullptr)
                    {
                        CM_ASSERTMESSAGE("Error: Invalid kernel pointer.");
                        MosSafeDeleteArray(threadSpaceMapping[i]);
                        hr = CM_FAILURE;
                        goto finish;
                    }
                }

                kernelIndex = kernelTmp->GetIndexInTask();
                threadSpaceMapping[kernelIndex][threadSpaceUnit[j].threadId] = 1;
                kernelInScoreboard[kernelIndex] = 1;
            }

            if( kernelInScoreboard[i] )
            {
                kernelRT->SetAssociatedToTSFlag(true);
                for( j = 0; j < threadCount; ++j )
                {
                    if( threadSpaceMapping[i][j] == 0 )
                    {
                        unassociated++;
                        break;
                    }
                }
            }
            MosSafeDeleteArray(threadSpaceMapping[i]);
        }

        if( unassociated != 0 )
        {
            CM_ASSERTMESSAGE("Error: kernel threadspace is not associated.");
            hr = CM_KERNEL_THREADSPACE_THREADS_NOT_ASSOCIATED;
            goto finish;
        }
    }

finish:

    MosSafeDeleteArray(threadSpaceMapping);
    MosSafeDeleteArray(kernelInScoreboard);

    return (hr == CM_SUCCESS)? true: false;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Insert synchronization point before next kernel
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmTaskRT::AddSync( void )
{
    INSERT_API_CALL_LOG();

    if (m_kernelCount > 0)
    {
        m_syncBitmap |= (uint64_t)1 << (m_kernelCount - 1);
    }

    return CM_SUCCESS;
}

uint64_t CmTaskRT::GetSyncBitmap()
{
    return m_syncBitmap;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Set the conditional value, compare mask and handle for the
//|             surface associated with index
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskRT::SetConditionalEndInfo(SurfaceIndex* index,
                                        uint32_t offset,
                                        CM_CONDITIONAL_END_PARAM *conditionalParam)
{
    CmSurface*        surface = nullptr;
    CmSurfaceManager* surfaceMgr = nullptr;
    uint32_t          surfIndex = 0;
    uint32_t          handle = 0;

    m_device->GetSurfaceManager(surfaceMgr);
    if (!surfaceMgr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to surface manager is null.");
        return CM_NULL_POINTER;
    }

    surfIndex = index->get_data();

    surfaceMgr->GetSurface(surfIndex, surface);
    if (!surface)
    {
        CM_ASSERTMESSAGE("Error: Pointer to surface is null.");
        return CM_NULL_POINTER;
    }

    if (surface->Type() == CM_ENUM_CLASS_TYPE_CMBUFFER_RT)
    {
        CmBuffer_RT* surf1D = static_cast<CmBuffer_RT*> (surface);

        surf1D->GetHandle(handle);
        m_conditionalEndInfo[m_kernelCount].compareValue = conditionalParam->opValue;
        m_conditionalEndInfo[m_kernelCount].bufferTableIndex = handle;
        m_conditionalEndInfo[m_kernelCount].disableCompareMask = !conditionalParam->opMask;
        m_conditionalEndInfo[m_kernelCount].endCurrentLevel = conditionalParam->opLevel;
        m_conditionalEndInfo[m_kernelCount].operatorCode = conditionalParam->opCode;
        m_conditionalEndInfo[m_kernelCount].offset = offset;
    }
    else
    {
        return CM_NOT_IMPLEMENTED;
    }

    return CM_SUCCESS;
}

uint64_t CmTaskRT::GetConditionalEndBitmap()
{
    return m_conditionalEndBitmap;
}

CM_HAL_CONDITIONAL_BB_END_INFO* CmTaskRT::GetConditionalEndInfo()
{
    return m_conditionalEndInfo;
}

PCM_POWER_OPTION CmTaskRT::GetPowerOption()
{
    return &m_powerOption;
}

#if CM_LOG_ON
std::string CmTaskRT::Log()
{
    std::ostringstream  oss;

    oss << " Kernel Count:" << m_kernelCount
        << " Sync Bit:"<<m_syncBitmap
        << " Conditional End Bit: " << m_conditionalEndBitmap
        << std::endl;

    for (uint32_t i=0 ; i< m_kernelCount; i++)
    {
        CmKernelRT* kernel = (CmKernelRT*)m_kernelArray[i];
        oss << kernel->Log(); // log each kernel
    }
    return oss.str();
}
#endif

CM_RT_API int32_t CmTaskRT::SetProperty(const CM_TASK_CONFIG &taskConfig)
{
    m_taskConfig = taskConfig;
    return CM_SUCCESS;
}

PCM_TASK_CONFIG CmTaskRT::GetTaskConfig()
{
    return &m_taskConfig;
}

CM_RT_API int32_t CmTaskRT::AddConditionalEnd(
                                SurfaceIndex* conditionalSurfaceIndex,
                                uint32_t offset,
                                CM_CONDITIONAL_END_PARAM *conditionalParam)
{
    INSERT_API_CALL_LOG();

    int32_t hr = CM_SUCCESS;

    m_conditionalEndBitmap |= (uint64_t)1 << m_kernelCount;

    hr = SetConditionalEndInfo(conditionalSurfaceIndex, offset, conditionalParam);

    return hr;
}
}