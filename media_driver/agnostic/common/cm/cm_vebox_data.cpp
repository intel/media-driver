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
int32_t CmVeboxData::Create( uint8_t *stateData, uint8_t *surfaceData, CmVeboxData*& veboxData )
{
    int32_t result = CM_SUCCESS;
    veboxData = new (std::nothrow) CmVeboxData( stateData, surfaceData );
    if( veboxData )
    {
        veboxData->Acquire();
        result = veboxData->Initialize();
        if( result != CM_SUCCESS )
        {
            CmVeboxData::Destroy( veboxData );
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
int32_t CmVeboxData::Destroy( CmVeboxData* &veboxData )
{
    if (veboxData)
    {
        veboxData->SafeRelease();
        veboxData = nullptr;
    }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    CM Vebox Data constructor
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmVeboxData::CmVeboxData( uint8_t *stateData, uint8_t *surfaceData ):
    m_stateDataSize( 0 ),
    m_stateData( stateData ),
    m_surfaceDataSize( 0 ),
    m_surfaceData( surfaceData ),
    m_refCount(0)
{
}

//*-----------------------------------------------------------------------------
//| Purpose:    CM Vebox Data Destructor
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmVeboxData::~CmVeboxData( void )
{
    MosSafeDeleteArray( m_stateData );
    MosSafeDeleteArray( m_surfaceData );
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
int32_t CmVeboxData::GetData( uint8_t*& stateData, uint8_t*& surfaceData )
{
    stateData   = m_stateData;
    surfaceData = m_surfaceData;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    SET Vebox Data size
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmVeboxData::SetVeboxDataSize(uint32_t stateDataSize, uint32_t surfaceDataSize)
{
    m_stateDataSize   = stateDataSize;
    m_surfaceDataSize = surfaceDataSize;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get Vebox Data size
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmVeboxData::GetVeboxDataSize(uint32_t& stateDataSize, uint32_t& surfaceDataSize)
{
    stateDataSize   = m_stateDataSize;
    surfaceDataSize = m_surfaceDataSize;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Increase Reference count
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmVeboxData::Acquire( void )
{
    ++m_refCount;
    return m_refCount;
}

//*-----------------------------------------------------------------------------
//| Purpose:    De of Cm Event
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmVeboxData::SafeRelease( )
{
    --m_refCount;
    if( m_refCount == 0 )
    {
        delete this;
        return 0;
    }
    else
    {
        return m_refCount;
    }
}
}  // namespace
