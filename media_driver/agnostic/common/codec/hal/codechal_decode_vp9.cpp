/*
* Copyright (c) 2012-2018, Intel Corporation
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
//! \file     codechal_decode_vp9.cpp
//! \brief    Implements the decode interface extension for VP9.
//! \details  Implements all functions required by CodecHal for VP9 decoding.
//!

#include "codechal_decoder.h"
#include "codechal_secure_decode_interface.h"
#include "codechal_decode_vp9.h"
#include "codechal_mmc_decode_vp9.h"
#include "hal_oca_interface.h"
#if USE_CODECHAL_DEBUG_TOOL
#include <sstream>
#include <fstream>
#include "codechal_debug.h"
#endif

CodechalDecodeVp9 :: ~CodechalDecodeVp9 ()
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    m_osInterface->pfnDestroySyncResource(m_osInterface, &m_resSyncObject);
    m_osInterface->pfnDestroySyncResource(m_osInterface, &m_resSyncObjectWaContextInUse);
    m_osInterface->pfnDestroySyncResource(m_osInterface, &m_resSyncObjectVideoContextInUse);

    CodecHalFreeDataList(m_vp9RefList, CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9);

    if (!Mos_ResourceIsNull(&m_resDeblockingFilterLineRowStoreScratchBuffer))
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_resDeblockingFilterLineRowStoreScratchBuffer);
    }

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_resDeblockingFilterTileRowStoreScratchBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_resDeblockingFilterColumnRowStoreScratchBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_resMetadataLineBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_resMetadataTileLineBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_resMetadataTileColumnBuffer);

    if (!Mos_ResourceIsNull(&m_resHvcLineRowstoreBuffer))
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_resHvcLineRowstoreBuffer);
    }

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_resHvcTileRowstoreBuffer);

    for (uint8_t i = 0; i < CODEC_VP9_NUM_CONTEXTS + 1; i++)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_resVp9ProbBuffer[i]);
    }

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_resVp9SegmentIdBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_resSegmentIdBuffReset);

    for (uint8_t i = 0; i < CODECHAL_VP9_NUM_MV_BUFFERS; i++)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_resVp9MvTemporalBuffer[i]);
    }

    if (!Mos_ResourceIsNull(&m_resCopyDataBuffer))
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_resCopyDataBuffer);
    }

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_resHucSharedBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_resDmemBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_resInterProbSaveBuffer);

    if (m_picMhwParams.PipeModeSelectParams)
    {
        MOS_Delete(m_picMhwParams.PipeModeSelectParams);
        m_picMhwParams.PipeModeSelectParams = nullptr;
    }
    for (uint8_t i = 0; i < 4; i++)
    {
        if (m_picMhwParams.SurfaceParams[i])
        {
            MOS_Delete(m_picMhwParams.SurfaceParams[i]);
            m_picMhwParams.SurfaceParams[i] = nullptr;
        }
    }
    if (m_picMhwParams.PipeBufAddrParams)
    {
        MOS_Delete(m_picMhwParams.PipeBufAddrParams);
        m_picMhwParams.PipeBufAddrParams = nullptr;
    }
    if (m_picMhwParams.IndObjBaseAddrParams)
    {
        MOS_Delete(m_picMhwParams.IndObjBaseAddrParams);
        m_picMhwParams.IndObjBaseAddrParams = nullptr;
    }
    if (m_picMhwParams.Vp9PicState)
    {
        MOS_Delete(m_picMhwParams.Vp9PicState);
        m_picMhwParams.Vp9PicState = nullptr;
    }
    if (m_picMhwParams.Vp9SegmentState)
    {
        MOS_Delete(m_picMhwParams.Vp9SegmentState);
        m_picMhwParams.Vp9SegmentState = nullptr;
    }
}

CodechalDecodeVp9 ::CodechalDecodeVp9(
    CodechalHwInterface *   hwInterface,
    CodechalDebugInterface *debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo) : CodechalDecode(hwInterface, debugInterface, standardInfo),
                                            m_usFrameWidthAlignedMinBlk(0),
                                            m_usFrameHeightAlignedMinBlk(0),
                                            m_vp9DepthIndicator(0),
                                            m_chromaFormatinProfile(0),
                                            m_dataSize(0),
                                            m_dataOffset(0),
                                            m_frameCtxIdx(0),
                                            m_curMvTempBufIdx(0),
                                            m_colMvTempBufIdx(0),
                                            m_copyDataBufferSize(0),
                                            m_copyDataOffset(0),
                                            m_copyDataBufferInUse(false),
                                            m_hcpDecPhase(0),
                                            m_prevFrmWidth(0),
                                            m_prevFrmHeight(0),
                                            m_allocatedWidthInSb(0),
                                            m_allocatedHeightInSb(0),
                                            m_mvBufferSize(0),
                                            m_resetSegIdBuffer(false),
                                            m_pendingResetPartial(0),
                                            m_saveInterProbs(0),
                                            m_fullProbBufferUpdate(false),
                                            m_dmemBufferSize(0)
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    MOS_ZeroMemory(&m_resDeblockingFilterLineRowStoreScratchBuffer, sizeof(m_resDeblockingFilterLineRowStoreScratchBuffer));
    MOS_ZeroMemory(&m_resDeblockingFilterTileRowStoreScratchBuffer, sizeof(m_resDeblockingFilterTileRowStoreScratchBuffer));
    MOS_ZeroMemory(&m_resDeblockingFilterColumnRowStoreScratchBuffer, sizeof(m_resDeblockingFilterColumnRowStoreScratchBuffer));
    MOS_ZeroMemory(&m_resMetadataLineBuffer, sizeof(m_resMetadataLineBuffer));
    MOS_ZeroMemory(&m_resMetadataTileLineBuffer, sizeof(m_resMetadataTileLineBuffer));
    MOS_ZeroMemory(&m_resMetadataTileColumnBuffer, sizeof(m_resMetadataTileColumnBuffer));
    MOS_ZeroMemory(&m_resHvcLineRowstoreBuffer, sizeof(m_resHvcLineRowstoreBuffer));
    MOS_ZeroMemory(&m_resHvcTileRowstoreBuffer, sizeof(m_resHvcTileRowstoreBuffer));
    MOS_ZeroMemory(&m_resVp9SegmentIdBuffer, sizeof(m_resVp9SegmentIdBuffer));
    MOS_ZeroMemory(&m_resVp9MvTemporalBuffer, sizeof(m_resVp9MvTemporalBuffer));
    MOS_ZeroMemory(&m_resCopyDataBuffer, sizeof(m_resCopyDataBuffer));
    MOS_ZeroMemory(&m_segTreeProbs, sizeof(m_segTreeProbs));
    MOS_ZeroMemory(&m_segPredProbs, sizeof(m_segPredProbs));
    MOS_ZeroMemory(&m_interProbSaved, sizeof(m_interProbSaved));
    MOS_ZeroMemory(&m_resDmemBuffer, sizeof(m_resDmemBuffer));
    MOS_ZeroMemory(&m_resInterProbSaveBuffer, sizeof(m_resInterProbSaveBuffer));
    MOS_ZeroMemory(&m_probUpdateFlags, sizeof(m_probUpdateFlags));
    MOS_ZeroMemory(&m_resSegmentIdBuffReset, sizeof(m_resSegmentIdBuffReset));
    MOS_ZeroMemory(&m_resHucSharedBuffer, sizeof(m_resHucSharedBuffer));
    MOS_ZeroMemory(&m_picMhwParams, sizeof(m_picMhwParams));
    MOS_ZeroMemory(&m_destSurface, sizeof(m_destSurface));
    MOS_ZeroMemory(&m_lastRefSurface, sizeof(m_lastRefSurface));
    MOS_ZeroMemory(&m_goldenRefSurface, sizeof(m_goldenRefSurface));
    MOS_ZeroMemory(&m_altRefSurface, sizeof(m_altRefSurface));
    MOS_ZeroMemory(&m_resDataBuffer, sizeof(m_resDataBuffer));
    MOS_ZeroMemory(&m_resCoefProbBuffer, sizeof(m_resCoefProbBuffer));
    MOS_ZeroMemory(&m_resSyncObject, sizeof(m_resSyncObject));
    MOS_ZeroMemory(&m_resSyncObjectWaContextInUse, sizeof(m_resSyncObjectWaContextInUse));
    MOS_ZeroMemory(&m_resSyncObjectVideoContextInUse, sizeof(m_resSyncObjectVideoContextInUse));

    m_prevFrameParams.value = 0;

    for (uint8_t i = 0; i < CODEC_VP9_NUM_CONTEXTS; i++)
    {
        m_pendingResetFullTables[i] = 0;
        m_pendingCopySegProbs[i]    = 0;
    }

    m_hcpInUse = true;
}

MOS_STATUS CodechalDecodeVp9 :: ProbBufferPartialUpdatewithDrv()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CodechalResLock ResourceLock(m_osInterface, &m_resVp9ProbBuffer[m_frameCtxIdx]);
    auto data = (uint8_t*)ResourceLock.Lock(CodechalResLock::writeOnly);
    CODECHAL_DECODE_CHK_NULL_RETURN(data);

    if (m_probUpdateFlags.bSegProbCopy)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            (data + CODEC_VP9_SEG_PROB_OFFSET),
            7,
            m_probUpdateFlags.SegTreeProbs,
            7));
        CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            (data + CODEC_VP9_SEG_PROB_OFFSET + 7),
            3,
            m_probUpdateFlags.SegPredProbs,
            3));
    }

    if (m_probUpdateFlags.bProbSave)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            m_interProbSaved,
            CODECHAL_VP9_INTER_PROB_SIZE,
            data + CODEC_VP9_INTER_PROB_OFFSET,
            CODECHAL_VP9_INTER_PROB_SIZE));
    }

    if (m_probUpdateFlags.bProbReset)
    {
        if (m_probUpdateFlags.bResetFull)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(ContextBufferInit(
                data, (m_probUpdateFlags.bResetKeyDefault ? true : false)));
        }
        else
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(CtxBufDiffInit(
                data, (m_probUpdateFlags.bResetKeyDefault ? true : false)));
        }
    }

    if (m_probUpdateFlags.bProbRestore)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            data + CODEC_VP9_INTER_PROB_OFFSET,
            CODECHAL_VP9_INTER_PROB_SIZE,
            m_interProbSaved,
            CODECHAL_VP9_INTER_PROB_SIZE));
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeVp9 :: ProbBufFullUpdatewithDrv()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CodechalResLock ResourceLock(m_osInterface, &m_resVp9ProbBuffer[m_frameCtxIdx]);
    auto data = (uint8_t*)ResourceLock.Lock(CodechalResLock::writeOnly);
    CODECHAL_DECODE_CHK_NULL_RETURN(data);

    CODECHAL_DECODE_CHK_STATUS_RETURN(ContextBufferInit(
        data, (m_probUpdateFlags.bResetKeyDefault ? true : false)));
    CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
        (data + CODEC_VP9_SEG_PROB_OFFSET),
        7,
        m_probUpdateFlags.SegTreeProbs,
        7));
    CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
        (data + CODEC_VP9_SEG_PROB_OFFSET + 7),
        3,
        m_probUpdateFlags.SegPredProbs,
        3));

    return eStatus;
}

MOS_STATUS CodechalDecodeVp9 :: ResetSegIdBufferwithDrv()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CodechalResLock ResourceLock(m_osInterface, &m_resVp9SegmentIdBuffer);
    auto data = (uint8_t*)ResourceLock.Lock(CodechalResLock::writeOnly);
    CODECHAL_DECODE_CHK_NULL_RETURN(data);

    MOS_ZeroMemory(
        data,
        m_allocatedWidthInSb * m_allocatedHeightInSb * CODECHAL_CACHELINE_SIZE);

    return eStatus;
}

MOS_STATUS CodechalDecodeVp9 :: ProbBufFullUpdatewithHucStreamout(
    PMOS_COMMAND_BUFFER         cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    m_osInterface->pfnSetPerfTag(
        m_osInterface,
        (uint16_t)(((m_mode << 4) & 0xF0) | COPY_TYPE));
    m_osInterface->pfnResetPerfBufferID(m_osInterface);

    uint32_t bufSize = CODEC_VP9_PROB_MAX_NUM_ELEM; // 16 byte aligned

    CodechalResLock ResourceLock(m_osInterface, &m_resVp9ProbBuffer[CODEC_VP9_NUM_CONTEXTS]);
    auto data = (uint8_t*)ResourceLock.Lock(CodechalResLock::writeOnly);
    CODECHAL_DECODE_CHK_NULL_RETURN(data);

    CODECHAL_DECODE_CHK_STATUS_RETURN(ContextBufferInit(
        data,
        (m_probUpdateFlags.bResetKeyDefault ? true : false)));
    CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
        (data + CODEC_VP9_SEG_PROB_OFFSET),
        7,
        m_probUpdateFlags.SegTreeProbs,
        7));
    CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
        (data + CODEC_VP9_SEG_PROB_OFFSET + 7),
        3,
        m_probUpdateFlags.SegPredProbs,
        3));

    CODECHAL_DECODE_CHK_STATUS_RETURN(HucCopy(
        cmdBuffer,                                    // cmdBuffer
        &m_resVp9ProbBuffer[CODEC_VP9_NUM_CONTEXTS],  // presSrc
        &m_resVp9ProbBuffer[m_frameCtxIdx],           // presDst
        bufSize,                                      // u32CopyLength
        0,                                            // u32CopyInputOffset
        0));                                          // u32CopyOutputOffset

    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
        cmdBuffer,
        &flushDwParams));

    return eStatus;
}

MOS_STATUS CodechalDecodeVp9 :: ResetSegIdBufferwithHucStreamout(
    PMOS_COMMAND_BUFFER         cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    m_osInterface->pfnSetPerfTag(
        m_osInterface,
        (uint16_t)(((m_mode << 4) & 0xF0) | COPY_TYPE));
    m_osInterface->pfnResetPerfBufferID(m_osInterface);

    uint32_t bufSize =
        m_allocatedWidthInSb * m_allocatedHeightInSb * CODECHAL_CACHELINE_SIZE;  // 16 byte aligned

    CODECHAL_DECODE_CHK_STATUS_RETURN(HucCopy(
        cmdBuffer,                 // cmdBuffer
        &m_resSegmentIdBuffReset,  // presSrc
        &m_resVp9SegmentIdBuffer,  // presDst
        bufSize,                   // u32CopyLength
        0,                         // u32CopyInputOffset
        0));                       // u32CopyOutputOffset

    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
        cmdBuffer,
        &flushDwParams));

    return eStatus;
}

MOS_STATUS CodechalDecodeVp9 :: DetermineInternalBufferUpdate()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    bool    keyFrame       = !m_vp9PicParams->PicFlags.fields.frame_type;
    bool    intraOnly      = m_vp9PicParams->PicFlags.fields.intra_only;
    uint8_t curFrameCtxIdx = (uint8_t)m_vp9PicParams->PicFlags.fields.frame_context_idx;
    bool    isScaling      = (m_destSurface.dwWidth == m_prevFrmWidth) &&
                             (m_destSurface.dwHeight == m_prevFrmHeight)
                         ? false
                         : true;
    bool resetAll       = (keyFrame |
                         m_vp9PicParams->PicFlags.fields.error_resilient_mode ||
                     (m_vp9PicParams->PicFlags.fields.reset_frame_context == 3 &&
                         m_vp9PicParams->PicFlags.fields.intra_only));
    bool resetSpecified = (m_vp9PicParams->PicFlags.fields.reset_frame_context == 2 &&
                           m_vp9PicParams->PicFlags.fields.intra_only);

    bool copySegProbs       = false;  // indicating if current frame's prob buffer need to update seg tree/pred probs.
    bool resetFullTbl       = false;  // indicating if current frame will do full frame context table reset
    bool resetPartialTbl    = false;  // indicating if current frame need to do partial reset from offset 1667 to 2010.
    bool restoreInterProbs  = false;  // indicating if current frame need to restore offset 1667 to 2010 from saved one, this is only for prob buffer 0.
    bool saveInterProbsTmp  = false;  // indicating if current frame need to save offset from 1667 to 2010 for prob buffer 0.

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
        for ( uint8_t ctxIdx = 0; ctxIdx < CODEC_VP9_NUM_CONTEXTS; ctxIdx++)
        {
            m_pendingCopySegProbs[ctxIdx] = true;
        }
        //set current frame's prob buffer pending copy to false
        m_pendingCopySegProbs[m_frameCtxIdx] = false;

        //save probs for pending copy
        MOS_SecureMemcpy(m_segTreeProbs, 7, m_vp9PicParams->SegTreeProbs, 7);
        MOS_SecureMemcpy(m_segPredProbs, 3, m_vp9PicParams->SegPredProbs, 3);
    }
    else if (m_vp9PicParams->PicFlags.fields.segmentation_enabled &&
             m_pendingCopySegProbs[m_frameCtxIdx])
    {
        copySegProbs = true;
        m_pendingCopySegProbs[m_frameCtxIdx] = false;
    }

    //check if probs in frame context table need to be updated for current frame's prob buffer
    //and also mark the flag bPendingResetFullTables for other prob buffers
    if (resetAll)
    {
        resetFullTbl = true;
        m_pendingResetPartial = (keyFrame || intraOnly);

        //prob buffer 0 will be used for current frame decoding
        for ( uint8_t ctxIdx = 1; ctxIdx < CODEC_VP9_NUM_CONTEXTS; ctxIdx++)
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
            resetFullTbl = true;
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
                    saveInterProbsTmp = true;
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
                saveInterProbsTmp = true;
                m_saveInterProbs  = true;
            }
            resetPartialTbl = true;
        }
    }
    else if (m_pendingResetFullTables[curFrameCtxIdx])
    {
        //here curFrameCtxIdx != 0, frame is inter frame
        resetFullTbl = true;
        m_pendingResetFullTables[curFrameCtxIdx] = false;
    }
    else if (curFrameCtxIdx == 0 && m_pendingResetPartial)
    {
        //here curFrameCtxIdx = 0, frame is inter frame
        resetPartialTbl = true;
        m_pendingResetPartial = false;
    }
    else if (curFrameCtxIdx == 0 && m_saveInterProbs)
    {
        //here curFrameCtxIdx = 0, frame is inter frame
        restoreInterProbs = true;
        m_saveInterProbs  = false;
    }

    //decide if prob buffer need to do a full udpate or partial upate
    if (resetFullTbl && copySegProbs)
    {
        //update the whole prob buffer
        m_fullProbBufferUpdate = true;
    }
    else
    {
        //partial buffer update
        m_fullProbBufferUpdate = false;
    }

    //propogate ProbUpdateFlags
    MOS_ZeroMemory(&m_probUpdateFlags, sizeof(m_probUpdateFlags));
    if (copySegProbs)
    {
        m_probUpdateFlags.bSegProbCopy = true;
        MOS_SecureMemcpy(m_probUpdateFlags.SegTreeProbs, 7, m_segTreeProbs, 7);
        MOS_SecureMemcpy(m_probUpdateFlags.SegPredProbs, 3, m_segPredProbs, 3);
    }
    m_probUpdateFlags.bProbReset       = resetFullTbl || resetPartialTbl;
    m_probUpdateFlags.bResetFull       = resetFullTbl;
    m_probUpdateFlags.bResetKeyDefault = (keyFrame || intraOnly);
    m_probUpdateFlags.bProbSave        = saveInterProbsTmp;
    m_probUpdateFlags.bProbRestore     = restoreInterProbs;

    return eStatus;
}

MOS_STATUS CodechalDecodeVp9 :: AllocateResourcesFixedSizes()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateSyncResource(
        m_osInterface, &m_resSyncObject));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateSyncResource(
        m_osInterface, &m_resSyncObjectWaContextInUse));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateSyncResource(
        m_osInterface, &m_resSyncObjectVideoContextInUse));

    CodecHalAllocateDataList(
        m_vp9RefList,
        CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9);

    // VP9 Probability buffer
    for (uint8_t i = 0; i < CODEC_VP9_NUM_CONTEXTS + 1; i++)
    {
        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                      &m_resVp9ProbBuffer[i],
                                                      MOS_ALIGN_CEIL(CODEC_VP9_PROB_MAX_NUM_ELEM, CODECHAL_PAGE_SIZE),
                                                      "Vp9ProbabilityBuffer"),
            "Failed to allocate VP9 probability Buffer.");

        CodechalResLock ResourceLock(m_osInterface, &m_resVp9ProbBuffer[i]);
        auto data = (uint8_t*)ResourceLock.Lock(CodechalResLock::writeOnly);
        CODECHAL_DECODE_CHK_NULL_RETURN(data);

        MOS_ZeroMemory(data, CODEC_VP9_PROB_MAX_NUM_ELEM);
        //initialize seg_tree_prob and seg_pred_prob
        MOS_FillMemory((data + CODEC_VP9_SEG_PROB_OFFSET), 7, CODEC_VP9_MAX_PROB);
        MOS_FillMemory((data + CODEC_VP9_SEG_PROB_OFFSET + 7), 3, CODEC_VP9_MAX_PROB);
    }

    // DMEM buffer send to HuC FW
    m_dmemBufferSize = MOS_ALIGN_CEIL(sizeof(CODECHAL_DECODE_VP9_PROB_UPDATE), CODECHAL_CACHELINE_SIZE);
    CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                  &m_resDmemBuffer,
                                                  m_dmemBufferSize,
                                                  "DmemBuffer"),
        "Failed to allocate Dmem Buffer.");

    // VP9 Interprobs save buffer
    CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                  &m_resInterProbSaveBuffer,
                                                  MOS_ALIGN_CEIL(CODECHAL_VP9_INTER_PROB_SIZE, CODECHAL_PAGE_SIZE),
                                                  "VP9InterProbsSaveBuffer"),
        "Failed to allocate VP9 inter probability save Buffer.");

    // VP9 shared buffer with HuC FW, mapping to region 15
    CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                  &m_resHucSharedBuffer,
                                                  MOS_ALIGN_CEIL(CODEC_VP9_PROB_MAX_NUM_ELEM, CODECHAL_PAGE_SIZE),
                                                  "VP9HucSharedBuffer"),
        "Failed to allocate VP9 Huc shared Buffer.");

    return eStatus;
}

MOS_STATUS CodechalDecodeVp9 :: AllocateResourcesVariableSizes()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    uint32_t widthInSb   = MOS_ROUNDUP_DIVIDE(m_width, CODEC_VP9_SUPER_BLOCK_WIDTH);
    uint32_t heightInSb  = MOS_ROUNDUP_DIVIDE(m_height, CODEC_VP9_SUPER_BLOCK_HEIGHT);
    uint8_t  maxBitDepth  = 8 + m_vp9DepthIndicator * 2;
    uint8_t  chromaFormat = m_chromaFormatinProfile;

    MHW_VDBOX_HCP_BUFFER_SIZE_PARAMS    hcpBufSizeParam;
    MOS_ZeroMemory(&hcpBufSizeParam, sizeof(hcpBufSizeParam));
    hcpBufSizeParam.ucMaxBitDepth  = maxBitDepth;
    hcpBufSizeParam.ucChromaFormat = chromaFormat;
    hcpBufSizeParam.dwPicWidth     = widthInSb;
    hcpBufSizeParam.dwPicHeight    = heightInSb;

    MHW_VDBOX_HCP_BUFFER_REALLOC_PARAMS reallocParam;
    MOS_ZeroMemory(&reallocParam, sizeof(reallocParam));
    reallocParam.ucMaxBitDepth      = maxBitDepth;
    reallocParam.ucChromaFormat     = chromaFormat;
    reallocParam.dwPicWidth         = widthInSb;
    reallocParam.dwPicWidthAlloced  = m_allocatedWidthInSb;
    reallocParam.dwPicHeight        = heightInSb;
    reallocParam.dwPicHeightAlloced = m_allocatedHeightInSb;

    if (!m_hcpInterface->IsVp9DfRowstoreCacheEnabled())
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->IsVp9BufferReallocNeeded(
            MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_LINE,
            &reallocParam));
        if (reallocParam.bNeedBiggerSize ||
            Mos_ResourceIsNull(&m_resDeblockingFilterLineRowStoreScratchBuffer))
        {
            if (!Mos_ResourceIsNull(&m_resDeblockingFilterLineRowStoreScratchBuffer))
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &m_resDeblockingFilterLineRowStoreScratchBuffer);
            }

            // Deblocking Filter Line Row Store Scratch data surface
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->GetVp9BufferSize(
                MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_LINE,
                &hcpBufSizeParam));

            CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                          &m_resDeblockingFilterLineRowStoreScratchBuffer,
                                                          hcpBufSizeParam.dwBufferSize,
                                                          "DeblockingLineScratchBuffer"),
                "Failed to allocate deblocking line scratch Buffer.");
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->IsVp9BufferReallocNeeded(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_LINE,
        &reallocParam));
    if (reallocParam.bNeedBiggerSize ||
        Mos_ResourceIsNull(&m_resDeblockingFilterTileRowStoreScratchBuffer))
    {
        if (!Mos_ResourceIsNull(&m_resDeblockingFilterTileRowStoreScratchBuffer))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_resDeblockingFilterTileRowStoreScratchBuffer);
        }

        // Deblocking Filter Tile Row Store Scratch data surface
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->GetVp9BufferSize(
            MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_LINE,
            &hcpBufSizeParam));

        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                      &m_resDeblockingFilterTileRowStoreScratchBuffer,
                                                      hcpBufSizeParam.dwBufferSize,
                                                      "DeblockingTileScratchBuffer"),
            "Failed to allocate deblocking tile scratch Buffer.");
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->IsVp9BufferReallocNeeded(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_COL,
        &reallocParam));
    if (reallocParam.bNeedBiggerSize ||
        Mos_ResourceIsNull(&m_resDeblockingFilterColumnRowStoreScratchBuffer))
    {
        if (!Mos_ResourceIsNull(&m_resDeblockingFilterColumnRowStoreScratchBuffer))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_resDeblockingFilterColumnRowStoreScratchBuffer);
        }
        // Deblocking Filter Column Row Store Scratch data surface
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->GetVp9BufferSize(
            MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_COL,
            &hcpBufSizeParam));

        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                      &m_resDeblockingFilterColumnRowStoreScratchBuffer,
                                                      hcpBufSizeParam.dwBufferSize,
                                                      "DeblockingColumnScratchBuffer"),
            "Failed to allocate deblocking column scratch Buffer.");
    }

    if (!m_hcpInterface->IsVp9DatRowstoreCacheEnabled())
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->IsVp9BufferReallocNeeded(
            MHW_VDBOX_HCP_INTERNAL_BUFFER_META_LINE,
            &reallocParam));
        if (reallocParam.bNeedBiggerSize ||
            Mos_ResourceIsNull(&m_resMetadataLineBuffer))
        {
            if (!Mos_ResourceIsNull(&m_resMetadataLineBuffer))
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &m_resMetadataLineBuffer);
            }

            // Metadata Line buffer
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->GetVp9BufferSize(
                MHW_VDBOX_HCP_INTERNAL_BUFFER_META_LINE,
                &hcpBufSizeParam));

            CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                          &m_resMetadataLineBuffer,
                                                          hcpBufSizeParam.dwBufferSize,
                                                          "MetadataLineBuffer"),
                "Failed to allocate meta data line Buffer.");
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->IsVp9BufferReallocNeeded(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_LINE,
        &reallocParam));
    if (reallocParam.bNeedBiggerSize ||
        Mos_ResourceIsNull(&m_resMetadataTileLineBuffer))
    {
        if (!Mos_ResourceIsNull(&m_resMetadataTileLineBuffer))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_resMetadataTileLineBuffer);
        }
        // Metadata Tile Line buffer
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->GetVp9BufferSize(
            MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_LINE,
            &hcpBufSizeParam));

        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                      &m_resMetadataTileLineBuffer,
                                                      hcpBufSizeParam.dwBufferSize,
                                                      "MetadataTileLineBuffer"),
            "Failed to allocate meta data tile line Buffer.");
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->IsVp9BufferReallocNeeded(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_COL,
        &reallocParam));
    if (reallocParam.bNeedBiggerSize ||
        Mos_ResourceIsNull(&m_resMetadataTileColumnBuffer))
    {
        if (!Mos_ResourceIsNull(&m_resMetadataTileColumnBuffer))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_resMetadataTileColumnBuffer);
        }
        // Metadata Tile Column buffer
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->GetVp9BufferSize(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_COL,
        &hcpBufSizeParam));

        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                      &m_resMetadataTileColumnBuffer,
                                                      hcpBufSizeParam.dwBufferSize,
                                                      "MetadataTileColumnBuffer"),
            "Failed to allocate meta data tile column Buffer.");
    }

    if (!m_hcpInterface->IsVp9HvdRowstoreCacheEnabled())
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->IsVp9BufferReallocNeeded(
            MHW_VDBOX_VP9_INTERNAL_BUFFER_HVD_LINE,
            &reallocParam));
        if (reallocParam.bNeedBiggerSize ||
            Mos_ResourceIsNull(&m_resHvcLineRowstoreBuffer))
        {
            if (!Mos_ResourceIsNull(&m_resHvcLineRowstoreBuffer))
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &m_resHvcLineRowstoreBuffer);
            }

            // HVC Line Row Store Buffer
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->GetVp9BufferSize(
                MHW_VDBOX_VP9_INTERNAL_BUFFER_HVD_LINE,
                &hcpBufSizeParam));

            CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                          &m_resHvcLineRowstoreBuffer,
                                                          hcpBufSizeParam.dwBufferSize,
                                                          "HvcLineRowStoreBuffer"),
                "Failed to allocate Hvc line row store Buffer.");
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->IsVp9BufferReallocNeeded(
        MHW_VDBOX_VP9_INTERNAL_BUFFER_HVD_TILE,
        &reallocParam));
    if (reallocParam.bNeedBiggerSize ||
        Mos_ResourceIsNull(&m_resHvcTileRowstoreBuffer))
    {
        if (!Mos_ResourceIsNull(&m_resHvcTileRowstoreBuffer))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_resHvcTileRowstoreBuffer);
        }
        // HVC Tile Row Store Buffer
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->GetVp9BufferSize(
            MHW_VDBOX_VP9_INTERNAL_BUFFER_HVD_TILE,
            &hcpBufSizeParam));

        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                      &m_resHvcTileRowstoreBuffer,
                                                      hcpBufSizeParam.dwBufferSize,
                                                      "HvcTileRowStoreBuffer"),
            "Failed to allocate Hvc tile row store Buffer.");
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->IsVp9BufferReallocNeeded(
            MHW_VDBOX_VP9_INTERNAL_BUFFER_SEGMENT_ID,
            &reallocParam));
    if (reallocParam.bNeedBiggerSize ||
        Mos_ResourceIsNull(&m_resVp9SegmentIdBuffer))
    {
        if (!Mos_ResourceIsNull(&m_resVp9SegmentIdBuffer))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_resVp9SegmentIdBuffer);
        }
        // VP9 Segment ID buffer
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->GetVp9BufferSize(
            MHW_VDBOX_VP9_INTERNAL_BUFFER_SEGMENT_ID,
            &hcpBufSizeParam));

        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                      &m_resVp9SegmentIdBuffer,
                                                      hcpBufSizeParam.dwBufferSize,
                                                      "Vp9SegmentIdBuffer"),
            "Failed to allocate VP9 segment ID Buffer.");
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->IsVp9BufferReallocNeeded(
        MHW_VDBOX_VP9_INTERNAL_BUFFER_SEGMENT_ID,
        &reallocParam));
    if (reallocParam.bNeedBiggerSize || Mos_ResourceIsNull(&m_resSegmentIdBuffReset))
    {
        if (!Mos_ResourceIsNull(&m_resSegmentIdBuffReset))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_resSegmentIdBuffReset);
        }
        // VP9 Segment ID Reset buffer
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->GetVp9BufferSize(
            MHW_VDBOX_VP9_INTERNAL_BUFFER_SEGMENT_ID,
            &hcpBufSizeParam));

        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                      &m_resSegmentIdBuffReset,
                                                      hcpBufSizeParam.dwBufferSize,
                                                      "SegmentIdBuffreset",
                                                      true,
                                                      0),
            "Failed to allocate segment ID reset Buffer.");
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->IsVp9BufferReallocNeeded(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_CURR_MV_TEMPORAL,
        &reallocParam));
    if (reallocParam.bNeedBiggerSize || m_mvBufferSize == 0)
    {
        for (uint8_t i = 0; i < CODECHAL_VP9_NUM_MV_BUFFERS; i++)
        {
            if (!Mos_ResourceIsNull(&m_resVp9MvTemporalBuffer[i]))
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &m_resVp9MvTemporalBuffer[i]);
            }
        }

        // VP9 MV Temporal buffers
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->GetVp9BufferSize(
            MHW_VDBOX_HCP_INTERNAL_BUFFER_CURR_MV_TEMPORAL,
            &hcpBufSizeParam));

        for (uint8_t i = 0; i < CODECHAL_VP9_NUM_MV_BUFFERS; i++)
        {
            CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                          &m_resVp9MvTemporalBuffer[i],
                                                          hcpBufSizeParam.dwBufferSize,
                                                          "MvTemporalBuffer"),
                "Failed to allocate Mv temporal Buffer.");
        }

        m_mvBufferSize = hcpBufSizeParam.dwBufferSize;
    }

    if (m_secureDecoder)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->AllocateResource(this));
    }

    //backup allocated memory size
    m_allocatedWidthInSb  = widthInSb;
    m_allocatedHeightInSb = heightInSb;

    return eStatus;
}

MOS_STATUS CodechalDecodeVp9 :: InitializeBeginFrame()
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    m_incompletePicture = false;
    m_copyDataBufferInUse = false;
    m_copyDataOffset      = 0;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeVp9 :: CopyDataSurface()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContext(
        m_osInterface,
        m_videoContextForWa));
    m_osInterface->pfnResetOsStates(m_osInterface);

    m_osInterface->pfnSetPerfTag(
        m_osInterface,
        (uint16_t)(((m_mode << 4) & 0xF0) | COPY_TYPE));
    m_osInterface->pfnResetPerfBufferID(m_osInterface);

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        0));

    // Send command buffer header at the beginning (OS dependent)
    CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(
        &cmdBuffer,
        false));

    CODECHAL_DECODE_CHK_STATUS_RETURN(HucCopy(
        &cmdBuffer,            // pCmdBuffer
        &m_resDataBuffer,      // presSrc
        &m_resCopyDataBuffer,  // presDst
        m_dataSize,            // u32CopyLength
        m_dataOffset,          // u32CopyInputOffset
        m_copyDataOffset));    // u32CopyOutputOffset

    m_copyDataOffset += MOS_ALIGN_CEIL(m_dataSize, MHW_CACHELINE_SIZE);

    MHW_MI_FLUSH_DW_PARAMS  flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
        &cmdBuffer,
        &flushDwParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
        &cmdBuffer,
        nullptr));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    // sync resource
    if (!m_incompletePicture)
    {
        MOS_SYNC_PARAMS syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContext;
        syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));

        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContextForWa;
        syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        m_videoContextForWaUsesNullHw));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContext(
        m_osInterface,
        m_videoContext));

    return eStatus;
}

MOS_STATUS CodechalDecodeVp9 :: CheckAndCopyBitStream()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    uint32_t badSliceChopping = 0;
    // No pVp9SliceParams is set from APP.
    if (m_vp9SliceParams == nullptr)
    {
        badSliceChopping = 0;
    }
    else
    {
        badSliceChopping = m_vp9SliceParams->wBadSliceChopping;
    }

    // No BSBytesInBuffer is sent from App, driver here just give an estimation.
    if (badSliceChopping != 0)
    {
        m_vp9PicParams->BSBytesInBuffer =
            (m_vp9PicParams->FrameWidthMinus1 + 1) * (m_vp9PicParams->FrameHeightMinus1 + 1) * 6;
    }

    if (IsFirstExecuteCall()) // first exec call
    {
        if (m_dataSize < m_vp9PicParams->BSBytesInBuffer)  // Current bitstream buffer is not big enough
        {
            // Allocate or reallocate the copy data buffer.
            if (m_copyDataBufferSize < MOS_ALIGN_CEIL(m_vp9PicParams->BSBytesInBuffer, 64))
            {
                if (!Mos_ResourceIsNull(&m_resCopyDataBuffer))
                {
                    m_osInterface->pfnFreeResource(
                        m_osInterface,
                        &m_resCopyDataBuffer);
                }

                m_copyDataBufferSize = MOS_ALIGN_CEIL(m_vp9PicParams->BSBytesInBuffer, 64);

                CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                              &m_resCopyDataBuffer,
                                                              m_copyDataBufferSize,
                                                              "Vp9CopyDataBuffer"),
                    "Failed to allocate Vp9 copy data Buffer.");
            }

            // Copy bitstream into the copy buffer
            if (m_dataSize)
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(CopyDataSurface());

                m_copyDataBufferInUse = true;
            }

            m_incompletePicture = true;
        }
    }
    else // second and later exec calls
    {
        CODECHAL_DECODE_CHK_COND_RETURN(
            (m_copyDataOffset + m_dataSize > m_copyDataBufferSize),
            "Bitstream size exceeds copy data buffer size!");

        // Copy bitstream into the copy buffer
        if (m_dataSize)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(CopyDataSurface());
        }

        if (m_copyDataOffset >= m_vp9PicParams->BSBytesInBuffer || badSliceChopping == 2)
        {
            m_incompletePicture = false;
        }
    }

    return eStatus;
}
MOS_STATUS CodechalDecodeVp9 :: InitializeDecodeMode ()
{
    //do nothing for VP9 Base class. will be overloaded by inherited class to support dynamic mode switch
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeVp9::InitSfcState()
{
    // Default no SFC support
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeVp9::SetFrameStates ()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);
    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(m_decodeParams.m_destSurface);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_decodeParams.m_dataBuffer);

    m_dataSize         = m_decodeParams.m_dataSize;
    m_dataOffset       = m_decodeParams.m_dataOffset;
    m_vp9PicParams     = (PCODEC_VP9_PIC_PARAMS)m_decodeParams.m_picParams;
    m_vp9SegmentParams = (PCODEC_VP9_SEGMENT_PARAMS)m_decodeParams.m_iqMatrixBuffer;
    m_vp9SliceParams   = (PCODEC_VP9_SLICE_PARAMS)m_decodeParams.m_sliceParams;

    CODECHAL_DECODE_CHK_NULL_RETURN(m_vp9SegmentParams);

    m_destSurface   = *(m_decodeParams.m_destSurface);
    m_resDataBuffer = *(m_decodeParams.m_dataBuffer);
    if (m_decodeParams.m_coefProbBuffer)        // This is an optional buffer passed from App. To be removed once VP9 FF Decode Driver is mature.
    {
        m_resCoefProbBuffer = *(m_decodeParams.m_coefProbBuffer);
    }

    if (IsFirstExecuteCall())
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(InitializeBeginFrame());
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(CheckAndCopyBitStream());

    m_cencBuf = m_decodeParams.m_cencBuf;

    // Bitstream is incomplete, don't do any decoding work.
    if (m_incompletePicture)
    {
        eStatus = MOS_STATUS_SUCCESS;
        return eStatus;
    }

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            m_copyDataBufferInUse ? &m_resCopyDataBuffer : &m_resDataBuffer,
            CodechalDbgAttr::attrBitstream,
            "_DEC",
            m_copyDataBufferInUse ? m_copyDataBufferSize : m_dataSize,
            m_copyDataBufferInUse ? 0 : m_dataOffset,
            CODECHAL_NUM_MEDIA_STATES));)

    m_statusReportFeedbackNumber = m_vp9PicParams->StatusReportFeedbackNumber;
    m_width =
        MOS_MAX(m_width, (uint32_t)(m_vp9PicParams->FrameWidthMinus1 + 1));
    m_height =
        MOS_MAX(m_height, (uint32_t)(m_vp9PicParams->FrameHeightMinus1 + 1));
    m_usFrameWidthAlignedMinBlk =
        MOS_ALIGN_CEIL(m_vp9PicParams->FrameWidthMinus1 + 1, CODEC_VP9_MIN_BLOCK_WIDTH);
    m_usFrameHeightAlignedMinBlk =
        MOS_ALIGN_CEIL(m_vp9PicParams->FrameHeightMinus1 + 1, CODEC_VP9_MIN_BLOCK_WIDTH);

    // Overwrite the actual surface height with the coded height and width of the frame
    // for VP9 since it's possible for a VP9 frame to change size during playback
    m_destSurface.dwWidth  = m_vp9PicParams->FrameWidthMinus1 + 1;
    m_destSurface.dwHeight = m_vp9PicParams->FrameHeightMinus1 + 1;

    PCODEC_REF_LIST destEntry = m_vp9RefList[m_vp9PicParams->CurrPic.FrameIdx];

    // Clear FilterLevel Array inside segment data when filter_level inside picparam is zero
    if (m_cencBuf == nullptr)
    {
        MOS_ZeroMemory(destEntry, sizeof(CODEC_REF_LIST));
        // Clear FilterLevel Array inside segment data when filter_level inside picparam is zero
        if ((!m_vp9PicParams->filter_level))
        {
            PCODEC_VP9_SEG_PARAMS vp9SegData = &m_vp9SegmentParams->SegData[0];

            for (uint8_t i = 0; i < 8; i++)
            {
                *((uint32_t *)&vp9SegData->FilterLevel[0][0]) = 0;
                *((uint32_t *)&vp9SegData->FilterLevel[2][0]) = 0;
                vp9SegData++;      // Go on to next record.
            }
        }
    }
    destEntry->resRefPic     = m_destSurface.OsResource;
    destEntry->dwFrameWidth  = m_vp9PicParams->FrameWidthMinus1 + 1;
    destEntry->dwFrameHeight = m_vp9PicParams->FrameHeightMinus1 + 1;

    if (m_hcpInterface->IsRowStoreCachingSupported() &&
        m_usFrameWidthAlignedMinBlk != MOS_ALIGN_CEIL(m_prevFrmWidth, CODEC_VP9_MIN_BLOCK_WIDTH))
    {
        MHW_VDBOX_ROWSTORE_PARAMS rowstoreParams;
        uint8_t usChromaSamplingFormat;
        if (m_vp9PicParams->subsampling_x == 1 && m_vp9PicParams->subsampling_y == 1)
        {
            usChromaSamplingFormat = HCP_CHROMA_FORMAT_YUV420;
        }
        else if (m_vp9PicParams->subsampling_x == 0 && m_vp9PicParams->subsampling_y == 0)
        {
            usChromaSamplingFormat = HCP_CHROMA_FORMAT_YUV444;
        }
        else
        {
            CODECHAL_DECODE_ASSERTMESSAGE("Invalid Chroma sampling format!");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }
        MOS_ZeroMemory(&rowstoreParams, sizeof(rowstoreParams));
        rowstoreParams.dwPicWidth       = m_usFrameWidthAlignedMinBlk;
        rowstoreParams.bMbaff       = false;
        rowstoreParams.Mode         = CODECHAL_DECODE_MODE_VP9VLD;
        rowstoreParams.ucBitDepthMinus8 = m_vp9PicParams->BitDepthMinus8;
        rowstoreParams.ucChromaFormat   = usChromaSamplingFormat;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->SetRowstoreCachingOffsets(&rowstoreParams));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(InitializeDecodeMode());
    CODECHAL_DECODE_CHK_STATUS_RETURN(InitSfcState());

    // Allocate internal buffer or reallocate When current resolution is bigger than allocated internal buffer size
    CODECHAL_DECODE_CHK_STATUS_RETURN(AllocateResourcesVariableSizes());

    CODECHAL_DECODE_CHK_STATUS_RETURN(DetermineInternalBufferUpdate());

    m_hcpDecPhase = CodechalHcpDecodePhaseInitialized;

    m_perfType = m_vp9PicParams->PicFlags.fields.frame_type ? P_TYPE : I_TYPE;

    m_crrPic = m_vp9PicParams->CurrPic;

    if (m_vp9PicParams->PicFlags.fields.frame_type == CODEC_VP9_INTER_FRAME &&
        !m_vp9PicParams->PicFlags.fields.intra_only)
    {
        m_curMvTempBufIdx = (m_curMvTempBufIdx + 1) % CODECHAL_VP9_NUM_MV_BUFFERS;
        m_colMvTempBufIdx = (m_curMvTempBufIdx < 1) ? (CODECHAL_VP9_NUM_MV_BUFFERS - 1) : (m_curMvTempBufIdx - 1);
    }

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_NULL_RETURN(m_debugInterface);
        m_vp9PicParams->CurrPic.PicFlags = PICTURE_FRAME;
        m_debugInterface->m_currPic      = m_crrPic;
        m_debugInterface->m_frameType    = m_perfType;

        CODECHAL_DECODE_CHK_STATUS_RETURN(DumpDecodePicParams(
            m_vp9PicParams));

        if (m_vp9SegmentParams) {
            CODECHAL_DECODE_CHK_STATUS_RETURN(DumpDecodeSegmentParams(
                m_vp9SegmentParams));
        })

    return eStatus;
}

MOS_STATUS CodechalDecodeVp9 :: DetermineDecodePhase()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    uint32_t curPhase = m_hcpDecPhase;
    switch (curPhase)
    {
    case CodechalHcpDecodePhaseInitialized:
        m_hcpDecPhase = CodechalHcpDecodePhaseLegacyLong;
        break;
    default:
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CODECHAL_DECODE_ASSERTMESSAGE("invalid decode phase.");
        return eStatus;
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeVp9 :: InitPicStateMhwParams()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    PMOS_RESOURCE usedDummyReference = nullptr;

    CODECHAL_DECODE_FUNCTION_ENTER;

    // Reset all pic Mhw Params
    *m_picMhwParams.PipeModeSelectParams = {};
    *m_picMhwParams.PipeBufAddrParams = {};
    MOS_ZeroMemory(m_picMhwParams.IndObjBaseAddrParams, sizeof(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS));
    MOS_ZeroMemory(m_picMhwParams.Vp9PicState, sizeof(MHW_VDBOX_VP9_PIC_STATE));
    MOS_ZeroMemory(m_picMhwParams.Vp9SegmentState, sizeof(MHW_VDBOX_VP9_SEGMENT_STATE));

    PCODEC_PICTURE refFrameList      = &(m_vp9PicParams->RefFrameList[0]);
    uint8_t        lastRefPicIndex   = m_vp9PicParams->PicFlags.fields.LastRefIdx;
    uint8_t        goldenRefPicIndex = m_vp9PicParams->PicFlags.fields.GoldenRefIdx;
    uint8_t        altRefPicIndex    = m_vp9PicParams->PicFlags.fields.AltRefIdx;
    if (m_vp9PicParams->PicFlags.fields.frame_type == CODEC_VP9_KEY_FRAME ||
        m_vp9PicParams->PicFlags.fields.intra_only)
    {
        // reference surface should be nullptr when key_frame == true or intra only frame
        m_presLastRefSurface   = nullptr;
        m_presGoldenRefSurface = nullptr;
        m_presAltRefSurface    = nullptr;
    }
    else
    {
        if (lastRefPicIndex > 7 || goldenRefPicIndex > 7 || altRefPicIndex > 7)
        {
            CODECHAL_DECODE_ASSERTMESSAGE("invalid ref index (should be in [0,7]) in pic parameter!");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }

        if (refFrameList[lastRefPicIndex].FrameIdx >= CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9)
        {
            refFrameList[lastRefPicIndex].FrameIdx = CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9 - 1;
        }
        if (refFrameList[goldenRefPicIndex].FrameIdx >= CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9)
        {
            refFrameList[goldenRefPicIndex].FrameIdx = CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9 - 1;
        }
        if (refFrameList[altRefPicIndex].FrameIdx >= CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9)
        {
            refFrameList[altRefPicIndex].FrameIdx = CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9 - 1;
        }
        PCODEC_REF_LIST *vp9RefList = &(m_vp9RefList[0]);
        m_presLastRefSurface        = &(vp9RefList[refFrameList[lastRefPicIndex].FrameIdx]->resRefPic);
        m_presGoldenRefSurface      = &(vp9RefList[refFrameList[goldenRefPicIndex].FrameIdx]->resRefPic);
        m_presAltRefSurface         = &(vp9RefList[refFrameList[altRefPicIndex].FrameIdx]->resRefPic);
    }

    uint16_t usChromaSamplingFormat;
    if (m_vp9PicParams->subsampling_x == 1 && m_vp9PicParams->subsampling_y == 1)
    {
        usChromaSamplingFormat = HCP_CHROMA_FORMAT_YUV420;
    }
    else if (m_vp9PicParams->subsampling_x == 0 && m_vp9PicParams->subsampling_y == 0)
    {
        usChromaSamplingFormat = HCP_CHROMA_FORMAT_YUV444;
    }
    else
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Invalid Chroma sampling format!");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    m_picMhwParams.PipeModeSelectParams->Mode                  = m_mode;
    m_picMhwParams.PipeModeSelectParams->bStreamOutEnabled     = m_streamOutEnabled;

    // Populate surface param for decoded picture
    m_picMhwParams.SurfaceParams[0]->Mode               = m_mode;
    m_picMhwParams.SurfaceParams[0]->psSurface              = &m_destSurface;
    m_picMhwParams.SurfaceParams[0]->ChromaType         = (uint8_t)usChromaSamplingFormat;
    m_picMhwParams.SurfaceParams[0]->ucSurfaceStateId   = CODECHAL_HCP_DECODED_SURFACE_ID;
    m_picMhwParams.SurfaceParams[0]->ucBitDepthLumaMinus8   = m_vp9PicParams->BitDepthMinus8;
    m_picMhwParams.SurfaceParams[0]->ucBitDepthChromaMinus8 = m_vp9PicParams->BitDepthMinus8;
    m_picMhwParams.SurfaceParams[0]->dwUVPlaneAlignment = 8;

    if (MEDIA_IS_WA(m_waTable, WaDummyReference) &&
        !Mos_ResourceIsNull(&m_dummyReference.OsResource))
    {
        usedDummyReference = &m_dummyReference.OsResource;
    }
    else
    {
        usedDummyReference = &m_destSurface.OsResource;
    }

    // Populate surface param for reference pictures
    if (m_vp9PicParams->PicFlags.fields.frame_type == CODEC_VP9_INTER_FRAME &&
        !m_vp9PicParams->PicFlags.fields.intra_only &&
        m_presLastRefSurface != nullptr &&
        m_presGoldenRefSurface != nullptr &&
        m_presAltRefSurface != nullptr)
    {
        if (Mos_ResourceIsNull(m_presLastRefSurface))
        {
            m_presLastRefSurface = usedDummyReference;
        }
        if (Mos_ResourceIsNull(m_presGoldenRefSurface))
        {
            m_presGoldenRefSurface = usedDummyReference;
        }
        if (Mos_ResourceIsNull(m_presAltRefSurface))
        {
            m_presAltRefSurface = usedDummyReference;
        }

        //MOS_SURFACE lastRefSurface;
        CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &m_lastRefSurface.OsResource,
            sizeof(MOS_RESOURCE),
            m_presLastRefSurface,
            sizeof(MOS_RESOURCE)));
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
            m_osInterface,
            &m_lastRefSurface));

        //MOS_SURFACE goldenRefSurface;
        CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &m_goldenRefSurface.OsResource,
            sizeof(MOS_RESOURCE),
            m_presGoldenRefSurface,
            sizeof(MOS_RESOURCE)));
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
            m_osInterface,
            &m_goldenRefSurface));

        //MOS_SURFACE altRefSurface;
        CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &m_altRefSurface.OsResource,
            sizeof(MOS_RESOURCE),
            m_presAltRefSurface,
            sizeof(MOS_RESOURCE)));
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
            m_osInterface,
            &m_altRefSurface));

        for (uint8_t i = 1; i < 4; i++)
        {
            m_picMhwParams.SurfaceParams[i]->Mode               = m_mode;
            m_picMhwParams.SurfaceParams[i]->ChromaType         = (uint8_t)usChromaSamplingFormat;
            m_picMhwParams.SurfaceParams[i]->dwUVPlaneAlignment = 8;

            switch (i)
            {
                case 1:
                    m_picMhwParams.SurfaceParams[i]->psSurface          = &m_lastRefSurface;
                    m_picMhwParams.SurfaceParams[i]->ucSurfaceStateId   = CODECHAL_HCP_LAST_SURFACE_ID;
                    break;
                case 2:
                    m_picMhwParams.SurfaceParams[i]->psSurface          = &m_goldenRefSurface;
                    m_picMhwParams.SurfaceParams[i]->ucSurfaceStateId   = CODECHAL_HCP_GOLDEN_SURFACE_ID;
                    break;
                case 3:
                    m_picMhwParams.SurfaceParams[i]->psSurface          = &m_altRefSurface;
                    m_picMhwParams.SurfaceParams[i]->ucSurfaceStateId   = CODECHAL_HCP_ALTREF_SURFACE_ID;
                    break;
            }
        }
    }

    m_picMhwParams.PipeBufAddrParams->Mode                                          = m_mode;
    m_picMhwParams.PipeBufAddrParams->psPreDeblockSurface                           = &m_destSurface;

    m_picMhwParams.PipeBufAddrParams->presReferences[CodechalDecodeLastRef]      = m_presLastRefSurface;
    m_picMhwParams.PipeBufAddrParams->presReferences[CodechalDecodeGoldenRef]    = m_presGoldenRefSurface;
    m_picMhwParams.PipeBufAddrParams->presReferences[CodechalDecodeAlternateRef] = m_presAltRefSurface;

    // set all ref pic addresses in HCP_PIPE_BUF_ADDR_STATE command to valid addresses for error concealment purpose, set the unused ones to the first used one
    for (uint8_t i = 0; i < CODECHAL_DECODE_VP9_MAX_NUM_REF_FRAME; i++)
    {
        if (!m_picMhwParams.PipeBufAddrParams->presReferences[i])
        {
            m_picMhwParams.PipeBufAddrParams->presReferences[i] = usedDummyReference;
        }
    }

#ifdef _MMC_SUPPORTED
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->SetPipeBufAddr(m_picMhwParams.PipeBufAddrParams));
#endif

    if (m_streamOutEnabled)
    {
        m_picMhwParams.PipeBufAddrParams->presStreamOutBuffer =
            &(m_streamOutBuffer[m_streamOutCurrBufIdx]);
    }

#ifdef _MMC_SUPPORTED
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->CheckReferenceList(m_picMhwParams.PipeBufAddrParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->SetRefrenceSync(m_disableDecodeSyncLock, m_disableLockForTranscode));
#endif

    m_picMhwParams.PipeBufAddrParams->presMfdDeblockingFilterRowStoreScratchBuffer =
        &m_resDeblockingFilterLineRowStoreScratchBuffer;
    m_picMhwParams.PipeBufAddrParams->presDeblockingFilterTileRowStoreScratchBuffer =
        &m_resDeblockingFilterTileRowStoreScratchBuffer;
    m_picMhwParams.PipeBufAddrParams->presDeblockingFilterColumnRowStoreScratchBuffer =
        &m_resDeblockingFilterColumnRowStoreScratchBuffer;

    m_picMhwParams.PipeBufAddrParams->presMetadataLineBuffer       = &m_resMetadataLineBuffer;
    m_picMhwParams.PipeBufAddrParams->presMetadataTileLineBuffer   = &m_resMetadataTileLineBuffer;
    m_picMhwParams.PipeBufAddrParams->presMetadataTileColumnBuffer = &m_resMetadataTileColumnBuffer;
    m_picMhwParams.PipeBufAddrParams->presVp9ProbBuffer            = &m_resVp9ProbBuffer[m_frameCtxIdx];
    m_picMhwParams.PipeBufAddrParams->presVp9SegmentIdBuffer       = &m_resVp9SegmentIdBuffer;
    m_picMhwParams.PipeBufAddrParams->presHvdLineRowStoreBuffer    = &m_resHvcLineRowstoreBuffer;
    m_picMhwParams.PipeBufAddrParams->presHvdTileRowStoreBuffer    = &m_resHvcTileRowstoreBuffer;

    if (m_vp9PicParams->PicFlags.fields.frame_type == CODEC_VP9_INTER_FRAME &&
        !m_vp9PicParams->PicFlags.fields.intra_only)
    {
        m_picMhwParams.PipeBufAddrParams->presCurMvTempBuffer = &m_resVp9MvTemporalBuffer[m_curMvTempBufIdx];

        if (!m_prevFrameParams.fields.KeyFrame && !m_prevFrameParams.fields.IntraOnly)
        {
            // For VP9, only index 0 is required to be filled
            m_picMhwParams.PipeBufAddrParams->presColMvTempBuffer[0] = &m_resVp9MvTemporalBuffer[m_colMvTempBufIdx];
        }
    }

    m_picMhwParams.IndObjBaseAddrParams->Mode           = m_mode;
    m_picMhwParams.IndObjBaseAddrParams->dwDataSize     = m_copyDataBufferInUse ? m_copyDataBufferSize : m_dataSize;
    m_picMhwParams.IndObjBaseAddrParams->dwDataOffset   = m_copyDataBufferInUse ? 0 : m_dataOffset;
    m_picMhwParams.IndObjBaseAddrParams->presDataBuffer = m_copyDataBufferInUse ? &m_resCopyDataBuffer : &m_resDataBuffer;

    if (m_secureDecoder)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->SetBitstreamBuffer(m_picMhwParams.IndObjBaseAddrParams));
    }

    m_picMhwParams.Vp9PicState->pVp9PicParams         = m_vp9PicParams;
    m_picMhwParams.Vp9PicState->ppVp9RefList          = &(m_vp9RefList[0]);
    m_picMhwParams.Vp9PicState->PrevFrameParams.value = m_prevFrameParams.value;
    m_picMhwParams.Vp9PicState->dwPrevFrmWidth        = m_prevFrmWidth;
    m_picMhwParams.Vp9PicState->dwPrevFrmHeight       = m_prevFrmHeight;

    m_prevFrameParams.fields.KeyFrame  = !m_vp9PicParams->PicFlags.fields.frame_type;
    m_prevFrameParams.fields.IntraOnly = m_vp9PicParams->PicFlags.fields.intra_only;
    m_prevFrameParams.fields.Display   = m_vp9PicParams->PicFlags.fields.show_frame;
    m_prevFrmWidth                     = m_vp9PicParams->FrameWidthMinus1 + 1;
    m_prevFrmHeight                    = m_vp9PicParams->FrameHeightMinus1 + 1;

    m_picMhwParams.Vp9SegmentState->Mode                = m_mode;
    m_picMhwParams.Vp9SegmentState->pVp9SegmentParams   = m_vp9SegmentParams;

    return eStatus;
}

MOS_STATUS CodechalDecodeVp9 :: AddPicStateMhwCmds(
    PMOS_COMMAND_BUFFER       cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(cmdBuffer);

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPipeModeSelectCmd(
        cmdBuffer,
        m_picMhwParams.PipeModeSelectParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpSurfaceCmd(
        cmdBuffer,
        m_picMhwParams.SurfaceParams[0]));

    // For non-key frame, send extra surface commands for reference pictures
    if (m_vp9PicParams->PicFlags.fields.frame_type == CODEC_VP9_INTER_FRAME &&
        !m_vp9PicParams->PicFlags.fields.intra_only)
    {
        for (uint8_t i = 1; i < 4; i++)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpSurfaceCmd(
                cmdBuffer,
                m_picMhwParams.SurfaceParams[i]));
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPipeBufAddrCmd(
        cmdBuffer,
        m_picMhwParams.PipeBufAddrParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpIndObjBaseAddrCmd(
        cmdBuffer,
        m_picMhwParams.IndObjBaseAddrParams));

    if (m_cencBuf)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(SetCencBatchBuffer(cmdBuffer));
    }
    else
    {
        for (uint8_t i = 0; i < CODEC_VP9_MAX_SEGMENTS; i++)
        {
            // Error handling for illegal programming on segmentation fields @ KEY/INTRA_ONLY frames
            PCODEC_VP9_SEG_PARAMS vp9SegData = &(m_picMhwParams.Vp9SegmentState->pVp9SegmentParams->SegData[i]);
            if (vp9SegData->SegmentFlags.fields.SegmentReferenceEnabled &&
                (!m_vp9PicParams->PicFlags.fields.frame_type || m_vp9PicParams->PicFlags.fields.intra_only))
            {
                vp9SegData->SegmentFlags.fields.SegmentReference = CODECHAL_DECODE_VP9_INTRA_FRAME;
            }

            m_picMhwParams.Vp9SegmentState->ucCurrentSegmentId = i;
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpVp9SegmentStateCmd(
                cmdBuffer,
                nullptr,
                m_picMhwParams.Vp9SegmentState));

            if (m_vp9PicParams->PicFlags.fields.segmentation_enabled == 0)
            {
                break;
            }
        }

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpVp9PicStateCmd(
            cmdBuffer,
            nullptr,
            m_picMhwParams.Vp9PicState));

        if (m_secureDecoder)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->AddHcpSecureState(
                cmdBuffer,
                this));
        }
    }
    return eStatus;
}

MOS_STATUS CodechalDecodeVp9 :: UpdatePicStateBuffers(
    PMOS_COMMAND_BUFFER       cmdBuffe)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (m_resetSegIdBuffer)
    {
        if (m_osInterface->osCpInterface->IsHMEnabled())
        {
            if (m_secureDecoder)
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->ResetVP9SegIdBufferWithHuc(this, cmdBuffe));
            }
        }
        else
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(ResetSegIdBufferwithDrv());
        }
    }

    if (m_osInterface->osCpInterface->IsHMEnabled())
    {
        if (m_secureDecoder)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->UpdateVP9ProbBufferWithHuc(m_fullProbBufferUpdate, this, cmdBuffe));
        }
    }
    else
    {
        if (m_fullProbBufferUpdate)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(ProbBufFullUpdatewithDrv());
        }
        else
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(ProbBufferPartialUpdatewithDrv());
        }
    }

    if (m_osInterface->osCpInterface->IsHMEnabled())
    {
        if (m_secureDecoder && MEDIA_IS_WA(m_hwInterface->GetWaTable(), WaSecureDecodeTDR))
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->InitAuxSurface(&m_destSurface.OsResource, cmdBuffe));
        }
    }

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_resVp9SegmentIdBuffer,
            CodechalDbgAttr::attrSegId,
            "SegId_beforeHCP",
            (m_allocatedWidthInSb * m_allocatedHeightInSb * CODECHAL_CACHELINE_SIZE)));
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &(m_resVp9ProbBuffer[m_frameCtxIdx]),
            CodechalDbgAttr::attrCoefProb,
            "PakHwCoeffProbs_beforeHCP",
            CODEC_VP9_PROB_MAX_NUM_ELEM));)

    return eStatus;
}

MOS_STATUS CodechalDecodeVp9 :: DecodeStateLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);
    CODECHAL_DECODE_FUNCTION_ENTER;

    if (m_secureDecoder && m_hcpDecPhase == CodechalHcpDecodePhaseInitialized)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->Execute(this));
    }

    //HCP Decode Phase State Machine
    DetermineDecodePhase();

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        0));

    auto mmioRegisters = m_hwInterface->GetMfxInterface()->GetMmioRegisters(m_vdboxIndex);
    HalOcaInterface::On1stLevelBBStart(cmdBuffer, *m_osInterface->pOsContext, m_osInterface->CurrentGpuContextHandle, *m_miInterface, *mmioRegisters);

    //Frame tracking functionality is called at the start of a command buffer.
    //Called at FE decode phase, since BE decode phase will only construct BE batch buffers.
    CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(
        &cmdBuffer, true));

    CODECHAL_DECODE_CHK_STATUS_RETURN(InitPicStateMhwParams());

    CODECHAL_DECODE_CHK_STATUS_RETURN(UpdatePicStateBuffers(&cmdBuffer));

    if (m_statusQueryReportingEnabled)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(
            StartStatusReport(&cmdBuffer));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(AddPicStateMhwCmds(
        &cmdBuffer));
    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    return eStatus;
}

MOS_STATUS CodechalDecodeVp9 :: DecodePrimitiveLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    CODECHAL_DECODE_FUNCTION_ENTER;

    // Bitstream is incomplete, don't do any decoding work.
    if (m_incompletePicture)
    {
        eStatus = MOS_STATUS_SUCCESS;
        return eStatus;
    }

    CODECHAL_DECODE_CHK_COND_RETURN(
        (m_vdboxIndex > m_mfxInterface->GetMaxVdboxIndex()),
        "ERROR - vdbox index exceed the maximum");

    m_osInterface->pfnSetPerfTag(
        m_osInterface,
        (uint16_t)(((m_mode << 4) & 0xF0) | (m_perfType & 0xF)));
    m_osInterface->pfnResetPerfBufferID(m_osInterface);

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        0));

    if (m_cencBuf == nullptr)
    {
        MHW_VDBOX_HCP_BSD_PARAMS bsdParams;
        MOS_ZeroMemory(&bsdParams, sizeof(bsdParams));
        bsdParams.dwBsdDataLength =
            m_vp9PicParams->BSBytesInBuffer - m_vp9PicParams->UncompressedHeaderLengthInBytes;
        bsdParams.dwBsdDataStartOffset = m_vp9PicParams->UncompressedHeaderLengthInBytes;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpBsdObjectCmd(
            &cmdBuffer,
            &bsdParams));
    }

    // Send VD Pipe Flush command for SKL+
    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdpipeFlushParams;
    MOS_ZeroMemory(&vdpipeFlushParams, sizeof(vdpipeFlushParams));
    vdpipeFlushParams.Flags.bWaitDoneHEVC = 1;
    vdpipeFlushParams.Flags.bFlushHEVC = 1;
    vdpipeFlushParams.Flags.bWaitDoneVDCmdMsgParser = 1;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(
        &cmdBuffer,
        &vdpipeFlushParams));

    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
        &cmdBuffer,
        &flushDwParams));

    MOS_SYNC_PARAMS syncParams;
    syncParams          = g_cInitSyncParams;
    syncParams.GpuContext               = m_videoContext;
    syncParams.presSyncResource         = &m_destSurface.OsResource;
    syncParams.bReadOnly                = false;
    syncParams.bDisableDecodeSyncLock   = m_disableDecodeSyncLock;
    syncParams.bDisableLockForTranscode = m_disableLockForTranscode;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnPerformOverlaySync(m_osInterface, &syncParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));

    // Update the resource tag (s/w tag) for On-Demand Sync
    m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);

    // Update the tag in GPU Sync eStatus buffer (H/W Tag) to match the current S/W tag
    if (m_osInterface->bTagResourceSync)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->WriteSyncTagToResource(
            &cmdBuffer,
            &syncParams));
    }

    if (m_statusQueryReportingEnabled)
    {
        CodechalDecodeStatusReport decodeStatusReport;

        decodeStatusReport.m_statusReportNumber = m_statusReportFeedbackNumber;
        decodeStatusReport.m_currDecodedPic     = m_vp9PicParams->CurrPic;
        decodeStatusReport.m_currDeblockedPic   = m_vp9PicParams->CurrPic;
        decodeStatusReport.m_codecStatus        = CODECHAL_STATUS_UNAVAILABLE;
        decodeStatusReport.m_numMbsAffected     = m_usFrameWidthAlignedMinBlk * m_usFrameHeightAlignedMinBlk;
        decodeStatusReport.m_currDecodedPicRes  = m_vp9RefList[m_vp9PicParams->CurrPic.FrameIdx]->resRefPic;

        // VP9 plug-in/out was NOT fully enabled; this is just to make sure driver would not crash in CodecHal_DecodeEndFrame(),
        // which requires the value of DecodeStatusReport.presCurrDecodedPic
        CODECHAL_DEBUG_TOOL(
            decodeStatusReport.m_frameType = m_perfType;
        )

        CODECHAL_DECODE_CHK_STATUS_RETURN(EndStatusReport(
            decodeStatusReport,
            &cmdBuffer));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
        &cmdBuffer,
        &flushDwParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
        &cmdBuffer,
        nullptr));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            "_DEC"));

        //CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHal_DbgReplaceAllCommands(
        //    m_debugInterface,
        //    &cmdBuffer));

        m_mmc->UpdateUserFeatureKey(&m_destSurface);)

    bool syncCompleteFrame = m_copyDataBufferInUse;

    if (syncCompleteFrame)
    {
        //Sync up complete frame
        MOS_SYNC_PARAMS copyDataSyncParams = g_cInitSyncParams;
        copyDataSyncParams.GpuContext = m_videoContextForWa;
        copyDataSyncParams.presSyncResource = &m_resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &copyDataSyncParams));

        copyDataSyncParams = g_cInitSyncParams;
        copyDataSyncParams.GpuContext = m_videoContext;
        copyDataSyncParams.presSyncResource = &m_resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &copyDataSyncParams));
    }

    uint32_t renderingFlags = m_videoContextUsesNullHw;

    HalOcaInterface::On1stLevelBBEnd(cmdBuffer, *m_osInterface->pOsContext);

    //submit command buffer
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        renderingFlags));

    // Reset status report
    if (m_statusQueryReportingEnabled)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(ResetStatusReport(
            m_videoContextUsesNullHw));
    }

#ifdef CODECHAL_HUC_KERNEL_DEBUG
    CODECHAL_DEBUG_TOOL(
    CODECHAL_DECODE_CHK_STATUS(m_debugInterface->DumpHucRegion(
        &pVp9State->resHucSharedBuffer,
        0,
        CODEC_VP9_PROB_MAX_NUM_ELEM,
        15,
        "",
        false,
        0,
        CodechalHucRegionDumpType::hucRegionDumpDefault));

    )
#endif

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_resVp9SegmentIdBuffer,
            CodechalDbgAttr::attrSegId,
            "SegId",
            (m_allocatedWidthInSb * m_allocatedHeightInSb * CODECHAL_CACHELINE_SIZE)));
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &(m_resVp9ProbBuffer[m_frameCtxIdx]),
            CodechalDbgAttr::attrCoefProb,
            "PakHwCoeffProbs",
            CODEC_VP9_PROB_MAX_NUM_ELEM));)

    // Needs to be re-set for Linux buffer re-use scenarios
    // pVp9RefList[pVp9PicParams->ucCurrPicIndex]->resRefPic =
    //    sDestSurface.OsResource;

    // Send the signal to indicate decode completion, in case On-Demand Sync is not present
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceSignal(m_osInterface, &syncParams));

    return eStatus;
}

MOS_STATUS CodechalDecodeVp9::InitMmcState()
{
#ifdef _MMC_SUPPORTED
    m_mmc = MOS_New(CodechalMmcDecodeVp9, m_hwInterface, this);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_mmc);
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeVp9 :: AllocateStandard (
    CodechalSetting *settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(settings);

    CODECHAL_DECODE_CHK_STATUS_RETURN(InitMmcState());

    m_width                      = settings->width;
    m_height                     = settings->height;
    if (settings->lumaChromaDepth & CODECHAL_LUMA_CHROMA_DEPTH_8_BITS)
        m_vp9DepthIndicator = 0;
    if (settings->lumaChromaDepth & CODECHAL_LUMA_CHROMA_DEPTH_10_BITS)
        m_vp9DepthIndicator = 1;
    if (settings->lumaChromaDepth & CODECHAL_LUMA_CHROMA_DEPTH_12_BITS)
        m_vp9DepthIndicator = 2;
    m_chromaFormatinProfile = settings->chromaFormat;

    MHW_VDBOX_STATE_CMDSIZE_PARAMS      stateCmdSizeParams;
    stateCmdSizeParams.bHucDummyStream = false;

    // Picture Level Commands
    m_hwInterface->GetHxxStateCommandSize(
        m_mode,
        &m_commandBufferSizeNeeded,
        &m_commandPatchListSizeNeeded,
        &stateCmdSizeParams);

    // Primitive Level Commands
    m_hwInterface->GetHxxPrimitiveCommandSize(
        m_mode,
        &m_standardDecodeSizeNeeded,
        &m_standardDecodePatchListSizeNeeded,
        false);

    CODECHAL_DECODE_CHK_STATUS_RETURN(AllocateResourcesFixedSizes());

    // Prepare Pic Params
    m_picMhwParams.PipeModeSelectParams = MOS_New(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS);
    m_picMhwParams.PipeBufAddrParams = MOS_New(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS);
    m_picMhwParams.IndObjBaseAddrParams = MOS_New(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS);
    m_picMhwParams.Vp9PicState = MOS_New(MHW_VDBOX_VP9_PIC_STATE);
    m_picMhwParams.Vp9SegmentState = MOS_New(MHW_VDBOX_VP9_SEGMENT_STATE);

    MOS_ZeroMemory(m_picMhwParams.IndObjBaseAddrParams, sizeof(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS));
    MOS_ZeroMemory(m_picMhwParams.Vp9PicState, sizeof(MHW_VDBOX_VP9_PIC_STATE));
    MOS_ZeroMemory(m_picMhwParams.Vp9SegmentState, sizeof(MHW_VDBOX_VP9_SEGMENT_STATE));

    for (uint16_t i = 0; i < 4; i++)
    {
        m_picMhwParams.SurfaceParams[i] = MOS_New(MHW_VDBOX_SURFACE_PARAMS);
        MOS_ZeroMemory(m_picMhwParams.SurfaceParams[i], sizeof(MHW_VDBOX_SURFACE_PARAMS));
    }

    return eStatus;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS CodechalDecodeVp9::DumpDecodePicParams(
    PCODEC_VP9_PIC_PARAMS picParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrPicParams))
    {
        return MOS_STATUS_SUCCESS;
    }
    CODECHAL_DEBUG_CHK_NULL(picParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "CurrPic FrameIdx: " << std::hex << +picParams->CurrPic.FrameIdx << std::endl;
    oss << "CurrPic PicFlags: " << std::hex << +picParams->CurrPic.PicFlags << std::endl;

    for (uint8_t i = 0; i < 8; ++i)
    {
        oss << "RefFrameList["<<+i<<"] FrameIdx:" << std::hex << +picParams->RefFrameList[i].FrameIdx << std::endl;
        oss << "RefFrameList["<<+i<<"] PicFlags:" << std::hex << +picParams->RefFrameList[i].PicFlags << std::endl;
    }
    oss << "FrameWidthMinus1: " << std::hex << +picParams->FrameWidthMinus1 << std::endl;
    oss << "FrameHeightMinus1: " << std::hex << +picParams->FrameHeightMinus1 << std::endl;
    oss << "PicFlags value: " << std::hex << +picParams->PicFlags.value << std::endl;
    oss << "frame_type: " << std::hex << +picParams->PicFlags.fields.frame_type << std::endl;
    oss << "show_frame: " << std::hex << +picParams->PicFlags.fields.show_frame << std::endl;
    oss << "error_resilient_mode: " << std::hex << +picParams->PicFlags.fields.error_resilient_mode << std::endl;
    oss << "intra_only: " << std::hex << +picParams->PicFlags.fields.intra_only << std::endl;
    oss << "LastRefIdx: " << std::hex << +picParams->PicFlags.fields.LastRefIdx << std::endl;
    oss << "LastRefSignBias: " << std::hex << +picParams->PicFlags.fields.LastRefSignBias << std::endl;
    oss << "GoldenRefIdx: " << std::hex << +picParams->PicFlags.fields.GoldenRefIdx << std::endl;
    oss << "GoldenRefSignBias: " << std::hex << +picParams->PicFlags.fields.GoldenRefSignBias << std::endl;
    oss << "AltRefIdx: " << std::hex << +picParams->PicFlags.fields.AltRefIdx << std::endl;
    oss << "AltRefSignBias: " << std::hex << +picParams->PicFlags.fields.AltRefSignBias << std::endl;
    oss << "allow_high_precision_mv: " << std::hex << +picParams->PicFlags.fields.allow_high_precision_mv << std::endl;
    oss << "mcomp_filter_type: " << std::hex << +picParams->PicFlags.fields.mcomp_filter_type << std::endl;
    oss << "frame_parallel_decoding_mode: " << std::hex << +picParams->PicFlags.fields.frame_parallel_decoding_mode << std::endl;
    oss << "segmentation_enabled: " << std::hex << +picParams->PicFlags.fields.segmentation_enabled << std::endl;
    oss << "segmentation_temporal_update: " << std::hex << +picParams->PicFlags.fields.segmentation_temporal_update << std::endl;
    oss << "segmentation_update_map: " << std::hex << +picParams->PicFlags.fields.segmentation_update_map << std::endl;
    oss << "reset_frame_context: " << std::hex << +picParams->PicFlags.fields.reset_frame_context << std::endl;
    oss << "refresh_frame_context: " << std::hex << +picParams->PicFlags.fields.refresh_frame_context << std::endl;
    oss << "frame_context_idx: " << std::hex << +picParams->PicFlags.fields.frame_context_idx << std::endl;
    oss << "LosslessFlag: " << std::hex << +picParams->PicFlags.fields.LosslessFlag << std::endl;
    oss << "ReservedField: " << std::hex << +picParams->PicFlags.fields.ReservedField << std::endl;
    oss << "filter_level: " << std::hex << +picParams->filter_level << std::endl;
    oss << "sharpness_level: " << std::hex << +picParams->sharpness_level << std::endl;
    oss << "log2_tile_rows: " << std::hex << +picParams->log2_tile_rows << std::endl;
    oss << "log2_tile_columns: " << std::hex << +picParams->log2_tile_columns << std::endl;
    oss << "UncompressedHeaderLengthInBytes: " << std::hex << +picParams->UncompressedHeaderLengthInBytes << std::endl;
    oss << "FirstPartitionSize: " << std::hex << +picParams->FirstPartitionSize << std::endl;
    oss << "profile: " << std::hex << +picParams->profile << std::endl;
    oss << "BitDepthMinus8: " << std::hex << +picParams->BitDepthMinus8 << std::endl;
    oss << "subsampling_x: " << std::hex << +picParams->subsampling_x << std::endl;
    oss << "subsampling_y: " << std::hex << +picParams->subsampling_y << std::endl;

    for (uint8_t i = 0; i < 7; ++i)
    {
        oss << "SegTreeProbs["<<+i<<"]: " << std::hex << +picParams->SegTreeProbs[i] << std::endl;
    }
    for (uint8_t i = 0; i < 3; ++i)
    {
        oss << "SegPredProbs["<<+i<<"]: " << std::hex << +picParams->SegPredProbs[i] << std::endl;
    }
    oss << "BSBytesInBuffer: " << std::hex << +picParams->BSBytesInBuffer << std::endl;
    oss << "StatusReportFeedbackNumber: " << std::hex << +picParams->StatusReportFeedbackNumber << std::endl;

    const char* fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufPicParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeVp9::DumpDecodeSegmentParams(
    PCODEC_VP9_SEGMENT_PARAMS segmentParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSegmentParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(segmentParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    for (uint8_t i = 0; i < 8; ++i)
    {
        oss << "SegData["<<+i<<"] SegmentFlags value: " << std::hex << +segmentParams->SegData[i].SegmentFlags.value << std::endl;
        oss << "SegData["<<+i<<"] SegmentReferenceEnabled: " << std::hex << +segmentParams->SegData[i].SegmentFlags.fields.SegmentReferenceEnabled << std::endl;
        oss << "SegData["<<+i<<"] SegmentReference: " << std::hex << +segmentParams->SegData[i].SegmentFlags.fields.SegmentReference << std::endl;
        oss << "SegData["<<+i<<"] SegmentReferenceSkipped: " << std::hex << +segmentParams->SegData[i].SegmentFlags.fields.SegmentReferenceSkipped << std::endl;
        oss << "SegData["<<+i<<"] ReservedField3: " << std::hex << +segmentParams->SegData[i].SegmentFlags.fields.ReservedField3 << std::endl;

        for (uint8_t j = 0; j < 4; ++j)
        {
            oss << "SegData["<<+i<<"] FilterLevel["<<+j<<"]:";
            oss << std::hex << +segmentParams->SegData[i].FilterLevel[j][0]<<" ";
            oss << std::hex<< +segmentParams->SegData[i].FilterLevel[j][1] << std::endl;
        }
        oss << "SegData["<<+i<<"] LumaACQuantScale: " << std::hex << +segmentParams->SegData[i].LumaACQuantScale << std::endl;
        oss << "SegData["<<+i<<"] LumaDCQuantScale: " << std::hex << +segmentParams->SegData[i].LumaDCQuantScale << std::endl;
        oss << "SegData["<<+i<<"] ChromaACQuantScale: " << std::hex << +segmentParams->SegData[i].ChromaACQuantScale << std::endl;
        oss << "SegData["<<+i<<"] ChromaDCQuantScale: " << std::hex << +segmentParams->SegData[i].ChromaDCQuantScale << std::endl;
    }

    const char* fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufSegmentParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
    return MOS_STATUS_SUCCESS;
}

#endif

MOS_STATUS CodechalDecodeVp9::CtxBufDiffInit(
    uint8_t             *ctxBuffer,
    bool                 setToKey)
{
    int32_t i, j;
    uint32_t byteCnt = CODEC_VP9_INTER_PROB_OFFSET;
    //inter mode probs. have to be zeros for Key frame
    for (i = 0; i < CODEC_VP9_INTER_MODE_CONTEXTS; i++)
    {
        for (j = 0; j < CODEC_VP9_INTER_MODES - 1; j++)
        {
            if (!setToKey)
            {
                ctxBuffer[byteCnt++] = DefaultInterModeProbs[i][j];
            }
            else
            {
                //zeros for key frame
                byteCnt++;
            }
        }
    }
    //switchable interprediction probs
    for (i = 0; i < CODEC_VP9_SWITCHABLE_FILTERS + 1; i++)
    {
        for (j = 0; j < CODEC_VP9_SWITCHABLE_FILTERS - 1; j++)
        {
            if (!setToKey)
            {
                ctxBuffer[byteCnt++] = DefaultSwitchableInterpProb[i][j];
            }
            else
            {
                //zeros for key frame
                byteCnt++;
            }
        }
    }
    //intra inter probs
    for (i = 0; i < CODEC_VP9_INTRA_INTER_CONTEXTS; i++)
    {
        if (!setToKey)
        {
            ctxBuffer[byteCnt++] = DefaultIntraInterProb[i];
        }
        else
        {
            //zeros for key frame
            byteCnt++;
        }
    }
    //comp inter probs
    for (i = 0; i < CODEC_VP9_COMP_INTER_CONTEXTS; i++)
    {
        if (!setToKey)
        {
            ctxBuffer[byteCnt++] = DefaultCompInterProb[i];
        }
        else
        {
            //zeros for key frame
            byteCnt++;
        }
    }
    //single ref probs
    for (i = 0; i < CODEC_VP9_REF_CONTEXTS; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if (!setToKey)
            {
                ctxBuffer[byteCnt++] = DefaultSingleRefProb[i][j];
            }
            else
            {
                //zeros for key frame
                byteCnt++;
            }
        }
    }
    //comp ref probs
    for (i = 0; i < CODEC_VP9_REF_CONTEXTS; i++)
    {
        if (!setToKey)
        {
            ctxBuffer[byteCnt++] = DefaultCompRefProb[i];
        }
        else
        {
            //zeros for key frame
            byteCnt++;
        }
    }
    //y mode probs
    for (i = 0; i < CODEC_VP9_BLOCK_SIZE_GROUPS; i++)
    {
        for (j = 0; j < CODEC_VP9_INTRA_MODES - 1; j++)
        {
            if (!setToKey)
            {
                ctxBuffer[byteCnt++] = DefaultIFYProb[i][j];
            }
            else
            {
                //zeros for key frame, since HW will not use this buffer, but default right buffer.
                byteCnt++;
            }
        }
    }
    //partition probs, key & intra-only frames use key type, other inter frames use inter type
    for (i = 0; i < CODECHAL_VP9_PARTITION_CONTEXTS; i++)
    {
        for (j = 0; j < CODEC_VP9_PARTITION_TYPES - 1; j++)
        {
            if (setToKey)
            {
                ctxBuffer[byteCnt++] = DefaultKFPartitionProb[i][j];
            }
            else
            {
                ctxBuffer[byteCnt++] = DefaultPartitionProb[i][j];
            }
        }
    }
    //nmvc joints
    for (i = 0; i < (CODEC_VP9_MV_JOINTS - 1); i++)
    {
        if (!setToKey)
        {
            ctxBuffer[byteCnt++] = DefaultNmvContext.joints[i];
        }
        else
        {
            //zeros for key frame
            byteCnt++;
        }
    }
    //nmvc comps
    for (i = 0; i < 2; i++)
    {
        if (!setToKey)
        {
            ctxBuffer[byteCnt++] = DefaultNmvContext.comps[i].sign;
            for (j = 0; j < (CODEC_VP9_MV_CLASSES - 1); j++)
            {
                ctxBuffer[byteCnt++] = DefaultNmvContext.comps[i].classes[j];
            }
            for (j = 0; j < (CODECHAL_VP9_CLASS0_SIZE - 1); j++)
            {
                ctxBuffer[byteCnt++] = DefaultNmvContext.comps[i].class0[j];
            }
            for (j = 0; j < CODECHAL_VP9_MV_OFFSET_BITS; j++)
            {
                ctxBuffer[byteCnt++] = DefaultNmvContext.comps[i].bits[j];
            }
        }
        else
        {
            byteCnt += 1;
            byteCnt += (CODEC_VP9_MV_CLASSES - 1);
            byteCnt += (CODECHAL_VP9_CLASS0_SIZE - 1);
            byteCnt += (CODECHAL_VP9_MV_OFFSET_BITS);
        }
    }
    for (i = 0; i < 2; i++)
    {
        if (!setToKey)
        {
            for (j = 0; j < CODECHAL_VP9_CLASS0_SIZE; j++)
            {
                for (int32_t k = 0; k < (CODEC_VP9_MV_FP_SIZE - 1); k++)
                {
                    ctxBuffer[byteCnt++] = DefaultNmvContext.comps[i].class0_fp[j][k];
                }
            }
            for (j = 0; j < (CODEC_VP9_MV_FP_SIZE - 1); j++)
            {
                ctxBuffer[byteCnt++] = DefaultNmvContext.comps[i].fp[j];
            }
        }
        else
        {
            byteCnt += (CODECHAL_VP9_CLASS0_SIZE * (CODEC_VP9_MV_FP_SIZE - 1));
            byteCnt += (CODEC_VP9_MV_FP_SIZE - 1);
        }
    }
    for (i = 0; i < 2; i++)
    {
        if (!setToKey)
        {
            ctxBuffer[byteCnt++] = DefaultNmvContext.comps[i].class0_hp;
            ctxBuffer[byteCnt++] = DefaultNmvContext.comps[i].hp;
        }
        else
        {
            byteCnt += 2;
        }
    }

    //47 bytes of zeros
    byteCnt += 47;

    //uv mode probs
    for (i = 0; i < CODEC_VP9_INTRA_MODES; i++)
    {
        for (j = 0; j < CODEC_VP9_INTRA_MODES - 1; j++)
        {
            if (setToKey)
            {
                ctxBuffer[byteCnt++] = DefaultKFUVModeProb[i][j];
            }
            else
            {
                ctxBuffer[byteCnt++] = DefaultIFUVProbs[i][j];
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeVp9::ContextBufferInit(
    uint8_t             *ctxBuffer,
    bool                 setToKey)
{

    MOS_ZeroMemory(ctxBuffer, CODEC_VP9_SEG_PROB_OFFSET);

    int32_t i, j;
    uint32_t byteCnt = 0;
    //TX probs
    for (i = 0; i < CODEC_VP9_TX_SIZE_CONTEXTS; i++)
    {
        for (j = 0; j < CODEC_VP9_TX_SIZES - 3; j++)
        {
            ctxBuffer[byteCnt++] = DefaultTxProbs.p8x8[i][j];
        }
    }
    for (i = 0; i < CODEC_VP9_TX_SIZE_CONTEXTS; i++)
    {
        for (j = 0; j < CODEC_VP9_TX_SIZES - 2; j++)
        {
            ctxBuffer[byteCnt++] = DefaultTxProbs.p16x16[i][j];
        }
    }
    for (i = 0; i < CODEC_VP9_TX_SIZE_CONTEXTS; i++)
    {
        for (j = 0; j < CODEC_VP9_TX_SIZES - 1; j++)
        {
            ctxBuffer[byteCnt++] = DefaultTxProbs.p32x32[i][j];
        }
    }

    //52 bytes of zeros
    byteCnt += 52;

    uint8_t blocktype = 0;
    uint8_t reftype = 0;
    uint8_t coeffbands = 0;
    uint8_t unConstrainedNodes = 0;
    uint8_t prevCoefCtx = 0;
    //coeff probs
    for (blocktype = 0; blocktype < CODEC_VP9_BLOCK_TYPES; blocktype++)
    {
        for (reftype = 0; reftype < CODEC_VP9_REF_TYPES; reftype++)
        {
            for (coeffbands = 0; coeffbands < CODEC_VP9_COEF_BANDS; coeffbands++)
            {
                uint8_t numPrevCoeffCtxts = (coeffbands == 0) ? 3 : CODEC_VP9_PREV_COEF_CONTEXTS;
                for (prevCoefCtx = 0; prevCoefCtx < numPrevCoeffCtxts; prevCoefCtx++)
                {
                    for (unConstrainedNodes = 0; unConstrainedNodes < CODEC_VP9_UNCONSTRAINED_NODES; unConstrainedNodes++)
                    {
                        ctxBuffer[byteCnt++] = DefaultCoefProbs4x4[blocktype][reftype][coeffbands][prevCoefCtx][unConstrainedNodes];
                    }
                }
            }
        }
    }

    for (blocktype = 0; blocktype < CODEC_VP9_BLOCK_TYPES; blocktype++)
    {
        for (reftype = 0; reftype < CODEC_VP9_REF_TYPES; reftype++)
        {
            for (coeffbands = 0; coeffbands < CODEC_VP9_COEF_BANDS; coeffbands++)
            {
                uint8_t numPrevCoeffCtxts = (coeffbands == 0) ? 3 : CODEC_VP9_PREV_COEF_CONTEXTS;
                for (prevCoefCtx = 0; prevCoefCtx < numPrevCoeffCtxts; prevCoefCtx++)
                {
                    for (unConstrainedNodes = 0; unConstrainedNodes < CODEC_VP9_UNCONSTRAINED_NODES; unConstrainedNodes++)
                    {
                        ctxBuffer[byteCnt++] = DefaultCoefPprobs8x8[blocktype][reftype][coeffbands][prevCoefCtx][unConstrainedNodes];
                    }
                }
            }
        }
    }

    for (blocktype = 0; blocktype < CODEC_VP9_BLOCK_TYPES; blocktype++)
    {
        for (reftype = 0; reftype < CODEC_VP9_REF_TYPES; reftype++)
        {
            for (coeffbands = 0; coeffbands < CODEC_VP9_COEF_BANDS; coeffbands++)
            {
                uint8_t numPrevCoeffCtxts = (coeffbands == 0) ? 3 : CODEC_VP9_PREV_COEF_CONTEXTS;
                for (prevCoefCtx = 0; prevCoefCtx < numPrevCoeffCtxts; prevCoefCtx++)
                {
                    for (unConstrainedNodes = 0; unConstrainedNodes < CODEC_VP9_UNCONSTRAINED_NODES; unConstrainedNodes++)
                    {
                        ctxBuffer[byteCnt++] = DefaultCoefProbs16x16[blocktype][reftype][coeffbands][prevCoefCtx][unConstrainedNodes];
                    }
                }
            }
        }
    }

    for (blocktype = 0; blocktype < CODEC_VP9_BLOCK_TYPES; blocktype++)
    {
        for (reftype = 0; reftype < CODEC_VP9_REF_TYPES; reftype++)
        {
            for (coeffbands = 0; coeffbands < CODEC_VP9_COEF_BANDS; coeffbands++)
            {
                uint8_t numPrevCoeffCtxts = (coeffbands == 0) ? 3 : CODEC_VP9_PREV_COEF_CONTEXTS;
                for (prevCoefCtx = 0; prevCoefCtx < numPrevCoeffCtxts; prevCoefCtx++)
                {
                    for (unConstrainedNodes = 0; unConstrainedNodes < CODEC_VP9_UNCONSTRAINED_NODES; unConstrainedNodes++)
                    {
                        ctxBuffer[byteCnt++] = DefaultCoefProbs32x32[blocktype][reftype][coeffbands][prevCoefCtx][unConstrainedNodes];
                    }
                }
            }
        }
    }

    //16 bytes of zeros
    byteCnt += 16;

    // mb skip probs
    for (i = 0; i < CODEC_VP9_MBSKIP_CONTEXTS; i++)
    {
        ctxBuffer[byteCnt++] = DefaultMbskipProbs[i];
    }

    // populate prob values which are different between Key and Non-Key frame
    CtxBufDiffInit(ctxBuffer, setToKey);

    //skip Seg tree/pred probs, updating not done in this function.
    byteCnt = CODEC_VP9_SEG_PROB_OFFSET;
    byteCnt += 7;
    byteCnt += 3;

    //28 bytes of zeros
    for (i = 0; i < 28; i++)
    {
        ctxBuffer[byteCnt++] = 0;
    }

    //Just a check.
    if (byteCnt > CODEC_VP9_PROB_MAX_NUM_ELEM)
    {
        CODECHAL_PUBLIC_ASSERTMESSAGE("Error: FrameContext array out-of-bounds, byteCnt = %d!\n", byteCnt);
        return MOS_STATUS_NO_SPACE;
    }
    else
    {
        return MOS_STATUS_SUCCESS;
    }
}

MOS_STATUS CodechalDecodeVp9::SetCencBatchBuffer(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    CODECHAL_DECODE_CHK_NULL_RETURN(cmdBuffer);

    MHW_BATCH_BUFFER        batchBuffer;
    MOS_ZeroMemory(&batchBuffer, sizeof(MHW_BATCH_BUFFER));
    MOS_RESOURCE *resHeap = nullptr;
    CODECHAL_DECODE_CHK_NULL_RETURN(resHeap = m_cencBuf->secondLvlBbBlock->GetResource());
    batchBuffer.OsResource   = *resHeap;
    batchBuffer.dwOffset     = m_cencBuf->secondLvlBbBlock->GetOffset() + VP9_CENC_PRIMITIVE_CMD_OFFSET_IN_DW * 4;
    batchBuffer.iSize        = m_cencBuf->secondLvlBbBlock->GetSize() - VP9_CENC_PRIMITIVE_CMD_OFFSET_IN_DW * 4;
    batchBuffer.bSecondLevel = true;
#if (_DEBUG || _RELEASE_INTERNAL)
    batchBuffer.iLastCurrent = batchBuffer.iSize;
#endif  // (_DEBUG || _RELEASE_INTERNAL)

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(
        cmdBuffer,
        &batchBuffer));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->Dump2ndLvlBatch(
            &batchBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            "_2ndLvlBatch_Pic_Cmd"));)

    batchBuffer.dwOffset     = m_cencBuf->secondLvlBbBlock->GetOffset();
    batchBuffer.iSize        = VP9_CENC_PRIMITIVE_CMD_OFFSET_IN_DW * 4;
    batchBuffer.bSecondLevel = true;
#if (_DEBUG || _RELEASE_INTERNAL)
    batchBuffer.iLastCurrent = batchBuffer.iSize;
#endif  // (_DEBUG || _RELEASE_INTERNAL)

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(
        cmdBuffer,
        &batchBuffer));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->Dump2ndLvlBatch(
            &batchBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            "_2ndLvlBatch_Primitive_Cmd"));)

    // Update GlobalCmdBufId
    MHW_MI_STORE_DATA_PARAMS miStoreDataParams;
    MOS_ZeroMemory(&miStoreDataParams, sizeof(miStoreDataParams));
    miStoreDataParams.pOsResource = m_cencBuf->resTracker;
    miStoreDataParams.dwValue     = m_cencBuf->trackerId;
    CODECHAL_DECODE_VERBOSEMESSAGE("dwCmdBufId = %d", miStoreDataParams.dwValue);
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
        cmdBuffer,
        &miStoreDataParams));
    return MOS_STATUS_SUCCESS;
}
