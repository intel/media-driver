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
//! \file      cm_surface_vme.cpp 
//! \brief     Contains Class CmSurfaceVme  definitions 
//!

#include "cm_surface_vme.h"
#include "cm_mem.h"
#include "cm_hal.h"
#include "cm_execution_adv.h"

namespace CMRT_UMD
{
int32_t CmSurfaceVme::Create(
    uint32_t index,
    uint32_t indexFor2DCurrent,
    uint32_t indexFor2DForward,
    uint32_t indexFor2DBackward,
    uint32_t indexForCurrent,
    uint32_t indexForForward,
    uint32_t indexForBackward,
    CmSurfaceManager* surfaceManager,
    CmSurfaceVme* &surface )
{
    int32_t result = CM_SUCCESS;

    surface = new (std::nothrow) CmSurfaceVme( indexFor2DCurrent, indexFor2DForward, indexFor2DBackward, indexForCurrent, indexForForward, indexForBackward, surfaceManager );
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
        CM_ASSERTMESSAGE("Error: Failed to create CmSurfaceVme due to out of system memory.");
        result = CM_OUT_OF_HOST_MEMORY;
    }

    return result;
}

int32_t CmSurfaceVme::Create( uint32_t index,
                              uint32_t indexFor2DCurSurface,
                              uint32_t *forwardSurface,
                              uint32_t *backwardSurface,
                              uint32_t currentIndex,
                              uint32_t *forwardCmIndex,
                              uint32_t *backwardCmIndex,
                              const uint32_t surfaceFCount,
                              const uint32_t surfaceBCount,
                              CmSurfaceManager* surfaceManager,
                              CmSurfaceVme* &surface )
{
    int32_t result = CM_SUCCESS;

    surface = new (std::nothrow) CmSurfaceVme(surfaceFCount, surfaceBCount, indexFor2DCurSurface, forwardSurface, backwardSurface, currentIndex, forwardCmIndex, backwardCmIndex,  surfaceManager);
    if( surface )
    {
        result = surface->Initialize(index);
        if( result != CM_SUCCESS )
        {
            CmSurface* baseSurface = surface;
            CmSurface::Destroy( baseSurface );
        }
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to create CmSurfaceVme due to out of system memory.");
        result = CM_OUT_OF_HOST_MEMORY;
    }

    return result;
}

CmSurfaceVme::CmSurfaceVme(
    uint32_t indexFor2DCurrent,
    uint32_t indexFor2DForward,
    uint32_t indexFor2DBackward,
    uint32_t indexForCurrent,
    uint32_t indexForForward,
    uint32_t indexForBackward,
    CmSurfaceManager* surfaceManager ):
    CmSurface( surfaceManager,true ),
    m_indexFor2DCurrent( indexFor2DCurrent ),
    m_indexFor2DForward( indexFor2DForward ),
    m_indexFor2DBackward( indexFor2DBackward ),
    m_cmIndexForCurrent( indexForCurrent ),
    m_cmIndexForForward( indexForForward ),
    m_cmIndexForBackward( indexForBackward ),
    m_forwardSurfaceArray(nullptr),
    m_backwardSurfaceArray(nullptr),
    m_forwardCmIndexArray(nullptr),
    m_backwardCmIndexArray(nullptr),
    m_isGen75(false),
    m_surfStateWidth(0),
    m_surfStateHeight(0),
    m_argValue(nullptr),
    m_surfState(nullptr),
    m_advExec(nullptr)
{
    if (indexForForward != CM_INVALID_VME_SURFACE)
    {
        m_surfaceFCount = 1;
    }
    else
    {
        m_surfaceFCount = 0;
    }

    if (indexForBackward != CM_INVALID_VME_SURFACE)
    {
        m_surfaceBCount = 1;
    }
    else
    {
        m_surfaceBCount = 0;
    }
}

CmSurfaceVme::CmSurfaceVme(
                            const uint32_t surfaceFCount,
                            const uint32_t surfaceBCount,
                            uint32_t indexFor2DCurSurface,
                            uint32_t *forwardSurface,
                            uint32_t *backwardSurface,
                            uint32_t currentIndex,
                            uint32_t *forwardCmIndex,
                            uint32_t *backwardCmIndex,
                            CmSurfaceManager* surfaceManager):
                            CmSurface( surfaceManager, false),
                            m_indexFor2DCurrent(indexFor2DCurSurface),
                            m_indexFor2DForward(0),
                            m_indexFor2DBackward(0),
                            m_cmIndexForForward(0),
                            m_cmIndexForBackward(0),
                            m_forwardSurfaceArray(forwardSurface),
                            m_backwardSurfaceArray(backwardSurface),
                            m_cmIndexForCurrent( currentIndex ),
                            m_forwardCmIndexArray(forwardCmIndex),
                            m_backwardCmIndexArray(backwardCmIndex),
                            m_surfaceFCount(surfaceFCount),
                            m_surfaceBCount(surfaceBCount),
                            m_isGen75(true),
                            m_surfStateWidth(0),
                            m_surfStateHeight(0),
                            m_argValue(nullptr),
                            m_surfState(nullptr),
                            m_advExec(nullptr)
{
}

CmSurfaceVme::~CmSurfaceVme( void )
{
    MosSafeDeleteArray(m_forwardSurfaceArray);
    MosSafeDeleteArray(m_backwardSurfaceArray);
    MosSafeDeleteArray(m_forwardCmIndexArray);
    MosSafeDeleteArray(m_backwardCmIndexArray);
    MosSafeDeleteArray(m_argValue);
    if (m_advExec)
    {
        m_advExec->DeleteSurfStateVme(m_surfState);
    }

}

int32_t CmSurfaceVme::Initialize( uint32_t index )
{
    return CmSurface::Initialize( index );
}

int32_t CmSurfaceVme::GetIndex( SurfaceIndex*& index )
{
    index = m_index;
    return CM_SUCCESS;
}

int32_t CmSurfaceVme::GetIndexCurrent( uint32_t& index )
{
    index = m_indexFor2DCurrent;
    return CM_SUCCESS;
}

int32_t CmSurfaceVme::GetIndexForward( uint32_t& index )
{
    index = m_indexFor2DForward;
    return CM_SUCCESS;
}

int32_t CmSurfaceVme::GetIndexBackward( uint32_t& index )
{
    index = m_indexFor2DBackward;
    return CM_SUCCESS;
}

int32_t CmSurfaceVme::GetIndexForwardArray( uint32_t *&indexArray)
{
    indexArray = m_forwardSurfaceArray;
    return CM_SUCCESS;
}

int32_t CmSurfaceVme::GetIndexBackwardArray( uint32_t *& indexArray)
{
    indexArray = m_backwardSurfaceArray;
    return CM_SUCCESS;
}

int32_t CmSurfaceVme::GetIndexForwardCount( uint32_t &count)
{
    count= m_surfaceFCount;
    return CM_SUCCESS;
}

/////////////
int32_t CmSurfaceVme::GetCmIndexCurrent( uint16_t & index )
{
    index = (uint16_t)m_cmIndexForCurrent;
    return CM_SUCCESS;
}

int32_t CmSurfaceVme::GetCmIndexForward( uint16_t& index )
{
    index = (uint16_t)m_cmIndexForForward;
    return CM_SUCCESS;
}

int32_t CmSurfaceVme::GetCmIndexBackward( uint16_t& index )
{
    index = (uint16_t)m_cmIndexForBackward;
    return CM_SUCCESS;
}

int32_t CmSurfaceVme::GetCmIndexForwardArray( uint32_t *&indexArray)
{
    indexArray = m_forwardCmIndexArray;
    return CM_SUCCESS;
}

int32_t CmSurfaceVme::GetCmIndexBackwardArray( uint32_t *& indexArray)
{
    indexArray = m_backwardCmIndexArray;
    return CM_SUCCESS;
}
/////////////

int32_t CmSurfaceVme::GetIndexBackwardCount( uint32_t & count)
{
    count = m_surfaceBCount;
    return CM_SUCCESS;
}

bool CmSurfaceVme::IsVmeSurfaceGen7_5()
{
    return m_isGen75;
}

int32_t CmSurfaceVme::GetSurfaceStateResolution(uint32_t& width, uint32_t& height)
{
    width = m_surfStateWidth;
    height = m_surfStateHeight;
    return CM_SUCCESS;
}

int32_t CmSurfaceVme::SetSurfaceStateResolution(uint32_t width, uint32_t height)
{
    m_surfStateWidth = width;
    m_surfStateHeight = height;
    return CM_SUCCESS;
}

int32_t CmSurfaceVme::GetTotalSurfacesCount()
{
    return m_surfaceFCount + m_surfaceBCount + 1;
}

int32_t CmSurfaceVme::GetVmeCmArgSize()
{
    return sizeof(CM_HAL_VME_ARG_VALUE) + (m_surfaceFCount + m_surfaceBCount) * sizeof(uint32_t);
}

void CmSurfaceVme::SetSurfState(CmExecutionAdv *advExec, uint8_t *argValue, CmSurfaceStateVME *surfState)
{
    MosSafeDeleteArray(m_argValue);
    if (advExec)
    {
        advExec->DeleteSurfStateVme(m_surfState);
    }

    m_advExec = advExec;
    m_argValue = argValue;
    m_surfState = surfState;
}

}

