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
//! \file      cm_surface_sampler8x8.cpp  
//! \brief     Contains Class CmSurfaceSampler8x8  definitions  
//!

#include "cm_surface_sampler8x8.h"

#include "cm_debug.h"
#include "cm_surface_manager.h"
#include "cm_device_rt.h"
#include "cm_hal.h"

namespace CMRT_UMD
{
//*-----------------------------------------------------------------------------
//| Purpose:    Create CmSurfaceSampler8x8
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmSurfaceSampler8x8::Create( 
    uint32_t index, 
    uint32_t indexFor2D,            //indexing resource array of HalCm
    uint32_t cmIndex,      //SurfaceIndex's ID for 2D, also indexing surface array in cmrt@umd
    CmSurfaceManager* pSurfaceManager,
    CmSurfaceSampler8x8* &pSurface,
    CM_SAMPLER8x8_SURFACE sampler8x8_type,
    CM_SURFACE_ADDRESS_CONTROL_MODE mode,
    CM_FLAG* pFlag)
{
    int32_t result = CM_SUCCESS;

    pSurface = new ( std::nothrow ) CmSurfaceSampler8x8( cmIndex, indexFor2D, pSurfaceManager, sampler8x8_type, mode, pFlag);
    if ( pSurface )
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
        CM_ASSERTMESSAGE("Error: Failed to create CmSurfaceSampler8x8 due to out of system memory.");
        result = CM_OUT_OF_HOST_MEMORY;
    }

    return result;

}

// Constructor of CmSurfaceSampler8x8
CmSurfaceSampler8x8::CmSurfaceSampler8x8( 
    uint32_t cmIndex,      //SurfaceIndex's ID for 2D, also indexing surface array in cmrt@umd
    uint32_t indexFor2D,
    CmSurfaceManager* pSurfaceManager,
    CM_SAMPLER8x8_SURFACE sampler8x8_type,
    CM_SURFACE_ADDRESS_CONTROL_MODE mode,
    CM_FLAG* pFlag) :
    CmSurface(pSurfaceManager, false ),
    m_indexFor2D( indexFor2D ),
    m_CmIndex(cmIndex),
    m_sampler8x8_type( sampler8x8_type ),
    m_nAddressMode( mode )
{
    if (pFlag != nullptr)
    {
        m_Flag.rotationFlag = pFlag->rotationFlag;
        m_Flag.chromaSiting = pFlag->chromaSiting;
    }
}

// Destructor of CmSurfaceSampler8x8
CmSurfaceSampler8x8::~CmSurfaceSampler8x8( void )
{
}

//*-----------------------------------------------------------------------------
//| Purpose:    Initialize CmSurfaceSampler8x8
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmSurfaceSampler8x8::Initialize( uint32_t index )
{
    CmSurfaceManager* pSurfMgr = m_SurfaceMgr;

    pSurfMgr->UpdateSurface2DTableRotation(m_indexFor2D, m_Flag.rotationFlag);
    pSurfMgr->UpdateSurface2DTableChromaSiting(m_indexFor2D, m_Flag.chromaSiting);

    return CmSurface::Initialize( index );
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the index of CmSurfaceSampler8x8
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmSurfaceSampler8x8::GetIndex( SurfaceIndex*& pIndex ) 
{ 
    pIndex = m_pIndex; 
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the current index of CmSurfaceSampler8x8
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmSurfaceSampler8x8::GetIndexCurrent( uint32_t& index ) 
{ 
    index = m_indexFor2D; 
    return CM_SUCCESS;
}

int32_t CmSurfaceSampler8x8::GetCmIndex( uint16_t& index ) 
{ 
    index = (uint16_t)m_CmIndex; 
    return CM_SUCCESS;
}

CM_SAMPLER8x8_SURFACE CmSurfaceSampler8x8::GetSampler8x8SurfaceType()
{
    return m_sampler8x8_type;
}

CM_SURFACE_ADDRESS_CONTROL_MODE CmSurfaceSampler8x8::GetAddressControlMode()
{
    return m_nAddressMode;
}

int32_t CmSurfaceSampler8x8::SetMemoryObjectControl(MEMORY_OBJECT_CONTROL mem_ctrl, MEMORY_TYPE mem_type, uint32_t age)
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

    if (m_sampler8x8_type == CM_VA_SURFACE)
    {
        argType = ARG_KIND_SURFACE_SAMPLER8X8_VA;
    }
    else
    {
        argType = ARG_KIND_SURFACE_SAMPLER8X8_AVS;
    }

    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->cmHalState->pfnSetSurfaceMOCS(pCmData->cmHalState, m_indexFor2D, mocs, argType));

finish:
    return hr;
}
}
