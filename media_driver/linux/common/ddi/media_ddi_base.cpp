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
//! \file     media_ddi_base.cpp
//! \brief    Defines base class for DDI media encode/decoder.
//!

#include "media_ddi_base.h"

inline size_t IncrementRTEntryIdx(size_t curr_idx)
{
    return (curr_idx == DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT - 1) ? 0 : ++curr_idx;
}

inline size_t DecrementRTEntryIdx(size_t curr_idx)
{
    return (curr_idx == 0) ? (DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT - 1) : --curr_idx;
}

inline size_t GetIdxByOrder(DDI_CODEC_RENDER_TARGET_TABLE *rtTbl, size_t order)
{
    if (order < rtTbl->iNextRingBufferPosition)
    {
        return rtTbl->iNextRingBufferPosition - 1 - order;
    }

    return DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT + rtTbl->iNextRingBufferPosition - 1 - order;
}

VAStatus RemoveRTEntryByOrder(DDI_CODEC_RENDER_TARGET_TABLE *rtTbl, size_t order)
{
    if (order >= rtTbl->iNumRenderTargets)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    size_t idx = GetIdxByOrder(rtTbl, order);
    rtTbl->pRT[idx] = nullptr;
    rtTbl->ucRTFlag[idx] = SURFACE_STATE_INACTIVE;

    // Restore buffer continuity
    DDI_MEDIA_SURFACE *tmp = nullptr;

    if (order < rtTbl->iNumRenderTargets / 2)
    { // Surface to evict is closer to the start of the buffer
        size_t currPos = idx;
        size_t nextPos = IncrementRTEntryIdx(currPos);
        for (size_t i = 0; i < order; i++)
        {
            tmp = rtTbl->pRT[currPos];
            rtTbl->pRT[currPos] = rtTbl->pRT[nextPos];
            rtTbl->pRT[nextPos] = tmp;
            currPos = nextPos;
            nextPos = IncrementRTEntryIdx(currPos);
        }
        rtTbl->iNextRingBufferPosition = DecrementRTEntryIdx(rtTbl->iNextRingBufferPosition);
    }
    else
    { // Surface to evict is closer to the end of the buffer
        size_t currPos = idx;
        size_t prevPos = DecrementRTEntryIdx(currPos);
        for (size_t i = 0; i < rtTbl->iNumRenderTargets - order - 1; i++)
        {
            tmp = rtTbl->pRT[currPos];
            rtTbl->pRT[currPos] = rtTbl->pRT[prevPos];
            rtTbl->pRT[prevPos] = tmp;
            currPos = prevPos;
            prevPos = DecrementRTEntryIdx(currPos);
        }
    }

    --rtTbl->iNumRenderTargets;

    return VA_STATUS_SUCCESS;
}

int32_t DdiMediaBase::GetRenderTargetID(DDI_CODEC_RENDER_TARGET_TABLE *rtTbl, DDI_MEDIA_SURFACE *surface)
{
    if((nullptr == surface) || (nullptr == rtTbl))
    {
        return DDI_CODEC_INVALID_FRAME_INDEX;
    }

    if (0 == rtTbl->iNumRenderTargets)
    {
        return DDI_CODEC_INVALID_FRAME_INDEX;
    }

    for(int32_t i = 0; i < DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT; i++)
    {
        if(rtTbl->pRT[i] == surface)
        {
            return i;
        }
    }
    return DDI_CODEC_INVALID_FRAME_INDEX;
}

VAStatus DdiMediaBase::RegisterRTSurfaces(DDI_CODEC_RENDER_TARGET_TABLE *rtTbl, DDI_MEDIA_SURFACE *surface)
{
    DDI_CHK_NULL(surface, "nullptr surface", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(rtTbl, "nullptr rtTbl", VA_STATUS_ERROR_INVALID_PARAMETER);

    uint32_t i = 0;
    uint32_t nextPos = rtTbl->iNextRingBufferPosition;
    uint32_t currPos = nextPos;

    // Find out whether or not the surface is in the buffer already
    for (i = 0; i < rtTbl->iNumRenderTargets; i++)
    {
        currPos = DecrementRTEntryIdx(currPos);
        if (rtTbl->pRT[currPos] == surface)
        {
            //surface has already been registered
            return VA_STATUS_SUCCESS;
        }
    }

    // If the surface is not in the buffer and we have free space,
    // insert the surface.
    if (rtTbl->iNumRenderTargets < DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT)
    {
        rtTbl->pRT[nextPos] = surface;
        rtTbl->ucRTFlag[nextPos] = SURFACE_STATE_ACTIVE_IN_CURFRAME;
        ++rtTbl->iNumRenderTargets;
        rtTbl->iNextRingBufferPosition = IncrementRTEntryIdx(nextPos);
        return VA_STATUS_SUCCESS;
    }

    // The buffer is full - evict the oldest (farthest from the buffer head)
    // unused surface and try again
    currPos = nextPos;
    for(i = rtTbl->iNumRenderTargets - 1; i >=0; --i)
    {
        if(rtTbl->ucRTFlag[currPos] == SURFACE_STATE_INACTIVE)
        {
            VAStatus sts = RemoveRTEntryByOrder(rtTbl, i);

            if (sts != VA_STATUS_SUCCESS)
            {
                return sts;
            }

            return RegisterRTSurfaces(rtTbl, surface);
        }
        currPos = IncrementRTEntryIdx(currPos);
    }

    DDI_VERBOSEMESSAGE("RT table is full, and no surface can be reused");
    return VA_STATUS_ERROR_INVALID_PARAMETER;
}

VAStatus DdiMediaBase::ClearRefList(DDI_CODEC_RENDER_TARGET_TABLE *rtTbl, bool withDpb)
{
    DDI_CHK_NULL(rtTbl, "nullptr rtTbl", VA_STATUS_ERROR_INVALID_PARAMETER);

    if(withDpb)
    {
        for(int32_t i = 0; i < DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT; i ++)
        {
            if(rtTbl->ucRTFlag[i] == SURFACE_STATE_ACTIVE_IN_LASTFRAME)
            {
                rtTbl->ucRTFlag[i] = SURFACE_STATE_INACTIVE;
            }
            else if(rtTbl->ucRTFlag[i] == SURFACE_STATE_ACTIVE_IN_CURFRAME)
            {
                rtTbl->ucRTFlag[i] = SURFACE_STATE_ACTIVE_IN_LASTFRAME;
            }
        }
    }
    else
    {
        for(int32_t i = 0; i < DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT; i ++)
        {
            if(rtTbl->ucRTFlag[i])
            {
               rtTbl->ucRTFlag[i] --;
            }
        }
    }
    return VA_STATUS_SUCCESS;
}

VAStatus DdiMediaBase::UpdateRegisteredRTSurfaceFlag(DDI_CODEC_RENDER_TARGET_TABLE *rtTbl, DDI_MEDIA_SURFACE *surface)
{
    DDI_CHK_NULL(rtTbl, "nullptr rtTbl", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(surface, "nullptr surface", VA_STATUS_ERROR_INVALID_PARAMETER);

    for(int32_t i = 0; i < DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT; i ++)
    {
        if(rtTbl->pRT[i] == surface)
        {
            rtTbl->ucRTFlag[i] = SURFACE_STATE_ACTIVE_IN_CURFRAME;
            return VA_STATUS_SUCCESS;
        }
    }
    DDI_VERBOSEMESSAGE("frame was not registered in the RTtbl");
    return VA_STATUS_ERROR_INVALID_PARAMETER;
}

VAStatus DdiMediaBase::UnRegisterRTSurfaces(DDI_CODEC_RENDER_TARGET_TABLE *rtTbl, DDI_MEDIA_SURFACE *surface)
{
    DDI_CHK_NULL(rtTbl, "nullptr rtTbl", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(surface, "nullptr surface", VA_STATUS_ERROR_INVALID_PARAMETER);

    uint32_t i = 0;
    uint32_t currPos = rtTbl->iNextRingBufferPosition;

    for (i = 0; i < rtTbl->iNumRenderTargets; i++)
    {
        currPos = DecrementRTEntryIdx(currPos);
        if (rtTbl->pRT[currPos] == surface)
        {
            return RemoveRTEntryByOrder(rtTbl, i);
        }
    }

    DDI_VERBOSEMESSAGE("The surface to be unregistered cannot be found in RTTbl!");
    return VA_STATUS_ERROR_INVALID_PARAMETER;
}

