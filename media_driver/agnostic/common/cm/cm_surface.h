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
#include "cm_surface_manager.h"
#include "frame_tracker.h"

namespace CMRT_UMD
{

class CmSurfaceManager;
class CmEventRT;
class CSync;

class CmSurface
{
public:
    static int32_t Destroy( CmSurface* &surface );
    bool IsCmCreated( void ){ return m_isCmCreated; }
    virtual CM_ENUM_CLASS_TYPE Type() const = 0;
    int32_t TouchDeviceQueue();
    virtual int32_t WaitForReferenceFree();
    int32_t SetMemoryObjectControl(MEMORY_OBJECT_CONTROL memCtrl, MEMORY_TYPE memType, uint32_t age);
    int32_t SetResourceUsage(MOS_HW_RESOURCE_DEF mosUsage);

    std::string GetFormatString(CM_SURFACE_FORMAT format);
    virtual void DumpContent(uint32_t kernelNumber, char *kernelName, int32_t taskId, uint32_t argIndex, uint32_t vectorIndex) { return; }
    virtual void Log(std::ostringstream &oss) { return; }
    inline void SetRenderTracker(uint32_t index, uint32_t tracker) {m_lastRenderTracker.Merge(index, tracker); }
    inline void SetFastTracker(uint32_t index, uint32_t tracker) {m_lastFastTracker.Merge(index, tracker); }
    inline void SetVeboxTracker(uint32_t tracker) {m_lastVeboxTracker = tracker; }
    inline void DelayDestroy() { m_released = true; }
    inline bool IsDelayDestroyed() {return m_released; }
    inline bool AllReferenceCompleted() {
            // not called in render, otherwise it finished execution in render
        return (m_lastRenderTracker.IsExpired())
           // not called in enqueuefast, otherwise it finished execution in enqueuefast
           && (m_lastFastTracker.IsExpired())
           // not called in vebox, otherwise it finished execution in vebox
           && (m_lastVeboxTracker == 0 || ((int)(m_lastVeboxTracker - m_surfaceMgr->LatestVeboxTracker()) <= 0));
        }
    inline bool CanBeDestroyed() {
        return m_released && AllReferenceCompleted();
        }

    inline CmSurface*& DelayDestroyPrev() {return m_delayDestroyPrev; }
    inline CmSurface*& DelayDestroyNext() {return m_delayDestroyNext; } 

    inline uint8_t GetPropertyIndex() {return m_propertyIndex; }

protected:
    CmSurface( CmSurfaceManager* surfMgr , bool isCmCreated );
    virtual ~CmSurface( void );
    int32_t Initialize( uint32_t index );

    int32_t FlushDeviceQueue( CmEventRT* event );
    bool MemoryObjectCtrlPolicyCheck(MEMORY_OBJECT_CONTROL memCtrl);

#if MDF_SURFACE_CONTENT_DUMP
    MOS_CONTEXT* GetMosContext();
#endif  // #if MDF_SURFACE_CONTENT_DUMP

    SurfaceIndex* m_index;

    CmSurfaceManager* m_surfaceMgr;

    bool m_isCmCreated;

    CM_SURFACE_MEM_OBJ_CTRL m_memObjCtrl;

    FrameTrackerToken m_lastRenderTracker;

    FrameTrackerToken m_lastFastTracker;

    uint32_t m_lastVeboxTracker;

    bool m_released; // if true, means it is been destroyed by app and added in the delaydestroy queue in surfmgr

    CmSurface *m_delayDestroyPrev; // previous node in bi-directional list

    CmSurface *m_delayDestroyNext; // next node in bi-directional list

    uint8_t m_propertyIndex; // Index to the current surface properties

private:
    CmSurface (const CmSurface& other);
    CmSurface& operator= (const CmSurface& other);
};
}; //namespace
