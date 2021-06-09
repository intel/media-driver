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
//! \file      cm_vebox_rt.cpp
//! \brief     Contains CmVeboxRT implementations.
//!

#include "cm_vebox_rt.h"

#include "cm_device_rt.h"
#include "cm_surface_2d_rt.h"

namespace CMRT_UMD
{
//*-----------------------------------------------------------------------------
//| Purpose:    Create Vebox
//*-----------------------------------------------------------------------------
int32_t CmVeboxRT::Create( CmDeviceRT* device, uint32_t index, CmVeboxRT* & cmVebox )
{
    int32_t result = CM_SUCCESS;

    cmVebox = new (std::nothrow) CmVeboxRT(device, index);
    if (cmVebox)
    {
        result = cmVebox->Initialize();
        if( result != CM_SUCCESS )
        {
            CmVeboxRT::Destroy(cmVebox);
        }
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to create CmVebox due to out of system memory.");
        result = CM_OUT_OF_HOST_MEMORY;
    }
    return result;

}

//*-----------------------------------------------------------------------------
//| Purpose:    Destroy Vebox
//*-----------------------------------------------------------------------------
int32_t CmVeboxRT::Destroy( CmVeboxRT* & cmVebox )
{
    if(cmVebox)
    {
        /*need some work to delete vebox state*/
        delete cmVebox;
        cmVebox = nullptr;
    }
    return CM_SUCCESS;
}

CmVeboxRT::CmVeboxRT( CmDeviceRT* device, uint32_t index ):
            m_device( device ),
            m_maxSurfaceIndex(VEBOX_MAX_SURFACE_COUNT),
            m_paramBuffer( nullptr ),
            m_indexInVeboxArray( index )
{
    MOS_ZeroMemory(&m_veboxState, sizeof(m_veboxState));
    MOS_ZeroMemory(&m_surface2D, sizeof(m_surface2D));
    MOS_ZeroMemory(&m_surfaceCtrlBits, sizeof(m_surfaceCtrlBits));
}

CmVeboxRT::~CmVeboxRT( void )
{
}

int32_t CmVeboxRT::Initialize()
{

    for (int32_t i = 0; i < VEBOX_MAX_SURFACE_COUNT; i++)
    {
        m_surface2D[i] = nullptr;
        m_surfaceCtrlBits[i] = 0;

    }

    return CM_SUCCESS;
}

CM_RT_API int32_t CmVeboxRT::SetParam(CmBufferUP *paramBuffer)
{
    m_paramBuffer = paramBuffer;
    return CM_SUCCESS;
}
CM_RT_API int32_t CmVeboxRT::SetState(CM_VEBOX_STATE& veboxState)
{

    m_veboxState = veboxState;
    return CM_SUCCESS;

}

int32_t CmVeboxRT::SetSurfaceInternal(VEBOX_SURF_USAGE surfUsage, CmSurface2D* surface)
{
    if( (uint32_t)surfUsage <  m_maxSurfaceIndex)
    {
        m_surface2D[surfUsage] = static_cast<CmSurface2DRT *>(surface);
        return CM_SUCCESS;
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to set the internal surface.");
        return CM_FAILURE;
    }
}

int32_t CmVeboxRT::SetSurfaceControlBitsInternal(VEBOX_SURF_USAGE surfUsage, const uint16_t ctrlBits)
{

    if( (uint32_t)surfUsage < VEBOX_SURFACE_NUMBER )
    {
        m_surfaceCtrlBits[surfUsage] = ctrlBits;
        return CM_SUCCESS;
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to set the internal surface control bits.");
        return CM_FAILURE;
    }
}

CM_RT_API int32_t CmVeboxRT::SetCurFrameInputSurface( CmSurface2D * surface )
{
    return SetSurfaceInternal(VEBOX_CURRENT_FRAME_INPUT_SURF, surface);
}

CM_RT_API int32_t CmVeboxRT::SetCurFrameInputSurfaceControlBits( const uint16_t ctrlBits )
{
    return SetSurfaceControlBitsInternal(VEBOX_CURRENT_FRAME_INPUT_SURF, ctrlBits);
}

CM_RT_API int32_t CmVeboxRT::SetPrevFrameInputSurface( CmSurface2D * surface )
{
    return SetSurfaceInternal(VEBOX_PREVIOUS_FRAME_INPUT_SURF, surface);
}

CM_RT_API int32_t CmVeboxRT::SetPrevFrameInputSurfaceControlBits( const uint16_t ctrlBits )
{
    return SetSurfaceControlBitsInternal(VEBOX_PREVIOUS_FRAME_INPUT_SURF, ctrlBits);
}

CM_RT_API int32_t CmVeboxRT::SetSTMMInputSurface( CmSurface2D* surface )
{
    return SetSurfaceInternal(VEBOX_STMM_INPUT_SURF, surface);
}

CM_RT_API int32_t CmVeboxRT::SetSTMMInputSurfaceControlBits( const uint16_t ctrlBits )
{
    return SetSurfaceControlBitsInternal(VEBOX_STMM_INPUT_SURF, ctrlBits);
}

CM_RT_API int32_t CmVeboxRT::SetSTMMOutputSurface( CmSurface2D* surface )
{
    return SetSurfaceInternal(VEBOX_STMM_OUTPUT_SURF, surface);
}

CM_RT_API int32_t CmVeboxRT::SetSTMMOutputSurfaceControlBits( const uint16_t ctrlBits )
{
    return SetSurfaceControlBitsInternal(VEBOX_STMM_OUTPUT_SURF, ctrlBits);
}

CM_RT_API int32_t CmVeboxRT::SetDenoisedCurFrameOutputSurface( CmSurface2D* surface )
{
    return SetSurfaceInternal(VEBOX_DN_CURRENT_FRAME_OUTPUT_SURF, surface);
}

CM_RT_API int32_t CmVeboxRT::SetDenoisedCurOutputSurfaceControlBits( const uint16_t ctrlBits )
{
    return SetSurfaceControlBitsInternal(VEBOX_DN_CURRENT_FRAME_OUTPUT_SURF, ctrlBits);
}

CM_RT_API int32_t CmVeboxRT::SetCurFrameOutputSurface( CmSurface2D* surface )
{
    int32_t ret = SetSurfaceInternal(VEBOX_CURRENT_FRAME_OUTPUT_SURF, surface);
    CmSurface2DRT* surf2D = static_cast<CmSurface2DRT *>(surface);
    if (m_surface2D[VEBOX_LACE_ACE_RGB_HISTOGRAM_OUTPUT_SURF] == nullptr)
    {
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t pixelsize = 0;
        CM_SURFACE_FORMAT format = CM_SURFACE_FORMAT_INVALID;

        if (ret == CM_SUCCESS)
        {
            ret = surf2D->GetSurfaceDesc(width, height, format, pixelsize);
        }
        if (ret == CM_SUCCESS)
        {
            CmSurface2D *surface2DBase = nullptr;
            ret = m_device->CreateSurface2D(width, height, format, surface2DBase); // allocate the histogram surface if CmIECP Enabled
            if (surface2DBase != nullptr)
            {
                m_surface2D[VEBOX_LACE_ACE_RGB_HISTOGRAM_OUTPUT_SURF] = static_cast<CmSurface2DRT *>(surface2DBase);
                m_surfaceCtrlBits[VEBOX_LACE_ACE_RGB_HISTOGRAM_OUTPUT_SURF] = m_surfaceCtrlBits[VEBOX_CURRENT_FRAME_OUTPUT_SURF];
            }
        }
    }
    return ret;
}

CM_RT_API int32_t CmVeboxRT::SetCurFrameOutputSurfaceControlBits( const uint16_t ctrlBits )
{
    return SetSurfaceControlBitsInternal(VEBOX_CURRENT_FRAME_OUTPUT_SURF, ctrlBits);
}

CM_RT_API int32_t CmVeboxRT::SetPrevFrameOutputSurface( CmSurface2D* surface )
{
    return SetSurfaceInternal(VEBOX_PREVIOUS_FRAME_OUTPUT_SURF, surface);
}

CM_RT_API int32_t CmVeboxRT::SetPrevFrameOutputSurfaceControlBits( const uint16_t ctrlBits )
{
    return SetSurfaceControlBitsInternal(VEBOX_PREVIOUS_FRAME_OUTPUT_SURF, ctrlBits);
}

CM_RT_API int32_t CmVeboxRT::SetStatisticsOutputSurface( CmSurface2D* surface )
{
    return SetSurfaceInternal(VEBOX_STATISTICS_OUTPUT_SURF, surface);
}

CM_RT_API int32_t CmVeboxRT::SetStatisticsOutputSurfaceControlBits( const uint16_t ctrlBits )
{
    return SetSurfaceControlBitsInternal(VEBOX_STATISTICS_OUTPUT_SURF, ctrlBits);
}

int32_t CmVeboxRT::GetSurface(uint32_t surfUsage, CmSurface2DRT*& surface)
{
    int hr = CM_SUCCESS;

    if (surfUsage < VEBOX_SURFACE_NUMBER)
    {
        surface = m_surface2D[surfUsage];
    }
    else
    {
        surface = nullptr;
        hr = CM_FAILURE;
    }

    return hr;
}

uint32_t CmVeboxRT::GetIndexInVeboxArray()
{
    return m_indexInVeboxArray;
}

CM_VEBOX_STATE  CmVeboxRT::GetState()
{
    return m_veboxState;
}

CmBufferUP * CmVeboxRT::GetParam( )
{
    return m_paramBuffer;
}

uint16_t CmVeboxRT::GetSurfaceControlBits(uint32_t usage)
{
    if (usage < VEBOX_MAX_SURFACE_COUNT)
        return m_surfaceCtrlBits[usage];
    else
        return CM_FAILURE;
}
}
