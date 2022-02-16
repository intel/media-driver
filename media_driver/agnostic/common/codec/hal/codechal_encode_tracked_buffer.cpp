/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     codechal_encode_tracked_buffer.cpp
//! \brief    Class to manage tracked buffer used in encoder
//!

#include "codechal_encode_tracked_buffer.h"
#include "codechal_encoder_base.h"

MOS_STATUS CodechalEncodeTrackedBuffer::AllocateForCurrFrame()
{
    CODEC_REF_LIST* currRefList = m_encoder->m_currRefList;

    // in case of resolution change, defer-deallocate remaining 3 buffers from last session
    if (m_trackedBufCountResize)
    {
        DeferredDeallocateOnResChange();
        m_trackedBufCountResize--;
    }

    // update the last 3 buffer index, find a new slot for current frame
    m_trackedBufAnteIdx = m_trackedBufPenuIdx;
    m_trackedBufPenuIdx = m_trackedBufCurrIdx;
    m_trackedBufCurrIdx = LookUpBufIndex(currRefList->RefList, currRefList->ucNumRef, currRefList->bUsedAsRef);

    CODECHAL_ENCODE_CHK_COND_RETURN(m_trackedBufCurrIdx >= CODEC_NUM_TRACKED_BUFFERS, "No tracked buffer is available!");

    // wait to re-use once # of non-ref slots being used reaches 3
    m_waitTrackedBuffer = (m_trackedBufCurrIdx >= CODEC_NUM_REF_BUFFERS && m_trackedBufCountNonRef >= CODEC_NUM_NON_REF_BUFFERS);

    CODECHAL_ENCODE_NORMALMESSAGE("currFrame = %d, currRef = %d, ucNumRef = %d, usedAsRef = %d, tracked buf index = %d",
        m_encoder->m_currOriginalPic.FrameIdx, m_encoder->m_currReconstructedPic.FrameIdx,
        currRefList->ucNumRef, currRefList->bUsedAsRef, m_trackedBufCurrIdx);

    if (m_allocateMbCode)
    {
        LookUpBufIndexMbCode();
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateMbCodeResources(m_mbCodeCurrIdx));

        // for non-AVC codec, MbCode and MvData surface are combined, this function won't be called
        if (m_encoder->m_mvDataSize)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateMvDataResources(m_trackedBufCurrIdx));
        }
    }

    // allocate MV temporal buffer
    AllocateMvTemporalBuffer(m_trackedBufCurrIdx);

    // allocate VDEnc downscaled recon surface
    if (m_encoder->m_vdencEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateDsReconSurfacesVdenc());
        //re-allocate VDEnc downscaled recon surface in case there is resolution change
        if (CODECHAL_VP9 == m_standard)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(ResizeDsReconSurfacesVdenc());
        }
    }
    return MOS_STATUS_SUCCESS;
}

/*
* When resolution changes, tracked buffers used by earlier submitted frames may not have finished execution,
* destruction of these in-the-fly buffers need to be deferred after execution completes
* We make a resonable assumption that the number of unfinished frames should not exceed 3, and free all other
* existing buffers except the last 3 used
* Inside LookUpBufIndex(), the counter is checked and decremented, each time freeing
* one of the remaining 3 buffers in previous encode session/sequence (with old resolution)
* The net result is that 3 frames into the new encode session/sequence, all tracked buffers have been re-allocated
* according to the new resolution
*/
void CodechalEncodeTrackedBuffer::Resize()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    // free existing allocations except last 3 slots
    m_trackedBufCountResize = CODEC_NUM_NON_REF_BUFFERS;
    for (uint8_t i = 0; i < CODEC_NUM_TRACKED_BUFFERS; i++)
    {
        if (m_trackedBufAnteIdx != i && m_trackedBufPenuIdx != i && m_trackedBufCurrIdx != i)
        {
            if (m_mbCodeIsTracked)
            {
                ReleaseMbCode(i);
            }
            ReleaseMvData(i);
            ReleaseDsRecon(i);
            if (m_encoder->m_cscDsState)
                ReleaseSurfaceDS(i);
            if (m_encoder->m_vdencEnabled)
                m_allocator->ReleaseResource(m_standard, mvTemporalBuffer, i);

            // this slot can now be re-used
            m_tracker[i].ucSurfIndex7bits = PICTURE_MAX_7BITS;
        }
        else
        {
            m_tracker[i].ucSurfIndex7bits = PICTURE_RESIZE;
        }
    }
    if (m_encoder->m_cscDsState)
        ResizeCsc();

    return;
}

void CodechalEncodeTrackedBuffer::ResizeCsc()
{
    // free CSC surfaces except last 3 slots
    for (uint8_t i = 0; i < CODEC_NUM_TRACKED_BUFFERS; i++)
    {
        if (m_cscBufAnteIdx != i && m_cscBufPenuIdx != i && m_cscBufCurrIdx != i)
        {
            ReleaseSurfaceCsc(i);
        }
    }
}

void CodechalEncodeTrackedBuffer::AllocateForCurrFramePreenc(uint8_t bufIndex)
{
    // update the last 3 buffer index
    m_trackedBufAnteIdx = m_trackedBufPenuIdx;
    m_trackedBufPenuIdx = m_trackedBufCurrIdx;
    // use the buffer index passed in by Preenc
    m_trackedBufCurrIdx = bufIndex;
}

void CodechalEncodeTrackedBuffer::ResetUsedForCurrFrame()
{
    for (auto i = 0; i < CODEC_NUM_TRACKED_BUFFERS; i++)
    {
        m_tracker[i].bUsedforCurFrame = false;
    }
}

uint8_t CodechalEncodeTrackedBuffer::PreencLookUpBufIndex(
    uint8_t         frameIdx,
    bool            *inCache)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    *inCache = false;
    uint8_t j = frameIdx % CODEC_NUM_TRACKED_BUFFERS;
    uint8_t emptyEntry = CODEC_NUM_TRACKED_BUFFERS;

    for (auto i = 0; i < CODEC_NUM_TRACKED_BUFFERS; i++)
    {
        if (m_tracker[j].ucSurfIndex7bits == frameIdx)
        {
            //this frame is already in cache
            *inCache = true;
            m_tracker[j].bUsedforCurFrame = true;

            return emptyEntry = j;
        }
        j = (j + 1) % CODEC_NUM_TRACKED_BUFFERS;
    }

    j = frameIdx % CODEC_NUM_TRACKED_BUFFERS;
    for (auto i = 0; i < CODEC_NUM_TRACKED_BUFFERS; i++)
    {
        if (!m_tracker[j].bUsedforCurFrame)
        {
            //find the first empty entry
            emptyEntry = j;
            break;
        }
        j = (j + 1) % CODEC_NUM_TRACKED_BUFFERS;
    }

    if (emptyEntry < CODEC_NUM_TRACKED_BUFFERS)
    {
        m_tracker[emptyEntry].ucSurfIndex7bits = frameIdx;
        m_tracker[emptyEntry].bUsedforCurFrame = true;
    }

    return emptyEntry;
}

uint8_t CodechalEncodeTrackedBuffer::LookUpBufIndex(
    PCODEC_PICTURE refList,
    uint8_t        numRefFrame,
    bool           usedAsRef)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint8_t index = PICTURE_MAX_7BITS;
    if (usedAsRef && numRefFrame <= CODEC_MAX_NUM_REF_FRAME && !m_encoder->m_gopIsIdrFrameOnly)
    {
        uint8_t refPicIdx[CODEC_MAX_NUM_REF_FRAME];
        bool notFound = true;

        for (auto i = 0; i < numRefFrame; i++)
        {
            refPicIdx[i] = refList[i].FrameIdx;
        }

        // find the first empty slot to re-use
        for (uint8_t i = 0; i < CODEC_NUM_REF_BUFFERS; i++)
        {
            tracker* trackedBuffer = &m_tracker[i];
            uint8_t refFrameIdx = trackedBuffer->ucSurfIndex7bits;

            if (refFrameIdx != PICTURE_MAX_7BITS && refFrameIdx != PICTURE_RESIZE)
            {
                // check whether this ref frame is still active
                uint8_t j = 0;
                for (j = 0; j < numRefFrame; j++)
                {
                    if (refFrameIdx == refPicIdx[j])
                        break;
                }

                if (j == numRefFrame)
                {
                    // this ref frame is no longer active, can be re-used
                    trackedBuffer->ucSurfIndex7bits = PICTURE_MAX_7BITS;
                }
            }

            if (notFound && PICTURE_MAX_7BITS == trackedBuffer->ucSurfIndex7bits)
            {
                index = i;
                notFound = false;
                continue;
            }
        }
    }
    else
    {
        if (!m_encoder->m_waitForPak)
        {
            m_trackedBufCountNonRef += m_trackedBufCountNonRef < CODEC_NUM_NON_REF_BUFFERS;
            CODECHAL_ENCODE_NORMALMESSAGE("Tracked buffer count = %d", m_trackedBufCountNonRef);
        }
        else
        {
            m_trackedBufCountNonRef = 0;
        }

        m_trackedBufNonRefIdx = (m_trackedBufNonRefIdx + 1) % CODEC_NUM_NON_REF_BUFFERS;
        index = CODEC_NUM_REF_BUFFERS + m_trackedBufNonRefIdx;
    }

    if (index < CODEC_NUM_TRACKED_BUFFERS)
    {
        m_tracker[index].ucSurfIndex7bits = m_encoder->m_currReconstructedPic.FrameIdx;
    }

    return index;
}

uint8_t CodechalEncodeTrackedBuffer::LookUpBufIndexCsc()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_encoder->m_useRawForRef)
    {
        return m_trackedBufCurrIdx;
    }
    else
    {
        // if Raw won't be used as Ref, can use the size-3 ring buffer
        if (!m_encoder->m_waitForPak)
        {
            m_cscBufCountNonRef += m_cscBufCountNonRef <= CODEC_NUM_NON_REF_BUFFERS;
            CODECHAL_ENCODE_NORMALMESSAGE("CSC buffer count = %d", m_cscBufCountNonRef);
        }
        else
        {
            m_cscBufCountNonRef = 0;
        }

        m_cscBufNonRefIdx %= CODEC_NUM_NON_REF_BUFFERS;
        return m_cscBufNonRefIdx += CODEC_NUM_REF_BUFFERS;
    }
}

void CodechalEncodeTrackedBuffer::DeferredDeallocateOnResChange()
{
    if (m_trackedBufAnteIdx != m_trackedBufPenuIdx && m_trackedBufAnteIdx != m_trackedBufCurrIdx)
    {
        if (m_mbCodeIsTracked)
        {
            ReleaseMbCode(m_trackedBufAnteIdx);
        }
        ReleaseMvData(m_trackedBufAnteIdx);
        ReleaseDsRecon(m_trackedBufAnteIdx);
        if (m_encoder->m_cscDsState)
        {
            ReleaseSurfaceDS(m_trackedBufAnteIdx);
        }
        if (m_encoder->m_vdencEnabled)
            m_allocator->ReleaseResource(m_standard, mvTemporalBuffer, m_trackedBufAnteIdx);

        m_tracker[m_trackedBufAnteIdx].ucSurfIndex7bits = PICTURE_MAX_7BITS;
        CODECHAL_ENCODE_NORMALMESSAGE("Tracked buffer = %d re-allocated", m_trackedBufAnteIdx);
    }

    if (m_encoder->m_cscDsState && m_cscBufAnteIdx != m_cscBufPenuIdx && m_cscBufAnteIdx != m_cscBufCurrIdx)
    {
        ReleaseSurfaceCsc(m_cscBufAnteIdx);
        CODECHAL_ENCODE_NORMALMESSAGE("CSC buffer = %d re-allocated", m_cscBufAnteIdx);
    }
}

MOS_STATUS CodechalEncodeTrackedBuffer::AllocateMbCodeResources(uint8_t bufIndex)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_COND_RETURN(
        bufIndex >= CODEC_NUM_TRACKED_BUFFERS,
        "No MbCode buffer is available!");

    MEDIA_WA_TABLE *waTable = m_osInterface->pfnGetWaTable(m_osInterface);
    uint32_t        memType = (MEDIA_IS_WA(waTable, WaForceAllocateLML4) && m_standard == CODECHAL_AVC) ? MOS_MEMPOOL_DEVICEMEMORY : 0;

    // early exit if already allocated
    if ((m_trackedBufCurrMbCode = (MOS_RESOURCE*)m_allocator->GetResource(m_standard, mbCodeBuffer, bufIndex)))
    {
        return MOS_STATUS_SUCCESS;
    }

    // Must reserve at least 8 cachelines after MI_BATCH_BUFFER_END_CMD since HW prefetch max 8 cachelines from BB everytime
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_trackedBufCurrMbCode = (MOS_RESOURCE *)m_allocator->AllocateResource(
        m_standard, m_encoder->m_mbCodeSize + 8 * CODECHAL_CACHELINE_SIZE, 1, mbCodeBuffer, "mbCodeBuffer", bufIndex, true, Format_Buffer, MOS_TILE_LINEAR, memType));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeTrackedBuffer::AllocateMvDataResources(uint8_t bufIndex)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MEDIA_WA_TABLE *waTable = m_osInterface->pfnGetWaTable(m_osInterface);
    uint32_t        memType = (MEDIA_IS_WA(waTable, WaForceAllocateLML4) && m_standard == CODECHAL_AVC) ? MOS_MEMPOOL_DEVICEMEMORY : 0;

    // early exit if already allocated
    if ((m_trackedBufCurrMvData = (MOS_RESOURCE*)m_allocator->GetResource(m_standard, mvDataBuffer, bufIndex)))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_trackedBufCurrMvData = (MOS_RESOURCE*)m_allocator->AllocateResource(
        m_standard, m_encoder->m_mvDataSize, 1, mvDataBuffer, "mvDataBuffer", bufIndex, true, Format_Buffer, MOS_TILE_LINEAR, memType));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeTrackedBuffer::AllocateSurfaceCsc()
{
    // update the last 3 buffer index, find a new slot for current frame
    m_cscBufAnteIdx = m_cscBufPenuIdx;
    m_cscBufPenuIdx = m_cscBufCurrIdx;
    m_cscBufCurrIdx = LookUpBufIndexCsc();

    CODECHAL_ENCODE_CHK_COND_RETURN(m_cscBufCurrIdx >= CODEC_NUM_TRACKED_BUFFERS, "No CSC buffer is available!");

    // wait to re-use once # of non-ref slots being used reaches 3
    m_waitCscSurface = (m_cscBufCurrIdx >= CODEC_NUM_REF_BUFFERS && m_cscBufCountNonRef > CODEC_NUM_NON_REF_BUFFERS);

    if ((m_trackedBufCurrCsc = (MOS_SURFACE*)m_allocator->GetResource(m_standard, cscSurface, m_cscBufCurrIdx)))
    {
        return MOS_STATUS_SUCCESS;
    }

    uint32_t width = 0, height = 0;
    MOS_FORMAT format = Format_Invalid;
    m_encoder->m_cscDsState->GetCscAllocation(width, height, format);

    // allocating Csc surface
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_trackedBufCurrCsc = (MOS_SURFACE*)m_allocator->AllocateResource(
        m_standard, width, height, cscSurface, "cscSurface", m_cscBufCurrIdx, false, format, MOS_TILE_Y));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(m_osInterface, m_trackedBufCurrCsc));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeTrackedBuffer::AllocateSurfaceCopy(MOS_FORMAT format, uint32_t cp_tag)
{
    // update the last 3 buffer index, find a new slot for current frame
    m_cscBufAnteIdx = m_cscBufPenuIdx;
    m_cscBufPenuIdx = m_cscBufCurrIdx;
    m_cscBufCurrIdx = LookUpBufIndexCsc();

    CODECHAL_ENCODE_CHK_COND_RETURN(m_cscBufCurrIdx >= CODEC_NUM_TRACKED_BUFFERS, "No Copy buffer is available!");

    if ((m_trackedBufCurrCsc = (MOS_SURFACE *)m_allocator->GetResource(m_standard, cscSurface, m_cscBufCurrIdx)))
    {
        return MOS_STATUS_SUCCESS;
    }

    // allocating csc surface for copy
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_trackedBufCurrCsc = (MOS_SURFACE *)m_allocator->AllocateResource(
                                        m_standard, m_encoder->m_frameWidth, m_encoder->m_frameHeight,
                                        cscSurface, "cscSurface", m_cscBufCurrIdx, false, format, MOS_TILE_Y));

    m_trackedBufCurrCsc->OsResource.pGmmResInfo->GetSetCpSurfTag(true, cp_tag);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(m_osInterface, m_trackedBufCurrCsc));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeTrackedBuffer::ResizeSurfaceDS() {

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    //Get the 4x 16x and 32x DS surfaces
    m_trackedBufCurrDs4x = (MOS_SURFACE*)m_allocator->GetResource(m_standard, ds4xSurface, m_trackedBufCurrIdx);

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_trackedBufCurrDs4x);

    if (m_encoder->m_16xMeSupported)
    {
        m_trackedBufCurrDs16x = (MOS_SURFACE*)m_allocator->GetResource(m_standard, ds16xSurface, m_trackedBufCurrIdx);
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_trackedBufCurrDs16x);
    }

    if (m_encoder->m_32xMeSupported)
    {
        m_trackedBufCurrDs32x = (MOS_SURFACE*)m_allocator->GetResource(m_standard, ds32xSurface, m_trackedBufCurrIdx);
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_trackedBufCurrDs32x);
    }

    uint32_t downscaledSurfaceWidth4x, downscaledSurfaceHeight4x;
    uint32_t downscaledSurfaceWidth16x, downscaledSurfaceHeight16x;
    uint32_t downscaledSurfaceWidth32x, downscaledSurfaceHeight32x;
    //Get the dimensions of the 4x 16x and 32x surfaces
    if (m_encoder->m_useCommonKernel)
    {
        downscaledSurfaceWidth4x = CODECHAL_GET_4xDS_SIZE_32ALIGNED(m_encoder->m_frameWidth);
        downscaledSurfaceHeight4x = CODECHAL_GET_4xDS_SIZE_32ALIGNED(m_encoder->m_frameHeight);

        downscaledSurfaceWidth16x = CODECHAL_GET_4xDS_SIZE_32ALIGNED(downscaledSurfaceWidth4x);
        downscaledSurfaceHeight16x = CODECHAL_GET_4xDS_SIZE_32ALIGNED(downscaledSurfaceHeight4x);

        downscaledSurfaceWidth32x = CODECHAL_GET_2xDS_SIZE_32ALIGNED(downscaledSurfaceWidth16x);
        downscaledSurfaceHeight32x = CODECHAL_GET_2xDS_SIZE_32ALIGNED(downscaledSurfaceHeight16x);
    }
    else
    {
        // MB-alignment not required since dataport handles out-of-bound pixel replication, but IME requires this.
        downscaledSurfaceWidth4x = m_encoder->m_downscaledWidth4x;
        // Account for field case, offset needs to be 4K aligned if tiled for DI surface state.
        // Width will be allocated tile Y aligned, so also tile align height.
        downscaledSurfaceHeight4x = ((m_encoder->m_downscaledHeight4x / CODECHAL_MACROBLOCK_HEIGHT + 1) >> 1) * CODECHAL_MACROBLOCK_HEIGHT;
        downscaledSurfaceHeight4x = MOS_ALIGN_CEIL(downscaledSurfaceHeight4x, MOS_YTILE_H_ALIGNMENT) << 1;

        downscaledSurfaceWidth16x = m_encoder->m_downscaledWidth16x;
        downscaledSurfaceHeight16x = ((m_encoder->m_downscaledHeight16x / CODECHAL_MACROBLOCK_HEIGHT + 1) >> 1) * CODECHAL_MACROBLOCK_HEIGHT;
        downscaledSurfaceHeight16x = MOS_ALIGN_CEIL(downscaledSurfaceHeight16x, MOS_YTILE_H_ALIGNMENT) << 1;

        downscaledSurfaceWidth32x = m_encoder->m_downscaledWidth32x;
        downscaledSurfaceHeight32x = ((m_encoder->m_downscaledHeight32x / CODECHAL_MACROBLOCK_HEIGHT + 1) >> 1) * CODECHAL_MACROBLOCK_HEIGHT;
        downscaledSurfaceHeight32x = MOS_ALIGN_CEIL(downscaledSurfaceHeight32x, MOS_YTILE_H_ALIGNMENT) << 1;
    }

    bool dsCurr4xAvailable = true;
    bool dsCurr16xAvailable = true;
    bool dsCurr32xAvailable = true;

    if ((m_trackedBufCurrDs4x->dwWidth < downscaledSurfaceWidth4x) || (m_trackedBufCurrDs4x->dwHeight < downscaledSurfaceHeight4x))
    {
        m_allocator->ReleaseResource(m_standard, ds4xSurface, m_trackedBufCurrIdx);
        dsCurr4xAvailable = false;
    }

    if (m_encoder->m_16xMeSupported)
    {
        if ((m_trackedBufCurrDs16x->dwWidth < downscaledSurfaceWidth16x) || (m_trackedBufCurrDs16x->dwHeight < downscaledSurfaceHeight16x))
        {
            m_allocator->ReleaseResource(m_standard, ds16xSurface, m_trackedBufCurrIdx);
            dsCurr16xAvailable = false;
        }
    }

    if (m_encoder->m_32xMeSupported)
    {
        if ((m_trackedBufCurrDs32x->dwWidth < downscaledSurfaceWidth32x) || (m_trackedBufCurrDs32x->dwHeight < downscaledSurfaceHeight32x))
        {
            m_allocator->ReleaseResource(m_standard, ds32xSurface, m_trackedBufCurrIdx);
            dsCurr32xAvailable = false;
        }
    }

    if (dsCurr4xAvailable && dsCurr16xAvailable && dsCurr32xAvailable)
    {
        //No need to resize
        return MOS_STATUS_SUCCESS;
    }

    if (!dsCurr4xAvailable)
    {
        // allocating 4x DS surface
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_trackedBufCurrDs4x = (MOS_SURFACE*)m_allocator->AllocateResource(
            m_standard, downscaledSurfaceWidth4x, downscaledSurfaceHeight4x, ds4xSurface, "ds4xSurface", m_trackedBufCurrIdx, false, Format_NV12, MOS_TILE_Y));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(m_osInterface, m_trackedBufCurrDs4x));
    }

    if (m_encoder->m_16xMeSupported && !dsCurr16xAvailable)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_trackedBufCurrDs16x = (MOS_SURFACE*)m_allocator->AllocateResource(
            m_standard, downscaledSurfaceWidth16x, downscaledSurfaceHeight16x, ds16xSurface, "ds16xSurface", m_trackedBufCurrIdx, false, Format_NV12, MOS_TILE_Y));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(m_osInterface, m_trackedBufCurrDs16x));
    }

    // allocate 32x DS surface
    if (m_encoder->m_32xMeSupported && !dsCurr32xAvailable)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_trackedBufCurrDs32x = (MOS_SURFACE*)m_allocator->AllocateResource(
            m_standard, downscaledSurfaceWidth32x, downscaledSurfaceHeight32x, ds32xSurface, "ds32xSurface", m_trackedBufCurrIdx, false, Format_NV12, MOS_TILE_Y));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(m_osInterface, m_trackedBufCurrDs32x));
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeTrackedBuffer::AllocateSurfaceDS()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MEDIA_WA_TABLE* waTable = m_osInterface->pfnGetWaTable(m_osInterface);
    uint32_t memType = (MEDIA_IS_WA(waTable, WaForceAllocateLML4)) ? MOS_MEMPOOL_DEVICEMEMORY : 0;

    // early exit if already allocated
    if ((m_trackedBufCurrDs4x = (MOS_SURFACE*)m_allocator->GetResource(m_standard, ds4xSurface, m_trackedBufCurrIdx)))
    {
        if (m_encoder->m_16xMeSupported)
        {
            m_trackedBufCurrDs16x = (MOS_SURFACE*)m_allocator->GetResource(m_standard, ds16xSurface, m_trackedBufCurrIdx);
        }

        if (m_encoder->m_32xMeSupported)
        {
            m_trackedBufCurrDs32x = (MOS_SURFACE*)m_allocator->GetResource(m_standard, ds32xSurface, m_trackedBufCurrIdx);
        }
        return MOS_STATUS_SUCCESS;
    }

    uint32_t downscaledSurfaceWidth4x, downscaledSurfaceHeight4x;
    uint32_t downscaledSurfaceWidth16x, downscaledSurfaceHeight16x;
    uint32_t downscaledSurfaceWidth32x, downscaledSurfaceHeight32x;
    if (m_encoder->m_useCommonKernel)
    {
        downscaledSurfaceWidth4x = m_encoder->m_downscaledWidth4x;
        downscaledSurfaceHeight4x = MOS_ALIGN_CEIL(m_encoder->m_downscaledHeight4x, MOS_YTILE_H_ALIGNMENT);
        downscaledSurfaceWidth16x = m_encoder->m_downscaledWidth16x;
        downscaledSurfaceHeight16x = MOS_ALIGN_CEIL(m_encoder->m_downscaledHeight16x, MOS_YTILE_H_ALIGNMENT);
        downscaledSurfaceWidth32x = m_encoder->m_downscaledWidth32x;
        downscaledSurfaceHeight32x = MOS_ALIGN_CEIL(m_encoder->m_downscaledHeight32x, MOS_YTILE_H_ALIGNMENT);
    }
    else
    {
        // MB-alignment not required since dataport handles out-of-bound pixel replication, but IME requires this.
        downscaledSurfaceWidth4x = m_encoder->m_downscaledWidth4x;
        // Account for field case, offset needs to be 4K aligned if tiled for DI surface state.
        // Width will be allocated tile Y aligned, so also tile align height.
        downscaledSurfaceHeight4x = ((m_encoder->m_downscaledHeight4x / CODECHAL_MACROBLOCK_HEIGHT + 1) >> 1) * CODECHAL_MACROBLOCK_HEIGHT;
        downscaledSurfaceHeight4x = MOS_ALIGN_CEIL(downscaledSurfaceHeight4x, MOS_YTILE_H_ALIGNMENT) << 1;

        downscaledSurfaceWidth16x = m_encoder->m_downscaledWidth16x;
        downscaledSurfaceHeight16x = ((m_encoder->m_downscaledHeight16x / CODECHAL_MACROBLOCK_HEIGHT + 1) >> 1) * CODECHAL_MACROBLOCK_HEIGHT;
        downscaledSurfaceHeight16x = MOS_ALIGN_CEIL(downscaledSurfaceHeight16x, MOS_YTILE_H_ALIGNMENT) << 1;

        downscaledSurfaceWidth32x = m_encoder->m_downscaledWidth32x;
        downscaledSurfaceHeight32x = ((m_encoder->m_downscaledHeight32x / CODECHAL_MACROBLOCK_HEIGHT + 1) >> 1) * CODECHAL_MACROBLOCK_HEIGHT;
        downscaledSurfaceHeight32x = MOS_ALIGN_CEIL(downscaledSurfaceHeight32x, MOS_YTILE_H_ALIGNMENT) << 1;
    }

    // allocating 4x DS surface
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_trackedBufCurrDs4x = (MOS_SURFACE*)m_allocator->AllocateResource(
                                        m_standard, downscaledSurfaceWidth4x, downscaledSurfaceHeight4x, ds4xSurface, "ds4xSurface", m_trackedBufCurrIdx, false, Format_NV12, MOS_TILE_Y, memType));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(m_osInterface, m_trackedBufCurrDs4x));

    // allocate 16x DS surface
    if (m_encoder->m_16xMeSupported)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_trackedBufCurrDs16x = (MOS_SURFACE*)m_allocator->AllocateResource(
                                            m_standard, downscaledSurfaceWidth16x, downscaledSurfaceHeight16x, ds16xSurface, "ds16xSurface", m_trackedBufCurrIdx, false, Format_NV12, MOS_TILE_Y, memType));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(m_osInterface, m_trackedBufCurrDs16x));
    }

    // allocate 32x DS surface
    if (m_encoder->m_32xMeSupported)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_trackedBufCurrDs32x = (MOS_SURFACE*)m_allocator->AllocateResource(
                                            m_standard, downscaledSurfaceWidth32x, downscaledSurfaceHeight32x, ds32xSurface, "ds32xSurface", m_trackedBufCurrIdx, false, Format_NV12, MOS_TILE_Y, memType));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(m_osInterface, m_trackedBufCurrDs32x));
    }

    if (!m_encoder->m_fieldScalingOutputInterleaved)
    {
        // Separated scaled surfaces
        // Height should be 4K aligned for DI surface state, assume always Y tiled
        m_encoder->m_scaledBottomFieldOffset = MOS_ALIGN_CEIL(
            (m_trackedBufCurrDs4x->dwPitch * (m_trackedBufCurrDs4x->dwHeight / 2)), CODECHAL_PAGE_SIZE);

        if (m_encoder->m_16xMeSupported)
        {
            // Height should be 4K aligned for DI surface state, assume always Y tiled
            m_encoder->m_scaled16xBottomFieldOffset = MOS_ALIGN_CEIL(
                (m_trackedBufCurrDs16x->dwPitch * (m_trackedBufCurrDs16x->dwHeight / 2)), CODECHAL_PAGE_SIZE);
        }

        if (m_encoder->m_32xMeSupported)
        {
            // Height should be 4K aligned for DI surface state, assume always Y tiled
            m_encoder->m_scaled32xBottomFieldOffset = MOS_ALIGN_CEIL(
                (m_trackedBufCurrDs32x->dwPitch * (m_trackedBufCurrDs32x->dwHeight / 2)), CODECHAL_PAGE_SIZE);
        }

    }
    else
    {
        // Interleaved scaled surfaces
        m_encoder->m_scaledBottomFieldOffset =
        m_encoder->m_scaled16xBottomFieldOffset =
        m_encoder->m_scaled32xBottomFieldOffset = 0;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeTrackedBuffer::AllocateSurface2xDS()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MEDIA_WA_TABLE* waTable = m_osInterface->pfnGetWaTable(m_osInterface);
    uint32_t memType = (MEDIA_IS_WA(waTable, WaForceAllocateLML4)) ? MOS_MEMPOOL_DEVICEMEMORY : 0;

    // early exit if already allocated
    if ((m_trackedBufCurrDs2x = (MOS_SURFACE*)m_allocator->GetResource(m_standard, ds2xSurface, m_trackedBufCurrIdx)))
    {
        return MOS_STATUS_SUCCESS;
    }

    uint32_t surfaceWidth, surfaceHeight;
    if (m_encoder->m_useCommonKernel)
    {
        surfaceWidth = CODECHAL_GET_2xDS_SIZE_32ALIGNED(m_encoder->m_frameWidth);
        surfaceHeight = CODECHAL_GET_2xDS_SIZE_32ALIGNED(m_encoder->m_frameHeight);
    }
    else
    {
        surfaceWidth = MOS_ALIGN_CEIL(m_encoder->m_frameWidth, 64) >> 1;
        surfaceHeight = MOS_ALIGN_CEIL(m_encoder->m_frameHeight, 64) >> 1;
    }

    MOS_FORMAT format = Format_NV12;
    if ((uint8_t)HCP_CHROMA_FORMAT_YUV422 == m_encoder->m_outputChromaFormat)
    {
        format = Format_YUY2;
        surfaceWidth >>= 1;
        surfaceHeight <<= 1;
    }

    // allocate 2x DS surface
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_trackedBufCurrDs2x = (MOS_SURFACE*)m_allocator->AllocateResource(
                                        m_standard, surfaceWidth, surfaceHeight, ds2xSurface, "ds2xSurface", m_trackedBufCurrIdx, false, format, MOS_TILE_Y, memType));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(m_osInterface, m_trackedBufCurrDs2x));

    if ((uint8_t)HCP_CHROMA_FORMAT_YUV422 == m_encoder->m_outputChromaFormat)
    {
        m_trackedBufCurrDs2x->Format = Format_YUY2V;
        m_trackedBufCurrDs2x->dwWidth = surfaceWidth << 1;
        m_trackedBufCurrDs2x->dwHeight = surfaceHeight >> 1;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeTrackedBuffer::ResizeDsReconSurfacesVdenc()
{
    MOS_STATUS estatus = MOS_STATUS_SUCCESS;

    //Get 4x buffer
    m_trackedBufCurr4xDsRecon = (MOS_SURFACE*)m_allocator->GetResource(m_standard, ds4xRecon, m_trackedBufCurrIdx);

    if (m_trackedBufCurr4xDsRecon == nullptr)  {
        //SURFACE IS NOT ALLOCATED
        return estatus;
    }

    //Get 8x buffer
    m_trackedBufCurr8xDsRecon = (MOS_SURFACE*)m_allocator->GetResource(m_standard, ds8xRecon, m_trackedBufCurrIdx);

    if (m_trackedBufCurr8xDsRecon == nullptr) {
        //SURFACE IS NOT ALLOCATED
        return estatus;
    }
    uint32_t allocated4xWidth = m_trackedBufCurr4xDsRecon->dwWidth;
    uint32_t allocated4xHeight = m_trackedBufCurr4xDsRecon->dwHeight;

    uint32_t allocated8xWidth = m_trackedBufCurr8xDsRecon->dwWidth;
    uint32_t allocated8xHeight = m_trackedBufCurr8xDsRecon->dwHeight;

    uint32_t requiredDownscaledSurfaceWidth4x = m_encoder->m_downscaledWidthInMb4x * CODECHAL_MACROBLOCK_WIDTH;
    // Account for field case, offset needs to be 4K aligned if tiled for DI surface state.
    // Width will be allocated tile Y aligned, so also tile align height.
    uint32_t requiredDownscaledSurfaceHeight4x = ((m_encoder->m_downscaledHeightInMb4x + 1) >> 1) * CODECHAL_MACROBLOCK_HEIGHT;
    requiredDownscaledSurfaceHeight4x = MOS_ALIGN_CEIL(requiredDownscaledSurfaceHeight4x, MOS_YTILE_H_ALIGNMENT) << 1;

    uint32_t requiredDownscaledSurfaceWidth8x = requiredDownscaledSurfaceWidth4x >> 1;
    uint32_t requiredDownscaledSurfaceHeight8x = requiredDownscaledSurfaceHeight4x >> 1;

    if (requiredDownscaledSurfaceHeight4x != allocated4xHeight || requiredDownscaledSurfaceWidth4x != allocated4xWidth)
    {
        //re-allocate 4xDsRecon based on current resolution
        m_allocator->ReleaseResource(m_standard, ds4xRecon, m_trackedBufCurrIdx);
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_trackedBufCurr4xDsRecon = (MOS_SURFACE*)m_allocator->AllocateResource(
            m_standard, requiredDownscaledSurfaceWidth4x, requiredDownscaledSurfaceHeight4x, ds4xRecon, "ds4xRecon", m_trackedBufCurrIdx, false, Format_NV12, MOS_TILE_Y));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(m_osInterface, m_trackedBufCurr4xDsRecon));
    }

    if (requiredDownscaledSurfaceHeight8x != allocated8xHeight || requiredDownscaledSurfaceWidth8x != allocated8xWidth)
    {
        //re-allocate 8xDsRecon based on current resolution
        m_allocator->ReleaseResource(m_standard, ds8xRecon, m_trackedBufCurrIdx);
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_trackedBufCurr8xDsRecon = (MOS_SURFACE*)m_allocator->AllocateResource(
            m_standard, requiredDownscaledSurfaceWidth8x, requiredDownscaledSurfaceHeight8x, ds8xRecon, "ds8xRecon", m_trackedBufCurrIdx, false, Format_NV12, MOS_TILE_Y));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(m_osInterface, m_trackedBufCurr8xDsRecon));
    }
    return estatus;
}

MOS_STATUS CodechalEncodeTrackedBuffer::AllocateDsReconSurfacesVdenc()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    // early exit if already allocated
    if ((m_trackedBufCurr4xDsRecon = (MOS_SURFACE*)m_allocator->GetResource(m_standard, ds4xRecon, m_trackedBufCurrIdx)))
    {
        m_trackedBufCurr8xDsRecon = (MOS_SURFACE*)m_allocator->GetResource(m_standard, ds8xRecon, m_trackedBufCurrIdx);
        return MOS_STATUS_SUCCESS;
    }

    // MB-alignment not required since dataport handles out-of-bound pixel replication, but HW IME requires this.
    uint32_t downscaledSurfaceWidth4x = m_encoder->m_downscaledWidthInMb4x * CODECHAL_MACROBLOCK_WIDTH;
    // Account for field case, offset needs to be 4K aligned if tiled for DI surface state.
    // Width will be allocated tile Y aligned, so also tile align height.
    uint32_t downscaledSurfaceHeight4x = ((m_encoder->m_downscaledHeightInMb4x + 1) >> 1) * CODECHAL_MACROBLOCK_HEIGHT;
    downscaledSurfaceHeight4x = MOS_ALIGN_CEIL(downscaledSurfaceHeight4x, MOS_YTILE_H_ALIGNMENT) << 1;

    // Allocating VDEnc 4x DsRecon surface
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_trackedBufCurr4xDsRecon = (MOS_SURFACE*)m_allocator->AllocateResource(
        m_standard, downscaledSurfaceWidth4x, downscaledSurfaceHeight4x, ds4xRecon, "ds4xRecon", m_trackedBufCurrIdx, false, Format_NV12, MOS_TILE_Y));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(m_osInterface, m_trackedBufCurr4xDsRecon));

    // Allocating VDEnc 8x DsRecon surfaces
    uint32_t downscaledSurfaceWidth8x  = downscaledSurfaceWidth4x >> 1;
    uint32_t downscaledSurfaceHeight8x = MOS_ALIGN_CEIL(downscaledSurfaceHeight4x >> 1, MOS_YTILE_H_ALIGNMENT) << 1;
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_trackedBufCurr8xDsRecon = (MOS_SURFACE*)m_allocator->AllocateResource(
        m_standard, downscaledSurfaceWidth8x, downscaledSurfaceHeight8x, ds8xRecon, "ds8xRecon", m_trackedBufCurrIdx, false, Format_NV12, MOS_TILE_Y));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(m_osInterface, m_trackedBufCurr8xDsRecon));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeTrackedBuffer::AllocateMvTemporalBuffer(uint8_t bufIndex)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_encoder->m_vdencEnabled && m_encoder->m_vdencMvTemporalBufferSize)
    {
        // Allocate/fetch buffer if current frame can be used for reference or it's a special buffer for pre-generated I frame MV streamout surface
        if((m_encoder->m_currRefList && m_encoder->m_currRefList->bUsedAsRef) || bufIndex == CODEC_NUM_REF_BUFFERS)
        {
            if ((m_trackedBufCurrMvTemporal = (MOS_RESOURCE*)m_allocator->GetResource(m_standard, mvTemporalBuffer, bufIndex)))
            {
                return MOS_STATUS_SUCCESS;
            }

            CODECHAL_ENCODE_CHK_NULL_RETURN(m_trackedBufCurrMvTemporal = (MOS_RESOURCE*)m_allocator->AllocateResource(
                m_standard, m_encoder->m_vdencMvTemporalBufferSize, 1, mvTemporalBuffer, "mvTemporalBuffer", bufIndex, false));
        }
    }

    return MOS_STATUS_SUCCESS;
}

void CodechalEncodeTrackedBuffer::ReleaseMbCode(uint8_t bufIndex)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_allocator->ReleaseResource(m_standard, mbCodeBuffer, bufIndex);
}

void CodechalEncodeTrackedBuffer::ReleaseMvData(uint8_t bufIndex)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_allocator->ReleaseResource(m_standard, mvDataBuffer, bufIndex);
}

void CodechalEncodeTrackedBuffer::ReleaseSurfaceCsc(uint8_t bufIndex)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_allocator->ReleaseResource(m_standard, cscSurface, bufIndex);
}

void CodechalEncodeTrackedBuffer::ReleaseSurfaceDS(uint8_t bufIndex)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_allocator->ReleaseResource(m_standard, ds4xSurface, bufIndex);
    m_allocator->ReleaseResource(m_standard, ds2xSurface, bufIndex);
    m_allocator->ReleaseResource(m_standard, ds16xSurface, bufIndex);
    m_allocator->ReleaseResource(m_standard, ds32xSurface, bufIndex);
}

void CodechalEncodeTrackedBuffer::ReleaseDsRecon(uint8_t bufIndex)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_allocator->ReleaseResource(m_standard, ds4xRecon, bufIndex);
    m_allocator->ReleaseResource(m_standard, ds8xRecon, bufIndex);
}

CodechalEncodeTrackedBuffer::CodechalEncodeTrackedBuffer(CodechalEncoderState* encoder)
{
    // Initilize interface pointers
    m_encoder = encoder;
    m_allocator = encoder->m_allocator;
    m_standard = encoder->m_standard;
    m_osInterface = encoder->GetOsInterface();
    m_mbCodeIsTracked = true;

    for (auto i = 0; i < CODEC_NUM_TRACKED_BUFFERS; i++)
    {
        // Init all tracked buffer slots to usable
        m_tracker[i].bUsedforCurFrame = false;
        m_tracker[i].ucSurfIndex7bits = PICTURE_MAX_7BITS;
    }
}

CodechalEncodeTrackedBuffer::~CodechalEncodeTrackedBuffer()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
}
