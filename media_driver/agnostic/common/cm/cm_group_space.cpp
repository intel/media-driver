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
//! \file      cm_group_space.cpp  
//! \brief     Contains Class CmThreadGroupSpace definitions  
//!

#include "cm_group_space.h"
#include "cm_device_rt.h"
#include "cm_kernel.h"
#include "cm_mem.h"

namespace CMRT_UMD
{
int32_t CmThreadGroupSpace::Create(CmDeviceRT* pDevice, uint32_t index, uint32_t thrdSpaceWidth, uint32_t thrdSpaceHeight, uint32_t thrdSpaceDepth,  uint32_t grpSpaceWidth, uint32_t grpSpaceHeight, uint32_t grpSpaceDepth, CmThreadGroupSpace* & pTGS)
{
    CM_HAL_MAX_VALUES* pHalMaxValues = nullptr;
    CM_HAL_MAX_VALUES_EX* pHalMaxValuesEx = nullptr;
    pDevice->GetHalMaxValues(pHalMaxValues, pHalMaxValuesEx);
    if( (thrdSpaceWidth == 0) || (thrdSpaceHeight == 0) || (thrdSpaceDepth == 0) || (grpSpaceWidth == 0)
		|| (grpSpaceHeight == 0) || (grpSpaceDepth==0) 
		|| (thrdSpaceHeight > MAX_THREAD_SPACE_HEIGHT_PERGROUP)
		|| (thrdSpaceWidth > MAX_THREAD_SPACE_WIDTH_PERGROUP)
		|| (thrdSpaceDepth > MAX_THREAD_SPACE_DEPTH_PERGROUP)
        || (thrdSpaceHeight * thrdSpaceWidth  * thrdSpaceDepth > pHalMaxValuesEx->iMaxUserThreadsPerThreadGroup))
    {
        CM_ASSERTMESSAGE("Error: Exceed thread group size limitation.");
        return CM_INVALID_THREAD_GROUP_SPACE;
    }

    int32_t result = CM_SUCCESS;
    pTGS = new (std::nothrow) CmThreadGroupSpace(pDevice, index, thrdSpaceWidth, thrdSpaceHeight, thrdSpaceDepth, grpSpaceWidth, grpSpaceHeight, grpSpaceDepth);  //YiGe
    if( pTGS )
    {
        result = pTGS->Initialize( );
        if( result != CM_SUCCESS )
        {
            CmThreadGroupSpace::Destroy( pTGS);
        }
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to create CmThreadGroupSpace due to out of system memory.");
        result = CM_OUT_OF_HOST_MEMORY;
    }
    return result;
}

int32_t CmThreadGroupSpace::Destroy( CmThreadGroupSpace* &pTGS )
{
    CmSafeDelete( pTGS );
    pTGS = nullptr;
    return CM_SUCCESS;
}

int32_t CmThreadGroupSpace::GetThreadGroupSpaceSize(uint32_t & thrdSpaceWidth, 
                                                uint32_t & thrdSpaceHeight, 
                                                uint32_t & thrdSpaceDepth,
                                                uint32_t & grpSpaceWidth, 
                                                uint32_t & grpSpaceHeight,
                                                uint32_t & grpSpaceDepth) const
{
    thrdSpaceWidth = m_threadSpaceWidth;
    thrdSpaceHeight = m_threadSpaceHeight;
    thrdSpaceDepth = m_threadSpaceDepth;
    grpSpaceWidth = m_groupSpaceWidth;
    grpSpaceHeight = m_groupSpaceHeight;
    grpSpaceDepth = m_groupSpaceDepth;   

    return CM_SUCCESS;
}

CmThreadGroupSpace::CmThreadGroupSpace( CmDeviceRT* pCmDev, 
                                        uint32_t index,
                                        uint32_t thrdSpaceWidth, 
                                        uint32_t thrdSpaceHeight,
                                        uint32_t thrdSpaceDepth,
                                        uint32_t grpSpaceWidth, 
                                        uint32_t grpSpaceHeight,
                                        uint32_t grpSpaceDepth) :
                                        m_pCmDev(pCmDev),
                                        m_threadSpaceWidth(thrdSpaceWidth), 
                                        m_threadSpaceHeight(thrdSpaceHeight),
                                        m_threadSpaceDepth(thrdSpaceDepth),
                                        m_groupSpaceWidth(grpSpaceWidth), 
                                        m_groupSpaceHeight(grpSpaceHeight),
                                        m_groupSpaceDepth(grpSpaceDepth),
                                        m_IndexInTGSArray(index)
{
}

CmThreadGroupSpace::CmThreadGroupSpace(CmDeviceRT* pCmDev,
    uint32_t index,
    uint32_t thrdSpaceWidth,
    uint32_t thrdSpaceHeight,
    uint32_t grpSpaceWidth,
    uint32_t grpSpaceHeight) :
    m_pCmDev(pCmDev),
    m_threadSpaceWidth(thrdSpaceWidth),
    m_threadSpaceHeight(thrdSpaceHeight),
    m_groupSpaceWidth(grpSpaceWidth),
    m_groupSpaceHeight(grpSpaceHeight),
    m_IndexInTGSArray(index)
{
}
CmThreadGroupSpace::~CmThreadGroupSpace( void )
{
}

int32_t CmThreadGroupSpace::Initialize( void )
{
    return CM_SUCCESS;
}

uint32_t CmThreadGroupSpace::GetIndexInTGsArray( void )
{
    return m_IndexInTGSArray;
}

#if CM_LOG_ON
std::string CmThreadGroupSpace::Log()
{
    std::ostringstream oss;

    oss << "Thread Group Space "
        << " TsWidth :"<< m_threadSpaceWidth
        << " TsHeight :" << m_threadSpaceHeight
        << " TsDepth :" << m_threadSpaceDepth
        << " GroupSpaceWidth :" <<m_groupSpaceWidth
        << " GroupSpaceHeight :" <<m_groupSpaceHeight
        << " GroupSpaceDepth :" << m_groupSpaceDepth
        << std::endl;

    return oss.str();

}
#endif
}
