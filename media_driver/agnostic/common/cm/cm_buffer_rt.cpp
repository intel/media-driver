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
//! \file      cm_buffer_rt.cpp  
//! \brief     Contains implementation of CmBuffer_RT.  
//!

#include "cm_buffer_rt.h"

#include "cm_surface_manager.h"
#include "cm_event_rt.h"
#include "cm_device_rt.h"
#include "cm_log.h"
#include "cm_mem.h"
#include "cm_hal.h"
namespace CMRT_UMD
{
//*-----------------------------------------------------------------------------
//| Purpose:    Create CM Buffer 
//| Returns:    Result of the operation
//*-----------------------------------------------------------------------------
int32_t CmBuffer_RT::Create( uint32_t index, uint32_t handle, uint32_t size, bool bIsCmCreated, CmSurfaceManager* pSurfaceManager, uint32_t uiBufferType, bool isCMRTAllocatedSVM, void  *pSysMem, CmBuffer_RT* &pSurface, bool isConditionalBuffer, uint32_t comparisonValue, bool enableCompareMask )
{
    int32_t result = CM_SUCCESS;

    pSurface = new (std::nothrow) CmBuffer_RT( handle, size, bIsCmCreated, pSurfaceManager, uiBufferType, isCMRTAllocatedSVM, pSysMem, isConditionalBuffer, comparisonValue, enableCompareMask);
    if( pSurface )
    {
        result = pSurface->Initialize( index );
        if( result != CM_SUCCESS )
        {
            CmSurface* pBaseSurface = pSurface;
            CmSurface::Destroy( pBaseSurface );
        }
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to create CmBuffer due to out of system memory.");
        result = CM_OUT_OF_HOST_MEMORY;
    }

    return result;
}

CmBuffer_RT::CmBuffer_RT( uint32_t handle, uint32_t size, bool bIsCmCreated, CmSurfaceManager* pSurfaceManager, uint32_t uiBufferType, bool isCMRTAllocatedSVM, void  *pSysMem, bool isConditionalBuffer, uint32_t comparisonValue, bool enableCompareMask ): 
    CmSurface( pSurfaceManager,bIsCmCreated ), 
    m_Handle( handle ), 
    m_Size( size ),
    m_uiBufferType(uiBufferType),
    m_pSysMem( pSysMem ),
    m_isCMRTAllocatedSVMBuffer( isCMRTAllocatedSVM ),
    m_isConditionalBuffer( isConditionalBuffer ),
    m_comparisonValue( comparisonValue ),
    m_enableCompareMask( enableCompareMask ),
    m_numAliases(0)

{
    CmSurface::SetMemoryObjectControl(MEMORY_OBJECT_CONTROL_UNKNOW, CM_USE_PTE, 0);
    CmSafeMemSet(m_pAliasIndexes, 0, sizeof(SurfaceIndex*) * CM_HAL_MAX_NUM_BUFFER_ALIASES);
}

CmBuffer_RT::~CmBuffer_RT( void )
{
    for( uint32_t i = 0; i < CM_HAL_MAX_NUM_BUFFER_ALIASES; ++i )
    {
        MosSafeDelete(m_pAliasIndexes[i]);
    }
}

int32_t CmBuffer_RT::Initialize( uint32_t index )
{
    return CmSurface::Initialize( index );
}

int32_t CmBuffer_RT::GetHandle( uint32_t& handle)
{
    handle = m_Handle;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Write data from pSysMem to Buffer
//| Returns:    Result of the operation
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmBuffer_RT::WriteSurface( const unsigned char* pSysMem, CmEvent* pEvent, uint64_t sysMemSize )
{
    INSERT_API_CALL_LOG();

    CM_RETURN_CODE  hr      = CM_SUCCESS;
    uint8_t         *pDst    = nullptr;
    uint8_t         *pSurf   = nullptr; 
    size_t CopySize = MOS_MIN((size_t)sysMemSize, m_Size);

    if (pSysMem == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to system memory is null.");
        return CM_NULL_POINTER;
    }

    // It makes sense to flush the whole enqueued tasks for each surface read
    // because usually we read the output of the last task.
    // Update: using event not to flush the whole enqueued tasks
    if( pEvent )
    {
        FlushDeviceQueue( static_cast<CmEventRT*>(pEvent) ); // wait specific owner task finished
    }

    WaitForReferenceFree(); // wait all owner task finished

    // Lock Buffer first
    CmDeviceRT * pCmDevice = nullptr;
    m_SurfaceMgr->GetCmDevice(pCmDevice);
    PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)pCmDevice->GetAccelData();

    CM_HAL_BUFFER_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_HAL_BUFFER_PARAM ) );
    inParam.iLockFlag = CM_HAL_LOCKFLAG_WRITEONLY;
    inParam.dwHandle = m_Handle;

    // Lock Buffer:
    // Lock Buffer may fail due to the out of memory/out of page-in in KMD.
    // Touch queue for the buffer/surface data release
    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->pCmHalState->pfnLockBuffer(pCmData->pCmHalState, &inParam));
    CMCHK_NULL(inParam.pData);

    // Memory copy : Source ->System Memory  Dest -> Vedio Memory
    pDst  = ( uint8_t *)(inParam.pData);
    pSurf = ( uint8_t *)pSysMem;

    CmFastMemCopyWC(pDst, pSurf, CopySize);

    //Unlock Buffer
    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->pCmHalState->pfnUnlockBuffer(pCmData->pCmHalState, &inParam));

finish:
    if (hr < CM_MOS_STATUS_CONVERTED_CODE_OFFSET) {
        hr = CM_LOCK_SURFACE_FAIL;
    }
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Read data from pSysMem to Buffer
//| Returns:    Result of the operation
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmBuffer_RT::ReadSurface( unsigned char* pSysMem, CmEvent* pEvent, uint64_t sysMemSize )
{
    INSERT_API_CALL_LOG();

    CM_RETURN_CODE  hr          = CM_SUCCESS;

    size_t CopySize = MOS_MIN((size_t)sysMemSize, m_Size);

    if (pSysMem == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to system memory is null.");
        return CM_NULL_POINTER;
    }

    // It makes sense to flush the whole enqueued tasks for each surface read
    // because usually we read the output of the last task.
    // Update: using event not to flush the whole enqueued tasks
    if( pEvent )
    {
        FlushDeviceQueue( static_cast<CmEventRT *>(pEvent) ); // wait specific owner task finished
    }

    WaitForReferenceFree();    // wait all owner task finished

    // Lock Buffer first
    CmDeviceRT * pCmDevice = nullptr;
    m_SurfaceMgr->GetCmDevice(pCmDevice);
    PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)pCmDevice->GetAccelData();

    CM_HAL_BUFFER_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_HAL_BUFFER_PARAM ) );
    inParam.iLockFlag = CM_HAL_LOCKFLAG_READONLY;
    inParam.dwHandle = m_Handle;

    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->pCmHalState->pfnLockBuffer(pCmData->pCmHalState, &inParam));
    CMCHK_NULL(inParam.pData);

    // Memory copy : Dest ->System Memory  Source -> Vedio Memory
    CmFastMemCopyFromWC(pSysMem, inParam.pData, CopySize, GetCpuInstructionLevel());

    //Unlock Buffer
    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->pCmHalState->pfnUnlockBuffer(pCmData->pCmHalState, &inParam));

finish:
    if (hr < CM_MOS_STATUS_CONVERTED_CODE_OFFSET) {
        hr = CM_LOCK_SURFACE_FAIL;
    }
    return hr;
}

CM_RT_API int32_t CmBuffer_RT::GetIndex( SurfaceIndex*& pIndex ) 
{ 
    pIndex = m_pIndex; 
    return CM_SUCCESS;
}

CM_RT_API int32_t CmBuffer_RT::InitSurface(const uint32_t initValue, CmEvent* pEvent)
{
    INSERT_API_CALL_LOG();

    CM_RETURN_CODE  hr          = CM_SUCCESS;

    // It makes sense to flush the whole enqueued tasks for each surface read
    // because usually we read the output of the last task.
    // Update: using event not to flush the whole enqueued tasks
    if( pEvent )
    {
        FlushDeviceQueue( static_cast<CmEventRT *>(pEvent) );
    }

    CmDeviceRT* pCmDevice = nullptr;
    m_SurfaceMgr->GetCmDevice( pCmDevice );
    CM_ASSERT( pCmDevice );

    PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)pCmDevice->GetAccelData();

    CM_HAL_BUFFER_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_HAL_BUFFER_PARAM ) );
    inParam.dwHandle = m_Handle;
    inParam.iLockFlag = CM_HAL_LOCKFLAG_WRITEONLY;

    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->pCmHalState->pfnLockBuffer(pCmData->pCmHalState, &inParam));
    CMCHK_NULL(inParam.pData);

    CmDwordMemSet(inParam.pData, initValue, m_Size);

    // unlock 
    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->pCmHalState->pfnUnlockBuffer(pCmData->pCmHalState, &inParam));

finish:
    if (hr < CM_MOS_STATUS_CONVERTED_CODE_OFFSET) {
        hr = CM_LOCK_SURFACE_FAIL;
    }
    return hr;
}

int32_t CmBuffer_RT::SetMemoryObjectControl( MEMORY_OBJECT_CONTROL mem_ctrl, MEMORY_TYPE mem_type, uint32_t age)
{
    INSERT_API_CALL_LOG();

    CM_RETURN_CODE  hr = CM_SUCCESS;
    uint16_t mocs = 0;

    CmSurface::SetMemoryObjectControl( mem_ctrl, mem_type, age );

    CmDeviceRT *pCmDevice = nullptr;
    m_SurfaceMgr->GetCmDevice(pCmDevice);
    PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)pCmDevice->GetAccelData();
    CMCHK_NULL(pCmData);

    mocs = (m_MemObjCtrl.mem_ctrl << 8) | (m_MemObjCtrl.mem_type<<4) | m_MemObjCtrl.age;

    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->pCmHalState->pfnSetSurfaceMOCS(pCmData->pCmHalState, m_Handle, mocs, ARG_KIND_SURFACE_1D));

finish:
    return hr;
}

CM_RT_API int32_t CmBuffer_RT::SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL mem_ctrl)
{
    return SetMemoryObjectControl(mem_ctrl, CM_USE_PTE, 0);
}

CM_RT_API int32_t CmBuffer_RT::SetSurfaceStateParam(SurfaceIndex *pSurfIndex, const CM_BUFFER_STATE_PARAM *pSSParam)
{
    CM_RETURN_CODE  hr          = CM_SUCCESS;
    uint32_t        new_size    = 0;
    if(pSSParam->uiBaseAddressOffset + pSSParam->uiSize > m_Size)
    {
        CM_ASSERTMESSAGE("Error: The offset exceeds the buffer size.");
        return CM_INVALID_ARG_VALUE;
    }
    if(pSSParam->uiBaseAddressOffset%16) // the offset must be 16-aligned, otherwise it will cause a GPU hang
    {
        CM_ASSERTMESSAGE("Error: The offset must be 16-aligned, otherwise it will cause GPU hang.");
        return CM_INVALID_ARG_VALUE;
    }

    if (pSSParam->uiSize)
    {
        new_size = pSSParam->uiSize;
    }
    else
    {
        new_size = m_Size - pSSParam->uiBaseAddressOffset;
    }
    CmDeviceRT* pCmDevice = nullptr;
    m_SurfaceMgr->GetCmDevice( pCmDevice );
    if (nullptr == pCmDevice)
    {
        CM_ASSERTMESSAGE("Error: Invalid CmDevice.");
        return CM_NULL_POINTER;
    }

    PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)pCmDevice->GetAccelData();
    if (nullptr == pCmData)
    {
        CM_ASSERTMESSAGE("Error: Invalid CM context data.");
        return CM_NULL_POINTER;
    }


    CM_HAL_BUFFER_SURFACE_STATE_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( inParam ) );

    if( pSurfIndex )
    {
        inParam.iAliasIndex  = pSurfIndex->get_data();
    }
    else
    {
        inParam.iAliasIndex  = m_pIndex->get_data();
    }
    inParam.dwHandle = m_Handle;
    inParam.iOffset  = pSSParam->uiBaseAddressOffset;
    inParam.iSize    = new_size;
    inParam.wMOCS    = (uint16_t)((pSSParam->mocs.mem_ctrl << 8)|(pSSParam->mocs.mem_type << 4)|(pSSParam->mocs.age));
    
    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->pCmHalState->pfnSetBufferSurfaceStatePara(pCmData->pCmHalState, &inParam));
    
finish:
    return hr;
}


int32_t CmBuffer_RT::GetSize( uint32_t& size ) 
{ 
    size = m_Size; 
    return CM_SUCCESS;
}

int32_t CmBuffer_RT::SetSize( uint32_t size ) 
{ 
    m_Size = size; 
    return CM_SUCCESS;
}

bool CmBuffer_RT::IsUpSurface() 
{
    return (m_uiBufferType == CM_BUFFER_UP);
}

bool CmBuffer_RT::IsSVMSurface()
{
    return (m_uiBufferType == CM_BUFFER_SVM);
}

CM_RT_API int32_t CmBuffer_RT::GetAddress( void  *&pAddr)
{

    pAddr = m_pSysMem;
    return CM_SUCCESS;
}

bool CmBuffer_RT::IsCMRTAllocatedSVMBuffer()
{
    return m_isCMRTAllocatedSVMBuffer;
}

bool CmBuffer_RT::IsConditionalSurface()
{
    return m_isConditionalBuffer;
}

uint32_t CmBuffer_RT::GetConditionalCompareValue()
{
    return m_comparisonValue;
}

bool CmBuffer_RT::IsCompareMaskEnabled()
{
    return m_enableCompareMask;
}

int32_t CmBuffer_RT::CreateBufferAlias(SurfaceIndex* & pAliasIndex)
{
    uint32_t surfArraySize = 0;
    uint32_t newIndex = 0;
    uint32_t origIndex = 0;

    if( m_numAliases < CM_HAL_MAX_NUM_BUFFER_ALIASES )
    {
        origIndex = m_pIndex->get_data();
        m_SurfaceMgr->GetSurfaceArraySize(surfArraySize);
        newIndex = origIndex + ( (m_numAliases + 1) * surfArraySize);
        m_pAliasIndexes[m_numAliases] = MOS_New(SurfaceIndex, newIndex);
        if( m_pAliasIndexes[m_numAliases] )
        {
            pAliasIndex = m_pAliasIndexes[m_numAliases];
            m_numAliases++;
            return CM_SUCCESS;
        }
        else
        {
            CM_ASSERTMESSAGE("Error: Failed to create CmBufferAlias due to out of system memory.");
            return CM_OUT_OF_HOST_MEMORY;
        }
    }
    else
    {
        return CM_EXCEED_MAX_NUM_BUFFER_ALIASES;
    }
}

int32_t CmBuffer_RT::GetNumAliases(uint32_t& numAliases)
{
    numAliases = m_numAliases;
    return CM_SUCCESS;
}

void CmBuffer_RT::Log(std::ostringstream &oss)
{
#if CM_LOG_ON
    oss << " Surface Buffer Info "
        << " Size:"         << m_Size 
        << " Buffer Type:"  << m_uiBufferType
        << " Sys Address:"  << m_pSysMem
        << " Handle:"       << m_Handle
        << " SurfaceIndex:" << m_pIndex->get_data()
        << " IsCmCreated:"  << m_IsCmCreated
        << std::endl;
#endif
}

//*-----------------------------------------------------------------------------
//| Purpose:    Dump Surface's data into files
//| Returns:    None
//| Notes:      Must be called after task finished.
//*-----------------------------------------------------------------------------
void CmBuffer_RT::DumpContent(uint32_t kernelNumber, int32_t taskId, uint32_t argIndex)
{
#if MDF_SURFACE_CONTENT_DUMP
    std::ostringstream outputFileName;
    static uint32_t bufferDumpNumber = 0;
    outputFileName << "t_" << taskId
        << "_k_" << kernelNumber
        <<"_argi_"<< argIndex
        << "_buffer_surfi_" << m_pIndex->get_data()
        <<"_w_"<< m_Size
        <<"_"<< bufferDumpNumber;
    
    std::ofstream  outputFileStream;
    outputFileStream.open(outputFileName.str().c_str(), std::ofstream::binary);

    if (m_pSysMem != nullptr)
    { // Buffer Up
        outputFileStream.write((char *)m_pSysMem, m_Size);
    }
    else
    { // Buffer
        std::vector<char>buffer(m_Size);

        CmDeviceRT *pCmDevice = nullptr;
        m_SurfaceMgr->GetCmDevice(pCmDevice);
        PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)pCmDevice->GetAccelData();
        CM_HAL_BUFFER_PARAM inParam;
        CmSafeMemSet(&inParam, 0, sizeof(CM_HAL_BUFFER_PARAM));
        inParam.iLockFlag = CM_HAL_LOCKFLAG_READONLY;
        inParam.dwHandle = m_Handle;
        pCmData->pCmHalState->pfnLockBuffer(pCmData->pCmHalState, &inParam);
        if (inParam.pData == nullptr)
            return;
        CmFastMemCopyFromWC((unsigned char *)&buffer[0], inParam.pData, m_Size, GetCpuInstructionLevel());
        pCmData->pCmHalState->pfnUnlockBuffer(pCmData->pCmHalState, &inParam);

        outputFileStream.write(&buffer[0], m_Size);
    }
    outputFileStream.close();
    bufferDumpNumber++;
#endif
}
}