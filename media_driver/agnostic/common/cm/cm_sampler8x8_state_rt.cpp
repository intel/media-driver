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
//! \file      cm_sampler8x8_state_rt.cpp
//! \brief     Contains CmSampler8x8State_RT implementations.
//!

#include "cm_sampler8x8_state_rt.h"

#include "cm_device.h"
#include "cm_debug.h"
#include "cm_mem.h"

namespace CMRT_UMD
{
//*-----------------------------------------------------------------------------
//| Purpose:    Get the Index of CmSampler8x8State_RT
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmSampler8x8State_RT::GetIndex( SamplerIndex* & index )
{
    index=m_index;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create the CmSampler8x8State_RT
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmSampler8x8State_RT::Create( const CM_SAMPLER_8X8_DESCR& sampleState, uint32_t index, CmSampler8x8State_RT* & sampler )
{
    int32_t result = CM_SUCCESS;
    sampler = new (std::nothrow) CmSampler8x8State_RT( sampleState );
    if( sampler )
    {
        result = sampler->Initialize( index );
        if( result != CM_SUCCESS )
        {
            CmSampler8x8State_RT::Destroy( sampler );
        }

    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to create CmSampler8x8State due to out of system memory.")
        result = CM_OUT_OF_HOST_MEMORY;
    }

    return result;

}

//*-----------------------------------------------------------------------------
//| Purpose:    Destroy CmSampler8x8State_RT
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmSampler8x8State_RT::Destroy( CmSampler8x8State_RT* &sampler )
{
    CmSafeDelete( sampler );
    return CM_SUCCESS;

}

//*-----------------------------------------------------------------------------
//| Purpose:    Constructor of CmSampler8x8State_RT
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmSampler8x8State_RT::CmSampler8x8State_RT( const CM_SAMPLER_8X8_DESCR& sampleState ):
                    m_index( nullptr )
{
     CmSafeMemSet( & m_avsState,       0, sizeof(CM_AVS_STATE_MSG));
     CmSafeMemSet( & m_convolveState,  0, sizeof(CM_CONVOLVE_STATE_MSG));
     CmSafeMemSet( & m_miscState,      0, sizeof(CM_MISC_STATE_MSG));

    if(sampleState.stateType == CM_SAMPLER8X8_AVS)
    {
        CmSafeMemCopy( &this->m_avsState,  sampleState.avs, sizeof(CM_AVS_STATE_MSG) );
    } else if(sampleState.stateType == CM_SAMPLER8X8_CONV)
    {
        CmSafeMemCopy( &this->m_convolveState,  sampleState.conv, sizeof(CM_CONVOLVE_STATE_MSG) );
    } else if(sampleState.stateType == CM_SAMPLER8X8_MISC)
    {
        CmSafeMemCopy( &this->m_miscState,  sampleState.misc, sizeof(CM_MISC_STATE_MSG) );
    }  else {
        CM_ASSERTMESSAGE("Error: Invalid sampler8x8 state type.")
    }

    m_stateType = sampleState.stateType;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destructor of CmSampler8x8State_RT
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmSampler8x8State_RT::~CmSampler8x8State_RT( void )
{
    MosSafeDelete(m_index);
}

//*-----------------------------------------------------------------------------
//| Purpose:    Initialize CmSampler8x8State_RT
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmSampler8x8State_RT::Initialize( uint32_t index )
{
    int status = CM_FAILURE;
    // using CM compiler data structure
    m_index = MOS_New(SamplerIndex, index);
    if( m_index )
    {
        status = CM_SUCCESS;
    }
    else
    {
        return CM_OUT_OF_HOST_MEMORY;
    }
    return status;
}
}  // namespace
