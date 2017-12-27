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
//! \file      cm_sampler_rt.cpp
//! \brief     Contains CmSamplerRT implementations.
//!

#include "cm_sampler_rt.h"

#include "cm_debug.h"
#include "cm_mem.h"

namespace CMRT_UMD
{
//*-----------------------------------------------------------------------------
//| Purpose:    Create CmSampler
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmSamplerRT::Create( uint32_t index, CmSamplerRT* & pSampler )
{
    int32_t result = CM_SUCCESS;
    pSampler = new (std::nothrow) CmSamplerRT();
    if( pSampler )
    {
        result = pSampler->Initialize( index );
        if( result != CM_SUCCESS )
        {
            CmSamplerRT::Destroy( pSampler );
        }
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to create CmSampler due to out of system memory.")
        result = CM_OUT_OF_HOST_MEMORY;
    }
    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destory CmSampler
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmSamplerRT::Destroy( CmSamplerRT* &pSampler )
{
    CmSafeDelete( pSampler );

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Construtor of CmSamplerRT
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmSamplerRT::CmSamplerRT( void ):
                    m_pIndex( nullptr )
{
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destructor of CmSamplerRT
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmSamplerRT::~CmSamplerRT( void )
{
    if( m_pIndex )
    {
        MOS_Delete(m_pIndex);
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    Initialize  CmSamplerRT and get its index
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmSamplerRT::Initialize( uint32_t index )
{
    // using CM compiler data structures
    m_pIndex = MOS_New(SamplerIndex, index);
    if( m_pIndex )
    {
        return CM_SUCCESS;
    }
    else
    {
        return CM_OUT_OF_HOST_MEMORY;
    }
}

//*-----------------------------------------------------------------------------
//! Each CmSamplerRT is assigned an index when it is created by the CmDevice_RT.
//! CmDevice_RT keep a mapping b/w index and CmSamplerRT.
//! The index is passed to CM kernel function (genx_main) as argument to indicate sampler.
//! INPUT:
//!     Reference to index
//! OUTPUT:
//!     CM_SUCCESS if index is successfully returned
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmSamplerRT::GetIndex( SamplerIndex* & pIndex )
{
    pIndex = m_pIndex;
    return CM_SUCCESS;
}

}  // namespace
