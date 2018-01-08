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
//! \file      cm_surface_sampler.cpp  
//! \brief     Contains Class CmSurfaceSampler definitions  
//!

#include "cm_surface_sampler.h"

#include "cm_surface_manager.h"
#include "cm_device_rt.h"
#include "cm_hal.h"

namespace CMRT_UMD
{
int32_t CmSurfaceSampler::Create( 
    uint32_t index,                //SurfaceSampler's index in surface array
    uint32_t handleFor2D3D,        //indexing resource array of HalCm
    uint32_t indexForCurrent,      //SurfaceIndex's ID for 2D/3D, also indexing surface array
    SAMPLER_SURFACE_TYPE surfaceType,
    CmSurfaceManager* pSurfaceManager,
    CmSurfaceSampler* &pSurfaceSampler,
    CM_FLAG* pFlag)
{
    int32_t result = CM_SUCCESS;

    pSurfaceSampler = new (std::nothrow) CmSurfaceSampler( indexForCurrent, handleFor2D3D, surfaceType, pSurfaceManager, pFlag);
    if( pSurfaceSampler )
    {
        result = pSurfaceSampler->Initialize( index );
        if( result != CM_SUCCESS )
        {
            CmSurface* pBaseSurface = pSurfaceSampler;
            CmSurface::Destroy( pBaseSurface );
        }
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to create CmSurfaceSampler due to out of system memory.");
        result = CM_OUT_OF_HOST_MEMORY;
    }

    return result;
}

CmSurfaceSampler::CmSurfaceSampler( 
    uint32_t indexForCurrent, 
    uint32_t handleFor2D3D,
    SAMPLER_SURFACE_TYPE surfaceType,
    CmSurfaceManager* pSurfaceManager,
    CM_FLAG* pFlag):
    CmSurface(pSurfaceManager,false ), 
    m_CmIndexForCurrent(indexForCurrent),
    m_handleFor2D3D( handleFor2D3D),
    m_surfaceType(surfaceType)
{
    if (pFlag != nullptr)
    {
        m_Flag.rotationFlag = pFlag->rotationFlag;
        m_Flag.chromaSiting = pFlag->chromaSiting;
    }
}

CmSurfaceSampler::~CmSurfaceSampler( void )
{
}

int32_t CmSurfaceSampler::Initialize( uint32_t index )
{
    CmSurfaceManager* pSurfMgr = m_SurfaceMgr;
    pSurfMgr->UpdateSurface2DTableRotation(m_handleFor2D3D, m_Flag.rotationFlag);
    return CmSurface::Initialize( index );
}

int32_t CmSurfaceSampler::GetSurfaceIndex( SurfaceIndex*& pIndex ) 
{ 
    pIndex = m_pIndex; 
    return CM_SUCCESS;
}

int32_t CmSurfaceSampler::GetHandle( uint32_t& handle ) 
{ 
    handle = m_handleFor2D3D; 
    return CM_SUCCESS;
}

int32_t CmSurfaceSampler::GetSurfaceType(SAMPLER_SURFACE_TYPE& type)
{
    type = m_surfaceType;
    return CM_SUCCESS;
}

int32_t CmSurfaceSampler::GetCmIndexCurrent( uint16_t & index ) 
{ 
    index = (uint16_t)m_CmIndexForCurrent; 
    return CM_SUCCESS;
}

int32_t CmSurfaceSampler::SetMemoryObjectControl( MEMORY_OBJECT_CONTROL mem_ctrl, MEMORY_TYPE mem_type, uint32_t age)
{
    CM_RETURN_CODE  hr = CM_SUCCESS;
    uint16_t mocs = 0;
    
    CmSurface::SetMemoryObjectControl( mem_ctrl, mem_type, age );

    CmDeviceRT *pCmDevice = nullptr;
    m_SurfaceMgr->GetCmDevice(pCmDevice);
    PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)pCmDevice->GetAccelData();
    CMCHK_NULL(pCmData);

    mocs = (m_MemObjCtrl.mem_ctrl << 8) | (m_MemObjCtrl.mem_type<<4) | m_MemObjCtrl.age;

    CM_ARG_KIND argType;

    if (m_surfaceType == SAMPLER_SURFACE_TYPE_2D)
    {
        argType = ARG_KIND_SURFACE_SAMPLER;
    }
    else if (m_surfaceType == SAMPLER_SURFACE_TYPE_2DUP)
    {
        argType = ARG_KIND_SURFACE2DUP_SAMPLER;
    }
    else
    {
        argType = ARG_KIND_SURFACE_3D;
    }

    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->cmHalState->pfnSetSurfaceMOCS(pCmData->cmHalState, m_handleFor2D3D, mocs, argType));

finish:
    return hr;
}
}
