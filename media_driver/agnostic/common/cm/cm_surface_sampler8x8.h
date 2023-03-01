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
//! \file      cm_surface_sampler8x8.h 
//! \brief     Contains Class CmSurfaceSampler8x8  definitions 
//!
#pragma once
#include "cm_surface.h"

namespace CMRT_UMD
{
class CmSurfaceSampler8x8 : public CmSurface
{
public:
    static int32_t Create(
        uint32_t index,
        uint32_t indexFor2D,
        uint32_t cmIndex,
        CmSurfaceManager*    surfaceManager,
        CmSurfaceSampler8x8* &surface,
        CM_SAMPLER8x8_SURFACE sampler8x8Type,
        CM_SURFACE_ADDRESS_CONTROL_MODE mode,
        CM_FLAG* flag);

    virtual int32_t GetIndex(SurfaceIndex*& index);
    int32_t GetIndexCurrent( uint32_t& index );
    int32_t GetCmIndex( uint16_t & index );
    CM_SAMPLER8x8_SURFACE GetSampler8x8SurfaceType();
    CM_SURFACE_ADDRESS_CONTROL_MODE GetAddressControlMode();
    int32_t SetMemoryObjectControl(MEMORY_OBJECT_CONTROL memCtrl, MEMORY_TYPE memType, uint32_t age);

    //NOT depend on RTTI::dynamic_cast
    CM_ENUM_CLASS_TYPE Type() const {return CM_ENUM_CLASS_TYPE_CMSURFACESAMPLER8X8;};

protected:
    CmSurfaceSampler8x8(
        uint32_t cmIndex,      //SurfaceIndex's ID for 2D, also indexing surface array in cmrt@umd
        uint32_t indexFor2D,
        CmSurfaceManager* surfaceManager,
        CM_SAMPLER8x8_SURFACE sampler8x8Type,
        CM_SURFACE_ADDRESS_CONTROL_MODE mode,
        CM_FLAG* flag);
    ~CmSurfaceSampler8x8( void );

    int32_t Initialize( uint32_t index );
    uint32_t m_indexFor2D;
    uint32_t m_surfaceIndex;
    CM_SAMPLER8x8_SURFACE m_sampler8x8Type;
    CM_SURFACE_ADDRESS_CONTROL_MODE m_nAddressMode;
    CM_FLAG m_flag;

private:
    CmSurfaceSampler8x8(const CmSurfaceSampler8x8& other);
    CmSurfaceSampler8x8& operator=(const CmSurfaceSampler8x8& other);
};
}; //namespace

