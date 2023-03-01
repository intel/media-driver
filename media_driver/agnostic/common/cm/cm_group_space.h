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
//! \file      cm_group_space.h
//! \brief     Contains Class CmThreadGroupSpace definitions 
//!

#ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMGROUPSPACE_H_
#define MEDIADRIVER_AGNOSTIC_COMMON_CM_CMGROUPSPACE_H_

#include "cm_def.h"
#include "cm_log.h"

namespace CMRT_UMD
{
class CmDeviceRT;

class CmThreadGroupSpace
{
public:
    static int32_t Create(CmDeviceRT* device, uint32_t index, uint32_t threadSpaceWidth, uint32_t threadSpaceHeight, uint32_t threadSpaceDepth, uint32_t groupSpaceWidth, uint32_t groupSpaceHeight, uint32_t groupSpaceDepth, CmThreadGroupSpace* & threadGroupSpace);
    static int32_t Destroy( CmThreadGroupSpace* &threadGroupSpace );

    int32_t GetThreadGroupSpaceSize(uint32_t & threadSpaceWidth, uint32_t & threadSpaceHeight, uint32_t & threadSpaceDepth, uint32_t & groupSpaceWidth, uint32_t & groupSpaceHeight, uint32_t &groupSpaceDepth) const;
    virtual uint32_t GetIndexInTGsArray();

#if CM_LOG_ON
    std::string Log();
#endif

protected:
    CmThreadGroupSpace(CmDeviceRT* device, uint32_t index, uint32_t threadSpaceWidth, uint32_t threadSpaceHeight, uint32_t groupSpaceWidth, uint32_t groupSpaceHeight);
    CmThreadGroupSpace(CmDeviceRT* device, uint32_t index, uint32_t threadSpaceWidth, uint32_t threadSpaceHeight, uint32_t threadSpaceDepth, uint32_t groupSpaceWidth, uint32_t groupSpaceHeight, uint32_t groupSpaceDepth);
    ~CmThreadGroupSpace( void );
    int32_t Initialize( void );

    CmDeviceRT* m_device;
    uint32_t m_threadSpaceWidth;
    uint32_t m_threadSpaceHeight;
    uint32_t m_threadSpaceDepth;
    uint32_t m_groupSpaceWidth;
    uint32_t m_groupSpaceHeight;
    uint32_t m_groupSpaceDepth;

    uint32_t m_indexInThreadGroupSpaceArray;
};
};//namespace

#endif  // #ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMGROUPSPACE_H_
