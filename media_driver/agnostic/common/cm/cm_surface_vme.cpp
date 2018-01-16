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
    CmSurfaceManager* pSurfaceManager,
    CmSurfaceVme* &pSurface )
{
    int32_t result = CM_SUCCESS;

    pSurface = new (std::nothrow) CmSurfaceVme( indexFor2DCurrent, indexFor2DForward, indexFor2DBackward, indexForCurrent, indexForForward, indexForBackward, pSurfaceManager );
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
        CM_ASSERTMESSAGE("Error: Failed to create CmSurfaceVme due to out of system memory.");
        result = CM_OUT_OF_HOST_MEMORY;
    }

    return result;
}

int32_t CmSurfaceVme::Create( uint32_t index,
                              uint32_t indexFor2DCurSurface,
                              uint32_t *pForwardSurface,
                              uint32_t *pBackwardSurface,
                              uint32_t indexCurrent,
                              uint32_t *pForward,
                              uint32_t *pBackward,
                              const uint32_t surfaceFCount,
                              const uint32_t surfaceBCount,
                              CmSurfaceManager* pSurfaceManager,
                              CmSurfaceVme* &pSurface )
{
    int32_t result = CM_SUCCESS;

    pSurface = new (std::nothrow) CmSurfaceVme(surfaceFCount, surfaceBCount, indexFor2DCurSurface, pForwardSurface, pBackwardSurface, indexCurrent, pForward, pBackward,  pSurfaceManager);
    if( pSurface )
    {
        result = pSurface->Initialize(index);
        if( result != CM_SUCCESS )
        {
            CmSurface* pBaseSurface = pSurface;
            CmSurface::Destroy( pBaseSurface );
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
    CmSurfaceManager* pSurfaceManager ):
    CmSurface( pSurfaceManager,true ),
    m_IndexFor2DCurrent( indexFor2DCurrent ),
    m_IndexFor2DForward( indexFor2DForward ),
    m_IndexFor2DBackward( indexFor2DBackward ),
    m_CmIndexForCurrent( indexForCurrent ),
    m_CmIndexForForward( indexForForward ),
    m_CmIndexForBackward( indexForBackward ),
    m_pForwardSurfaceArray(nullptr),
    m_pBackwardSurfaceArray(nullptr),
    m_pForwardCmIndexArray(nullptr),
    m_pBackwardCmIndexArray(nullptr),
    m_IsGen7_5(false),
    m_surfStateWidth(0),
    m_surfStateHeight(0)
{
    if (indexForForward != CM_INVALID_VME_SURFACE)
    {
        m_SurfaceFCount = 1;
    }
    else
    {
        m_SurfaceFCount = 0;
    }

    if (indexForBackward != CM_INVALID_VME_SURFACE)
    {
        m_SurfaceBCount = 1;
    }
    else
    {
        m_SurfaceBCount = 0;
    }
}

CmSurfaceVme::CmSurfaceVme(
                            const uint32_t surfaceFCount,
                            const uint32_t surfaceBCount,
                            uint32_t indexFor2DCurSurface,
                            uint32_t *pForwardSurface,
                            uint32_t *pBackwardSurface,
                            uint32_t indexCurrent,
                            uint32_t *pForward,
                            uint32_t *pBackward,
                            CmSurfaceManager* pSurfaceManager):
                            CmSurface( pSurfaceManager, false),
                            m_IndexFor2DCurrent(indexFor2DCurSurface),
                            m_IndexFor2DForward(0),
                            m_IndexFor2DBackward(0),
                            m_CmIndexForForward(0),
                            m_CmIndexForBackward(0),
                            m_pForwardSurfaceArray(pForwardSurface),
                            m_pBackwardSurfaceArray(pBackwardSurface),
                            m_CmIndexForCurrent( indexCurrent ),
                            m_pForwardCmIndexArray(pForward),
                            m_pBackwardCmIndexArray(pBackward),
                            m_SurfaceFCount(surfaceFCount),
                            m_SurfaceBCount(surfaceBCount),
                            m_IsGen7_5(true),
                            m_surfStateWidth(0),
                            m_surfStateHeight(0)
{
}

CmSurfaceVme::~CmSurfaceVme( void )
{
    MosSafeDeleteArray(m_pForwardSurfaceArray);
    MosSafeDeleteArray(m_pBackwardSurfaceArray);
    MosSafeDeleteArray(m_pForwardCmIndexArray);
    MosSafeDeleteArray(m_pBackwardCmIndexArray);

}

int32_t CmSurfaceVme::Initialize( uint32_t index )
{
    return CmSurface::Initialize( index );
}

int32_t CmSurfaceVme::GetIndex( SurfaceIndex*& pIndex )
{
    pIndex = m_pIndex;
    return CM_SUCCESS;
}

int32_t CmSurfaceVme::GetIndexCurrent( uint32_t& index )
{
    index = m_IndexFor2DCurrent;
    return CM_SUCCESS;
}

int32_t CmSurfaceVme::GetIndexForward( uint32_t& index )
{
    index = m_IndexFor2DForward;
    return CM_SUCCESS;
}

int32_t CmSurfaceVme::GetIndexBackward( uint32_t& index )
{
    index = m_IndexFor2DBackward;
    return CM_SUCCESS;
}

int32_t CmSurfaceVme::GetIndexForwardArray( uint32_t *&index_array)
{
    index_array = m_pForwardSurfaceArray;
    return CM_SUCCESS;
}

int32_t CmSurfaceVme::GetIndexBackwardArray( uint32_t *& index_array)
{
    index_array = m_pBackwardSurfaceArray;
    return CM_SUCCESS;
}

int32_t CmSurfaceVme::GetIndexForwardCount( uint32_t &count)
{
    count= m_SurfaceFCount;
    return CM_SUCCESS;
}

/////////////
int32_t CmSurfaceVme::GetCmIndexCurrent( uint16_t & index )
{
    index = (uint16_t)m_CmIndexForCurrent;
    return CM_SUCCESS;
}

int32_t CmSurfaceVme::GetCmIndexForward( uint16_t& index )
{
    index = (uint16_t)m_CmIndexForForward;
    return CM_SUCCESS;
}

int32_t CmSurfaceVme::GetCmIndexBackward( uint16_t& index )
{
    index = (uint16_t)m_CmIndexForBackward;
    return CM_SUCCESS;
}

int32_t CmSurfaceVme::GetCmIndexForwardArray( uint32_t *&index_array)
{
    index_array = m_pForwardCmIndexArray;
    return CM_SUCCESS;
}

int32_t CmSurfaceVme::GetCmIndexBackwardArray( uint32_t *& index_array)
{
    index_array = m_pBackwardCmIndexArray;
    return CM_SUCCESS;
}
/////////////

int32_t CmSurfaceVme::GetIndexBackwardCount( uint32_t & count)
{
    count = m_SurfaceBCount;
    return CM_SUCCESS;
}

bool CmSurfaceVme::IsVmeSurfaceGen7_5()
{
    return m_IsGen7_5;
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
    return m_SurfaceFCount + m_SurfaceBCount + 1;
}

int32_t CmSurfaceVme::GetVmeCmArgSize()
{
    return sizeof(CM_HAL_VME_ARG_VALUE) + (m_SurfaceFCount + m_SurfaceBCount) * sizeof(uint32_t);
}
}

