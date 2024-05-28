/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     decode_vp9_basic_feature.cpp
//! \brief    Defines the common interface for decode vp9 parameter
//!

#include "decode_vp9_basic_feature.h"
#include "decode_utils.h"
#include "decode_allocator.h"
#include "decode_resource_auto_lock.h"
#include "mos_os_cp_interface_specific.h"

namespace decode
{
Vp9BasicFeature::Vp9BasicFeature(DecodeAllocator *allocator, void *hwInterface, PMOS_INTERFACE osInterface) : DecodeBasicFeature(allocator, hwInterface, osInterface)
{
    if (hwInterface != nullptr)
    {
        m_osInterface  = osInterface;
        m_hcpItf       = ((CodechalHwInterfaceNext*)hwInterface)->GetHcpInterfaceNext();
    }

    for (uint8_t i = 0; i < CODEC_VP9_NUM_CONTEXTS; i++)
    {
        m_pendingResetFullTables[i] = 0;
        m_pendingCopySegProbs[i]    = 0;
    }

    MOS_ZeroMemory(&m_resVp9SegmentIdBuffer, sizeof(m_resVp9SegmentIdBuffer));
    MOS_ZeroMemory(&m_resVp9MvTemporalBuffer, sizeof(m_resVp9MvTemporalBuffer));

    MOS_ZeroMemory(&m_interProbSaved, sizeof(m_interProbSaved));
    MOS_ZeroMemory(&m_segTreeProbs, sizeof(m_segTreeProbs));
    MOS_ZeroMemory(&m_segPredProbs, sizeof(m_segPredProbs));
    MOS_ZeroMemory(&m_probUpdateFlags, sizeof(m_probUpdateFlags));

    MOS_ZeroMemory(&m_lastRefSurface, sizeof(m_lastRefSurface));
    MOS_ZeroMemory(&m_goldenRefSurface, sizeof(m_goldenRefSurface));
    MOS_ZeroMemory(&m_altRefSurface, sizeof(m_altRefSurface));
    MOS_ZeroMemory(&m_resDataBuffer, sizeof(m_resDataBuffer));

    m_prevFrameParams.value = 0;
}

Vp9BasicFeature::~Vp9BasicFeature()
{
    if (m_allocator != nullptr)
    {
        for (uint8_t i = 0; i < CODEC_VP9_NUM_CONTEXTS + 1; i++)
        {
            if (!m_allocator->ResourceIsNull(&m_resVp9ProbBuffer[i]->OsResource))
            {
                m_allocator->Destroy(m_resVp9ProbBuffer[i]);
            }
        }

        if (!m_allocator->ResourceIsNull(&m_resVp9SegmentIdBuffer->OsResource))
        {
            m_allocator->Destroy(m_resVp9SegmentIdBuffer);
        }
    }
}

MOS_STATUS Vp9BasicFeature::Init(void *setting)
{
    DECODE_FUNC_CALL();

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    DECODE_CHK_NULL(setting);

    DECODE_CHK_STATUS(DecodeBasicFeature::Init(setting));
    CodechalSetting *codecSettings = (CodechalSetting *)setting;

    if (codecSettings->lumaChromaDepth & CODECHAL_LUMA_CHROMA_DEPTH_8_BITS)
        m_vp9DepthIndicator = 0;
    if (codecSettings->lumaChromaDepth & CODECHAL_LUMA_CHROMA_DEPTH_10_BITS)
        m_vp9DepthIndicator = 1;
    if (codecSettings->lumaChromaDepth & CODECHAL_LUMA_CHROMA_DEPTH_12_BITS)
        m_vp9DepthIndicator = 2;

    DECODE_CHK_STATUS(m_refFrames.Init(this, *m_allocator));

    InitDefaultProbBufferTable();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9BasicFeature ::DetermineInternalBufferUpdate()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    DECODE_FUNC_CALL();

    bool    keyFrame       = !m_vp9PicParams->PicFlags.fields.frame_type;
    bool    intraOnly      = m_vp9PicParams->PicFlags.fields.intra_only;
    uint8_t curFrameCtxIdx = (uint8_t)m_vp9PicParams->PicFlags.fields.frame_context_idx;
    bool    isScaling      = ((m_vp9PicParams->FrameWidthMinus1 + 1) == m_prevFrmWidth) &&
                             ((m_vp9PicParams->FrameHeightMinus1 + 1) == m_prevFrmHeight)
                         ? false
                         : true;
    bool resetAll       = ((keyFrame ||
                         m_vp9PicParams->PicFlags.fields.error_resilient_mode) ||
                     (m_vp9PicParams->PicFlags.fields.reset_frame_context == 3 &&
                         m_vp9PicParams->PicFlags.fields.intra_only));
    bool resetSpecified = (m_vp9PicParams->PicFlags.fields.reset_frame_context == 2 &&
                           m_vp9PicParams->PicFlags.fields.intra_only);

    bool copySegProbs      = false;  // indicating if current frame's prob buffer need to update seg tree/pred probs.
    bool resetPartialTbl   = false;  // indicating if current frame need to do partial reset from offset 1667 to 2010.

    //propogate ProbUpdateFlags
    MOS_ZeroMemory(&m_probUpdateFlags, sizeof(m_probUpdateFlags));

    m_resetSegIdBuffer = keyFrame ||
                         isScaling ||
                         m_vp9PicParams->PicFlags.fields.error_resilient_mode ||
                         m_vp9PicParams->PicFlags.fields.intra_only;

    m_frameCtxIdx = curFrameCtxIdx;  //indicate which prob buffer need to be used by current frame decode
    if (!m_vp9PicParams->PicFlags.fields.frame_type ||
        m_vp9PicParams->PicFlags.fields.intra_only ||
        m_vp9PicParams->PicFlags.fields.error_resilient_mode)
    {
        //always use frame context idx 0 in this case
        m_frameCtxIdx = 0;
    }

    //check if seg tree/pred probs need to be updated in prob buffer of current frame
    //and also mark the flag bPendingResetSegProbs for other prob buffers

    if (m_vp9PicParams->PicFlags.fields.segmentation_enabled &&
        m_vp9PicParams->PicFlags.fields.segmentation_update_map)
    {
        copySegProbs = true;
        for (uint8_t ctxIdx = 0; ctxIdx < CODEC_VP9_NUM_CONTEXTS; ctxIdx++)
        {
            m_pendingCopySegProbs[ctxIdx] = true;
        }
        //set current frame's prob buffer pending copy to false
        m_pendingCopySegProbs[m_frameCtxIdx] = false;

        MOS_SecureMemcpy(m_segTreeProbs, 7, m_vp9PicParams->SegTreeProbs, 7);
        MOS_SecureMemcpy(m_segPredProbs, 3, m_vp9PicParams->SegPredProbs, 3);
    }
    else if (m_vp9PicParams->PicFlags.fields.segmentation_enabled &&
             m_pendingCopySegProbs[m_frameCtxIdx])
    {
        copySegProbs                         = true;
        m_pendingCopySegProbs[m_frameCtxIdx] = false;
    }

    //check if probs in frame context table need to be updated for current frame's prob buffer
    //and also mark the flag bPendingResetFullTables for other prob buffers
    if (resetAll)
    {
        m_probUpdateFlags.bResetFull = true;
        m_pendingResetPartial = (keyFrame || intraOnly);

        //prob buffer 0 will be used for current frame decoding
        for (uint8_t ctxIdx = 1; ctxIdx < CODEC_VP9_NUM_CONTEXTS; ctxIdx++)
        {
            m_pendingResetFullTables[ctxIdx] = true;
        }
        m_saveInterProbs = false;
    }
    else if (resetSpecified)
    {
        //intra only frame:prob buffer 0 will always be used for current frame decoding
        if (curFrameCtxIdx == 0)
        {
            //do prob table 0 reset
            m_probUpdateFlags.bResetFull = true;
            m_pendingResetPartial = true;
            m_saveInterProbs      = false;
        }
        else
        {
            //not do reset right now, pending the reset full table of specified ctx until a later frame use it to do decode
            m_pendingResetFullTables[curFrameCtxIdx] = true;
            if (!m_pendingResetPartial)
            {
                if (!m_saveInterProbs)
                {
                    m_probUpdateFlags.bProbSave = true;
                    m_saveInterProbs  = true;
                }
                resetPartialTbl = true;
            }
        }
    }
    else if (intraOnly)
    {
        //prob buffer 0 will be used for current frame decoding
        if (!m_pendingResetPartial)
        {
            if (!m_saveInterProbs)
            {
                m_probUpdateFlags.bProbSave = true;
                m_saveInterProbs  = true;
            }
            resetPartialTbl = true;
        }
    }
    else if (m_pendingResetFullTables[curFrameCtxIdx])
    {
        //here curFrameCtxIdx != 0, frame is inter frame
        m_probUpdateFlags.bResetFull             = true;
        m_pendingResetFullTables[curFrameCtxIdx] = false;
    }
    else if (curFrameCtxIdx == 0 && m_pendingResetPartial)
    {
        //here curFrameCtxIdx = 0, frame is inter frame
        resetPartialTbl       = true;
        m_pendingResetPartial = false;
    }
    else if (curFrameCtxIdx == 0 && m_saveInterProbs)
    {
        //here curFrameCtxIdx = 0, frame is inter frame
        m_probUpdateFlags.bProbRestore = true;
        m_saveInterProbs  = false;
    }

    //decide if prob buffer need to do a full udpate or partial upate
    if (m_probUpdateFlags.bResetFull && copySegProbs)
    {
        //update the whole prob buffer
        m_fullProbBufferUpdate = true;
    }
    else
    {
        //partial buffer update
        m_fullProbBufferUpdate = false;
    }
    
    if (copySegProbs)
    {
        m_probUpdateFlags.bSegProbCopy = true;
        MOS_SecureMemcpy(m_probUpdateFlags.SegTreeProbs, 7, m_segTreeProbs, 7);
        MOS_SecureMemcpy(m_probUpdateFlags.SegPredProbs, 3, m_segPredProbs, 3);
    }
    m_probUpdateFlags.bProbReset       = m_probUpdateFlags.bResetFull || resetPartialTbl;
    m_probUpdateFlags.bResetKeyDefault = (keyFrame || intraOnly);

    return eStatus;
}

MOS_STATUS Vp9BasicFeature ::AllocateSegmentBuffer()

{
    uint32_t widthInSb   = MOS_ROUNDUP_DIVIDE(m_width, CODEC_VP9_SUPER_BLOCK_WIDTH);
    uint32_t heightInSb  = MOS_ROUNDUP_DIVIDE(m_height, CODEC_VP9_SUPER_BLOCK_HEIGHT);
    uint8_t  maxBitDepth = 8 + m_vp9DepthIndicator * 2;

    mhw::vdbox::hcp::HcpBufferSizePar hcpBufSizeParam;
    MOS_ZeroMemory(&hcpBufSizeParam, sizeof(hcpBufSizeParam));
    hcpBufSizeParam.ucMaxBitDepth = maxBitDepth;
    hcpBufSizeParam.dwPicWidth    = widthInSb;
    hcpBufSizeParam.dwPicHeight   = heightInSb;

    // m_chromaFormat was initialized in decode_basic_feature.cpp and got from codechal settings
    hcpBufSizeParam.ucChromaFormat = m_chromaFormat;

    if (m_hcpItf->GetVp9BufferSize(mhw::vdbox::hcp::HCP_INTERNAL_BUFFER_TYPE::SEGMENT_ID, &hcpBufSizeParam) != MOS_STATUS_SUCCESS)
    {
        DECODE_ASSERTMESSAGE("Failed to get SegmentIdBuffer size.");
    }

    if (m_resVp9SegmentIdBuffer == nullptr)
    {
        m_resVp9SegmentIdBuffer = m_allocator->AllocateBuffer(
            hcpBufSizeParam.dwBufferSize, "Vp9SegmentIdBuffer", resourceInternalReadWriteCache, notLockableVideoMem);
        DECODE_CHK_NULL(m_resVp9SegmentIdBuffer);
    }
    else
    {
        DECODE_CHK_STATUS(m_allocator->Resize(
            m_resVp9SegmentIdBuffer, hcpBufSizeParam.dwBufferSize, notLockableVideoMem));
    }

    DECODE_CHK_NULL(m_resVp9SegmentIdBuffer);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9BasicFeature::Update(void *params)
{
    DECODE_FUNC_CALL();

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    DECODE_CHK_NULL(params);
    //DestSurface info dependency on m_vp9PicParams.
    CodechalDecodeParams *decodeParams = (CodechalDecodeParams *)params;
    m_vp9PicParams                     = static_cast<CODEC_VP9_PIC_PARAMS *>(decodeParams->m_picParams);
    DECODE_CHK_NULL(m_vp9PicParams);

    DECODE_CHK_STATUS(DecodeBasicFeature::Update(params));

    m_pictureCodingType = m_vp9PicParams->PicFlags.fields.frame_type ? P_TYPE : I_TYPE;

    m_vp9SegmentParams = static_cast<CODEC_VP9_SEGMENT_PARAMS *>(decodeParams->m_iqMatrixBuffer);
    DECODE_CHK_NULL(m_vp9SegmentParams);

    DECODE_CHK_STATUS(SetPictureStructs());

    m_vp9SliceParams = static_cast<CODEC_VP9_SLICE_PARAMS *>(decodeParams->m_sliceParams);

    // No BSBytesInBuffer is sent from App, driver here just give an estimation.
    if (m_vp9SliceParams != nullptr && m_vp9SliceParams->wBadSliceChopping != 0)
    {
        m_vp9PicParams->BSBytesInBuffer =
            (m_vp9PicParams->FrameWidthMinus1 + 1) * (m_vp9PicParams->FrameHeightMinus1 + 1) * 6;
    }

    //update bitstream size  for this picture :m_datasize
    DECODE_CHK_STATUS(SetRequiredBitstreamSize(m_vp9PicParams->BSBytesInBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9BasicFeature::SetRequiredBitstreamSize(uint32_t requiredSize)
{
    DECODE_FUNC_CALL();

    if (requiredSize > m_dataSize)
    {
        m_dataOffset = 0;
        m_dataSize   = MOS_ALIGN_CEIL(requiredSize, MHW_CACHELINE_SIZE);
    }

    DECODE_NORMALMESSAGE("Estimate bitstream size in this Frame: %u", requiredSize);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9BasicFeature::SetPictureStructs()
{
    DECODE_FUNC_CALL();

    m_curRenderPic = m_vp9PicParams->CurrPic;
    DECODE_CHK_COND(m_curRenderPic.FrameIdx >= CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9,
        "currPic.FrameIdx is out of range!");

    m_width  = (uint32_t)m_vp9PicParams->FrameWidthMinus1 + 1;
    m_height = (uint32_t)m_vp9PicParams->FrameHeightMinus1 + 1;

    m_frameWidthAlignedMinBlk  = MOS_ALIGN_CEIL(m_vp9PicParams->FrameWidthMinus1 + 1, CODEC_VP9_MIN_BLOCK_WIDTH);
    m_frameHeightAlignedMinBlk = MOS_ALIGN_CEIL(m_vp9PicParams->FrameHeightMinus1 + 1, CODEC_VP9_MIN_BLOCK_WIDTH);

    m_allocatedWidthInSb  = MOS_ROUNDUP_DIVIDE(m_width, CODEC_VP9_SUPER_BLOCK_WIDTH);
    m_allocatedHeightInSb = MOS_ROUNDUP_DIVIDE(m_height, CODEC_VP9_SUPER_BLOCK_HEIGHT);

    // Overwrite the actual surface height with the coded height and width of the frame
    // for VP9 since it's possible for a VP9 frame to change size during playback
    m_destSurface.dwWidth  = m_vp9PicParams->FrameWidthMinus1 + 1;
    m_destSurface.dwHeight = m_vp9PicParams->FrameHeightMinus1 + 1;


    //update MV temp buffer index
    if (m_vp9PicParams->PicFlags.fields.frame_type == CODEC_VP9_INTER_FRAME &&
        !m_vp9PicParams->PicFlags.fields.intra_only)
    {
        m_curMvTempBufIdx = (m_curMvTempBufIdx + 1) % CODECHAL_VP9_NUM_MV_BUFFERS;
        m_colMvTempBufIdx = (m_curMvTempBufIdx < 1) ? (CODECHAL_VP9_NUM_MV_BUFFERS - 1) : (m_curMvTempBufIdx - 1);
    }

    // Allocate segment buffer
    AllocateSegmentBuffer();
  
    // Allocate MV buffer 
    AllocateVP9MVBuffer();

    DECODE_CHK_STATUS(m_refFrames.UpdatePicture(*m_vp9PicParams));

    DECODE_CHK_STATUS(SetSegmentData());

    //set update flag
    DetermineInternalBufferUpdate();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9BasicFeature::SetSegmentData()
{
    DECODE_FUNC_CALL();

    if ((!m_vp9PicParams->filter_level))
    {
        PCODEC_VP9_SEG_PARAMS vp9SegData = &m_vp9SegmentParams->SegData[0];

        for (uint8_t i = 0; i < 8; i++)
        {
            *((uint32_t *)&vp9SegData->FilterLevel[0][0]) = 0;
            *((uint32_t *)&vp9SegData->FilterLevel[2][0]) = 0;
            vp9SegData++;  // Go on to next record.
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9BasicFeature::AllocateVP9MVBuffer()
{
    DECODE_FUNC_CALL();

    uint32_t widthInSb   = MOS_ROUNDUP_DIVIDE(m_width, CODEC_VP9_SUPER_BLOCK_WIDTH);
    uint32_t heightInSb  = MOS_ROUNDUP_DIVIDE(m_height, CODEC_VP9_SUPER_BLOCK_HEIGHT);
    uint8_t  maxBitDepth = 8 + m_vp9DepthIndicator * 2;

    mhw::vdbox::hcp::HcpBufferSizePar hcpBufSizeParam;
    MOS_ZeroMemory(&hcpBufSizeParam, sizeof(hcpBufSizeParam));
    hcpBufSizeParam.ucMaxBitDepth = maxBitDepth;
    hcpBufSizeParam.dwPicWidth    = widthInSb;
    hcpBufSizeParam.dwPicHeight   = heightInSb;

    // m_chromaFormat was initialized in decode_basic_feature.cpp and got from codechal settings
    hcpBufSizeParam.ucChromaFormat = m_chromaFormat;

    if (m_hcpItf->GetVp9BufferSize(mhw::vdbox::hcp::HCP_INTERNAL_BUFFER_TYPE::CURR_MV_TEMPORAL,
            &hcpBufSizeParam) != MOS_STATUS_SUCCESS)
    {
        DECODE_ASSERTMESSAGE("Failed to MvBuffer size.");
    }

    for (uint8_t i = 0; i < CODECHAL_VP9_NUM_MV_BUFFERS; i++)
    {
        if (m_resVp9MvTemporalBuffer[i] == nullptr)
        {
            m_resVp9MvTemporalBuffer[i] = m_allocator->AllocateBuffer(
                hcpBufSizeParam.dwBufferSize, "MvTemporalBuffer", resourceInternalReadWriteCache, notLockableVideoMem);
            DECODE_CHK_NULL(m_resVp9MvTemporalBuffer[i]);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_resVp9MvTemporalBuffer[i], hcpBufSizeParam.dwBufferSize, notLockableVideoMem));
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9BasicFeature ::InitDefaultProbBufferTable()
{
    DECODE_FUNC_CALL();

    for (uint8_t i = 0; i < CODEC_VP9_NUM_CONTEXTS + 1; i++)
    {
        if (m_osInterface->osCpInterface->IsHMEnabled())
        {
            m_resVp9ProbBuffer[i] = m_allocator->AllocateBuffer(
                MOS_ALIGN_CEIL(CODEC_VP9_PROB_MAX_NUM_ELEM, CODECHAL_PAGE_SIZE), "Vp9ProbabilityBuffer",
                resourceInternalRead, notLockableVideoMem);
            DECODE_CHK_NULL(m_resVp9ProbBuffer[i]);
        }
        else
        {
            m_resVp9ProbBuffer[i] = m_allocator->AllocateBuffer(
                MOS_ALIGN_CEIL(CODEC_VP9_PROB_MAX_NUM_ELEM, CODECHAL_PAGE_SIZE), "Vp9ProbabilityBuffer",
                resourceInternalRead, lockableVideoMem);
            DECODE_CHK_NULL(m_resVp9ProbBuffer[i]);

            ResourceAutoLock resLock(m_allocator, &m_resVp9ProbBuffer[i]->OsResource);
            auto             data = (uint8_t *)resLock.LockResourceForWrite();

            DECODE_CHK_NULL(data);

            MOS_ZeroMemory(data, CODEC_VP9_PROB_MAX_NUM_ELEM);
            //initialize seg_tree_prob and seg_pred_prob
            MOS_FillMemory((data + CODEC_VP9_SEG_PROB_OFFSET), 7, CODEC_VP9_MAX_PROB);
            MOS_FillMemory((data + CODEC_VP9_SEG_PROB_OFFSET + 7), 3, CODEC_VP9_MAX_PROB);\
        }
    }

    return MOS_STATUS_SUCCESS;
}

}  // namespace decode
