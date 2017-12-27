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
//! \file      cm_vebox_data.cpp  
//! \brief     Contains Class CmVeboxData definitions  
//!

#include "cm_vebox_data.h"

#include "cm_mem.h"

namespace CMRT_UMD
{
//*-----------------------------------------------------------------------------
//| Purpose:    Create Vebox Data 
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmVeboxData::Create( uint8_t *pStateData, uint8_t *pSurfaceData, CmVeboxData*& pVeboxData )
{
    int32_t result = CM_SUCCESS;
    pVeboxData = new (std::nothrow) CmVeboxData( pStateData, pSurfaceData );
    if( pVeboxData )
    {
        pVeboxData->Acquire();
        result = pVeboxData->Initialize();
        if( result != CM_SUCCESS )
        {
            CmVeboxData::Destroy( pVeboxData );
        }
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to create CmVeboxData due to out of system memory.");
        result = CM_OUT_OF_HOST_MEMORY;
    }
    return result;

}

//*-----------------------------------------------------------------------------
//| Purpose:    Destroy CM Vebox Data
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmVeboxData::Destroy( CmVeboxData* &pVeboxData )
{
    if (pVeboxData)
    {
        pVeboxData->SafeRelease();
        pVeboxData = nullptr;
    }
    
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    CM Vebox Data constructor
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmVeboxData::CmVeboxData( uint8_t *pStateData, uint8_t *pSurfaceData ):
    m_StateDataSize( 0 ),
    m_pStateData( pStateData ),
    m_SurfaceDataSize( 0 ),
    m_pSurfaceData( pSurfaceData ),
    m_RefCount(0)
{
}

//*-----------------------------------------------------------------------------
//| Purpose:    CM Vebox Data Destructor
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmVeboxData::~CmVeboxData( void )
{
    MosSafeDeleteArray( m_pStateData );
    MosSafeDeleteArray( m_pSurfaceData );
}

//*-----------------------------------------------------------------------------
//| Purpose:    Do nothing in initialization
//| Returns:    CM_SUCCESS
//*-----------------------------------------------------------------------------
int32_t CmVeboxData::Initialize( void )
{
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get Vebox Data pointer
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmVeboxData::GetData( uint8_t*& pStateData, uint8_t*& pSurfaceData )
{
    pStateData   = m_pStateData;
    pSurfaceData = m_pSurfaceData;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    SET Vebox Data size
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmVeboxData::SetVeboxDataSize(uint32_t uiStateDataSize, uint32_t uiSurfaceDataSize)
{
    m_StateDataSize   = uiStateDataSize;
    m_SurfaceDataSize = uiSurfaceDataSize;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get Vebox Data size
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmVeboxData::GetVeboxDataSize(uint32_t& uiStateDataSize, uint32_t& uiSurfaceDataSize)
{
    uiStateDataSize   = m_StateDataSize;
    uiSurfaceDataSize = m_SurfaceDataSize;

    return CM_SUCCESS;
}


//*-----------------------------------------------------------------------------
//| Purpose:    Increase Reference count 
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmVeboxData::Acquire( void )
{
    ++m_RefCount;
    return m_RefCount;
}

//*-----------------------------------------------------------------------------
//| Purpose:    De of Cm Event
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmVeboxData::SafeRelease( )
{
    --m_RefCount;
    if( m_RefCount == 0 )
    {
        delete this;
        return 0;
    }
    else
    {
        return m_RefCount;
    }
}
}  // namespace
