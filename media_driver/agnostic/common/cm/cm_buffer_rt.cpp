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
int32_t CmBuffer_RT::Create(uint32_t index, uint32_t handle, size_t size,
                            bool isCmCreated, CmSurfaceManager *surfaceManager,
                            uint32_t bufferType, bool isCMRTAllocatedSVM, void  *sysMem,
                            CmBuffer_RT *&surface, bool isConditionalBuffer,
                            uint32_t comparisonValue, uint64_t gfxMem, bool enableCompareMask)
{
    int32_t result = CM_SUCCESS;

    surface = new (std::nothrow)CmBuffer_RT(handle, size, isCmCreated, surfaceManager, bufferType,
                                            isCMRTAllocatedSVM, sysMem, isConditionalBuffer,
                                            comparisonValue, gfxMem, enableCompareMask);
    if( surface )
    {
        result = surface->Initialize( index );
        if( result != CM_SUCCESS )
        {
            CmSurface* baseSurface = surface;
            CmSurface::Destroy( baseSurface );
        }
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to create cmBuffer due to out of system memory.");
        result = CM_OUT_OF_HOST_MEMORY;
    }

    return result;
}

CmBuffer_RT::CmBuffer_RT(uint32_t handle, size_t size, bool isCmCreated,
                         CmSurfaceManager* surfaceManager, uint32_t bufferType,
                         bool isCMRTAllocatedSVM, void  *sysMem, bool isConditionalBuffer,
                         uint32_t comparisonValue, uint64_t gfxAddr, bool enableCompareMask ):
    CmSurface(surfaceManager, isCmCreated),
    m_handle(handle),
    m_size(size),
    m_bufferType(bufferType),
    m_sysMem(sysMem),
    m_gfxMem(gfxAddr),
    m_isCMRTAllocatedSVMBuffer(isCMRTAllocatedSVM),
    m_isConditionalBuffer(isConditionalBuffer),
    m_comparisonValue(comparisonValue),
    m_enableCompareMask(enableCompareMask),
    m_numAliases(0)
{
    CmSurface::SetMemoryObjectControl(MEMORY_OBJECT_CONTROL_UNKNOW, CM_USE_PTE, 0);
    CmSafeMemSet(m_aliasIndexes, 0, sizeof(SurfaceIndex*) * CM_HAL_MAX_NUM_BUFFER_ALIASES);
}

CmBuffer_RT::~CmBuffer_RT( void )
{
    for( uint32_t i = 0; i < CM_HAL_MAX_NUM_BUFFER_ALIASES; ++i )
    {
        MosSafeDelete(m_aliasIndexes[i]);
    }
}

int32_t CmBuffer_RT::Initialize( uint32_t index )
{
    return CmSurface::Initialize( index );
}

int32_t CmBuffer_RT::GetHandle( uint32_t& handle)
{
    handle = m_handle;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Write data from sysMem to Buffer
//| Returns:    Result of the operation
//*-----------------------------------------------------------------------------
int32_t CmBuffer_RT::WriteBuffer( const unsigned char* sysMem, CmEvent* event, uint64_t sysMemSize, size_t offset)
{
    INSERT_API_CALL_LOG();

    CM_RETURN_CODE  hr      = CM_SUCCESS;
    uint8_t *dst    = nullptr;
    uint8_t *surf   = nullptr;
    size_t copySize = MOS_MIN((size_t)sysMemSize, m_size);

    if (sysMem == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to system memory is null.");
        return CM_NULL_POINTER;
    }

    // It makes sense to flush the whole enqueued tasks for each surface read
    // because usually we read the output of the last task.
    // Update: using event not to flush the whole enqueued tasks
    if( event )
    {
        CmEventRT *eventRT = dynamic_cast<CmEventRT *>(event);
        if (eventRT)
        {
            FlushDeviceQueue(eventRT);
        }
        else
        {
            event->WaitForTaskFinished();
        }
    }

    WaitForReferenceFree(); // wait all owner task finished

    // Lock Buffer first
    CmDeviceRT * cmDevice = nullptr;
    m_surfaceMgr->GetCmDevice(cmDevice);
    CM_CHK_NULL_RETURN_CMERROR(cmDevice);
    
    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)cmDevice->GetAccelData();
    CM_CHK_NULL_RETURN_CMERROR(cmData);
    CM_CHK_NULL_RETURN_CMERROR(cmData->cmHalState);

    CM_HAL_BUFFER_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_HAL_BUFFER_PARAM ) );
    inParam.lockFlag = CM_HAL_LOCKFLAG_WRITEONLY;
    inParam.handle = m_handle;

    // Lock Buffer:
    // Lock Buffer may fail due to the out of memory/out of page-in in KMD.
    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnLockBuffer(cmData->cmHalState, &inParam));
    CM_CHK_NULL_GOTOFINISH_CMERROR(inParam.data);

    // Memory copy : Source ->System Memory  Dest -> Video Memory
    dst  = ( uint8_t *)(inParam.data) + offset;
    surf = ( uint8_t *)sysMem;

    CmFastMemCopyWC(dst, surf, copySize);

    //Unlock Buffer
    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnUnlockBuffer(cmData->cmHalState, &inParam));

finish:
    if (hr < CM_MOS_STATUS_CONVERTED_CODE_OFFSET) {
        hr = CM_LOCK_SURFACE_FAIL;
    }
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Read data from sysMem to Buffer
//| Returns:    Result of the operation
//*-----------------------------------------------------------------------------
int32_t CmBuffer_RT::ReadBuffer( unsigned char* sysMem, CmEvent* event, uint64_t sysMemSize, size_t offset)
{
    INSERT_API_CALL_LOG();

    CM_RETURN_CODE  hr          = CM_SUCCESS;
    uint8_t* surf = nullptr;
    uint8_t* dst = nullptr;
    size_t copySize = MOS_MIN((size_t)sysMemSize, m_size);

    if (sysMem == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to system memory is null.");
        return CM_NULL_POINTER;
    }

    // It makes sense to flush the whole enqueued tasks for each surface read
    // because usually we read the output of the last task.
    // Update: using event not to flush the whole enqueued tasks
    if( event )
    {
        CmEventRT *eventRT = dynamic_cast<CmEventRT *>(event);
        if (eventRT)
        {
            FlushDeviceQueue(eventRT);
        }
        else
        {
            event->WaitForTaskFinished();
        }
    }

    WaitForReferenceFree();    // wait all owner task finished

    // Lock Buffer first
    CmDeviceRT * cmDevice = nullptr;
    m_surfaceMgr->GetCmDevice(cmDevice);
    CM_CHK_NULL_RETURN_CMERROR(cmDevice);
    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)cmDevice->GetAccelData();
    CM_CHK_NULL_RETURN_CMERROR(cmData);
    CM_CHK_NULL_RETURN_CMERROR(cmData->cmHalState);

    CM_HAL_BUFFER_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_HAL_BUFFER_PARAM ) );
    inParam.lockFlag = CM_HAL_LOCKFLAG_READONLY;
    inParam.handle = m_handle;

    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnLockBuffer(cmData->cmHalState, &inParam));
    CM_CHK_NULL_GOTOFINISH_CMERROR(inParam.data);

    // Memory copy : Dest ->System Memory  Source -> Vedio Memory
    // CmFastMemCopyFromWC(sysMem, (void*)(&((unsigned char*)inParam.data)[offset]), copySize, GetCpuInstructionLevel());
    // Memory copy : Source ->System Memory  Dest -> Video Memory
    surf = (uint8_t *)(inParam.data) + offset;
    dst = (uint8_t *)sysMem;
    CmFastMemCopyFromWC(dst, surf, copySize, GetCpuInstructionLevel());
    //MOS_SecureMemcpy(dst, copySize, surf, copySize);
    //Unlock Buffer
    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnUnlockBuffer(cmData->cmHalState, &inParam));

finish:
    if (hr < CM_MOS_STATUS_CONVERTED_CODE_OFFSET) {
        hr = CM_LOCK_SURFACE_FAIL;
    }
    return hr;
}

CM_RT_API int32_t CmBuffer_RT::GetIndex( SurfaceIndex*& index )
{
    index = m_index;
    return CM_SUCCESS;
}


//*-----------------------------------------------------------------------------
//| Purpose:    Write data from sysMem to Buffer
//| Returns:    Result of the operation
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmBuffer_RT::WriteSurface(const unsigned char* sysMem, CmEvent* event, uint64_t sysMemSize)
{
    INSERT_API_CALL_LOG();

    CM_RETURN_CODE  hr = CM_SUCCESS;
    uint8_t* dst = nullptr;
    uint8_t* surf = nullptr;
    size_t copySize = MOS_MIN((size_t)sysMemSize, m_size);

    if (sysMem == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to system memory is null.");
        return CM_NULL_POINTER;
    }

    // It makes sense to flush the whole enqueued tasks for each surface read
    // because usually we read the output of the last task.
    // Update: using event not to flush the whole enqueued tasks
    if (event)
    {
        CmEventRT* eventRT = dynamic_cast<CmEventRT*>(event);
        if (eventRT)
        {
            FlushDeviceQueue(eventRT);
        }
        else
        {
            event->WaitForTaskFinished();
        }
    }

    WaitForReferenceFree(); // wait all owner task finished

    // Lock Buffer first
    CmDeviceRT* cmDevice = nullptr;
    m_surfaceMgr->GetCmDevice(cmDevice);
    CM_CHK_NULL_RETURN_CMERROR(cmDevice);

    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)cmDevice->GetAccelData();
    CM_CHK_NULL_RETURN_CMERROR(cmData);
    CM_CHK_NULL_RETURN_CMERROR(cmData->cmHalState);

    CM_HAL_BUFFER_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(CM_HAL_BUFFER_PARAM));
    inParam.lockFlag = CM_HAL_LOCKFLAG_WRITEONLY;
    inParam.handle = m_handle;

    // Lock Buffer:
    // Lock Buffer may fail due to the out of memory/out of page-in in KMD.
    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnLockBuffer(cmData->cmHalState, &inParam));
    CM_CHK_NULL_GOTOFINISH_CMERROR(inParam.data);

    // Memory copy : Source ->System Memory  Dest -> Vedio Memory
    dst = (uint8_t*)(inParam.data);
    surf = (uint8_t*)sysMem;

    CmFastMemCopyWC(dst, surf, copySize);

    //Unlock Buffer
    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnUnlockBuffer(cmData->cmHalState, &inParam));

finish:
    if (hr < CM_MOS_STATUS_CONVERTED_CODE_OFFSET) {
        hr = CM_LOCK_SURFACE_FAIL;
    }
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Read data from sysMem to Buffer
//| Returns:    Result of the operation
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmBuffer_RT::ReadSurface(unsigned char* sysMem, CmEvent* event, uint64_t sysMemSize)
{
    INSERT_API_CALL_LOG();

    CM_RETURN_CODE  hr = CM_SUCCESS;

    size_t copySize = MOS_MIN((size_t)sysMemSize, m_size);

    if (sysMem == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to system memory is null.");
        return CM_NULL_POINTER;
    }

    // It makes sense to flush the whole enqueued tasks for each surface read
    // because usually we read the output of the last task.
    // Update: using event not to flush the whole enqueued tasks
    if (event)
    {
        CmEventRT* eventRT = dynamic_cast<CmEventRT*>(event);
        if (eventRT)
        {
            FlushDeviceQueue(eventRT);
        }
        else
        {
            event->WaitForTaskFinished();
        }
    }

    WaitForReferenceFree();    // wait all owner task finished

    // Lock Buffer first
    CmDeviceRT* cmDevice = nullptr;
    m_surfaceMgr->GetCmDevice(cmDevice);
    CM_CHK_NULL_RETURN_CMERROR(cmDevice);
    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)cmDevice->GetAccelData();
    CM_CHK_NULL_RETURN_CMERROR(cmData);
    CM_CHK_NULL_RETURN_CMERROR(cmData->cmHalState);

    CM_HAL_BUFFER_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(CM_HAL_BUFFER_PARAM));
    inParam.lockFlag = CM_HAL_LOCKFLAG_READONLY;
    inParam.handle = m_handle;

    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnLockBuffer(cmData->cmHalState, &inParam));
    CM_CHK_NULL_GOTOFINISH_CMERROR(inParam.data);

    // Memory copy : Dest ->System Memory  Source -> Vedio Memory
    CmFastMemCopyFromWC(sysMem, inParam.data, copySize, GetCpuInstructionLevel());

    //Unlock Buffer
    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnUnlockBuffer(cmData->cmHalState, &inParam));

finish:
    if (hr < CM_MOS_STATUS_CONVERTED_CODE_OFFSET) {
        hr = CM_LOCK_SURFACE_FAIL;
    }
    return hr;
}


CM_RT_API int32_t CmBuffer_RT::InitSurface(const uint32_t initValue, CmEvent* event)
{
    INSERT_API_CALL_LOG();

    CM_RETURN_CODE  hr          = CM_SUCCESS;

    // It makes sense to flush the whole enqueued tasks for each surface read
    // because usually we read the output of the last task.
    // Update: using event not to flush the whole enqueued tasks
    if( event )
    {
        CmEventRT *eventRT = dynamic_cast<CmEventRT *>(event);
        if (eventRT)
        {
            FlushDeviceQueue(eventRT);
        }
        else
        {
            event->WaitForTaskFinished();
        }
    }

    CmDeviceRT* cmDevice = nullptr;
    m_surfaceMgr->GetCmDevice( cmDevice );
    CM_CHK_NULL_RETURN_CMERROR(cmDevice);

    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)cmDevice->GetAccelData();
    CM_CHK_NULL_RETURN_CMERROR(cmData);
    CM_CHK_NULL_RETURN_CMERROR(cmData->cmHalState);

    CM_HAL_BUFFER_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_HAL_BUFFER_PARAM ) );
    inParam.handle = m_handle;
    inParam.lockFlag = CM_HAL_LOCKFLAG_WRITEONLY;

    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnLockBuffer(cmData->cmHalState, &inParam));
    CM_CHK_NULL_GOTOFINISH_CMERROR(inParam.data);

    CmDwordMemSet(inParam.data, initValue, m_size);

    // unlock
    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnUnlockBuffer(cmData->cmHalState, &inParam));

finish:
    if (hr < CM_MOS_STATUS_CONVERTED_CODE_OFFSET) {
        hr = CM_LOCK_SURFACE_FAIL;
    }
    return hr;
}

int32_t CmBuffer_RT::SetMemoryObjectControl( MEMORY_OBJECT_CONTROL memCtrl, MEMORY_TYPE memType, uint32_t age)
{

    int32_t  hr = CM_SUCCESS;
    uint16_t mocs = 0;
    hr = CmSurface::SetMemoryObjectControl( memCtrl, memType, age );

    CmDeviceRT *cmDevice = nullptr;
    m_surfaceMgr->GetCmDevice(cmDevice);
    CM_CHK_NULL_RETURN_CMERROR(cmDevice);
    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)cmDevice->GetAccelData();
    CM_CHK_NULL_RETURN_CMERROR(cmData);
    CM_CHK_NULL_RETURN_CMERROR(cmData->cmHalState);

    mocs = (m_memObjCtrl.mem_ctrl << 8) | (m_memObjCtrl.mem_type<<4) | m_memObjCtrl.age;
    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnSetSurfaceMOCS(cmData->cmHalState, m_handle, mocs, ARG_KIND_SURFACE_1D));
    ++ m_propertyIndex;
finish:
    return hr;
}

CM_RT_API int32_t CmBuffer_RT::SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL memCtrl)
{
    INSERT_API_CALL_LOG();
    ++ m_propertyIndex;
    return SetMemoryObjectControl(memCtrl, CM_USE_PTE, 0);
}

CMRT_UMD_API int32_t CmBuffer_RT::SetResourceUsage(const MOS_HW_RESOURCE_DEF mosUsage)
{
    INSERT_API_CALL_LOG();
    int32_t  hr = CM_SUCCESS;
    uint16_t mocs = 0;
    hr = CmSurface::SetResourceUsage(mosUsage);

    CmDeviceRT *cmDevice = nullptr;
    m_surfaceMgr->GetCmDevice(cmDevice);
    CM_CHK_NULL_RETURN_CMERROR(cmDevice);
    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)cmDevice->GetAccelData();
    CM_CHK_NULL_RETURN_CMERROR(cmData);
    CM_CHK_NULL_RETURN_CMERROR(cmData->cmHalState);

    mocs = (m_memObjCtrl.mem_ctrl << 8) | (m_memObjCtrl.mem_type << 4) | m_memObjCtrl.age;
    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnSetSurfaceMOCS(cmData->cmHalState, m_handle, mocs, ARG_KIND_SURFACE_1D));
    ++ m_propertyIndex;
finish:
    return hr;
}

CM_RT_API int32_t CmBuffer_RT::SetSurfaceStateParam(SurfaceIndex *surfIndex, const CM_BUFFER_STATE_PARAM *bufferStateParam)
{
    CM_RETURN_CODE hr = CM_SUCCESS;
    size_t newSize = 0;
    if(bufferStateParam->uiBaseAddressOffset + bufferStateParam->uiSize > m_size)
    {
        CM_ASSERTMESSAGE("Error: The offset exceeds the buffer size.");
        return CM_INVALID_ARG_VALUE;
    }
    if(bufferStateParam->uiBaseAddressOffset%16) // the offset must be 16-aligned, otherwise it will cause a GPU hang
    {
        CM_ASSERTMESSAGE("Error: The offset must be 16-aligned, otherwise it will cause GPU hang.");
        return CM_INVALID_ARG_VALUE;
    }

    if (bufferStateParam->uiSize)
    {
        newSize = bufferStateParam->uiSize;
    }
    else
    {
        newSize = m_size - bufferStateParam->uiBaseAddressOffset;
    }
    CmDeviceRT* cmDevice = nullptr;
    m_surfaceMgr->GetCmDevice( cmDevice );
    CM_CHK_NULL_RETURN_CMERROR(cmDevice);

    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)cmDevice->GetAccelData();
    CM_CHK_NULL_RETURN_CMERROR(cmData);
    CM_CHK_NULL_RETURN_CMERROR(cmData->cmHalState);

    CM_HAL_BUFFER_SURFACE_STATE_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( inParam ) );

    if( surfIndex )
    {
        inParam.aliasIndex  = surfIndex->get_data();
    }
    else
    {
        inParam.aliasIndex  = m_index->get_data();
    }
    inParam.handle  = m_handle;
    inParam.offset  = bufferStateParam->uiBaseAddressOffset;
    inParam.size    = newSize;
    inParam.mocs    = (uint16_t)((bufferStateParam->mocs.mem_ctrl << 8)|(bufferStateParam->mocs.mem_type << 4)|(bufferStateParam->mocs.age));

    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnSetBufferSurfaceStatePara(cmData->cmHalState, &inParam));
    ++ m_propertyIndex;

finish:
    return hr;
}

void CmBuffer_RT::SetSize( size_t size )
{
    m_size = size;
}

bool CmBuffer_RT::IsUpSurface()
{
    return (m_bufferType == CM_BUFFER_UP);
}

bool CmBuffer_RT::IsSVMSurface()
{
    return (m_bufferType == CM_BUFFER_SVM);
}

CM_RT_API int32_t CmBuffer_RT::GetAddress( void  *&addr)
{

    addr = m_sysMem;
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

int32_t CmBuffer_RT::CreateBufferAlias(SurfaceIndex* & aliasIndex)
{
    uint32_t surfArraySize = 0;

    if( m_numAliases < CM_HAL_MAX_NUM_BUFFER_ALIASES )
    {
        uint32_t origIndex = m_index->get_data();
        m_surfaceMgr->GetSurfaceArraySize(surfArraySize);
        uint32_t newIndex = origIndex + ( (m_numAliases + 1) * surfArraySize);
        m_aliasIndexes[m_numAliases] = MOS_New(SurfaceIndex, newIndex);
        if( m_aliasIndexes[m_numAliases] )
        {
            aliasIndex = m_aliasIndexes[m_numAliases];
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
        << " Size:"         << m_size
        << " Buffer Type:"  << m_bufferType
        << " Sys Address:"  << m_sysMem
        << " Handle:"       << m_handle
        << " SurfaceIndex:" << m_index->get_data()
        << " IsCmCreated:"  << m_isCmCreated
        << std::endl;
#endif
}

//*-----------------------------------------------------------------------------
//| Purpose:    Dump Surface's data into files
//| Returns:    None
//| Notes:      Must be called after task finished.
//*-----------------------------------------------------------------------------
void CmBuffer_RT::DumpContent(uint32_t kernelNumber, char *kernelName, int32_t taskId, uint32_t argIndex, uint32_t vectorIndex)
{
#if MDF_SURFACE_CONTENT_DUMP
    std::ostringstream outputFileName;
    static uint32_t bufferDumpNumber = 0;
    char               fileNamePrefix[MAX_PATH] = {0};
    std::ofstream      outputFileStream;

    outputFileName << "t_" << taskId
        << "_k_" << kernelNumber
        << "_" << kernelName
        <<"_argi_"<< argIndex
        <<"_vector_index_"<< vectorIndex
        << "_buffer_surfi_" << m_index->get_data()
        <<"_w_"<< m_size
        <<"_"<< bufferDumpNumber;

    GetLogFileLocation(outputFileName.str().c_str(), fileNamePrefix);

    // Open file
    outputFileStream.open(fileNamePrefix, std::ios::app | std::ios::binary);
    CM_ASSERT(outputFileStream);

    if (m_sysMem != nullptr)
    { // Buffer Up
        outputFileStream.write((char *)m_sysMem, m_size);
    }
    else
    { // Buffer
        std::vector<char>buffer(m_size);

        CmDeviceRT *cmDevice = nullptr;
        m_surfaceMgr->GetCmDevice(cmDevice);
        CM_ASSERT(cmDevice);
        
        PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)cmDevice->GetAccelData();
        CM_ASSERT(cmData);
        CM_ASSERT(cmData->cmHalState);
        
        CM_HAL_BUFFER_PARAM inParam;
        CmSafeMemSet(&inParam, 0, sizeof(CM_HAL_BUFFER_PARAM));
        inParam.lockFlag = CM_HAL_LOCKFLAG_READONLY;
        inParam.handle = m_handle;
        cmData->cmHalState->pfnLockBuffer(cmData->cmHalState, &inParam);
        if (inParam.data == nullptr)
            return;
        CmFastMemCopyFromWC((unsigned char *)&buffer[0], inParam.data, m_size, GetCpuInstructionLevel());
        cmData->cmHalState->pfnUnlockBuffer(cmData->cmHalState, &inParam);

        outputFileStream.write(&buffer[0], m_size);
    }
    outputFileStream.close();
    bufferDumpNumber++;
#endif
}

int32_t CmBuffer_RT::UpdateResource(MOS_RESOURCE *resource)
{
    // get index
    int index = m_index->get_data();
    return m_surfaceMgr->UpdateBuffer(resource, index, m_handle);
}

int32_t CmBuffer_RT::UpdateProperty(uint32_t size)
{
    if( ( size < CM_MIN_SURF_WIDTH ) || ( size > CM_MAX_1D_SURF_WIDTH ) )
    {
        CM_ASSERTMESSAGE("Error: Invalid buffer size.");
        return CM_INVALID_WIDTH;
    }
    m_size = size;
    return CM_SUCCESS;
}

CM_RT_API int32_t CmBuffer_RT::GetGfxAddress(uint64_t &gfxAddr)
{
    gfxAddr = m_gfxMem;
    return CM_SUCCESS;
}

CM_RT_API int32_t CmBuffer_RT::GetSysAddress(void *&sysAddr)
{
    sysAddr = m_sysMem;
    return CM_SUCCESS;
}
}
