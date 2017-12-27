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
//! \file      cm_task_internal.cpp  
//! \brief     Contains Class CmTaskInternal  definitions  
//!

#include "cm_task_internal.h"

#include "cm_kernel_rt.h"
#include "cm_mem.h"
#include "cm_event_rt.h"
#include "cm_device_rt.h"
#include "cm_kernel_data.h"
#include "cm_thread_space_rt.h"
#include "cm_group_space.h"
#include "cm_vebox_rt.h"
#include "cm_vebox_data.h"
#include "cm_queue_rt.h"
#include "cm_surface_manager.h"
#include "cm_surface_2d_rt.h"

#if USE_EXTENSION_CODE
#include "cm_thread_space_ext.h"
#endif

namespace CMRT_UMD
{
//*-----------------------------------------------------------------------------
//| Purpose:    Create Task internal 
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::Create(const uint32_t kernelCount, const uint32_t totalThreadCount, CmKernelRT* pKernelArray[], const CmThreadSpaceRT* pTS, CmDeviceRT* pCmDevice, const uint64_t uiSyncBitmap, CmTaskInternal*& pTask, const uint64_t uiConditionalEndBitmap, PCM_HAL_CONDITIONAL_BB_END_INFO pConditionalEndInfo)
{
    int32_t result = CM_SUCCESS;
    pTask = new (std::nothrow) CmTaskInternal(kernelCount, totalThreadCount, pKernelArray, pCmDevice, uiSyncBitmap, uiConditionalEndBitmap, pConditionalEndInfo);
    if( pTask )
    {
        result = pTask->Initialize(pTS, false);
        if( result != CM_SUCCESS )
        {
            CmTaskInternal::Destroy( pTask);
        }
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to create CmTaskInternal due to out of system memory.");
        result = CM_OUT_OF_HOST_MEMORY;
    }
    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create Task internal with Thread Group Space 
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::Create( const uint32_t kernelCount, const uint32_t totalThreadCount, CmKernelRT* pKernelArray[], const CmThreadGroupSpace* pTGS, CmDeviceRT* pCmDevice, const uint64_t uiSyncBitmap, CmTaskInternal*& pTask )
{
    int32_t result = CM_SUCCESS;
    pTask = new (std::nothrow) CmTaskInternal(kernelCount, totalThreadCount, pKernelArray, pCmDevice, uiSyncBitmap, CM_NO_CONDITIONAL_END, nullptr);

    if( pTask )
    {
        result = pTask->Initialize(pTGS);
        if( result != CM_SUCCESS )
        {
            CmTaskInternal::Destroy( pTask);
        }
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to create CmTaskInternal due to out of system memory.");
        result = CM_OUT_OF_HOST_MEMORY;
    }
    return result;
}

int32_t CmTaskInternal::Create( CmDeviceRT* pCmDevice, CmVeboxRT* pVebox, CmTaskInternal*& pTask )
{
    int32_t result = CM_SUCCESS;
    pTask = new (std::nothrow) CmTaskInternal(0, 0, nullptr, pCmDevice, CM_NO_KERNEL_SYNC, CM_NO_CONDITIONAL_END, nullptr);
    if( pTask )
    {
        result = pTask->Initialize(pVebox);
        if( result != CM_SUCCESS )
        {
            CmTaskInternal::Destroy( pTask);
        }
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to create CmTaskInternal due to out of system memory.");
        result = CM_OUT_OF_HOST_MEMORY;
    }
    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create Task internal with hints
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::Create(const uint32_t kernelCount, const uint32_t totalThreadCount, CmKernelRT* pKernelArray[], CmTaskInternal*& pTask,  uint32_t numTasksGenerated, bool isLastTask, uint32_t hints, CmDeviceRT* pCmDevice)
{
    int32_t result = CM_SUCCESS;
    pTask = new (std::nothrow) CmTaskInternal(kernelCount, totalThreadCount, pKernelArray, pCmDevice, CM_NO_KERNEL_SYNC, CM_NO_CONDITIONAL_END, nullptr);
    if ( pTask )
    {
        result = pTask->Initialize(hints, numTasksGenerated, isLastTask);
        if ( result != CM_SUCCESS )
        {
            CmTaskInternal::Destroy( pTask );
        }
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to create CmTaskInternal due to out of system memory.");
        result = CM_OUT_OF_HOST_MEMORY;
    }
    return result;
}


//*-----------------------------------------------------------------------------
//| Purpose:    Destroy Task internal 
//| Returns:    None.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::Destroy( CmTaskInternal* &pTask )
{
    pTask->UpdateSurfaceStateOnTaskDestroy();
    CmSafeDelete( pTask );
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Constructor of  CmTaskInternal
//| Returns:    None.
//*-----------------------------------------------------------------------------
CmTaskInternal::CmTaskInternal(const uint32_t kernelCount, const uint32_t totalThreadCount, CmKernelRT* pKernelArray[], CmDeviceRT* pCmDevice, const uint64_t uiSyncBitmap, const uint64_t uiConditionalEndBitmap, PCM_HAL_CONDITIONAL_BB_END_INFO pConditionalEndInfo) :
    m_Kernels( kernelCount ),
    m_KernelData( kernelCount ),
    m_KernelCount( kernelCount ),
    m_TotalThreadCount(totalThreadCount),
    m_pTaskEvent( nullptr ),
    m_IsThreadSpaceCreated(false),
    m_IsThreadCoordinatesExisted(false),
    m_ThreadSpaceWidth(0),
    m_ThreadSpaceHeight(0),
    m_pThreadCoordinates(nullptr),
    m_DependencyPattern(CM_NONE_DEPENDENCY),
    m_WalkingPattern(CM_WALK_DEFAULT),
    m_MediaWalkerParamsSet( false ),
    m_DependencyVectorsSet( false ),
    m_pDependencyMasks( nullptr ),
    m_MediaWalkerGroupSelect(CM_MW_GROUP_NONE),
    m_IsThreadGroupSpaceCreated(false),
    m_GroupSpaceWidth(0),
    m_GroupSpaceHeight(0),
    m_SLMSize(0),
    m_SpillMemUsed(0),
    m_ColorCountMinusOne( 0 ),
    m_Hints(0),
    m_NumTasksGenerated( 0 ),
    m_IsLastTask( false ),
    m_ui64SyncBitmap (uiSyncBitmap ),
    m_ui64ConditionalEndBitmap(uiConditionalEndBitmap),
    m_pCmDevice( pCmDevice ),
    m_SurfaceArray (nullptr),
    m_IsSurfaceUpdateDone(false),
    m_TaskType(CM_TASK_TYPE_DEFAULT),
    m_media_state_ptr( nullptr )
{
    m_KernelSurfInfo.dwKrnNum = 0;
    m_KernelSurfInfo.pSurfEntryInfosArray = nullptr;
    m_pKernelCurbeOffsetArray = MOS_NewArray(uint32_t, kernelCount);
    CM_ASSERT(m_pKernelCurbeOffsetArray != nullptr);
    
    for( uint32_t i = 0 ; i < kernelCount; i ++ )
    {
        m_Kernels.SetElement( i, pKernelArray[ i ] );
        m_KernelData.SetElement( i, nullptr );
    }

    CmSafeMemSet( &m_WalkingParameters, 0, sizeof(m_WalkingParameters));
    CmSafeMemSet( &m_DependencyVectors, 0, sizeof(m_DependencyVectors));
    CmSafeMemSet( &m_TaskConfig, 0, sizeof(m_TaskConfig));
    if ( m_pKernelCurbeOffsetArray != nullptr )
    {
        CmSafeMemSet( m_pKernelCurbeOffsetArray, 0, sizeof(uint32_t) * kernelCount );
    }

    CmSafeMemSet(&m_TaskProfilingInfo, 0, sizeof(m_TaskProfilingInfo));

    if (pConditionalEndInfo != nullptr)
    {
        CmSafeMemCopy(&m_ConditionalEndInfo, pConditionalEndInfo, sizeof(m_ConditionalEndInfo));
    }
    else
    {
        CmSafeMemSet(&m_ConditionalEndInfo, 0, sizeof(m_ConditionalEndInfo));
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destructor of  CmTaskInternal
//| Returns:    None.
//*-----------------------------------------------------------------------------
CmTaskInternal::~CmTaskInternal( void )
{

    //Write Event Infos
    VtuneWriteEventInfo();

    //Release Profiling Info
    VtuneReleaseProfilingInfo();
    
    for( uint32_t i = 0; i < m_KernelCount; i ++ )
    {
        CmKernelRT *pKernel = (CmKernelRT*)m_Kernels.GetElement(i);
        CmKernelData* pKernelData = (CmKernelData*)m_KernelData.GetElement( i );
        if(pKernel && pKernelData)
        {
           pKernel->ReleaseKernelData(pKernelData);
           CmKernel *pKernelBase = pKernel;
           m_pCmDevice->DestroyKernel(pKernelBase);
        }
    }
    m_KernelData.Delete();
    m_Kernels.Delete();


    MosSafeDeleteArray(m_pKernelCurbeOffsetArray);

    if( m_pTaskEvent )
    {
        CmEvent *pEventBase = m_pTaskEvent;
        CmQueueRT *pCmQueue = nullptr;
        m_pTaskEvent->GetQueue(pCmQueue);
        pCmQueue->DestroyEvent(pEventBase); // need to update the m_EventArray
    }

    if(m_pThreadCoordinates){
        for (uint32_t i=0; i<m_KernelCount; i++)
        {
            if (m_pThreadCoordinates[i])
            {
                MosSafeDeleteArray(m_pThreadCoordinates[i]);
            } 
        }
        MosSafeDeleteArray( m_pThreadCoordinates );
    }

    if( m_pDependencyMasks )
    {
        for( uint32_t i = 0; i < m_KernelCount; ++i )
        {
            MosSafeDeleteArray(m_pDependencyMasks[i]);
        }
        MosSafeDeleteArray( m_pDependencyMasks );
    }

    if((m_KernelSurfInfo.dwKrnNum != 0)&&(m_KernelSurfInfo.pSurfEntryInfosArray != nullptr))
    {
        ClearKernelSurfInfo();
    }

    MosSafeDeleteArray(m_SurfaceArray);

}

//*-----------------------------------------------------------------------------
//| Purpose:    Initialize Class  CmTaskInternal
//| Returns:    None.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::Initialize(const CmThreadSpaceRT* pTS, bool isWithHints)
{
    uint32_t totalCurbeSize             = 0;
    uint32_t surfacePoolSize            = 0;
    uint32_t totalKernelBinarySize      = 0;
    uint32_t kernelCurbeSize            = 0;
    uint32_t kernelPayloadSize          = 0;
    CmSurfaceManager* pSurfaceMgr = nullptr;
    int32_t result              = CM_SUCCESS;
    CM_HAL_MAX_VALUES* pHalMaxValues = nullptr;
    CM_HAL_MAX_VALUES_EX* pHalMaxValuesEx = nullptr;
    m_pCmDevice->GetHalMaxValues( pHalMaxValues, pHalMaxValuesEx );
    PCM_HAL_STATE pCmHalState = ((PCM_CONTEXT_DATA)m_pCmDevice->GetAccelData())->pCmHalState;

    if (m_pCmDevice->IsPrintEnable())
    {
        SurfaceIndex *pPrintBufferIndex = nullptr;
        m_pCmDevice->GetPrintBufferIndex(pPrintBufferIndex);
        CM_ASSERT(pPrintBufferIndex);
        for (uint32_t i = 0; i < m_KernelCount; i++)
        {
            CmKernelRT* pKernel = (CmKernelRT*)m_Kernels.GetElement(i);
            if(pKernel == nullptr)
            {
                CM_ASSERTMESSAGE("Error: Invalid kernel pointer.");
                return CM_FAILURE;
            }
            if(FAILED(pKernel->SetStaticBuffer(CM_PRINTF_STATIC_BUFFER_ID, pPrintBufferIndex)))
            {
                CM_ASSERTMESSAGE("Error: Failed to set static buffer.");
                return CM_FAILURE;
            }
        }
    }

    m_pCmDevice->GetSurfaceManager( pSurfaceMgr );
    surfacePoolSize = pSurfaceMgr->GetSurfacePoolSize();

    m_SurfaceArray = MOS_NewArray(bool, surfacePoolSize);
    if (!m_SurfaceArray)
    {
        CM_ASSERTMESSAGE("Error: Out of system memory.");
        return CM_FAILURE;
    }
    CmSafeMemSet( m_SurfaceArray, 0, surfacePoolSize * sizeof( bool ) );

    for( uint32_t i = 0; i < m_KernelCount; i ++ )
    {

        CmKernelRT* pKernel = (CmKernelRT*)m_Kernels.GetElement( i );
        if(pKernel == nullptr)
        {
            CM_ASSERTMESSAGE("Error: Invalid kernel pointer.");
            return CM_FAILURE;
        }

        uint32_t totalSize =  0;
        CmKernelData* pKernelData = nullptr; 

        if ( isWithHints )
        {
            CmThreadSpaceRT* pKTS = nullptr;
            pKernel->GetThreadSpace(pKTS);
            if( pKTS )
            {
                for(uint32_t j = i; j > 0; --j)
                {
                    uint32_t width, height, myAdjY;
                    CmKernelRT* pTmpKern = (CmKernelRT*)m_Kernels.GetElement( j-1 );
                    if( !pTmpKern )
                    {
                        CM_ASSERTMESSAGE("Error: Invalid kernel pointer.");
                        return CM_FAILURE;
                    }
                    pTmpKern->GetThreadSpace(pKTS);
                    pKTS->GetThreadSpaceSize(width, height);
                    myAdjY = pKernel->GetAdjustedYCoord();
                    pKernel->SetAdjustedYCoord(myAdjY + height);
                }
            }
        }
#if USE_EXTENSION_CODE
        if (pTS == nullptr)
        {
            CmThreadSpaceRT* pKTS = nullptr;
            pKernel->GetThreadSpace(pKTS);
            if (pKTS && pKTS->threadSpaceExt)
            {
                pKTS->threadSpaceExt->SetPrivateArgToKernel(pKernel);
            }
        }

        if (pTS != nullptr && pTS->threadSpaceExt != nullptr)
        {
            pTS->threadSpaceExt->SetPrivateArgToKernel(pKernel);
        }
#endif
        pKernel->CollectKernelSurface();
        result = pKernel->CreateKernelData( pKernelData, totalSize, pTS );
        if( (pKernelData == nullptr) || (result != CM_SUCCESS))
        {
            CM_ASSERTMESSAGE("Error: Failed to create kernel data.");
            CmKernelData::Destroy( pKernelData );
            return result;
        }

        pKernel->GetSizeInPayload( kernelPayloadSize );
        pKernel->GetSizeInCurbe( kernelCurbeSize );

        if ( ( kernelCurbeSize + kernelPayloadSize ) > pHalMaxValues->iMaxArgByteSizePerKernel )
        {   //Failed, exceed the maximum of inline data
            CM_ASSERTMESSAGE("Error: Invalid kernel arg size.");
            return CM_EXCEED_KERNEL_ARG_SIZE_IN_BYTE;
        }
        else
        {
            kernelCurbeSize = pKernel->GetAlignedCurbeSize( kernelCurbeSize );
            totalCurbeSize += kernelCurbeSize;
        }
        m_pKernelCurbeOffsetArray[ i ] = totalCurbeSize - kernelCurbeSize;

        m_KernelData.SetElement( i, pKernelData );

        totalKernelBinarySize += pKernel->GetKernelGenxBinarySize();
        totalKernelBinarySize += CM_KERNEL_BINARY_PADDING_SIZE;  //Padding is necessary after kernel binary to avoid page fault issue

        bool *surfArray = nullptr;
        pKernel->GetKernelSurfaces(surfArray);
        for (uint32_t j = 0; j < surfacePoolSize; j ++)
        {
            m_SurfaceArray[j] |= surfArray[j];
        }
        pKernel->ResetKernelSurfaces();

        PCM_CONTEXT_DATA pCmData = ( PCM_CONTEXT_DATA )m_pCmDevice->GetAccelData();
        PCM_HAL_STATE pState = pCmData->pCmHalState;
        PRENDERHAL_MEDIA_STATE media_state_ptr = pState->pfnGetMediaStatePtrForKernel( pState, pKernel );

        if ( ( media_state_ptr != nullptr ) && ( m_media_state_ptr == nullptr ) )
        {
            m_media_state_ptr = media_state_ptr;
        }
        else if ( ( media_state_ptr != nullptr ) && ( m_media_state_ptr != nullptr ) )
        {
            CM_ASSERTMESSAGE( "Error: More than one media state heap are used in one task! User-provided state heap error.\n" );
            return CM_INVALID_ARG_VALUE;
        }
    }

    if (totalKernelBinarySize > pHalMaxValues->iMaxKernelBinarySize * pHalMaxValues->iMaxKernelsPerTask)
    {
        CM_ASSERTMESSAGE("Error: Invalid kernel arg size.");
        return CM_EXCEED_MAX_KERNEL_SIZE_IN_BYTE;
    }

    if (pTS)
    {
        if(FAILED(this->CreateThreadSpaceData(pTS)))
        {
            CM_ASSERTMESSAGE("Error: Failed to create thread space data.");
            return CM_FAILURE;
        }
        m_IsThreadSpaceCreated = true;
    }

    UpdateSurfaceStateOnTaskCreation();

    m_TaskType = CM_INTERNAL_TASK_WITH_THREADSPACE;

    if ( m_pCmDevice->CheckGTPinEnabled())
    {
        AllocateKernelSurfInfo();
    }

    this->VtuneInitProfilingInfo(pTS);

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Initialize Class  CmTaskInternal with thread group space
//| Returns:    None.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::Initialize(const CmThreadGroupSpace* pTGS)
{
    uint32_t totalCurbeSize         = 0;
    uint32_t surfacePoolSize        = 0;
    uint32_t totalKernelBinarySize  = 0;
    uint32_t kernelCurbeSize        = 0;
    uint32_t kernelPayloadSize      = 0;

    CmSurfaceManager* pSurfaceMgr = nullptr;
    CM_HAL_MAX_VALUES* pHalMaxValues = nullptr;
    CM_HAL_MAX_VALUES_EX* pHalMaxValuesEx = nullptr;
    m_pCmDevice->GetHalMaxValues( pHalMaxValues, pHalMaxValuesEx );

    m_pCmDevice->GetSurfaceManager( pSurfaceMgr );
    CM_ASSERT( pSurfaceMgr );
    surfacePoolSize = pSurfaceMgr->GetSurfacePoolSize();
    m_SurfaceArray = MOS_NewArray(bool, surfacePoolSize);
    if (!m_SurfaceArray)
    {
        CM_ASSERTMESSAGE("Error: Out of system memory.");
        return CM_OUT_OF_HOST_MEMORY;
    }
    CmSafeMemSet( m_SurfaceArray, 0, surfacePoolSize * sizeof( bool ) );

    if (m_pCmDevice->IsPrintEnable())
    {
        SurfaceIndex *pPrintBufferIndex = nullptr;
        m_pCmDevice->GetPrintBufferIndex(pPrintBufferIndex);
        CM_ASSERT(pPrintBufferIndex);
        for (uint32_t i = 0; i < m_KernelCount; i++)
        {
            CmKernelRT* pKernel = (CmKernelRT*)m_Kernels.GetElement(i);
            if(pKernel == nullptr)
            {
                CM_ASSERTMESSAGE("Error: Invalid kernel pointer.");
                return CM_FAILURE;
            }
            if(FAILED(pKernel->SetStaticBuffer(CM_PRINTF_STATIC_BUFFER_ID, pPrintBufferIndex)))
            {
                CM_ASSERTMESSAGE("Error: Failed to set static buffer.");
                return CM_FAILURE;
            }
        }
    }

    for( uint32_t i = 0; i < m_KernelCount; i ++ )
    {
        CmKernelRT* pKernel = (CmKernelRT*)m_Kernels.GetElement( i );
        if(pKernel == nullptr)
        {
            CM_ASSERTMESSAGE("Error: Invalid kernel pointer.");
            return CM_FAILURE;
        }

        pKernel->CollectKernelSurface();

        uint32_t totalSize =  0;
        CmKernelData* pKernelData = nullptr; 
        
        int32_t result = pKernel->CreateKernelData( pKernelData, totalSize, pTGS );
        if(result != CM_SUCCESS)
        {
            CM_ASSERTMESSAGE("Error: Failed to create kernel data.");
            CmKernelData::Destroy( pKernelData );
            return result;
        }

        pKernelData->SetKernelDataSize(totalSize);

        pKernel->GetSizeInPayload(kernelPayloadSize);
            
        PCM_HAL_KERNEL_PARAM  pHalKernelParam = pKernelData->GetHalCmKernelData();
        if (pHalKernelParam->iCrsThrdConstDataLn + pHalKernelParam->iCurbeSizePerThread + kernelPayloadSize > pHalMaxValues->iMaxArgByteSizePerKernel)
        {   //Failed, exceed the maximum of inline data
            CM_ASSERTMESSAGE("Error: Invalid kernel arg size.");
            return CM_EXCEED_KERNEL_ARG_SIZE_IN_BYTE;
        }
        else
        {
            pKernel->GetSizeInCurbe(kernelCurbeSize);
            kernelCurbeSize = pKernel->GetAlignedCurbeSize(kernelCurbeSize);
            totalCurbeSize += kernelCurbeSize;
        }
            
        m_pKernelCurbeOffsetArray[ i ] = totalCurbeSize - kernelCurbeSize;
                
        m_KernelData.SetElement( i, pKernelData );

        m_SLMSize = pKernel->GetSLMSize();

        m_SpillMemUsed = pKernel->GetSpillMemUsed();

        totalKernelBinarySize += pKernel->GetKernelGenxBinarySize();
        totalKernelBinarySize += CM_KERNEL_BINARY_PADDING_SIZE;

        bool *surfArray = nullptr;
        pKernel->GetKernelSurfaces(surfArray);
        for (uint32_t j = 0; j < surfacePoolSize; j ++)
        {
            m_SurfaceArray[j] |= surfArray[j];
        }
        pKernel->ResetKernelSurfaces();

        PCM_CONTEXT_DATA pCmData = ( PCM_CONTEXT_DATA )m_pCmDevice->GetAccelData();
        PCM_HAL_STATE pState = pCmData->pCmHalState;
        PRENDERHAL_MEDIA_STATE media_state_ptr = pState->pfnGetMediaStatePtrForKernel( pState, pKernel );

        if ( ( media_state_ptr != nullptr ) && ( m_media_state_ptr == nullptr ) )
        {
            m_media_state_ptr = media_state_ptr;
        }
        else if ( ( media_state_ptr != nullptr ) && ( m_media_state_ptr != nullptr ) )
        {
            CM_ASSERTMESSAGE("Error: More than one media state heap are used in one task! User-provided state heap error.\n" );
            return CM_INVALID_ARG_VALUE;
        }
    }

    if( totalKernelBinarySize > pHalMaxValues->iMaxKernelBinarySize * pHalMaxValues->iMaxKernelsPerTask)
    {
        CM_ASSERTMESSAGE("Error: Invalid kernel arg size.");
        return CM_EXCEED_MAX_KERNEL_SIZE_IN_BYTE;
    }

    UpdateSurfaceStateOnTaskCreation();

    m_TaskType = CM_INTERNAL_TASK_WITH_THREADGROUPSPACE;

    if (pTGS)
    {
        pTGS->GetThreadGroupSpaceSize(m_ThreadSpaceWidth, m_ThreadSpaceHeight, m_ThreadSpaceDepth,  m_GroupSpaceWidth, m_GroupSpaceHeight, m_GroupSpaceDepth);
        m_IsThreadGroupSpaceCreated = true;
    }

    if ( m_pCmDevice->CheckGTPinEnabled())
    {
        AllocateKernelSurfInfo();
    }
    
    this->VtuneInitProfilingInfo(pTGS);
    
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Initialize Class  CmTaskInternal
//| Returns:    None.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::Initialize(CmVeboxRT* pVebox)
{
    int32_t result = CM_SUCCESS; 
    CmSurfaceManager* pSurfaceMgr = nullptr;
    uint32_t surfacePoolSize = 0;

    m_pCmDevice->GetSurfaceManager( pSurfaceMgr );
    CM_ASSERT( pSurfaceMgr );
    surfacePoolSize = pSurfaceMgr->GetSurfacePoolSize();
    m_SurfaceArray = MOS_NewArray(bool, surfacePoolSize);
    if (!m_SurfaceArray)
    {
        CM_ASSERTMESSAGE("Error: Out of system memory.");
        return CM_FAILURE;
    }
    CmSafeMemSet( m_SurfaceArray, 0, surfacePoolSize * sizeof( bool ) );

    CmBufferUP *pParamBuffer = nullptr;

    pParamBuffer = pVebox->GetParam();
    m_VeboxState = pVebox->GetState();


    m_pVeboxParam = pParamBuffer;
    m_TaskType = CM_INTERNAL_TASK_VEBOX;

    //Update used surfaces
    for (int i = 0; i < VEBOX_SURFACE_NUMBER; i++)
    {
        CmSurface2DRT* pSurf = nullptr;
        uint32_t surfaceHandle = 0;
        pVebox->GetSurface(i, pSurf);
        if (pSurf)
        {
            SurfaceIndex* pSurfIndex = nullptr;
            pSurf->GetIndex(pSurfIndex);
            pSurf->GetHandle(surfaceHandle);
            m_SurfaceArray[pSurfIndex->get_data()] = true;
            m_VeboxSurfaceData.surfaceEntry[i].wSurfaceIndex = (uint16_t)surfaceHandle;
            m_VeboxSurfaceData.surfaceEntry[i].wSurfaceCtrlBits = pVebox->GetSurfaceControlBits(i);
        }
        else
        {
            m_VeboxSurfaceData.surfaceEntry[i].wSurfaceIndex = CM_INVALID_INDEX; 
            m_VeboxSurfaceData.surfaceEntry[i].wSurfaceCtrlBits = CM_INVALID_INDEX;  
        }
    }


    UpdateSurfaceStateOnTaskCreation();

    return result;
}


//*-----------------------------------------------------------------------------
//| Purpose:    Initialize Class  CmTaskInternal with hints
//| Returns:    Result of the operation
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::Initialize(uint32_t hints, uint32_t numTasksGenerated, bool isLastTask)
{
    CmThreadSpaceRT* pTS = nullptr;
    int32_t result = CM_SUCCESS;

    // use ThreadSpace Initialize function to create kernel data
    result = this->Initialize(pTS, true);

    // set hints in task
    m_Hints = hints;

    m_NumTasksGenerated = numTasksGenerated;
    m_IsLastTask = isLastTask;

    // set task type to be EnqueueWithHints
    m_TaskType = CM_INTERNAL_TASK_ENQUEUEWITHHINTS;

    return result;
}


//*-----------------------------------------------------------------------------
//| Purpose:    Get Kernel Count
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetKernelCount( uint32_t& count )
{
    count = m_KernelCount;
    return CM_SUCCESS;
}

int32_t CmTaskInternal::GetTaskSurfaces( bool  *&surfArray )
{
    surfArray = m_SurfaceArray;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Geth Kernel from the Kernel array
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetKernel( const uint32_t index, CmKernelRT* & pKernel )
{
    pKernel = nullptr;
    if( index < m_Kernels.GetSize() )
    {
        pKernel = (CmKernelRT*)m_Kernels.GetElement( index );
        return CM_SUCCESS;
    }
    else
    {
        return CM_FAILURE;
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    Geth Kernel data by kernel's index
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetKernelData( const uint32_t index, CmKernelData* & pKernelData )
{
    pKernelData = nullptr;
    if( index < m_KernelData.GetSize() )
    {
        pKernelData = (CmKernelData*)m_KernelData.GetElement( index );
        return CM_SUCCESS;
    }
    else
    {
        return CM_FAILURE;
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    Geth Kernel data size by kernel's index
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetKernelDataSize( const uint32_t index, uint32_t & size )
{
    size = 0;
    CmKernelData*  pKernelData = nullptr;
    if( index < m_KernelData.GetSize() )
    {
        pKernelData = (CmKernelData*)m_KernelData.GetElement( index );
        if (pKernelData == nullptr)
        {
            CM_ASSERTMESSAGE("Error: Invalid kernel data.");
            return CM_FAILURE;
        }
        size = pKernelData->GetKernelDataSize();
        return CM_SUCCESS;
    }
    else
    {
        return CM_FAILURE;
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get kernel's curbe offset
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------
uint32_t CmTaskInternal::GetKernelCurbeOffset( const uint32_t index )
{
    return ( uint32_t ) m_pKernelCurbeOffsetArray[ index ];
}

//*-----------------------------------------------------------------------------
//| Purpose:    Set task event, need add refcount hehe.
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::SetTaskEvent( CmEventRT* pEvent )
{
    m_pTaskEvent = pEvent;
    // add refCount
     m_pTaskEvent->Acquire();
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the task event
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetTaskEvent( CmEventRT* & pEvent )
{
    pEvent = m_pTaskEvent;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the task's status
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetTaskStatus(CM_STATUS & TaskStatus)
{
    if(m_pTaskEvent == nullptr)
    {
        return CM_FAILURE;
    }

    return m_pTaskEvent->GetStatusNoFlush(TaskStatus);
}

//*-----------------------------------------------------------------------------
//| Purpose:    Record CPU ticks for Flush Time
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::VtuneSetFlushTime()
{
    if(!m_pCmDevice->IsVtuneLogOn())
    {   // return directly if ETW log is off 
        return CM_SUCCESS;
    }
    
    MOS_QueryPerformanceCounter((uint64_t*)&m_TaskProfilingInfo.FlushTime.QuadPart);
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Initialize Profiling Information for Media Pipeline
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::VtuneInitProfilingInfo(const CmThreadSpaceRT *pPerTaskTs)
{
    CmKernelRT    *pCmKernel = nullptr;
    CmThreadSpaceRT *pPerKernelTS = nullptr;
    uint32_t    TsWidth = 0;
    uint32_t    TsHeight = 0;

    int32_t     hr = CM_SUCCESS;

    if(!m_pCmDevice->IsVtuneLogOn())
    {   // return directly if ETW log is off 
        return CM_SUCCESS;
    }

    CmSafeMemSet(&m_TaskProfilingInfo, 0, sizeof(m_TaskProfilingInfo));
    m_TaskProfilingInfo.dwKernelCount = m_KernelCount;
    m_TaskProfilingInfo.dwThreadID    = CmGetCurThreadId(); // Get Thread ID

    MOS_QueryPerformanceCounter((uint64_t*)&m_TaskProfilingInfo.EnqueueTime.QuadPart); // Get Enqueue Time


    //  Currently, the Kernel/ThreadSpace/ThreadGroupSpace could not be deleted before task finished.
    m_TaskProfilingInfo.pKernelNames = MOS_NewArray(char, (CM_MAX_KERNEL_NAME_SIZE_IN_BYTE * m_KernelCount));
    CMCHK_NULL(m_TaskProfilingInfo.pKernelNames);

    m_TaskProfilingInfo.pLocalWorkWidth = MOS_NewArray(uint32_t, m_KernelCount);
    CMCHK_NULL(m_TaskProfilingInfo.pLocalWorkWidth);

    m_TaskProfilingInfo.pLocalWorkHeight = MOS_NewArray(uint32_t, m_KernelCount);
    CMCHK_NULL(m_TaskProfilingInfo.pLocalWorkHeight);

    m_TaskProfilingInfo.pGlobalWorkWidth = MOS_NewArray(uint32_t, m_KernelCount);
    CMCHK_NULL(m_TaskProfilingInfo.pGlobalWorkWidth);

    m_TaskProfilingInfo.pGlobalWorkHeight = MOS_NewArray(uint32_t, m_KernelCount);
    CMCHK_NULL(m_TaskProfilingInfo.pGlobalWorkHeight);

    for (uint32_t i = 0; i < m_KernelCount; i++)
    {
        CMCHK_HR(GetKernel(i, pCmKernel));
        CMCHK_NULL(pCmKernel);

        //Copy Kernel Name
        MOS_SecureStrcpy(m_TaskProfilingInfo.pKernelNames + m_TaskProfilingInfo.dwKernelNameLen, 
                 CM_MAX_KERNEL_NAME_SIZE_IN_BYTE, pCmKernel->GetName());

        //Add Kernel Name Length
        m_TaskProfilingInfo.dwKernelNameLen += strlen(pCmKernel->GetName()) + 1;

        CMCHK_HR(pCmKernel->GetThreadSpace(pPerKernelTS));

        if (pPerTaskTs)
        {
            //Per Task Thread Space Exists
            m_TaskProfilingInfo.pLocalWorkWidth[i] = m_ThreadSpaceWidth;
            m_TaskProfilingInfo.pLocalWorkHeight[i] = m_ThreadSpaceHeight;
            m_TaskProfilingInfo.pGlobalWorkWidth[i] = m_ThreadSpaceWidth;
            m_TaskProfilingInfo.pGlobalWorkHeight[i] = m_ThreadSpaceHeight;
        }
        else if (pPerKernelTS)
        {
            //Fill each threads Space's info
            pPerKernelTS->GetThreadSpaceSize(TsWidth, TsHeight);
            m_TaskProfilingInfo.pLocalWorkWidth[i] = TsWidth;
            m_TaskProfilingInfo.pLocalWorkHeight[i] = TsHeight;
            m_TaskProfilingInfo.pGlobalWorkWidth[i] = TsWidth;
            m_TaskProfilingInfo.pGlobalWorkHeight[i] = TsHeight;
        }
        else
        {
            //Fill the thread count 
            uint32_t ThreadCount = 0;
            pCmKernel->GetThreadCount(ThreadCount);
            m_TaskProfilingInfo.pLocalWorkWidth[i] = ThreadCount;
            m_TaskProfilingInfo.pLocalWorkHeight[i] = 1;
            m_TaskProfilingInfo.pGlobalWorkWidth[i] = ThreadCount;
            m_TaskProfilingInfo.pGlobalWorkHeight[i] = 1;
        }

    }

finish:
    if (hr != CM_SUCCESS)
    {
        MosSafeDeleteArray(m_TaskProfilingInfo.pKernelNames);
        MosSafeDeleteArray(m_TaskProfilingInfo.pLocalWorkWidth);
        MosSafeDeleteArray(m_TaskProfilingInfo.pLocalWorkHeight);
        MosSafeDeleteArray(m_TaskProfilingInfo.pGlobalWorkWidth);
        MosSafeDeleteArray(m_TaskProfilingInfo.pGlobalWorkHeight);
    }
    return hr;

}

//*-----------------------------------------------------------------------------
//| Purpose:    Initialize Profiling Information
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::VtuneInitProfilingInfo(const CmThreadGroupSpace *pPerTaskThreadGroupSpace)
{
    CmKernelRT    *pCmKernel = nullptr;
    CmThreadGroupSpace *pPerKernelGroupSpace = nullptr;
    uint32_t    TsWidth = 0;
    uint32_t    TsHeight = 0;
    uint32_t    TsDepth = 0;
    uint32_t    TgsWidth = 0;
    uint32_t    TgsHeight = 0;
    uint32_t    TgsDepth = 0;
    int32_t     hr = CM_SUCCESS;

    if(!m_pCmDevice->IsVtuneLogOn())
    {   // return directly if ETW log is off 
        return CM_SUCCESS;
    }

    CmSafeMemSet(&m_TaskProfilingInfo, 0, sizeof(m_TaskProfilingInfo));
    m_TaskProfilingInfo.dwKernelCount = m_KernelCount;

    m_TaskProfilingInfo.dwThreadID    = CmGetCurThreadId(); // Get Thread ID

    MOS_QueryPerformanceCounter((uint64_t*)&m_TaskProfilingInfo.EnqueueTime.QuadPart); // Get Enqueue Time

    m_TaskProfilingInfo.pKernelNames = MOS_NewArray(char, (CM_MAX_KERNEL_NAME_SIZE_IN_BYTE * m_KernelCount));
    CMCHK_NULL(m_TaskProfilingInfo.pKernelNames);

    m_TaskProfilingInfo.pLocalWorkWidth = MOS_NewArray(uint32_t, m_KernelCount);
    CMCHK_NULL(m_TaskProfilingInfo.pLocalWorkWidth);

    m_TaskProfilingInfo.pLocalWorkHeight = MOS_NewArray(uint32_t, m_KernelCount);
    CMCHK_NULL(m_TaskProfilingInfo.pLocalWorkHeight);

    m_TaskProfilingInfo.pGlobalWorkWidth = MOS_NewArray(uint32_t, m_KernelCount);
    CMCHK_NULL(m_TaskProfilingInfo.pGlobalWorkWidth);

    m_TaskProfilingInfo.pGlobalWorkHeight = MOS_NewArray(uint32_t, m_KernelCount);
    CMCHK_NULL(m_TaskProfilingInfo.pGlobalWorkHeight);

    for (uint32_t i = 0; i < m_KernelCount; i++)
    {
        CMCHK_HR(GetKernel(i, pCmKernel));
        CMCHK_NULL(pCmKernel);
        
        //Copy Kernel Name
        MOS_SecureStrcpy(m_TaskProfilingInfo.pKernelNames + m_TaskProfilingInfo.dwKernelNameLen,
                 CM_MAX_KERNEL_NAME_SIZE_IN_BYTE, pCmKernel->GetName());

        //Add Kernel Name Length
        m_TaskProfilingInfo.dwKernelNameLen += strlen(pCmKernel->GetName()) + 1;

        CMCHK_HR(pCmKernel->GetThreadGroupSpace(pPerKernelGroupSpace));

        if (pPerTaskThreadGroupSpace)
        {  // Per Thread Group Space
            pPerTaskThreadGroupSpace->GetThreadGroupSpaceSize(TsWidth, TsHeight, TsDepth, TgsWidth, TgsHeight, TgsDepth); 
            m_TaskProfilingInfo.pLocalWorkWidth[i] = TsWidth;
            m_TaskProfilingInfo.pLocalWorkHeight[i] = TsHeight;
            m_TaskProfilingInfo.pGlobalWorkWidth[i] = TsWidth*TgsWidth;
            m_TaskProfilingInfo.pGlobalWorkHeight[i] = TsHeight*TgsHeight;

        }
        else if (pPerKernelGroupSpace)
        {
            //Fill each threads group space's info
            pPerKernelGroupSpace->GetThreadGroupSpaceSize(TsWidth, TsHeight, TsDepth, TgsWidth, TgsHeight, TgsDepth);
            m_TaskProfilingInfo.pLocalWorkWidth[i] = TsWidth;
            m_TaskProfilingInfo.pLocalWorkHeight[i] = TsHeight;
            m_TaskProfilingInfo.pGlobalWorkWidth[i] = TsWidth*TgsWidth;
            m_TaskProfilingInfo.pGlobalWorkHeight[i] = TsHeight*TgsHeight;  //Yi need to rethink
        }

    }

finish:
    if (hr != CM_SUCCESS)
    {
        MosSafeDeleteArray(m_TaskProfilingInfo.pKernelNames);
        MosSafeDeleteArray(m_TaskProfilingInfo.pLocalWorkWidth);
        MosSafeDeleteArray(m_TaskProfilingInfo.pLocalWorkHeight);
        MosSafeDeleteArray(m_TaskProfilingInfo.pGlobalWorkWidth);
        MosSafeDeleteArray(m_TaskProfilingInfo.pGlobalWorkHeight);
    }
    return hr;
}


//*-----------------------------------------------------------------------------
//| Purpose:    Release Profiling information
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::VtuneReleaseProfilingInfo()
{
    if(!m_pCmDevice->IsVtuneLogOn())
    {   // return directly if ETW log is off 
        return CM_SUCCESS;
    }
    
    MosSafeDeleteArray(m_TaskProfilingInfo.pKernelNames);
    MosSafeDeleteArray(m_TaskProfilingInfo.pLocalWorkWidth);
    MosSafeDeleteArray(m_TaskProfilingInfo.pLocalWorkHeight);
    MosSafeDeleteArray(m_TaskProfilingInfo.pGlobalWorkWidth);
    MosSafeDeleteArray(m_TaskProfilingInfo.pGlobalWorkHeight);

    return CM_SUCCESS;
}


//*-----------------------------------------------------------------------------
//| Purpose:    Reset KernelData status from IN_USE to IDLE. 
//              It is called immediately after the task being flushed.
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::ResetKernelDataStatus()
{
    int32_t     hr          = CM_SUCCESS;

    for(uint32_t KrnDataIndex =0 ; KrnDataIndex < m_KernelCount; KrnDataIndex++ )
    {
        CmKernelData    *pKernelData;
        CMCHK_HR(GetKernelData(KrnDataIndex, pKernelData));
        CMCHK_NULL(pKernelData);
        CMCHK_HR(pKernelData->ResetStatus());
    }

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create thread space data
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::CreateThreadSpaceData(const CmThreadSpaceRT* pTS)
{
    uint32_t i;
    uint32_t width, height;
    uint32_t *pKernelCoordinateIndex = nullptr;
    int hr = CM_SUCCESS;
    CmThreadSpaceRT *pTS_RT = const_cast<CmThreadSpaceRT*>(pTS);
    CmKernelRT* pKernel_inTS = nullptr;
    CmKernelRT* pKernel_inTask = nullptr;
    CMCHK_NULL_RETURN(pTS_RT, CM_NULL_POINTER);

    pTS_RT->GetThreadSpaceSize(m_ThreadSpaceWidth, m_ThreadSpaceHeight);

    if (pTS_RT->IsThreadAssociated())
    {
        m_pThreadCoordinates = MOS_NewArray(PCM_HAL_SCOREBOARD, m_KernelCount);
        CMCHK_NULL_RETURN(m_pThreadCoordinates, CM_FAILURE);
        CmSafeMemSet(m_pThreadCoordinates, 0, m_KernelCount*sizeof(PCM_HAL_SCOREBOARD));
        
        m_pDependencyMasks = MOS_NewArray(PCM_HAL_MASK_AND_RESET, m_KernelCount);
        CMCHK_NULL_RETURN(m_pDependencyMasks, CM_FAILURE);
        CmSafeMemSet(m_pDependencyMasks, 0, m_KernelCount*sizeof(PCM_HAL_MASK_AND_RESET));
        
        pKernelCoordinateIndex = MOS_NewArray(uint32_t, m_KernelCount);
        if(m_pThreadCoordinates && pKernelCoordinateIndex && m_pDependencyMasks)
        {
            CmSafeMemSet(pKernelCoordinateIndex, 0, m_KernelCount*sizeof(uint32_t));
            for (i = 0; i< m_KernelCount; i++)
            {
                pKernelCoordinateIndex[i] = 0;
                uint32_t threadCount;
                this->GetKernel(i, pKernel_inTask);

                if(pKernel_inTask == nullptr)
                {
                    CM_ASSERTMESSAGE("Error: Invalid kernel pointer in task.");
                    hr = CM_NULL_POINTER;
                    goto finish;
                }

                pKernel_inTask->GetThreadCount(threadCount);
                if (threadCount == 0)
                {
                    threadCount = m_ThreadSpaceWidth*m_ThreadSpaceHeight;
                }
                m_pThreadCoordinates[i] = MOS_NewArray(CM_HAL_SCOREBOARD, threadCount);
                if (m_pThreadCoordinates[i])
                {
                    CmSafeMemSet(m_pThreadCoordinates[i], 0, sizeof(CM_HAL_SCOREBOARD)* threadCount);
                }
                else 
                {
                    CM_ASSERTMESSAGE("Error: Pointer to thread coordinates is null.");
                    hr = CM_NULL_POINTER;
                    goto finish;
                }

                m_pDependencyMasks[i] = MOS_NewArray(CM_HAL_MASK_AND_RESET, threadCount);
                if( m_pDependencyMasks[i] )
                {
                    CmSafeMemSet(m_pDependencyMasks[i], 0, sizeof(CM_HAL_MASK_AND_RESET) * threadCount);
                }
                else
                {
                    CM_ASSERTMESSAGE("Error: Pointer to dependency masks is null.");
                    hr = CM_NULL_POINTER;
                    goto finish;
                }
            }

            CM_THREAD_SPACE_UNIT *pThreadSpaceUnit = nullptr;
            pTS_RT->GetThreadSpaceSize(width, height);
            pTS_RT->GetThreadSpaceUnit(pThreadSpaceUnit);

            uint32_t *pBoardOrder = nullptr;
            pTS_RT->GetBoardOrder(pBoardOrder);
            for (uint32_t tIndex=0; tIndex < height*width; tIndex ++)
            {
                pKernel_inTS = static_cast<CmKernelRT *>(pThreadSpaceUnit[pBoardOrder[tIndex]].pKernel);
                if (pKernel_inTS == nullptr)
                {
                    if (pTS_RT->GetNeedSetKernelPointer())
                    {
                        pKernel_inTS = pTS_RT->GetKernelPointer();
                    }
                    if (pKernel_inTS == nullptr)
                    {
                        CM_ASSERTMESSAGE("Error: Invalid kernel pointer in task.");
                        hr = CM_NULL_POINTER;
                        goto finish;
                    }
                }
                uint32_t kIndex = pKernel_inTS->GetIndexInTask();

                m_pThreadCoordinates[kIndex][pKernelCoordinateIndex[kIndex]].x = pThreadSpaceUnit[pBoardOrder[tIndex]].scoreboardCoordinates.x;
                m_pThreadCoordinates[kIndex][pKernelCoordinateIndex[kIndex]].y = pThreadSpaceUnit[pBoardOrder[tIndex]].scoreboardCoordinates.y;
                m_pThreadCoordinates[kIndex][pKernelCoordinateIndex[kIndex]].mask = pThreadSpaceUnit[pBoardOrder[tIndex]].dependencyMask;
                m_pThreadCoordinates[kIndex][pKernelCoordinateIndex[kIndex]].resetMask = pThreadSpaceUnit[pBoardOrder[tIndex]].reset;
                m_pThreadCoordinates[kIndex][pKernelCoordinateIndex[kIndex]].color = pThreadSpaceUnit[pBoardOrder[tIndex]].scoreboardColor;
                m_pThreadCoordinates[kIndex][pKernelCoordinateIndex[kIndex]].sliceSelect = pThreadSpaceUnit[pBoardOrder[tIndex]].sliceDestinationSelect;
                m_pThreadCoordinates[kIndex][pKernelCoordinateIndex[kIndex]].subSliceSelect = pThreadSpaceUnit[pBoardOrder[tIndex]].subSliceDestinationSelect;
                m_pDependencyMasks[kIndex][pKernelCoordinateIndex[kIndex]].mask = pThreadSpaceUnit[pBoardOrder[tIndex]].dependencyMask;
                m_pDependencyMasks[kIndex][pKernelCoordinateIndex[kIndex]].resetMask = pThreadSpaceUnit[pBoardOrder[tIndex]].reset;
                pKernelCoordinateIndex[kIndex] ++;
            }

            MosSafeDeleteArray(pKernelCoordinateIndex);
        }
        else 
        {
            CM_ASSERTMESSAGE("Error: Failed to create thread space data.");
            hr = CM_FAILURE;
            goto finish;
        }

        m_IsThreadCoordinatesExisted = true;
    }
    else 
    {
        m_pThreadCoordinates = nullptr;
        m_pDependencyMasks = nullptr;
        m_IsThreadCoordinatesExisted = false;
    }

    if (pTS_RT->IsDependencySet())
    {
        pTS_RT->GetDependencyPatternType(m_DependencyPattern);
    }

    pTS_RT->GetColorCountMinusOne(m_ColorCountMinusOne);
    pTS_RT->GetMediaWalkerGroupSelect(m_MediaWalkerGroupSelect);

    pTS_RT->GetWalkingPattern(m_WalkingPattern);

    m_MediaWalkerParamsSet = pTS_RT->CheckWalkingParametersSet();
    if( m_MediaWalkerParamsSet )
    {
        CM_WALKING_PARAMETERS tmpMWParams;
        CMCHK_HR(pTS_RT->GetWalkingParameters(tmpMWParams));
        CmSafeMemCopy(&m_WalkingParameters, &tmpMWParams, sizeof(tmpMWParams));
    }

    m_DependencyVectorsSet = pTS_RT->CheckDependencyVectorsSet();
    if( m_DependencyVectorsSet )
    {
        CM_HAL_DEPENDENCY tmpDepVectors;
        CMCHK_HR(pTS_RT->GetDependencyVectors(tmpDepVectors));
        CmSafeMemCopy(&m_DependencyVectors, &tmpDepVectors, sizeof(tmpDepVectors));
    }

finish:
    if(hr != CM_SUCCESS)
    {
        if(m_pThreadCoordinates )
        {
            for (i = 0; i< m_KernelCount; i++)
            {
                MosSafeDeleteArray(m_pThreadCoordinates[i]);
            }
        }
        
        if(m_pDependencyMasks)
        {
            for (i = 0; i< m_KernelCount; i++)
            {
                MosSafeDeleteArray(m_pDependencyMasks[i]);
            }
        }
        MosSafeDeleteArray(m_pThreadCoordinates);
        MosSafeDeleteArray(m_pDependencyMasks);
        MosSafeDeleteArray(pKernelCoordinateIndex);
    }
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get thread space's coordinates 
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetKernelCoordinates(const uint32_t index, void  *&pKernelCoordinates)
{
    if (m_pThreadCoordinates != nullptr)
    {
        pKernelCoordinates = (void *)m_pThreadCoordinates[index]; 
    }
    else
    {
        pKernelCoordinates = nullptr;
    }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get thread space's dependency masks
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetKernelDependencyMasks(const uint32_t index, void  *&pKernelDependencyMasks)
{
    if (m_pDependencyMasks != nullptr)
    {
        pKernelDependencyMasks = (void *)m_pDependencyMasks[index]; 
    }
    else
    {
        pKernelDependencyMasks = nullptr;
    }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get dependency pattern 
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetDependencyPattern(CM_DEPENDENCY_PATTERN &DependencyPattern)
{
    DependencyPattern = m_DependencyPattern;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get media walking pattern 
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetWalkingPattern(CM_WALKING_PATTERN &WalkingPattern)
{
    WalkingPattern = m_WalkingPattern;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get media walking parameters
//| Returns:    CM_FAILURE if dest ptr is nullptr, CM_SUCCESS otherwise
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetWalkingParameters(CM_WALKING_PARAMETERS &pWalkingParameters)
{
    CmSafeMemCopy(&pWalkingParameters, &m_WalkingParameters, sizeof(m_WalkingParameters));
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Check to see if media walking parameters have been set 
//| Returns:    true if media walking parameters set, false otherwise
//*-----------------------------------------------------------------------------
bool CmTaskInternal::CheckWalkingParametersSet( )
{
    return m_MediaWalkerParamsSet;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get dependency vectors 
//| Returns:    CM_FAILURE if dest ptr is nullptr, CM_SUCCESS otherwise
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetDependencyVectors(CM_HAL_DEPENDENCY &pDependencyVectors)
{
    CmSafeMemCopy(&pDependencyVectors, &m_DependencyVectors, sizeof(m_DependencyVectors));
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Check to see if dependency vectors have been set 
//| Returns:    true if dependency vectors are set, false otherwise
//*-----------------------------------------------------------------------------
bool CmTaskInternal::CheckDependencyVectorsSet( )
{
    return m_DependencyVectorsSet;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the total thread count
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetTotalThreadCount( uint32_t& totalThreadCount )
{
    totalThreadCount = m_TotalThreadCount;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the width,height of thread space 
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------

int32_t CmTaskInternal::GetThreadSpaceSize(uint32_t& width, uint32_t& height )
{
    width = m_ThreadSpaceWidth;
    height = m_ThreadSpaceHeight;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the color count minus one of the thread space
//|             Used to dispatch multiple sets of dependency threads
//|             for media walker
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------

int32_t CmTaskInternal::GetColorCountMinusOne( uint32_t& colorCount )
{
    colorCount = m_ColorCountMinusOne;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Whether thread space is created
//| Returns:    Boolean.
//*-----------------------------------------------------------------------------

bool CmTaskInternal::IsThreadSpaceCreated(void )
{
    return m_IsThreadSpaceCreated;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Whether thread coordinates are existed
//| Returns:    Boolean.
//*-----------------------------------------------------------------------------
bool CmTaskInternal::IsThreadCoordinatesExisted(void)
{
    return m_IsThreadCoordinatesExisted;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Whether thread coordinates are existed
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------


int32_t CmTaskInternal::GetThreadGroupSpaceSize(uint32_t& trdSpaceWidth, uint32_t& trdSpaceHeight, uint32_t& trdSpaceDepth, uint32_t& grpSpaceWidth, uint32_t& grpSpaceHeight, uint32_t& grpSpaceDepth)
{
    trdSpaceWidth = m_ThreadSpaceWidth;
    trdSpaceHeight = m_ThreadSpaceHeight;
    trdSpaceDepth  = m_ThreadSpaceDepth;
    grpSpaceWidth = m_GroupSpaceWidth;
    grpSpaceHeight = m_GroupSpaceHeight;
    grpSpaceDepth = m_GroupSpaceDepth;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the size of shared local memory
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetSLMSize(uint32_t& iSLMSize)
{
    iSLMSize = m_SLMSize;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the size of spill memory used
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetSpillMemUsed(uint32_t& iSpillMemUsed)
{
    iSpillMemUsed = m_SpillMemUsed;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the hints for EnqueueWithHints
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetHints(uint32_t& hints)
{
    hints = m_Hints;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Gets the number of tasks generated for EnqueueWithHints
//|             Used when splitting large task to smaller tasks
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetNumTasksGenerated(uint32_t& numTasksGenerated)
{
    numTasksGenerated = m_NumTasksGenerated;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Gets whether or not this task is the last task for EnqueueWithHints
//|             Used to identify last smaller task when splitting large task
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetLastTask(bool& isLastTask)
{
    isLastTask = m_IsLastTask;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Whether thread group space is created
//| Returns:    Value.
//*-----------------------------------------------------------------------------
bool CmTaskInternal::IsThreadGroupSpaceCreated(void)
{
    return m_IsThreadGroupSpaceCreated;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Allocate Space to record kernel surface's information
//| Returns:    result of operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::AllocateKernelSurfInfo()
{
    //Allocate Surf info array
    m_KernelSurfInfo.dwKrnNum = m_KernelCount;
    m_KernelSurfInfo.pSurfEntryInfosArray = (CM_HAL_SURFACE_ENTRY_INFO_ARRAY*)MOS_AllocAndZeroMemory(m_KernelCount *
                                sizeof(CM_HAL_SURFACE_ENTRY_INFO_ARRAY));
    if(m_KernelSurfInfo.pSurfEntryInfosArray == nullptr)
    {

        CM_ASSERTMESSAGE("Error: Mem allocation fail.");
        return CM_OUT_OF_HOST_MEMORY;
    }

    for( uint32_t i = 0; i < m_KernelCount; i ++ )
    {
        CmKernelRT * pTempCmKrn = nullptr;
        this->GetKernel(i, pTempCmKrn);
        if(pTempCmKrn == nullptr)
        {
            CM_ASSERTMESSAGE("Error: Invalid kernel pointer.");
            return CM_FAILURE;
        }

        CM_ARG* pArg=NULL;
        pTempCmKrn->GetArgs( pArg );

        uint32_t iArgCount = 0;
        pTempCmKrn->GetArgCount( iArgCount);
        //allocate memory for non_static buffer&2D&3D
        uint32_t iSurfEntryNum = 0;
        for( uint32_t j = 0; j < iArgCount; j ++ )
        {
            switch(pArg[ j ].unitKind)
            {
                case    ARG_KIND_SURFACE_1D:
                        iSurfEntryNum = iSurfEntryNum + pArg[ j ].unitCount * pArg[j].unitSize/sizeof(int);
                        break;

                case    ARG_KIND_SURFACE_2D:
                case    ARG_KIND_SURFACE_2D_UP:
                case    ARG_KIND_SURFACE_3D:
                case    ARG_KIND_SURFACE_SAMPLER8X8_AVS:
                case    ARG_KIND_SURFACE_SAMPLER8X8_VA:
                        iSurfEntryNum = iSurfEntryNum + 3 * pArg[ j ].unitCount * pArg[j].unitSize/sizeof(int);//one 2D or 3D can have upto 3 planes
                        break;

                case    ARG_KIND_SURFACE_VME:
                        iSurfEntryNum = iSurfEntryNum + 9 * pArg[ j ].unitCount;//surfaceVME will use upto 3 surfaces, each one can have upto 3 planes
                        break;

                default:
                    break;
            }
        }
        CM_HAL_SURFACE_ENTRY_INFO_ARRAY* pTempArray =  m_KernelSurfInfo.pSurfEntryInfosArray;
        if(iSurfEntryNum>0)
        {
            pTempArray[i].dwMaxEntryNum = iSurfEntryNum;
            pTempArray[i].pSurfEntryInfos = (CM_SURFACE_DETAILS*)MOS_AllocAndZeroMemory(iSurfEntryNum*sizeof(CM_SURFACE_DETAILS));

            if(pTempArray[i].pSurfEntryInfos == nullptr)
            {
                CM_ASSERTMESSAGE("Error: Mem allocation fail.");
                return CM_OUT_OF_HOST_MEMORY;
            }

        }

        //allocate memory for those 7 static buffers
        uint32_t iGBufNum=CM_GLOBAL_SURFACE_NUMBER + CM_GTPIN_BUFFER_NUM;
        pTempArray[i].dwGlobalSurfNum=iGBufNum;
        pTempArray[i].pGlobalSurfInfos = (CM_SURFACE_DETAILS*)MOS_AllocAndZeroMemory(
                                iGBufNum*sizeof(CM_SURFACE_DETAILS));
        if(pTempArray[i].pGlobalSurfInfos == nullptr)
        {
            CM_ASSERTMESSAGE("Mem allocation fail.");
            return CM_OUT_OF_HOST_MEMORY;
        }
    }
    return CM_SUCCESS;
}

int32_t CmTaskInternal::GetKernelSurfInfo(CM_HAL_SURFACE_ENTRY_INFO_ARRAYS & SurfEntryInfoArray)
{
    SurfEntryInfoArray = m_KernelSurfInfo;
    return CM_SUCCESS;
}

int32_t CmTaskInternal::ClearKernelSurfInfo()
{
    if (m_KernelSurfInfo.pSurfEntryInfosArray == nullptr)
    { // if pSurfEntryInfosArray is empty, return directly
        return CM_SUCCESS;
    }
    
    //free memory
    for( uint32_t i = 0; i < m_KernelCount; i ++ )
    {
        if (m_KernelSurfInfo.pSurfEntryInfosArray[i].pSurfEntryInfos != nullptr)
        {
            MosSafeDelete(m_KernelSurfInfo.pSurfEntryInfosArray[i].pSurfEntryInfos);
        }
        if (m_KernelSurfInfo.pSurfEntryInfosArray[i].pGlobalSurfInfos!= nullptr)
        {
            MosSafeDelete(m_KernelSurfInfo.pSurfEntryInfosArray[i].pGlobalSurfInfos);
        }
    }

    MosSafeDelete(m_KernelSurfInfo.pSurfEntryInfosArray);

    m_KernelSurfInfo.dwKrnNum = 0 ;
    m_KernelSurfInfo.pSurfEntryInfosArray = nullptr;

    return CM_SUCCESS;
}

int32_t CmTaskInternal::GetTaskType(uint32_t& taskType)
{
    taskType = m_TaskType;

    return CM_SUCCESS;
}


//*-----------------------------------------------------------------------------
//| Purpose:    Get vebox state
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetVeboxState(CM_VEBOX_STATE &pVeboxState)
{
    pVeboxState = m_VeboxState;

    return CM_SUCCESS;
}

int32_t CmTaskInternal::GetVeboxParam(CmBufferUP * &pVeboxParam)
{
    pVeboxParam = m_pVeboxParam;

    return CM_SUCCESS;
}

int32_t CmTaskInternal::GetVeboxSurfaceData(CM_VEBOX_SURFACE_DATA &VeboxSurfaceData)
{
    VeboxSurfaceData = m_VeboxSurfaceData;
    return CM_SUCCESS;
}

uint64_t CmTaskInternal::GetSyncBitmap()
{
    return m_ui64SyncBitmap;
}

uint64_t CmTaskInternal::GetConditionalEndBitmap()
{
    return m_ui64ConditionalEndBitmap;
}

CM_HAL_CONDITIONAL_BB_END_INFO* CmTaskInternal::GetConditionalEndInfo()
{
    return m_ConditionalEndInfo;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Set power option for this task
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::SetPowerOption( PCM_POWER_OPTION pPowerOption )
{
    if (pPowerOption == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to power option is null.");
        return CM_NULL_POINTER;
    }
    CmFastMemCopy( &m_PowerOption, pPowerOption, sizeof( m_PowerOption ) );
    return CM_SUCCESS;   
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get power option for this task
//| Returns:    Pointer to power option.
//*-----------------------------------------------------------------------------
PCM_POWER_OPTION CmTaskInternal::GetPowerOption()
{
    return &m_PowerOption;
}

#if _DEBUG
const char *g_DependencyPatternString[] =
{
    "DEPENDENCY_NONE",
    "DEPENDENCY_WAVEFRONT45",
    "DEPENDENCY_WAVEFRONT26"
};

//Only for debugging
int32_t CmTaskInternal::DisplayThreadSpaceData(uint32_t width, uint32_t height)
{
    uint32_t i;
    if (m_pThreadCoordinates != nullptr)
    {
        CM_NORMALMESSAGE("Score board[Kernel x: (x1, y1), (x2, y2)...]:");
        for (i = 0; i < m_KernelCount; i ++)
        {
            CmKernelRT *pKernel_RT = nullptr;
            GetKernel(i, pKernel_RT);
            if(nullptr == pKernel_RT)
            {
                return CM_FAILURE;
            }

            uint32_t threadCount;
            pKernel_RT->GetThreadCount(threadCount);
            if (threadCount == 0)
            {
                threadCount = m_ThreadSpaceWidth*m_ThreadSpaceHeight;
            }
            CM_NORMALMESSAGE("Kernel %d: ", i);
            for (uint32_t j=0; j<threadCount; j++)
            {
                CM_NORMALMESSAGE("(%d, %d) ", m_pThreadCoordinates[i][j].x, m_pThreadCoordinates[i][j].y);
            }
        }
    }
    else
    {
        CM_NORMALMESSAGE("Score Board is NULL.");
    }

    if (m_DependencyPattern <= CM_WAVEFRONT26)
    {
        CM_NORMALMESSAGE("Dependency Pattern: %s.", g_DependencyPatternString[m_DependencyPattern]);
    }
    else 
    {
        CM_NORMALMESSAGE("Dependency Pattern: UNASSIGNED.");
    }

    return CM_SUCCESS;
}
#endif

int32_t CmTaskInternal::GetMediaWalkerGroupSelect(CM_MW_GROUP_SELECT& groupSelect)
{
    groupSelect = m_MediaWalkerGroupSelect;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Update surface state on task destroy stage
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::UpdateSurfaceStateOnTaskCreation()
{
    CmSurfaceManager*   pSurfaceMgr = nullptr;
    int32_t             *pSurfState = nullptr;

    m_pCmDevice->GetSurfaceManager(pSurfaceMgr);
    if (pSurfaceMgr == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to surface manager is null.");
        return CM_NULL_POINTER;
    }

    pSurfaceMgr->GetSurfaceState(pSurfState);
    if (pSurfState == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to surface state is null.");
        return CM_NULL_POINTER;
    }

    uint32_t poolSize = pSurfaceMgr->GetSurfacePoolSize();
    
    CSync* pSurfaceLock = m_pCmDevice->GetSurfaceCreationLock();
    if (pSurfaceLock == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to surface creation lock is null.");
        return CM_NULL_POINTER;
    }

    pSurfaceLock->Acquire();

    if (!m_IsSurfaceUpdateDone)
    {
        for (uint32_t i = 0; i < poolSize; i++)
        {
            if (m_SurfaceArray[i])
            {
                pSurfState[i] ++;
            }
        }

        m_IsSurfaceUpdateDone = true;
    }

    pSurfaceLock->Release();
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Update surface state on task creation stage
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::UpdateSurfaceStateOnTaskDestroy()
{
    CmSurfaceManager*   pSurfaceMgr = nullptr;
    int32_t             *pSurfState  = nullptr;

    m_pCmDevice->GetSurfaceManager(pSurfaceMgr);
    if (!pSurfaceMgr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to surface manager is null.");
        return CM_NULL_POINTER;
    }

    pSurfaceMgr->GetSurfaceState(pSurfState);
    if (pSurfState == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to surface state is null.");
        return CM_NULL_POINTER;
    }

    uint32_t poolSize = pSurfaceMgr->GetSurfacePoolSize();
    
    CSync* pSurfaceLock = m_pCmDevice->GetSurfaceCreationLock();
    if (pSurfaceLock == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to surface creation lock is null.");
        return CM_NULL_POINTER;
    }

    pSurfaceLock->Acquire();

    if (m_IsSurfaceUpdateDone)
    {
        for (uint32_t i = 0; i < poolSize; i++)
        {
            if (m_SurfaceArray[i])
            {
                pSurfState[i] --;
            }
        }

        m_IsSurfaceUpdateDone = false;
    }

    pSurfaceLock->Release();
    return CM_SUCCESS;
}

#if CM_LOG_ON
std::string CmTaskInternal::Log()
{
    std::ostringstream  oss;

    oss << "Enqueue Task Type:" << m_TaskType 
        << " Kernel Count:" << m_KernelCount 
        << " Total Thread Count:" << m_TotalThreadCount
        << " Sync Bit:"<<m_ui64SyncBitmap
        << " Conditional End Bit:" << m_ui64ConditionalEndBitmap
        << std::endl;

    switch(m_TaskType)
    {
        case CM_INTERNAL_TASK_WITH_THREADSPACE:
            if ( m_IsThreadSpaceCreated )
            {
                oss << "Thread Space Width :" << m_ThreadSpaceWidth << " Height :" << m_ThreadSpaceHeight 
                    << "Walker Patten :" << (int)m_WalkingPattern << std::endl;
            }
            break;

        case CM_INTERNAL_TASK_WITH_THREADGROUPSPACE:
            if(m_IsThreadGroupSpaceCreated)
            {
                oss << "Thread Group Space Width:" << m_GroupSpaceWidth << " Height:" << m_GroupSpaceHeight
                    << "SLM Size:" <<m_SLMSize << std::endl;
            }
            break;

        case CM_INTERNAL_TASK_VEBOX:
            break;

        case CM_INTERNAL_TASK_ENQUEUEWITHHINTS:
            oss << " Hints :" << m_Hints 
                << " Thread Space Width :" << m_ThreadSpaceWidth 
                << " Height :" << m_ThreadSpaceHeight 
                << " Walker Patten :" << (int)m_WalkingPattern
                << std::endl;
            break;

        default:    // by default, assume the task is considered as general task: CM_INTERNAL_TASK_WITH_THREADSPACE
            break;
    }

    for (uint32_t i=0 ; i< m_KernelCount; i++)
    {
        CmKernelRT* pKernel = (CmKernelRT*)m_Kernels.GetElement( i );

        oss << pKernel->Log(); // log each kernel
    }

    return oss.str();
}
#endif


void CmTaskInternal::SurfaceDump(int32_t taskId)
{
#if MDF_SURFACE_CONTENT_DUMP
    for (uint32_t i=0 ; i< m_KernelCount; i++)
    {
        CmKernelRT* pKernel = (CmKernelRT*)m_Kernels.GetElement( i );
        pKernel->SurfaceDump(i, taskId);
    }
#endif
}


int32_t CmTaskInternal::SetProperty(CM_TASK_CONFIG * pTaskConfig)
{
    if (pTaskConfig == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to task config is null.");
        return CM_NULL_POINTER;
    }
    CmFastMemCopy(&m_TaskConfig, pTaskConfig, sizeof(m_TaskConfig));
    return CM_SUCCESS;
}

PCM_TASK_CONFIG CmTaskInternal::GetTaskConfig()
{
    return &m_TaskConfig;
}

void  *CMRT_UMD::CmTaskInternal::GetMediaStatePtr()
{
    return m_media_state_ptr;
}
}