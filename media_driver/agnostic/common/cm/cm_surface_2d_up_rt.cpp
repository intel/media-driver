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
//! \file      cm_surface_2d_up_rt.cpp
//! \brief     Contains Class CmSurface2DUPRT implementations.
//!

#include "cm_surface_2d_up_rt.h"

#include "cm_device_rt.h"
#include "cm_surface_manager.h"
#include "cm_hal.h"

namespace CMRT_UMD
{
//*-----------------------------------------------------------------------------
//| Purpose:    Create Surface 2D Up
//| Arguments :
//|               index             [in]     index in runtime Surface2D table
//|               handle            [in]     index in driver's surface2D table
//|               width             [in]     width of the  CmSurface2DUP
//|               height            [in]     height of the CmSurface2DUP
//|               pitch             [in]     pitch of the CmSurface2DUP
//|               format            [in]     format to CmSurface2DUP
//|               sysMem           [in]     user provided buffer to CmSurface2DUP
//|               isCmCreated       [out]    ture,if the surface created by CM;
//|                                          false,if the surface created externally
//|               surfaceManager   [out]    Pointer to CmSurfaceManager
//|               surface          [out]    Reference to the Pointer to CmSurface2DUP

//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmSurface2DUPRT::Create( uint32_t index, uint32_t handle, uint32_t width, uint32_t height, CM_SURFACE_FORMAT format, void  *sysMem, CmSurfaceManager* surfaceManager, CmSurface2DUPRT* &surface )
{
    int32_t result = CM_SUCCESS;

    surface = new (std::nothrow) CmSurface2DUPRT( handle, width, height, format, sysMem, surfaceManager );
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
        CM_ASSERTMESSAGE("Error: Failed to create CmSurface2DUP due to out of system memory.")
        result = CM_OUT_OF_HOST_MEMORY;
    }

    return result;

}

//*-----------------------------------------------------------------------------
//| Purpose:    Constructor of Surface 2DUP
//| Returns:    None.
//*-----------------------------------------------------------------------------
CmSurface2DUPRT::CmSurface2DUPRT( uint32_t handle, uint32_t width, uint32_t height, CM_SURFACE_FORMAT format, void  *sysMem, CmSurfaceManager* surfaceManager ):
    CmSurface( surfaceManager,true ),
    m_handle( handle ),
    m_width( width ),
    m_height( height ),
    m_format ( format ),
    m_frameType(CM_FRAME),
    m_sysMem(sysMem)
{
    CmSurface::SetMemoryObjectControl(MEMORY_OBJECT_CONTROL_UNKNOW, CM_USE_PTE, 0);
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destructor of Surface 2DUP
//| Returns:    None.
//*-----------------------------------------------------------------------------
CmSurface2DUPRT::~CmSurface2DUPRT( void )
{

}

//*-----------------------------------------------------------------------------
//| Purpose:    Initialize the Surface 2D up
//| Returns:    None.
//*-----------------------------------------------------------------------------
int32_t CmSurface2DUPRT::Initialize( uint32_t index )
{
    return CmSurface::Initialize( index );

}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the handle of Surface 2D up
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmSurface2DUPRT::GetHandle( uint32_t& handle)
{
    handle = m_handle;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get Surface Index
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmSurface2DUPRT::GetIndex(SurfaceIndex*& index)
{
    index = m_index;
    return CM_SUCCESS;
}

CM_RT_API int32_t CmSurface2DUPRT::SetProperty(CM_FRAME_TYPE frameType)
{
    m_frameType = frameType;
    m_surfaceMgr->UpdateSurface2DTableFrameType(m_handle, frameType);

    return CM_SUCCESS;
}

int32_t CmSurface2DUPRT::SetMemoryObjectControl( MEMORY_OBJECT_CONTROL memCtrl, MEMORY_TYPE memType, uint32_t age)
{
    CM_RETURN_CODE  hr = CM_SUCCESS;
    uint16_t mocs = 0;
    CmSurface::SetMemoryObjectControl( memCtrl, memType, age );

    CmDeviceRT *cmDevice = nullptr;
    m_surfaceMgr->GetCmDevice(cmDevice);
    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)cmDevice->GetAccelData();
    CMCHK_NULL(cmData);

    mocs = (m_memObjCtrl.mem_ctrl << 8) | (m_memObjCtrl.mem_type<<4) | m_memObjCtrl.age;

    CHK_MOSSTATUS_RETURN_CMERROR(cmData->cmHalState->pfnSetSurfaceMOCS(cmData->cmHalState, m_handle, mocs, ARG_KIND_SURFACE_2D_UP));

finish:
    return hr;
}

CM_RT_API int32_t CmSurface2DUPRT::SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL memCtrl)
{
    return SetMemoryObjectControl(memCtrl, CM_USE_PTE, 0);
}

//*-------------------------------------------------------------------------------------------
//| Purpose:    Get the description of surface 2d, width,height,format and the size PER pixel.
//| Arguments :
//|               width         [out]     Reference to  width of surface
//|               height        [out]     Reference to  height of surface
//|               format        [out]     Reference to  format of surface
//|               sizeperpixel  [out]     Reference to  the pixel's size in bytes
//|
//| Returns:    Result of the operation.
//*-------------------------------------------------------------------------------------------
CM_RT_API int32_t CmSurface2DUPRT::GetSurfaceDesc(uint32_t &width, uint32_t &height, CM_SURFACE_FORMAT &format,uint32_t &sizeperpixel)
{
    int ret = CM_SUCCESS;
    uint32_t updatedHeight = 0 ;

    width  = m_width;
    height = m_height;
    format = m_format;

    // Get size per pixel
    ret = m_surfaceMgr->GetPixelBytesAndHeight(width,  height,  format,  sizeperpixel, updatedHeight);

    return ret;
}

void CmSurface2DUPRT::Log(std::ostringstream &oss)
{
#if CM_LOG_ON
    oss << " Surface2D UP Info "
        << " Width:" << m_width
        << " Height:"<< m_height
        << " Format:" << GetFormatString(m_format)
        << " Handle:" << m_handle
        << " SurfaceIndex:" << m_index->get_data()
        << " Sys Address:" << m_sysMem
        << " IsCmCreated:"<<m_isCmCreated
        << std::endl;
#endif
}

void CmSurface2DUPRT::DumpContent(uint32_t kernelNumber, int32_t taskId, uint32_t argIndex)
{
#if MDF_SURFACE_CONTENT_DUMP
    std::ostringstream outputFileName;
    static uint32_t surface2DUPDumpNumber = 0;
    outputFileName << "t_" << taskId
        << "_k_" << kernelNumber
        << "_argi_" << argIndex
        << "_surf2d_surfi_"<< m_index->get_data()
        << "_w_" << m_width
        << "_h_" << m_height
        << "_f_" << GetFormatString(m_format)
        << "_" << surface2DUPDumpNumber;

    std::ofstream outputFileStream;
    outputFileStream.open(outputFileName.str().c_str(), std::ofstream::binary);

    uint32_t        sizePerPixel = 0;
    uint32_t        updatedHeight = 0;
    uint32_t        surfaceSize = 0;
    m_surfaceMgr->GetPixelBytesAndHeight(m_width, m_height, m_format, sizePerPixel, updatedHeight);
    surfaceSize = m_width*sizePerPixel*updatedHeight;
    outputFileStream.write((char *)m_sysMem, surfaceSize);
    outputFileStream.close();
    surface2DUPDumpNumber++;
#endif
}
}  // namespace
