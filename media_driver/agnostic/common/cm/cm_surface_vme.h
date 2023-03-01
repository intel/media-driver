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
//! \file      cm_surface_vme.h 
//! \brief     Contains Class CmSurfaceVme  definitions 
//!
#pragma once

#include "cm_surface.h"

class CmSurfaceStateVME;
class CmExecutionAdv;
namespace CMRT_UMD
{

class CmSurfaceVme : public CmSurface
{
public:
    static int32_t Create(
        uint32_t index,
        uint32_t indexFor2DCurrent,
        uint32_t indexFor2DForward,
        uint32_t indexFor2DBackward,
        uint32_t indexForCurrent,
        uint32_t indexForForward,
        uint32_t indexForBackward,
        CmSurfaceManager* surfaceManager,
        CmSurfaceVme* &surface );

    static int32_t Create(
        uint32_t index,
        uint32_t indexFor2DCurSurface,
        uint32_t *forwardSurface,
        uint32_t *backwardSurface,
        uint32_t currentIndex,
        uint32_t *forwardCmIndex,
        uint32_t *backwardCmIndex,
        const uint32_t forwardSurfaceCount,
        const uint32_t backSurfaceCount,
        CmSurfaceManager* surfaceManager,
        CmSurfaceVme* &surface );

    virtual int32_t GetIndex(SurfaceIndex*& index);
    int32_t GetIndexCurrent( uint32_t& index );
    int32_t GetIndexForward( uint32_t& index );
    int32_t GetIndexBackward( uint32_t& index );
    int32_t GetIndexForwardArray( uint32_t*& index );
    int32_t GetIndexBackwardArray( uint32_t*& index );
    int32_t GetIndexForwardCount( uint32_t &count);
    int32_t GetIndexBackwardCount( uint32_t & count);

    int32_t GetCmIndexCurrent( uint16_t& index );
    int32_t GetCmIndexForward( uint16_t& index );
    int32_t GetCmIndexBackward( uint16_t& index );
    int32_t GetCmIndexForwardArray( uint32_t*& index );
    int32_t GetCmIndexBackwardArray( uint32_t*& index );

    int32_t GetSurfaceStateResolution(uint32_t& width, uint32_t& height);
    virtual int32_t SetSurfaceStateResolution(uint32_t width, uint32_t height);

    bool IsVmeSurfaceGen7_5();

    void SetSurfState(CmExecutionAdv *advExec, uint8_t *argValue, CmSurfaceStateVME *surfState);

    inline CmSurfaceStateVME *GetSurfaceState() { return m_surfState; }

    CM_ENUM_CLASS_TYPE Type() const {return CM_ENUM_CLASS_TYPE_CMSURFACEVME;};

    // calculate the size needed for the pValue
    int32_t GetVmeCmArgSize();
    // return the surfaces number registered in a vme surface, including current, forward and backward
    int32_t GetTotalSurfacesCount();

    void DumpContent(uint32_t kernelNumber,
                     char *kernelName,
                     int32_t taskId,
                     uint32_t argIndex,
                     uint32_t vectorIndex);

protected:
    CmSurfaceVme(
        uint32_t indexFor2DCurrent,
        uint32_t indexFor2DForward,
        uint32_t indexFor2DBackward,
        uint32_t indexForCurrent,
        uint32_t indexForForward,
        uint32_t indexForBackward,
        CmSurfaceManager* surfaceManager );

    CmSurfaceVme(
        const uint32_t surfaceFCount,
        const uint32_t surfaceBCount,
        uint32_t indexFor2DCurSurface,
        uint32_t *forwardSurface,
        uint32_t *backwardSurface,
        uint32_t currentIndex,
        uint32_t *forwardCmIndex,
        uint32_t *backwardCmIndex,
        CmSurfaceManager* surfaceManager);

    ~CmSurfaceVme( void );

    int32_t Initialize( uint32_t index );

    uint32_t m_indexFor2DCurrent;
    uint32_t m_indexFor2DForward;
    uint32_t m_indexFor2DBackward;
    uint32_t *m_forwardSurfaceArray;
    uint32_t *m_backwardSurfaceArray;

    uint32_t m_cmIndexForCurrent;
    uint32_t m_cmIndexForForward;
    uint32_t m_cmIndexForBackward;
    uint32_t *m_forwardCmIndexArray;
    uint32_t *m_backwardCmIndexArray;

    uint32_t m_surfaceFCount;
    uint32_t m_surfaceBCount;

    uint32_t m_surfStateWidth;
    uint32_t m_surfStateHeight;

    uint8_t *m_argValue;
    CmSurfaceStateVME *m_surfState;
    CmExecutionAdv *m_advExec;

    bool m_isGen75;

private:
    CmSurfaceVme(const CmSurfaceVme& other);
    CmSurfaceVme& operator=(const CmSurfaceVme& other);
};
}; //namespace
