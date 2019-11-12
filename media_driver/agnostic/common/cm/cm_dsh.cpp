/*
* Copyright (c) 2018-2019, Intel Corporation
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
//! \file      cm_dsh.cpp
//! \brief     Contains Class CmDSH  definitions
//!

#include "cm_dsh.h"
#include "cm_media_state.h"
#include "heap_manager.h"

CmDSH::CmDSH(CM_HAL_STATE* cmhal) :
    m_heapMgr(nullptr),
    m_cmhal(cmhal)
{
}

CmDSH::~CmDSH()
{
    MOS_Delete(m_heapMgr);
}

MOS_STATUS CmDSH::Initialize(FrameTrackerProducer *trackerProducer)
{
    m_heapMgr = MOS_New(HeapManager);
    
    CM_CHK_NULL_RETURN_MOSERROR(m_heapMgr);
    CM_CHK_MOSSTATUS_RETURN(m_heapMgr->RegisterOsInterface(m_cmhal->osInterface));

    m_heapMgr->SetDefaultBehavior(HeapManager::Behavior::destructiveExtend);
    CM_CHK_MOSSTATUS_RETURN(m_heapMgr->SetInitialHeapSize(m_initSize));
    CM_CHK_MOSSTATUS_RETURN(m_heapMgr->SetExtendHeapSize(m_stepSize));
    CM_CHK_MOSSTATUS_RETURN(m_heapMgr->RegisterTrackerProducer(trackerProducer));
    // lock the heap in the beginning, so cpu doesn't need to wait gpu finishing occupying it to lock it again
    CM_CHK_MOSSTATUS_RETURN(m_heapMgr->LockHeapsOnAllocate());

    return MOS_STATUS_SUCCESS;
}

CmMediaState* CmDSH::CreateMediaState()
{
    CmMediaState *mediaState = MOS_New(CmMediaState, m_cmhal);
    if (mediaState == nullptr)
    {
        return nullptr;
    }
    mediaState->Initialize(m_heapMgr);

    return mediaState;
}

void CmDSH::DestroyMediaState(CmMediaState *mediaState)
{
    MOS_Delete(mediaState);
}

