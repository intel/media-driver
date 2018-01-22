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
//! \file      cm_surface.h
//! \brief     Contains Class CmSurface  definitions 
//!
#pragma once

#include "cm_def.h"

namespace CMRT_UMD
{

class CmSurfaceManager;
class CmEventRT;

class CmSurface
{
public:
    static int32_t Destroy( CmSurface* &surface );
    bool IsCmCreated( void ){ return m_isCmCreated; }
    virtual CM_ENUM_CLASS_TYPE Type() const = 0;
    int32_t TouchDeviceQueue();
    int32_t WaitForReferenceFree();
    int32_t SetMemoryObjectControl(MEMORY_OBJECT_CONTROL memCtrl, MEMORY_TYPE memType, uint32_t age);
    std::string GetFormatString(CM_SURFACE_FORMAT format);
    virtual void DumpContent(uint32_t kernelNumber, int32_t taskId, uint32_t argIndex) { return; }
    virtual void Log(std::ostringstream &oss) { return; }

protected:
    CmSurface( CmSurfaceManager* surfMgr , bool isCmCreated );
    virtual ~CmSurface( void );
    int32_t Initialize( uint32_t index );

    int32_t FlushDeviceQueue( CmEventRT* event );
    bool MemoryObjectCtrlPolicyCheck(MEMORY_OBJECT_CONTROL memCtrl);

    SurfaceIndex* m_index;

    CmSurfaceManager* m_surfaceMgr;

    bool m_isCmCreated;

    CM_SURFACE_MEM_OBJ_CTRL m_memObjCtrl;

private:
    CmSurface (const CmSurface& other);
    CmSurface& operator= (const CmSurface& other);
};
}; //namespace
