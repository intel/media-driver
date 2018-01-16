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
int32_t CmTaskRT::Create(CmDeviceRT *pCmDevice,
                         uint32_t index,
                         uint32_t max_kernel_count,
                         CmTaskRT* &pKernelArray)
{
    int32_t result = CM_SUCCESS;
    pKernelArray = new (std::nothrow) CmTaskRT( pCmDevice, index, max_kernel_count );
    if( pKernelArray )
    {
        result = pKernelArray->Initialize();
        if( result != CM_SUCCESS )
        {
            CmTaskRT::Destroy( pKernelArray);
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
int32_t CmTaskRT::Destroy( CmTaskRT* &pKernelArray )
{
    if( pKernelArray )
    {
        delete pKernelArray;
        pKernelArray = nullptr;
    }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Constructor of  CmTask
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmTaskRT::CmTaskRT(CmDeviceRT *pCmDevice,
                   uint32_t index,
                   uint32_t max_kernel_count):
    m_pKernelArray( nullptr ),
    m_KernelCount(0),
    m_MaxKernelCount( max_kernel_count ),
    m_IndexTaskArray(index),
    m_ui64SyncBitmap( 0 ),
    m_ui64ConditionalEndBitmap( 0 ),
    m_pCmDev( pCmDevice )
{
    CmSafeMemSet( &m_PowerOption, 0, sizeof( m_PowerOption ) );
    CmSafeMemSet(&m_ConditionalEndInfo, 0, sizeof(m_ConditionalEndInfo));
    CmSafeMemSet(&m_TaskConfig, 0, sizeof(m_TaskConfig));
    m_TaskConfig.turboBoostFlag = CM_TURBO_BOOST_DEFAULT;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destructor of  CmTask
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmTaskRT::~CmTaskRT( void )
{
    MosSafeDeleteArray(m_pKernelArray );
}

//*-----------------------------------------------------------------------------
//| Purpose:    Initialize CmTask
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskRT::Initialize( )
{
    m_pKernelArray = MOS_NewArray(CmKernelRT*, m_MaxKernelCount);
    if(m_pKernelArray)
    {
        CmSafeMemSet( m_pKernelArray, 0, sizeof(CmKernelRT*) * m_MaxKernelCount );
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
CM_RT_API int32_t CmTaskRT::AddKernel( CmKernel *pKernel )
{
    INSERT_API_CALL_LOG();

    // already reached max kernel count
    if(m_MaxKernelCount <= m_KernelCount)
    {
        return CM_EXCEED_MAX_KERNEL_PER_ENQUEUE;
    }
    // passed in nullptr pointer
    if(pKernel == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to kernel is null.");
        return CM_INVALID_ARG_VALUE;
    }

    CmKernelRT *pKernelRT = static_cast<CmKernelRT *>(pKernel);
    m_pKernelArray[m_KernelCount] = pKernelRT;
    pKernelRT->SetIndexInTask(m_KernelCount);

    m_KernelCount++;

#if USE_EXTENSION_CODE
    AddKernelForGTPin(pKernel);
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

    m_KernelCount = 0;
    m_ui64SyncBitmap = 0;
    m_ui64ConditionalEndBitmap = 0;
    CmSafeMemSet(&m_ConditionalEndInfo, 0, sizeof(m_ConditionalEndInfo));
    CmSafeMemSet(&m_TaskConfig, 0, sizeof(m_TaskConfig));
    m_TaskConfig.turboBoostFlag = CM_TURBO_BOOST_DEFAULT;
    if(m_pKernelArray)
    {
        CmSafeMemSet( m_pKernelArray, 0, sizeof(CmKernelRT*) * m_MaxKernelCount );
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
    return m_KernelCount;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the kernel pointer by its index
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmKernelRT* CmTaskRT::GetKernelPointer(uint32_t index)
{
    if(index >= m_KernelCount)
    {
        CM_ASSERTMESSAGE("Error: The kernel index exceeds the kernel count.");
        return nullptr;
    }
    return m_pKernelArray[index];
}

uint32_t CmTaskRT::GetIndexInTaskArray()
{
    return m_IndexTaskArray;
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
    CmKernelRT*             pKernel_RT          = nullptr;
    CmKernelRT*             pKernTmp            = nullptr;
    uint32_t              threadCount         = 0;
    CmThreadSpaceRT*        pKernelTS           = nullptr;
    uint32_t              width               = 0;
    uint32_t              height              = 0;
    byte**                pTSMapping          = nullptr;
    byte*                 pKernelInScoreboard = nullptr;
    CM_THREAD_SPACE_UNIT* pThreadSpaceUnit    = nullptr;
    uint32_t              kernelIndex         = 0;
    uint32_t              unassociated        = 0;

    kernelCount = this->GetKernelCount();

    pTSMapping = MOS_NewArray(byte*, kernelCount);
    pKernelInScoreboard = MOS_NewArray(byte, kernelCount);

    CMCHK_NULL_RETURN(pTSMapping, CM_OUT_OF_HOST_MEMORY);
    CMCHK_NULL_RETURN(pKernelInScoreboard, CM_OUT_OF_HOST_MEMORY);

    CmSafeMemSet(pTSMapping, 0, kernelCount*sizeof(byte *));
    CmSafeMemSet(pKernelInScoreboard, 0, kernelCount*sizeof(byte));

    for( i = 0; i < kernelCount; ++i )
    {
        pKernel_RT = this->GetKernelPointer(i);
        CMCHK_NULL(pKernel_RT);

        CMCHK_HR(pKernel_RT->GetThreadSpace(pKernelTS));
        CMCHK_NULL_RETURN(pKernelTS, CM_KERNEL_THREADSPACE_NOT_SET);

        CMCHK_HR(pKernelTS->GetThreadSpaceSize(width, height));
        CMCHK_HR(pKernel_RT->GetThreadCount(threadCount));
        if (threadCount == 0)
        {
            threadCount = width * height;
        }

        if( pKernelTS->IsThreadAssociated() )
        {
            pTSMapping[i] = MOS_NewArray(byte, threadCount);
            CMCHK_NULL_RETURN(pTSMapping[i], CM_OUT_OF_HOST_MEMORY);
            CmSafeMemSet(pTSMapping[i], 0, threadCount * sizeof(byte));
            pKernelInScoreboard[i] = false;

            hr = pKernelTS->GetThreadSpaceUnit(pThreadSpaceUnit);
            if( hr != CM_SUCCESS  || pThreadSpaceUnit == nullptr )
            {
                CM_ASSERTMESSAGE("Error: Invalid thread space unit");
                MosSafeDeleteArray(pTSMapping[i]);
                hr = CM_FAILURE;
                goto finish;
            }

            for( j = 0; j < width * height; ++j )
            {
                pKernTmp = static_cast<CmKernelRT*>(pThreadSpaceUnit[j].pKernel);
                if( pKernTmp == nullptr )
                {
                    if (pKernelTS->GetNeedSetKernelPointer())
                    {
                        pKernTmp = pKernelTS->GetKernelPointer();
                    }
                    if (pKernTmp == nullptr)
                    {
                        CM_ASSERTMESSAGE("Error: Invalid kernel pointer.");
                        MosSafeDeleteArray(pTSMapping[i]);
                        hr = CM_FAILURE;
                        goto finish;
                    }
                }

                kernelIndex = pKernTmp->GetIndexInTask();
                pTSMapping[kernelIndex][pThreadSpaceUnit[j].threadId] = 1;
                pKernelInScoreboard[kernelIndex] = 1;
            }

            if( pKernelInScoreboard[i] )
            {
                pKernel_RT->SetAssociatedToTSFlag(true);
                for( j = 0; j < threadCount; ++j )
                {
                    if( pTSMapping[i][j] == 0 )
                    {
                        unassociated++;
                        break;
                    }
                }
            }
            MosSafeDeleteArray(pTSMapping[i]);
        }

        if( unassociated != 0 )
        {
            CM_ASSERTMESSAGE("Error: kernel threadspace is not associated.");
            hr = CM_KERNEL_THREADSPACE_THREADS_NOT_ASSOCIATED;
            goto finish;
        }
    }

finish:

    MosSafeDeleteArray(pTSMapping);
    MosSafeDeleteArray(pKernelInScoreboard);

    return (hr == CM_SUCCESS)? true: false;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Insert synchronization point before next kernel
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmTaskRT::AddSync( void )
{
    INSERT_API_CALL_LOG();

    if (m_KernelCount > 0)
    {
        m_ui64SyncBitmap |= (uint64_t)1 << (m_KernelCount - 1);
    }

    return CM_SUCCESS;
}

uint64_t CmTaskRT::GetSyncBitmap()
{
    return m_ui64SyncBitmap;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Set the conditional value, compare mask and handle for the
//|             surface associated with pIndex
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskRT::SetConditionalEndInfo(SurfaceIndex* pIndex,
                                        uint32_t offset,
                                        CM_CONDITIONAL_END_PARAM *pCondParam)
{
    CmSurface*        pSurface = nullptr;
    CmSurfaceManager* pSurfaceMgr = nullptr;
    uint32_t          surfIndex = 0;
    uint32_t          handle = 0;

    m_pCmDev->GetSurfaceManager(pSurfaceMgr);
    if (!pSurfaceMgr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to surface manager is null.");
        return CM_NULL_POINTER;
    }

    surfIndex = pIndex->get_data();

    pSurfaceMgr->GetSurface(surfIndex, pSurface);
    if (!pSurface)
    {
        CM_ASSERTMESSAGE("Error: Pointer to surface is null.");
        return CM_NULL_POINTER;
    }

    if (pSurface->Type() == CM_ENUM_CLASS_TYPE_CMBUFFER_RT)
    {
        CmBuffer_RT* pSurf1D = static_cast<CmBuffer_RT*> (pSurface);

        pSurf1D->GetHandle(handle);
        m_ConditionalEndInfo[m_KernelCount].compareValue = pCondParam->opValue;
        m_ConditionalEndInfo[m_KernelCount].bufferTableIndex = handle;
        m_ConditionalEndInfo[m_KernelCount].disableCompareMask = !pCondParam->opMask;
        m_ConditionalEndInfo[m_KernelCount].endCurrentLevel = pCondParam->opLevel;
        m_ConditionalEndInfo[m_KernelCount].operatorCode = pCondParam->opCode;
        m_ConditionalEndInfo[m_KernelCount].offset = offset;
    }
    else
    {
        return CM_NOT_IMPLEMENTED;
    }

    return CM_SUCCESS;
}

uint64_t CmTaskRT::GetConditionalEndBitmap()
{
    return m_ui64ConditionalEndBitmap;
}

CM_HAL_CONDITIONAL_BB_END_INFO* CmTaskRT::GetConditionalEndInfo()
{
    return m_ConditionalEndInfo;
}

PCM_POWER_OPTION CmTaskRT::GetPowerOption()
{
    return &m_PowerOption;
}

#if CM_LOG_ON
std::string CmTaskRT::Log()
{
    std::ostringstream  oss;

    oss << " Kernel Count:" << m_KernelCount
        << " Sync Bit:"<<m_ui64SyncBitmap
        << " Conditional End Bit: " << m_ui64ConditionalEndBitmap
        << std::endl;

    for (uint32_t i=0 ; i< m_KernelCount; i++)
    {
        CmKernelRT* pKernel = (CmKernelRT*)m_pKernelArray[i];
        oss << pKernel->Log(); // log each kernel
    }
    return oss.str();
}
#endif

CM_RT_API int32_t CmTaskRT::SetProperty(const CM_TASK_CONFIG &taskConfig)
{
    m_TaskConfig = taskConfig;
    return CM_SUCCESS;
}

PCM_TASK_CONFIG CmTaskRT::GetTaskConfig()
{
    return &m_TaskConfig;
}

CM_RT_API int32_t CmTaskRT::AddConditionalEnd(
                                SurfaceIndex* pConditionalSurface,
                                uint32_t offset,
                                CM_CONDITIONAL_END_PARAM *pCondParam)
{
    INSERT_API_CALL_LOG();

    int32_t hr = CM_SUCCESS;

    m_ui64ConditionalEndBitmap |= (uint64_t)1 << m_KernelCount;

    hr = SetConditionalEndInfo(pConditionalSurface, offset, pCondParam);

    return hr;
}
}