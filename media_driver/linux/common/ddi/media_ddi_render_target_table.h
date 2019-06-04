/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     media_ddi_render_target_table.h
//! \brief    Declares the classes and structs for render target management.
//!

#ifndef _MEDIA_DDI_RENDER_TARGET_TABLE_H_
#define _MEDIA_DDI_RENDER_TARGET_TABLE_H_

#include "media_libva.h"

#include <map>
#include <vector>

#define INVALID_RT_TABLE_INDEX CODECHAL_INVALID_FRAME_INDEX
using RTTableIdx = uint8_t;

//!
//! \brief    DDI Codec Render Target Table
//! \details  This class tracks the surfaces registered during VA calls and assigns to each
//!           a unique internal index ("FrameIdx").
//!           The indices and the state variable are later used for reference picture
//!           management by the driver. Tracks VASurfaceIDs of the "current" render target
//!           and the "reconstructed frame" render target.
class MediaDdiRenderTargetTable
{
public:
    MediaDdiRenderTargetTable()
    {
        m_currentRtSurface = VA_INVALID_ID;
        m_currentReconTarget = VA_INVALID_ID;
    }

    void Init(size_t maxNumEntries);
    VAStatus RegisterRTSurface(VASurfaceID id);
    VAStatus UnregisterRTSurface(VASurfaceID id);
    bool     IsRegistered(VASurfaceID id) const;

    VAStatus SetCurrentRTSurface(VASurfaceID id);
    VASurfaceID GetCurrentRTSurface() const;

    VAStatus SetCurrentReconTarget(VASurfaceID id);
    VASurfaceID GetCurrentReconTarget() const;

    std::vector<VASurfaceID> GetRegisteredVAIDs() const;
    size_t GetNumRenderTargets() const;
    RTTableIdx GetFrameIdx(VASurfaceID id) const;
    VASurfaceID GetVAID(RTTableIdx FrameIdx) const;

protected:
    VASurfaceID m_currentRtSurface;
    VASurfaceID m_currentReconTarget;
    std::map<VASurfaceID, RTTableIdx> m_surfaceIdToIndexMap;
    std::vector<RTTableIdx> m_freeIndices;
    std::list<VASurfaceID> m_usageOrder;
};

#endif /* _MEDIA_DDI_RENDER_TARGET_TABLE_H_ */
