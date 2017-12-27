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
        ReleaseBufferOnResChange();
        m_trackedBufCountResize--;
    }

    // update the last 3 buffer index, find a new slot for current frame
    m_trackedBufAnteIdx = m_trackedBufPenuIdx;
    m_trackedBufPenuIdx = m_trackedBufCurrIdx;
    m_trackedBufCurrIdx = LookUpBufIndex(currRefList->RefList, currRefList->ucNumRef, currRefList->bUsedAsRef);

    CODECHAL_ENCODE_CHK_COND_RETURN(m_trackedBufCurrIdx >= CODEC_NUM_TRACKED_BUFFERS, "No tracked buffer is available!");

    // wait to re-use once # of non-ref slots being used reaches 3
    m_waitForTrackedBuffer = (m_trackedBufCurrIdx >= CODEC_NUM_REF_BUFFERS && m_trackedBufCountNonRef >= CODEC_NUM_NON_REF_BUFFERS);

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
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateDsReconSurfacesVdenc(m_trackedBufCurrIdx));
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
#ifndef _FULL_OPEN_SOURCE
            m_encoder->m_cscDsState->ReleaseSurfaceDS(i);
#endif
            // this slot can now be re-used
            m_trackedBuffer[i].ucSurfIndex7bits = PICTURE_MAX_7BITS;
        }
        else
        {
            m_trackedBuffer[i].ucSurfIndex7bits = PICTURE_RESIZE;
        }
    }

    return;
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
            PCODEC_TRACKED_BUFFER trackedBuffer = &m_trackedBuffer[i];
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
        m_trackedBuffer[index].ucSurfIndex7bits = m_encoder->m_currReconstructedPic.FrameIdx;
    }

    return index;
}

void CodechalEncodeTrackedBuffer::ReleaseBufferOnResChange()
{
    if ((m_trackedBufAnteIdx != m_trackedBufPenuIdx) &&
        (m_trackedBufAnteIdx != m_trackedBufCurrIdx))
    {
        ReleaseMbCode(m_trackedBufAnteIdx);
        ReleaseMvData(m_trackedBufAnteIdx);
        ReleaseDsRecon(m_trackedBufAnteIdx);
#ifndef _FULL_OPEN_SOURCE
        m_encoder->m_cscDsState->ReleaseSurfaceDS(m_trackedBufAnteIdx);
#endif
        m_trackedBuffer[m_trackedBufAnteIdx].ucSurfIndex7bits = PICTURE_MAX_7BITS;
        CODECHAL_ENCODE_NORMALMESSAGE("Tracked buffer = %d re-allocated", m_trackedBufAnteIdx);
    }
}

MOS_STATUS CodechalEncodeTrackedBuffer::AllocateMbCodeResources(uint8_t bufIndex)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_COND_RETURN(
        bufIndex >= CODEC_NUM_TRACKED_BUFFERS,
        "No MbCode buffer is available!");

    // early exit if already allocated
    if (m_trackedBufCurrMbCode = (MOS_RESOURCE*)m_allocator->GetResource(m_standard, mbCodeBuffer, bufIndex))
    {
        return MOS_STATUS_SUCCESS;
    }

    // Must reserve at least 8 cachelines after MI_BATCH_BUFFER_END_CMD since HW prefetch max 8 cachelines from BB everytime
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_trackedBufCurrMbCode = (MOS_RESOURCE*)m_allocator->AllocateResource(
        m_standard, m_encoder->m_mbCodeSize + 8 * CODECHAL_CACHELINE_SIZE, 1, mbCodeBuffer, bufIndex, true));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeTrackedBuffer::AllocateMvDataResources(uint8_t bufIndex)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    // early exit if already allocated
    if (m_trackedBufCurrMvData = (MOS_RESOURCE*)m_allocator->GetResource(m_standard, mvDataBuffer, bufIndex))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_trackedBufCurrMvData = (MOS_RESOURCE*)m_allocator->AllocateResource(
        m_standard, m_encoder->m_mvDataSize, 1, mvDataBuffer, bufIndex, true));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeTrackedBuffer::AllocateDsReconSurfacesVdenc(uint8_t bufIndex)
{
    CODECHAL_ENCODE_FUNCTION_ENTER; 
    
    // early exit if already allocated
    if (m_trackedBufCurr4xDsRecon = (MOS_SURFACE*)m_allocator->GetResource(m_standard, ds4xRecon, bufIndex))
    {
        m_trackedBufCurr8xDsRecon = (MOS_SURFACE*)m_allocator->GetResource(m_standard, ds8xRecon, bufIndex);
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
        m_standard, downscaledSurfaceWidth4x, downscaledSurfaceHeight4x, ds4xRecon, bufIndex, false, Format_NV12, MOS_TILE_Y));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(m_osInterface, m_trackedBufCurr4xDsRecon));

    // Allocating VDEnc 8x DsRecon surfaces
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_trackedBufCurr8xDsRecon = (MOS_SURFACE*)m_allocator->AllocateResource(
        m_standard, downscaledSurfaceWidth4x >> 1, downscaledSurfaceHeight4x >> 1, ds8xRecon, bufIndex, false, Format_NV12, MOS_TILE_Y));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(m_osInterface, m_trackedBufCurr8xDsRecon));

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
    m_trackedBuffer = encoder->m_trackedBuffer;
    m_mbCodeIsTracked = true;

    for (auto i = 0; i < CODEC_NUM_TRACKED_BUFFERS; i++)
    {
        // Init all tracked buffer slots to usable
        MOS_ZeroMemory(&m_trackedBuffer[i], sizeof(m_trackedBuffer[i]));
        m_trackedBuffer[i].ucSurfIndex7bits = PICTURE_MAX_7BITS;
    }
}

CodechalEncodeTrackedBuffer::~CodechalEncodeTrackedBuffer()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
}