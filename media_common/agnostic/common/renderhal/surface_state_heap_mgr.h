/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     surface_state_heap_mgr.h
//! \brief    Render Engine Interfaces shared across platforms
//! \details  Platform Independent Hardware Interfaces
//!
#ifndef __SURFACE_STATE_HEAP_MGR_H__
#define __SURFACE_STATE_HEAP_MGR_H__

#include <iostream>
#include "mos_os.h"
#include "mhw_utilities.h"
#include "mos_os_specific.h"
#include "mos_interface.h"
#include<vector>

#define MAX_SURFACE_STATES 512
using SURF_STATES_LIST = std::vector<int32_t>;
//!
//! \brief  Default size of area for sync, debugging, performance collecting
//!
#define SYNC_SIZE 128  // range: (128 ... 4096)

//!
//! \brief  VEBOX Heap State Structure
//!
typedef struct _SURFACE_STATES_OBJ
{
    bool     bBusy;      // true if the state is in use (must sync before use)
    uint32_t dwSyncTag;  // surface heap state sync tag
} SURFACE_STATES_OBJ, *PSURFACE_STATES_OBJ;

typedef struct _SURFACE_STATES_HEAP_OBJ
{
    uint32_t            uiCurState;            // Current surface State
    uint32_t            uiNextState;           // Next surface State
    uint32_t            uiOffsetSync;          // Offset of sync data in Heap
    uint32_t            uiInstanceSize;        // Size of single instance
    uint32_t            uiStateHeapSize;       // Total size of Surface States heap
    PSURFACE_STATES_OBJ pSurfStateObj;         // Array of SURFACE_STATES_SYNC_OBJ
    MOS_RESOURCE        osResource;            // Graphics memory
    uint8_t            *pLockedOsResourceMem;  // Locked resource memory

    // Synchronization
    volatile uint32_t  *pSync;      // Pointer to sync area (when locked)
    uint32_t            dwNextTag;  // Next sync tag value to use
    uint32_t            dwSyncTag;  // Last sync tag completed
} SURFACE_STATES_HEAP_OBJ, *PSURFACE_STATES_HEAP_OBJ;

class SurfaceStateHeapManager
{
public:
    SurfaceStateHeapManager(PMOS_INTERFACE pOsInterface);
    MOS_STATUS CreateHeap(size_t surfStateSize);

    void RefreshSync();

    MOS_STATUS DestroyHeap();

    MOS_STATUS AssignSurfaceState();

    MOS_STATUS AssignUsedSurfaceState(int32_t index)
    {
        m_usedStates.push_back(index);
        return MOS_STATUS_SUCCESS;
    }

    ~SurfaceStateHeapManager();

public:
    PMOS_INTERFACE          m_osInterface                    = nullptr;
    SURFACE_STATES_HEAP_OBJ *m_surfStateHeap                 = nullptr;
    int                      m_surfHeapInUse                 = 0;
    SURF_STATES_LIST         m_usedStates                    = {};
};

#endif  // __SURFACE_STATE_HEAP_MGR_H__