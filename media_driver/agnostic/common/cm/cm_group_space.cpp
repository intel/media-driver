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
int32_t CmThreadGroupSpace::Create(CmDeviceRT* device, uint32_t index, uint32_t threadSpaceWidth, uint32_t threadSpaceHeight, uint32_t threadSpaceDepth,  uint32_t groupSpaceWidth, uint32_t groupSpaceHeight, uint32_t groupSpaceDepth, CmThreadGroupSpace* & threadGroupSpace)
{
    CM_HAL_MAX_VALUES* halMaxValues = nullptr;
    CM_HAL_MAX_VALUES_EX* halMaxValuesEx = nullptr;
    device->GetHalMaxValues(halMaxValues, halMaxValuesEx);
    if( (threadSpaceWidth == 0) || (threadSpaceHeight == 0) || (threadSpaceDepth == 0) || (groupSpaceWidth == 0)
        || (groupSpaceHeight == 0) || (groupSpaceDepth==0)
        || (threadSpaceHeight > MAX_THREAD_SPACE_HEIGHT_PERGROUP)
        || (threadSpaceWidth > MAX_THREAD_SPACE_WIDTH_PERGROUP)
        || (threadSpaceDepth > MAX_THREAD_SPACE_DEPTH_PERGROUP)
        || (threadSpaceHeight * threadSpaceWidth  * threadSpaceDepth > halMaxValuesEx->maxUserThreadsPerThreadGroup))
    {
        CM_ASSERTMESSAGE("Error: Exceed thread group size limitation.");
        return CM_INVALID_THREAD_GROUP_SPACE;
    }

    int32_t result = CM_SUCCESS;
    threadGroupSpace = new (std::nothrow) CmThreadGroupSpace(device, index, threadSpaceWidth, threadSpaceHeight, threadSpaceDepth, groupSpaceWidth, groupSpaceHeight, groupSpaceDepth);
    if( threadGroupSpace )
    {
        result = threadGroupSpace->Initialize( );
        if( result != CM_SUCCESS )
        {
            CmThreadGroupSpace::Destroy( threadGroupSpace);
        }
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to create CmThreadGroupSpace due to out of system memory.");
        result = CM_OUT_OF_HOST_MEMORY;
    }
    return result;
}

int32_t CmThreadGroupSpace::Destroy( CmThreadGroupSpace* &threadGroupSpace )
{
    CmSafeDelete( threadGroupSpace );
    threadGroupSpace = nullptr;
    return CM_SUCCESS;
}

int32_t CmThreadGroupSpace::GetThreadGroupSpaceSize(uint32_t & threadSpaceWidth,
                                                uint32_t & threadSpaceHeight,
                                                uint32_t & threadSpaceDepth,
                                                uint32_t & groupSpaceWidth,
                                                uint32_t & groupSpaceHeight,
                                                uint32_t & groupSpaceDepth) const
{
    threadSpaceWidth = m_threadSpaceWidth;
    threadSpaceHeight = m_threadSpaceHeight;
    threadSpaceDepth = m_threadSpaceDepth;
    groupSpaceWidth = m_groupSpaceWidth;
    groupSpaceHeight = m_groupSpaceHeight;
    groupSpaceDepth = m_groupSpaceDepth;

    return CM_SUCCESS;
}

CmThreadGroupSpace::CmThreadGroupSpace( CmDeviceRT* cmDev,
                                        uint32_t index,
                                        uint32_t threadSpaceWidth,
                                        uint32_t threadSpaceHeight,
                                        uint32_t threadSpaceDepth,
                                        uint32_t groupSpaceWidth,
                                        uint32_t groupSpaceHeight,
                                        uint32_t groupSpaceDepth) :
                                        m_cmDev(cmDev),
                                        m_threadSpaceWidth(threadSpaceWidth),
                                        m_threadSpaceHeight(threadSpaceHeight),
                                        m_threadSpaceDepth(threadSpaceDepth),
                                        m_groupSpaceWidth(groupSpaceWidth),
                                        m_groupSpaceHeight(groupSpaceHeight),
                                        m_groupSpaceDepth(groupSpaceDepth),
                                        m_indexInThreadGroupSpaceArray(index)
{
}

CmThreadGroupSpace::CmThreadGroupSpace(CmDeviceRT* cmDev,
    uint32_t index,
    uint32_t threadSpaceWidth,
    uint32_t threadSpaceHeight,
    uint32_t groupSpaceWidth,
    uint32_t groupSpaceHeight) :
    m_cmDev(cmDev),
    m_threadSpaceWidth(threadSpaceWidth),
    m_threadSpaceHeight(threadSpaceHeight),
    m_groupSpaceWidth(groupSpaceWidth),
    m_groupSpaceHeight(groupSpaceHeight),
    m_indexInThreadGroupSpaceArray(index)
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
    return m_indexInThreadGroupSpaceArray;
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
