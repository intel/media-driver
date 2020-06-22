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
//! \file      cm_surface.cpp 
//! \brief     Contains Class CmSurface  definitions 
//!

#include "cm_surface.h"

#include "cm_device_rt.h"
#include "cm_event_rt.h"
#include "cm_hal.h"
#include "cm_mem.h"
#include "cm_queue_rt.h"
#include "cm_surface_manager.h"
#include "cm_execution_adv.h"

namespace CMRT_UMD
{
//*-----------------------------------------------------------------------------
//| Purpose:    Destory CmSurface
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmSurface::Destroy( CmSurface* &surface )
{
    CmSafeDelete( surface );

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Constructor of CmSurface
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmSurface::CmSurface( CmSurfaceManager* surfMgr ,bool isCmCreated):
    m_index( nullptr ),
    m_surfaceMgr( surfMgr ),
    m_isCmCreated (isCmCreated),
    m_lastVeboxTracker(0),
    m_released(false),
    m_delayDestroyPrev(nullptr),
    m_delayDestroyNext(nullptr),
    m_propertyIndex(0)
{
    MOS_ZeroMemory(&m_memObjCtrl, sizeof(m_memObjCtrl));
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destructor of CmSurface
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmSurface::~CmSurface( void )
{
    MosSafeDelete(m_index);
}

//*-----------------------------------------------------------------------------
//| Purpose:    Initialize CmSurface
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmSurface::Initialize( uint32_t index )
{
    // set the tracker producer
    CmDeviceRT* cmDevice =  nullptr;
    m_surfaceMgr->GetCmDevice(cmDevice);
    PCM_HAL_STATE  cmHalState = ((PCM_CONTEXT_DATA)cmDevice->GetAccelData())->cmHalState;
    if (cmHalState == nullptr)
    {
        return CM_FAILURE;
    }
    m_lastRenderTracker.SetProducer(&cmHalState->renderHal->trackerProducer);
    if (cmHalState->advExecutor)
    {
        m_lastFastTracker.SetProducer(cmHalState->advExecutor->GetFastTrackerProducer());
    }
    // using CM compiler data structure
    m_index = MOS_New(SurfaceIndex, index);
    if( m_index )
    {
        return CM_SUCCESS;
    }
    else
    {
        return CM_OUT_OF_HOST_MEMORY;
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    Flush the task, once flushed, lock will untill the task finishes
//|             the execution of kernels upon the surface
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmSurface::FlushDeviceQueue( CmEventRT* event )
{
    if( event == nullptr )
    {
        CM_ASSERTMESSAGE("Error: Pointer to CM event is null.")
        return CM_FAILURE;
    }

    CmDeviceRT* device = nullptr;
    m_surfaceMgr->GetCmDevice( device );
    CM_ASSERT( device );

    //Used for timeout detection
    CmQueueRT* cmQueue = nullptr;
    event->GetQueue(cmQueue);
    uint32_t numTasks;
    cmQueue->GetTaskCount(numTasks);
    LARGE_INTEGER freq;
    MOS_QueryPerformanceFrequency((uint64_t*)&freq.QuadPart);
    LARGE_INTEGER start;
    MOS_QueryPerformanceCounter((uint64_t*)&start.QuadPart);
    int64_t timeout = start.QuadPart + (CM_MAX_TIMEOUT * freq.QuadPart * numTasks); //Count to timeout at

    CM_STATUS status;
    event->GetStatusNoFlush( status );
    // Not necessary CM_STATUS_FINISHED, once flushed, lock will wait
    // untill the task finishes the execution of kernels upon the surface
    while( status == CM_STATUS_QUEUED )
    {
        LARGE_INTEGER current;
        MOS_QueryPerformanceCounter((uint64_t*)&current.QuadPart);

        if( current.QuadPart > timeout )
            return CM_EXCEED_MAX_TIMEOUT;

        event->GetStatusNoFlush( status );
    }

    return CM_SUCCESS;
}

int32_t CmSurface::TouchDeviceQueue()
{
    CmDeviceRT* device = nullptr;

    m_surfaceMgr->GetCmDevice(device);
    CM_ASSERT(device);

    std::vector<CmQueueRT *> &cmQueue = device->GetQueue();
    CSync *lock = device->GetQueueLock();
    lock->Acquire();
    for (auto iter = cmQueue.begin(); iter != cmQueue.end(); iter++)
    {
        int32_t result = (*iter)->TouchFlushedTasks();
        if (FAILED(result))
        {
            lock->Release();
            return result;
        }
    }

    lock->Release();
    return CM_SUCCESS;;
}

int32_t CmSurface::WaitForReferenceFree()
{
    // Make sure the surface is not referenced any more
    while (!AllReferenceCompleted())
    {
        if (FAILED(TouchDeviceQueue()))
        {
            CM_ASSERTMESSAGE("Error: Failed to touch device queue.")
            return CM_FAILURE;
        };
    };

    return CM_SUCCESS;
}

bool CmSurface::MemoryObjectCtrlPolicyCheck(MEMORY_OBJECT_CONTROL memCtrl)
{
    if (memCtrl == MEMORY_OBJECT_CONTROL_UNKNOW)
    {
        return true;
    }

    CmDeviceRT* cmDevice =  nullptr;
    m_surfaceMgr->GetCmDevice(cmDevice);
    if(cmDevice == nullptr)
    {
        return false;
    }

    PCM_HAL_STATE  cmHalState = ((PCM_CONTEXT_DATA)cmDevice->GetAccelData())->cmHalState;
    if (cmHalState == nullptr)
    {
        return false;
    }

    return cmHalState->cmHalInterface->MemoryObjectCtrlPolicyCheck(memCtrl);

}

int32_t CmSurface::SetMemoryObjectControl(MEMORY_OBJECT_CONTROL memCtrl, MEMORY_TYPE memType, uint32_t age)
{
    if (!MemoryObjectCtrlPolicyCheck(memCtrl))
    {
        return CM_FAILURE;
    }
    CmDeviceRT* cmDevice = nullptr;
    m_surfaceMgr->GetCmDevice(cmDevice);
    CM_CHK_NULL_RETURN_CMERROR(cmDevice);
    uint32_t platform = 0;
    cmDevice->GetGenPlatform(platform);

    m_memObjCtrl.mem_ctrl = memCtrl;
    m_memObjCtrl.mem_type = memType;
    m_memObjCtrl.age= age;

    if (platform > IGFX_GEN8_CORE)
    {
        MOS_HW_RESOURCE_DEF defaultMocs = MOS_CM_RESOURCE_USAGE_SurfaceState;
        PCM_HAL_STATE  cmHalState = ((PCM_CONTEXT_DATA)cmDevice->GetAccelData())->cmHalState;
        if (cmHalState && cmHalState->cmHalInterface)
        {
            defaultMocs = cmHalState->cmHalInterface->GetDefaultMOCS();
        }

        switch (memCtrl)
        {
        case MEMORY_OBJECT_CONTROL_DEFAULT:
            m_memObjCtrl.mem_ctrl = (unsigned int)defaultMocs;
            break;

        case MEMORY_OBJECT_CONTROL_NO_L3:
            m_memObjCtrl.mem_ctrl = MOS_CM_RESOURCE_USAGE_NO_L3_SurfaceState;
            break;

        case MEMORY_OBJECT_CONTROL_NO_LLC_ELLC:
            m_memObjCtrl.mem_ctrl = MOS_CM_RESOURCE_USAGE_NO_LLC_ELLC_SurfaceState;
            break;

        case MEMORY_OBJECT_CONTROL_NO_LLC:
            m_memObjCtrl.mem_ctrl = MOS_CM_RESOURCE_USAGE_NO_LLC_SurfaceState;
            break;

        case MEMORY_OBJECT_CONTROL_NO_ELLC:
            m_memObjCtrl.mem_ctrl = MOS_CM_RESOURCE_USAGE_NO_ELLC_SurfaceState;
            break;

        case MEMORY_OBJECT_CONTROL_NO_LLC_L3:
            m_memObjCtrl.mem_ctrl = MOS_CM_RESOURCE_USAGE_NO_LLC_L3_SurfaceState;
            break;

        case MEMORY_OBJECT_CONTROL_NO_ELLC_L3:
            m_memObjCtrl.mem_ctrl = MOS_CM_RESOURCE_USAGE_NO_ELLC_L3_SurfaceState;
            break;

        case MEMORY_OBJECT_CONTROL_NO_CACHE:
            m_memObjCtrl.mem_ctrl = MOS_CM_RESOURCE_USAGE_NO_CACHE_SurfaceState;
            break;

        case MEMORY_OBJECT_CONTROL_L1_ENABLED:
            m_memObjCtrl.mem_ctrl = MOS_CM_RESOURCE_USAGE_L1_Enabled_SurfaceState;
            break;

        default:
            // any invalid CM_HAL_MEMORY_OBJECT_CONTROL value is converted to default
            m_memObjCtrl.mem_ctrl = (unsigned int)defaultMocs;
            break;
        }
    }

    return CM_SUCCESS;
}

int32_t CmSurface::SetResourceUsage(MOS_HW_RESOURCE_DEF mosUsage)
{
    CmDeviceRT* cmDevice = nullptr;
    m_surfaceMgr->GetCmDevice(cmDevice);
    CM_CHK_NULL_RETURN_CMERROR(cmDevice);
    uint32_t platform = 0;
    cmDevice->GetGenPlatform(platform);
    // MOS usage memory object setting is only available to gen9+
    if (platform > IGFX_GEN8_CORE)
    if (mosUsage < MOS_HW_RESOURCE_DEF_MAX)
    {
        m_memObjCtrl.mem_ctrl = mosUsage;
        m_memObjCtrl.mem_type = CM_USE_PTE;
        m_memObjCtrl.age = 0;
        return CM_SUCCESS;
    }
    return CM_FAILURE;
}

std::string CmSurface::GetFormatString(CM_SURFACE_FORMAT format)
{
    switch (format)
    {
        case CM_SURFACE_FORMAT_A8R8G8B8:           return "argb";
        case CM_SURFACE_FORMAT_X8R8G8B8:           return "xrgb";
        case CM_SURFACE_FORMAT_A8B8G8R8:           return "abgr";
        case CM_SURFACE_FORMAT_A8:                 return "a8";
        case CM_SURFACE_FORMAT_P8:                 return "p8";
        case CM_SURFACE_FORMAT_R32F:               return "r32f";
        case CM_SURFACE_FORMAT_NV12:               return "nv12";
        case CM_SURFACE_FORMAT_P016:               return "p016";
        case CM_SURFACE_FORMAT_P010:               return "p010";
        case CM_SURFACE_FORMAT_P208:               return "p208";
        case CM_SURFACE_FORMAT_V8U8:               return "v8u8";
        case CM_SURFACE_FORMAT_A8L8:               return "a8l8";
        case CM_SURFACE_FORMAT_D16:                return "d16";
        case CM_SURFACE_FORMAT_A16B16G16R16F:      return "argb16f";
        case CM_SURFACE_FORMAT_R10G10B10A2:        return "r10g10b10a2";
        case CM_SURFACE_FORMAT_A16B16G16R16:       return "argb16";
        case CM_SURFACE_FORMAT_IRW0:               return "irw0";
        case CM_SURFACE_FORMAT_IRW1:               return "irw1";
        case CM_SURFACE_FORMAT_IRW2:               return "irw2";
        case CM_SURFACE_FORMAT_IRW3:               return "irw3";
        case CM_SURFACE_FORMAT_R32_SINT:           return "r32s";
        case CM_SURFACE_FORMAT_R16_FLOAT:          return "r16f";
        case CM_SURFACE_FORMAT_A8P8:               return "a8p8";
        case CM_SURFACE_FORMAT_I420:               return "i420";
        case CM_SURFACE_FORMAT_IMC3:               return "imc3";
        case CM_SURFACE_FORMAT_IA44:               return "ia44";
        case CM_SURFACE_FORMAT_AI44:               return "ai44";
        case CM_SURFACE_FORMAT_Y410:               return "y410";
        case CM_SURFACE_FORMAT_Y416:               return "y416";
        case CM_SURFACE_FORMAT_Y210:               return "y210";
        case CM_SURFACE_FORMAT_Y216:               return "y216";
        case CM_SURFACE_FORMAT_AYUV:               return "ayuv";
        case CM_SURFACE_FORMAT_YV12:               return "yv12";
        case CM_SURFACE_FORMAT_400P:               return "400p";
        case CM_SURFACE_FORMAT_411P:               return "411p";
        case CM_SURFACE_FORMAT_411R:               return "411r";
        case CM_SURFACE_FORMAT_422H:               return "422h";
        case CM_SURFACE_FORMAT_422V:               return "422v";
        case CM_SURFACE_FORMAT_444P:               return "444p";
        case CM_SURFACE_FORMAT_RGBP:               return "rgbp";
        case CM_SURFACE_FORMAT_BGRP:               return "bgrp";
        case CM_SURFACE_FORMAT_R8_UINT:            return "r8u";
        case CM_SURFACE_FORMAT_R32_UINT:           return "r32u";
        case CM_SURFACE_FORMAT_R16_SINT:           return "r16s";
        case CM_SURFACE_FORMAT_R16_UNORM:          return "r16un";
        case CM_SURFACE_FORMAT_R8G8_UNORM:         return "r8g8un";
        case CM_SURFACE_FORMAT_R16_UINT:           return "r16u";
        case CM_SURFACE_FORMAT_R16G16_UNORM:       return "r16g16un";
        case CM_SURFACE_FORMAT_L16:                return "l16";
        case CM_SURFACE_FORMAT_YUY2:               return "yuy2";
        case CM_SURFACE_FORMAT_L8:                 return "l8";
        case CM_SURFACE_FORMAT_UYVY:               return "uyvy";
        case CM_SURFACE_FORMAT_VYUY:               return "vyuy";
        case CM_SURFACE_FORMAT_R8G8_SNORM:         return "r8g8sn";
        case CM_SURFACE_FORMAT_Y16_SNORM:          return "y16sn";
        case CM_SURFACE_FORMAT_Y16_UNORM:          return "y16un";
        case CM_SURFACE_FORMAT_Y8_UNORM:           return "y8un";
        case CM_SURFACE_FORMAT_BUFFER_2D:          return "buffer2d";
        case CM_SURFACE_FORMAT_D32F:               return "d32f";
        case CM_SURFACE_FORMAT_D24_UNORM_S8_UINT:  return "d24uns8ui";
        case CM_SURFACE_FORMAT_D32F_S8X24_UINT:    return "d32fs8x24ui";
        case CM_SURFACE_FORMAT_R16G16_SINT:        return "r16g16si";
        case CM_SURFACE_FORMAT_R16_TYPELESS:       return "r16";
        case CM_SURFACE_FORMAT_R24G8_TYPELESS:     return "r24g8";
        case CM_SURFACE_FORMAT_R32_TYPELESS:       return "r32";
        case CM_SURFACE_FORMAT_R32G8X24_TYPELESS:  return "r32g8x24";
        case CM_SURFACE_FORMAT_R8_UNORM:           return "r8un";
        case CM_SURFACE_FORMAT_R32G32B32A32F:      return "rgba32f";
        default:                                   return "Invalid";
    }
}
}