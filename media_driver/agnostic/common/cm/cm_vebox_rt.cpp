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
int32_t CmVeboxRT::Create( CmDeviceRT* pCmDev, uint32_t index, CmVeboxRT* & pCmVebox )
{
    int32_t result = CM_SUCCESS;

    pCmVebox = new (std::nothrow) CmVeboxRT(pCmDev, index);
    if (pCmVebox)
    {
        result = pCmVebox->Initialize();
        if( result != CM_SUCCESS )
        {
            CmVeboxRT::Destroy(pCmVebox);
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
int32_t CmVeboxRT::Destroy( CmVeboxRT* & pCmVebox )
{
    if(pCmVebox)
    {
        /*need some work to delete vebox state*/
        delete pCmVebox;
        pCmVebox = nullptr;
    }
    return CM_SUCCESS;
}

CmVeboxRT::CmVeboxRT( CmDeviceRT* pCmDev, uint32_t index ):
            m_pCmDev( pCmDev ),
            m_IndexInVeboxArray( index ),
            m_MaxSurfaceIndex(VEBOX_MAX_SURFACE_COUNT)
{
}

CmVeboxRT::~CmVeboxRT( void )
{
}

int32_t CmVeboxRT::Initialize()
{

    for (int32_t i = 0; i < VEBOX_MAX_SURFACE_COUNT; i++)
    {
        m_pSurface2D[i] = nullptr;
        m_wSurfaceCtrlBits[i] = 0;

    }

    return CM_SUCCESS;
}

CM_RT_API int32_t CmVeboxRT::SetParam(CmBufferUP *pParamBuffer)
{
    m_ParamBuffer = pParamBuffer;
    return CM_SUCCESS;
}
CM_RT_API int32_t CmVeboxRT::SetState(CM_VEBOX_STATE& VeBoxState)
{

    m_VeboxState = VeBoxState;
    return CM_SUCCESS;

}

int32_t CmVeboxRT::SetSurfaceInternal(VEBOX_SURF_USAGE surfUsage, CmSurface2D* pSurf)
{
    if( (uint32_t)surfUsage <  m_MaxSurfaceIndex)
    {
        m_pSurface2D[surfUsage] = static_cast<CmSurface2DRT *>(pSurf);
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
        m_wSurfaceCtrlBits[surfUsage] = ctrlBits;
        return CM_SUCCESS;
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to set the internal surface control bits.");
        return CM_FAILURE;
    }
}

CM_RT_API int32_t CmVeboxRT::SetCurFrameInputSurface( CmSurface2D * pSurf )
{
    return SetSurfaceInternal(VEBOX_CURRENT_FRAME_INPUT_SURF, pSurf);
}

CM_RT_API int32_t CmVeboxRT::SetCurFrameInputSurfaceControlBits( const uint16_t ctrlBits )
{
    return SetSurfaceControlBitsInternal(VEBOX_CURRENT_FRAME_INPUT_SURF, ctrlBits);
}

CM_RT_API int32_t CmVeboxRT::SetPrevFrameInputSurface( CmSurface2D * pSurf )
{
    return SetSurfaceInternal(VEBOX_PREVIOUS_FRAME_INPUT_SURF, pSurf);
}

CM_RT_API int32_t CmVeboxRT::SetPrevFrameInputSurfaceControlBits( const uint16_t ctrlBits )
{
    return SetSurfaceControlBitsInternal(VEBOX_PREVIOUS_FRAME_INPUT_SURF, ctrlBits);
}

CM_RT_API int32_t CmVeboxRT::SetSTMMInputSurface( CmSurface2D* pSurf )
{
    return SetSurfaceInternal(VEBOX_STMM_INPUT_SURF, pSurf);
}

CM_RT_API int32_t CmVeboxRT::SetSTMMInputSurfaceControlBits( const uint16_t ctrlBits )
{
    return SetSurfaceControlBitsInternal(VEBOX_STMM_INPUT_SURF, ctrlBits);
}

CM_RT_API int32_t CmVeboxRT::SetSTMMOutputSurface( CmSurface2D* pSurf )
{
    return SetSurfaceInternal(VEBOX_STMM_OUTPUT_SURF, pSurf);
}

CM_RT_API int32_t CmVeboxRT::SetSTMMOutputSurfaceControlBits( const uint16_t ctrlBits )
{
    return SetSurfaceControlBitsInternal(VEBOX_STMM_OUTPUT_SURF, ctrlBits);
}

CM_RT_API int32_t CmVeboxRT::SetDenoisedCurFrameOutputSurface( CmSurface2D* pSurf )
{
    return SetSurfaceInternal(VEBOX_DN_CURRENT_FRAME_OUTPUT_SURF, pSurf);
}

CM_RT_API int32_t CmVeboxRT::SetDenoisedCurOutputSurfaceControlBits( const uint16_t ctrlBits )
{
    return SetSurfaceControlBitsInternal(VEBOX_DN_CURRENT_FRAME_OUTPUT_SURF, ctrlBits);
}

CM_RT_API int32_t CmVeboxRT::SetCurFrameOutputSurface( CmSurface2D* pSurf )
{
    int32_t ret = SetSurfaceInternal(VEBOX_CURRENT_FRAME_OUTPUT_SURF, pSurf);
    CmSurface2DRT* pSurf2D = static_cast<CmSurface2DRT *>(pSurf);
    uint32_t width;
    uint32_t height;
    CM_SURFACE_FORMAT format;
    uint32_t pixelsize;
    if (m_pSurface2D[VEBOX_LACE_ACE_RGB_HISTOGRAM_OUTPUT_SURF] == nullptr)
    {
        if (ret == CM_SUCCESS)
        {
            ret = pSurf2D->GetSurfaceDesc(width, height, format, pixelsize);
        }
        if (ret == CM_SUCCESS)
        {
            CmSurface2D *pSurface2DBase = nullptr;
            ret = m_pCmDev->CreateSurface2D(width, height, format, pSurface2DBase); // allocate the histogram surface if CmIECP Enabled
            if (pSurface2DBase != nullptr)
            {
                m_pSurface2D[VEBOX_LACE_ACE_RGB_HISTOGRAM_OUTPUT_SURF] = static_cast<CmSurface2DRT *>(pSurface2DBase);
            }
        }
    }
    return ret;
}

CM_RT_API int32_t CmVeboxRT::SetCurFrameOutputSurfaceControlBits( const uint16_t ctrlBits )
{
    return SetSurfaceControlBitsInternal(VEBOX_CURRENT_FRAME_OUTPUT_SURF, ctrlBits);
}

CM_RT_API int32_t CmVeboxRT::SetPrevFrameOutputSurface( CmSurface2D* pSurf )
{
    return SetSurfaceInternal(VEBOX_PREVIOUS_FRAME_OUTPUT_SURF, pSurf);
}

CM_RT_API int32_t CmVeboxRT::SetPrevFrameOutputSurfaceControlBits( const uint16_t ctrlBits )
{
    return SetSurfaceControlBitsInternal(VEBOX_PREVIOUS_FRAME_OUTPUT_SURF, ctrlBits);
}

CM_RT_API int32_t CmVeboxRT::SetStatisticsOutputSurface( CmSurface2D* pSurf )
{
    return SetSurfaceInternal(VEBOX_STATISTICS_OUTPUT_SURF, pSurf);
}

CM_RT_API int32_t CmVeboxRT::SetStatisticsOutputSurfaceControlBits( const uint16_t ctrlBits )
{
    return SetSurfaceControlBitsInternal(VEBOX_STATISTICS_OUTPUT_SURF, ctrlBits);
}

int32_t CmVeboxRT::GetSurface(uint32_t surfUsage, CmSurface2DRT*& pSurf)
{
    int hr = CM_SUCCESS;

    if (surfUsage < VEBOX_SURFACE_NUMBER)
    {
        pSurf = m_pSurface2D[surfUsage];
    }
    else
    {
        pSurf = nullptr;
        hr = CM_FAILURE;
    }

    return hr;
}

uint32_t CmVeboxRT::GetIndexInVeboxArray()
{
    return m_IndexInVeboxArray;
}

CM_VEBOX_STATE  CmVeboxRT::GetState()
{
    return m_VeboxState;
}

CmBufferUP * CmVeboxRT::GetParam( )
{
    return m_ParamBuffer;
}

uint16_t CmVeboxRT::GetSurfaceControlBits(uint32_t usage)
{
    if (usage < VEBOX_MAX_SURFACE_COUNT)
        return m_wSurfaceCtrlBits[usage];
    else
        return CM_FAILURE;
}
}