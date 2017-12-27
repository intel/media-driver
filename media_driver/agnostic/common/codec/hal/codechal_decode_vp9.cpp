/*
* Copyright (c) 2012-2017, Intel Corporation
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
#include "codechal_secure_decode.h"
#include "codechal_cenc_decode.h"
#include "codechal_decode_vp9.h"
#include "codechal_mmc_decode_vp9.h"
#if USE_CODECHAL_DEBUG_TOOL
#include <sstream>
#include <fstream>
#include "codechal_debug.h"
#endif

CodechalDecodeVp9 :: ~CodechalDecodeVp9 ()
{ 
    CODECHAL_DECODE_FUNCTION_ENTER; 

    m_osInterface->pfnDestroySyncResource(m_osInterface, &resSyncObject);
    m_osInterface->pfnDestroySyncResource(m_osInterface, &resSyncObjectWaContextInUse);
    m_osInterface->pfnDestroySyncResource(m_osInterface, &resSyncObjectVideoContextInUse);

    CodecHalFreeDataList(pVp9RefList, CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9);

    if (!Mos_ResourceIsNull(&resDeblockingFilterLineRowStoreScratchBuffer))
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &resDeblockingFilterLineRowStoreScratchBuffer);
    }

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &resDeblockingFilterTileRowStoreScratchBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &resDeblockingFilterColumnRowStoreScratchBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &resMetadataLineBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &resMetadataTileLineBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &resMetadataTileColumnBuffer);

    if (!Mos_ResourceIsNull(&resHvcLineRowstoreBuffer))
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &resHvcLineRowstoreBuffer);
    }

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &resHvcTileRowstoreBuffer);

    for (uint8_t i = 0; i < CODEC_VP9_NUM_CONTEXTS + 1; i++)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &resVp9ProbBuffer[i]);
    }

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &resVp9SegmentIdBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &resSegmentIdBuffReset);

    for (uint8_t i = 0; i < CODECHAL_VP9_NUM_MV_BUFFERS; i++)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &resVp9MvTemporalBuffer[i]);
    }

    if (!Mos_ResourceIsNull(&resCopyDataBuffer))
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &resCopyDataBuffer);
    }

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &resHucSharedBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &resDmemBuffer);
    
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &resInterProbSaveBuffer);

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

CodechalDecodeVp9 :: CodechalDecodeVp9(
    CodechalHwInterface   *hwInterface,
    CodechalDebugInterface* debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo) :
    CodechalDecode(hwInterface, debugInterface, standardInfo),
    ucCurMvTempBufIdx(0),
    ucColMvTempBufIdx(0),
    dwCopyDataBufferSize(0),
    dwPrevFrmWidth(0),
    dwPrevFrmHeight(0),
    dwAllocatedWidthInSB(0),
    dwAllocatedHeightInSB(0),
    dwMVBufferSize(0),
    bPendingResetPartial(0),
    bSaveInterProbs(0),
    bCopyDataBufferInUse(false)
{
    CODECHAL_DECODE_FUNCTION_ENTER;
    
    MOS_ZeroMemory(&resDeblockingFilterLineRowStoreScratchBuffer, sizeof(resDeblockingFilterLineRowStoreScratchBuffer));
    MOS_ZeroMemory(&resDeblockingFilterTileRowStoreScratchBuffer, sizeof(resDeblockingFilterTileRowStoreScratchBuffer));
    MOS_ZeroMemory(&resDeblockingFilterColumnRowStoreScratchBuffer, sizeof(resDeblockingFilterColumnRowStoreScratchBuffer));
    MOS_ZeroMemory(&resMetadataLineBuffer, sizeof(resMetadataLineBuffer));
    MOS_ZeroMemory(&resMetadataTileLineBuffer, sizeof(resMetadataTileLineBuffer));
    MOS_ZeroMemory(&resMetadataTileColumnBuffer, sizeof(resMetadataTileColumnBuffer));
    MOS_ZeroMemory(&resHvcLineRowstoreBuffer, sizeof(resHvcLineRowstoreBuffer));
    MOS_ZeroMemory(&resHvcTileRowstoreBuffer, sizeof(resHvcTileRowstoreBuffer));
    MOS_ZeroMemory(&resVp9SegmentIdBuffer, sizeof(resVp9SegmentIdBuffer));
    MOS_ZeroMemory(&resVp9MvTemporalBuffer, sizeof(resVp9MvTemporalBuffer));   
    MOS_ZeroMemory(&resCopyDataBuffer, sizeof(resCopyDataBuffer));
    MOS_ZeroMemory(&RefFrameMap, sizeof(RefFrameMap));    
    MOS_ZeroMemory(&SegTreeProbs, sizeof(SegTreeProbs));
    MOS_ZeroMemory(&SegPredProbs, sizeof(SegPredProbs));
    MOS_ZeroMemory(&InterProbSaved, sizeof(InterProbSaved));    
    MOS_ZeroMemory(&resDmemBuffer, sizeof(resDmemBuffer));
    MOS_ZeroMemory(&resInterProbSaveBuffer, sizeof(resInterProbSaveBuffer));
    MOS_ZeroMemory(&ProbUpdateFlags, sizeof(ProbUpdateFlags));
    MOS_ZeroMemory(&resSegmentIdBuffReset, sizeof(resSegmentIdBuffReset));
    MOS_ZeroMemory(&resHucSharedBuffer, sizeof(resHucSharedBuffer));
    MOS_ZeroMemory(&m_picMhwParams, sizeof(m_picMhwParams));

    PrevFrameParams.value           = 0;
    
    for (uint8_t i = 0; i < CODEC_VP9_NUM_CONTEXTS; i++)
    {
        bPendingResetFullTables[i]  = 0;
        bPendingCopySegProbs[i]     = 0;
    }   

    m_hcpInUse = true;
}

MOS_STATUS CodechalDecodeVp9 :: ProbBufferPartialUpdatewithDrv()
{    
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CodechalResLock ResourceLock(m_osInterface, &resVp9ProbBuffer[ucFrameCtxIdx]);
    auto data = (uint8_t*)ResourceLock.Lock(CodechalResLock::writeOnly);
    CODECHAL_DECODE_CHK_NULL_RETURN(data);

    if (ProbUpdateFlags.bSegProbCopy)
    {        
        CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            (data + CODEC_VP9_SEG_PROB_OFFSET), 
            7, 
            ProbUpdateFlags.SegTreeProbs, 
            7));
        CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            (data + CODEC_VP9_SEG_PROB_OFFSET + 7), 
            3, 
            ProbUpdateFlags.SegPredProbs, 
            3)); 
    }

    if (ProbUpdateFlags.bProbSave)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            InterProbSaved, 
            CODECHAL_VP9_INTER_PROB_SIZE, 
            data + CODEC_VP9_INTER_PROB_OFFSET, 
            CODECHAL_VP9_INTER_PROB_SIZE));
    }

    if (ProbUpdateFlags.bProbReset)
    {
        if (ProbUpdateFlags.bResetFull)
        {            
            CODECHAL_DECODE_CHK_STATUS_RETURN(ContextBufferInit(
                data, (ProbUpdateFlags.bResetKeyDefault ? true : false)));
        }
        else
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(CtxBufDiffInit(
                data, (ProbUpdateFlags.bResetKeyDefault ? true : false)));
        }
    }

    if (ProbUpdateFlags.bProbRestore)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            data + CODEC_VP9_INTER_PROB_OFFSET, 
            CODECHAL_VP9_INTER_PROB_SIZE,
            InterProbSaved, 
            CODECHAL_VP9_INTER_PROB_SIZE));
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeVp9 :: ProbBufFullUpdatewithDrv()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    
    CODECHAL_DECODE_FUNCTION_ENTER;

    CodechalResLock ResourceLock(m_osInterface, &resVp9ProbBuffer[ucFrameCtxIdx]);
    auto data = (uint8_t*)ResourceLock.Lock(CodechalResLock::writeOnly);
    CODECHAL_DECODE_CHK_NULL_RETURN(data);

    CODECHAL_DECODE_CHK_STATUS_RETURN(ContextBufferInit(
        data, (ProbUpdateFlags.bResetKeyDefault ? true : false)));
    CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
        (data + CODEC_VP9_SEG_PROB_OFFSET), 
        7, 
        ProbUpdateFlags.SegTreeProbs, 
        7));
    CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
        (data + CODEC_VP9_SEG_PROB_OFFSET + 7), 
        3, 
        ProbUpdateFlags.SegPredProbs, 
        3)); 

    return eStatus;
}

MOS_STATUS CodechalDecodeVp9 :: ResetSegIdBufferwithDrv()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CodechalResLock ResourceLock(m_osInterface, &resVp9SegmentIdBuffer);
    auto data = (uint8_t*)ResourceLock.Lock(CodechalResLock::writeOnly);
    CODECHAL_DECODE_CHK_NULL_RETURN(data);

    MOS_ZeroMemory(
        data,
        dwAllocatedWidthInSB * dwAllocatedHeightInSB * CODECHAL_CACHELINE_SIZE);

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

    CodechalResLock ResourceLock(m_osInterface, &resVp9ProbBuffer[CODEC_VP9_NUM_CONTEXTS]);
    auto data = (uint8_t*)ResourceLock.Lock(CodechalResLock::writeOnly);
    CODECHAL_DECODE_CHK_NULL_RETURN(data);

    CODECHAL_DECODE_CHK_STATUS_RETURN(ContextBufferInit(
        data,
        (ProbUpdateFlags.bResetKeyDefault ? true : false)));
    CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
        (data + CODEC_VP9_SEG_PROB_OFFSET), 
        7, 
        ProbUpdateFlags.SegTreeProbs, 7));
    CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
        (data + CODEC_VP9_SEG_PROB_OFFSET + 7), 
        3, 
        ProbUpdateFlags.SegPredProbs, 3)); 

    CODECHAL_DECODE_CHK_STATUS_RETURN(HucCopy(
        cmdBuffer,                                      // cmdBuffer
        &resVp9ProbBuffer[CODEC_VP9_NUM_CONTEXTS],   // presSrc
        &resVp9ProbBuffer[ucFrameCtxIdx],               // presDst
        bufSize,                                      // u32CopyLength
        0,                                              // u32CopyInputOffset
        0));                                            // u32CopyOutputOffset

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
        dwAllocatedWidthInSB *dwAllocatedHeightInSB * CODECHAL_CACHELINE_SIZE; // 16 byte aligned

    CODECHAL_DECODE_CHK_STATUS_RETURN(HucCopy(
        cmdBuffer,             // cmdBuffer
        &resSegmentIdBuffReset, // presSrc
        &resVp9SegmentIdBuffer, // presDst
        bufSize,              // u32CopyLength
        0,                      // u32CopyInputOffset
        0));                    // u32CopyOutputOffset

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
   
    bool keyFrame           = !pVp9PicParams->PicFlags.fields.frame_type;
    bool intraOnly          = pVp9PicParams->PicFlags.fields.intra_only;
    uint8_t curFrameCtxIdx  = (uint8_t)pVp9PicParams->PicFlags.fields.frame_context_idx;
    bool isScaling          = (sDestSurface.dwWidth == dwPrevFrmWidth) && 
                               (sDestSurface.dwHeight == dwPrevFrmHeight) ? false : true;
    bool resetAll           = (keyFrame                                           |
                             pVp9PicParams->PicFlags.fields.error_resilient_mode ||
                             (pVp9PicParams->PicFlags.fields.reset_frame_context == 3 &&
                             pVp9PicParams->PicFlags.fields.intra_only));
    bool resetSpecified     = (pVp9PicParams->PicFlags.fields.reset_frame_context == 2 &&
                               pVp9PicParams->PicFlags.fields.intra_only);
    
    bool copySegProbs       = false;  // indicating if current frame's prob buffer need to update seg tree/pred probs.
    bool resetFullTbl       = false;  // indicating if current frame will do full frame context table reset
    bool resetPartialTbl    = false;  // indicating if current frame need to do partial reset from offset 1667 to 2010.
    bool restoreInterProbs  = false;  // indicating if current frame need to restore offset 1667 to 2010 from saved one, this is only for prob buffer 0.
    bool saveInterProbsTmp  = false;  // indicating if current frame need to save offset from 1667 to 2010 for prob buffer 0.

    bResetSegIdBuffer  = keyFrame                                           || 
                         isScaling                                          ||
                         pVp9PicParams->PicFlags.fields.error_resilient_mode || 
                         pVp9PicParams->PicFlags.fields.intra_only;
    

    ucFrameCtxIdx = curFrameCtxIdx; //indicate which prob buffer need to be used by current frame decode
    if( !pVp9PicParams->PicFlags.fields.frame_type      || 
        pVp9PicParams->PicFlags.fields.intra_only       || 
        pVp9PicParams->PicFlags.fields.error_resilient_mode)
    {
        //always use frame context idx 0 in this case
        ucFrameCtxIdx = 0;
    }

    //check if seg tree/pred probs need to be updated in prob buffer of current frame
    //and also mark the flag bPendingResetSegProbs for other prob buffers
    if (pVp9PicParams->PicFlags.fields.segmentation_enabled &&
        pVp9PicParams->PicFlags.fields.segmentation_update_map)
    {
        copySegProbs = true;
        for ( uint8_t ctxIdx = 0; ctxIdx < CODEC_VP9_NUM_CONTEXTS; ctxIdx++)
        {
            bPendingCopySegProbs[ctxIdx] = true; 
        }        
        //set current frame's prob buffer pending copy to false
        bPendingCopySegProbs[ucFrameCtxIdx] = false; 
        
        //save probs for pending copy
        MOS_SecureMemcpy(SegTreeProbs, 7, pVp9PicParams->SegTreeProbs, 7);
        MOS_SecureMemcpy(SegPredProbs, 3, pVp9PicParams->SegPredProbs, 3);
    }
    else if (pVp9PicParams->PicFlags.fields.segmentation_enabled &&
             bPendingCopySegProbs[ucFrameCtxIdx])
    {
        copySegProbs = true;        
        bPendingCopySegProbs[ucFrameCtxIdx] = false; 
    }
    
    //check if probs in frame context table need to be updated for current frame's prob buffer
    //and also mark the flag bPendingResetFullTables for other prob buffers
    if (resetAll)
    {
        resetFullTbl = true;
        bPendingResetPartial = (keyFrame || intraOnly);         

        //prob buffer 0 will be used for current frame decoding
        for ( uint8_t ctxIdx = 1; ctxIdx < CODEC_VP9_NUM_CONTEXTS; ctxIdx++)
        {
            bPendingResetFullTables[ctxIdx] = true; 
        }
        bSaveInterProbs = false;
    }
    else if (resetSpecified)
    {
        //intra only frame:prob buffer 0 will always be used for current frame decoding
        if (curFrameCtxIdx == 0)
        {
            //do prob table 0 reset
            resetFullTbl = true;
            bPendingResetPartial = true;     
            bSaveInterProbs = false;
        }
        else
        {
            //not do reset right now, pending the reset full table of specified ctx until a later frame use it to do decode
            bPendingResetFullTables[curFrameCtxIdx] = true; 
            if (!bPendingResetPartial)
            {
                if (!bSaveInterProbs)
                {
                    saveInterProbsTmp = true;
                    bSaveInterProbs    = true;
                }
                resetPartialTbl = true;
            }
        }
    }
    else if (intraOnly) 
    {        
        //prob buffer 0 will be used for current frame decoding   
        if (!bPendingResetPartial)
        {
            if (!bSaveInterProbs)
            {
                saveInterProbsTmp = true;
                bSaveInterProbs    = true;
            }
            resetPartialTbl = true;
        }
    }
    else if (bPendingResetFullTables[curFrameCtxIdx])
    {
        //here curFrameCtxIdx != 0, frame is inter frame
        resetFullTbl = true;
        bPendingResetFullTables[curFrameCtxIdx] = false;
    }
    else if (curFrameCtxIdx == 0 && bPendingResetPartial)
    {
        //here curFrameCtxIdx = 0, frame is inter frame
        resetPartialTbl = true;        
        bPendingResetPartial = false;
    }
    else if (curFrameCtxIdx == 0 && bSaveInterProbs)
    {
        //here curFrameCtxIdx = 0, frame is inter frame
        restoreInterProbs = true;
        bSaveInterProbs =  false;
    }

    //decide if prob buffer need to do a full udpate or partial upate
    if (resetFullTbl && copySegProbs)
    {
        //update the whole prob buffer        
        bFullProbBufferUpdate = true;
    }
    else
    {
        //partial buffer update
        bFullProbBufferUpdate = false;
    }
    
    //propogate ProbUpdateFlags   
    MOS_ZeroMemory(&ProbUpdateFlags, sizeof(ProbUpdateFlags));
    if (copySegProbs)
    { 
        ProbUpdateFlags.bSegProbCopy = true;
        MOS_SecureMemcpy(ProbUpdateFlags.SegTreeProbs, 7, SegTreeProbs, 7);
        MOS_SecureMemcpy(ProbUpdateFlags.SegPredProbs, 3, SegPredProbs, 3);
    }    
    ProbUpdateFlags.bProbReset = resetFullTbl || resetPartialTbl;
    ProbUpdateFlags.bResetFull = resetFullTbl;
    ProbUpdateFlags.bResetKeyDefault = (keyFrame || intraOnly);
    ProbUpdateFlags.bProbSave    = saveInterProbsTmp;
    ProbUpdateFlags.bProbRestore = restoreInterProbs;
            
    return eStatus;
}

MOS_STATUS CodechalDecodeVp9 :: AllocateResourcesFixedSizes()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateSyncResource(
        m_osInterface, &resSyncObject));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateSyncResource(
        m_osInterface, &resSyncObjectWaContextInUse));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateSyncResource(
        m_osInterface, &resSyncObjectVideoContextInUse));

    CodecHalAllocateDataList(
        pVp9RefList,
        CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9);

    // VP9 Probability buffer
    for (uint8_t i = 0; i < CODEC_VP9_NUM_CONTEXTS + 1; i++)
    {
        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
            &resVp9ProbBuffer[i],
            MOS_ALIGN_CEIL(CODEC_VP9_PROB_MAX_NUM_ELEM, CODECHAL_PAGE_SIZE),
            "Vp9ProbabilityBuffer"),
            "Failed to allocate VP9 probability Buffer.");

        CodechalResLock ResourceLock(m_osInterface, &resVp9ProbBuffer[i]);
        auto data = (uint8_t*)ResourceLock.Lock(CodechalResLock::writeOnly);
        CODECHAL_DECODE_CHK_NULL_RETURN(data);

        MOS_ZeroMemory(data, CODEC_VP9_PROB_MAX_NUM_ELEM);
        //initialize seg_tree_prob and seg_pred_prob
        MOS_FillMemory((data + CODEC_VP9_SEG_PROB_OFFSET), 7, CODEC_VP9_MAX_PROB);
        MOS_FillMemory((data + CODEC_VP9_SEG_PROB_OFFSET + 7), 3, CODEC_VP9_MAX_PROB);
    }
    
    
    // DMEM buffer send to HuC FW
    dwDmemBufferSize = MOS_ALIGN_CEIL(sizeof(CODECHAL_DECODE_VP9_PROB_UPDATE), CODECHAL_CACHELINE_SIZE);
    CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
        &resDmemBuffer,
        dwDmemBufferSize,
        "DmemBuffer"),
        "Failed to allocate Dmem Buffer.");

    
    // VP9 Interprobs save buffer
    CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
        &resInterProbSaveBuffer,
        MOS_ALIGN_CEIL(CODECHAL_VP9_INTER_PROB_SIZE, CODECHAL_PAGE_SIZE),
        "VP9InterProbsSaveBuffer"),
        "Failed to allocate VP9 inter probability save Buffer.");

    // VP9 shared buffer with HuC FW, mapping to region 15
    CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
        &resHucSharedBuffer,
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
    uint8_t maxBitDepth  = 8 + ucVP9DepthIndicator*2;
    uint8_t chromaFormat = ucChromaFormatinProfile;

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
    reallocParam.dwPicWidthAlloced  = dwAllocatedWidthInSB;
    reallocParam.dwPicHeight        = heightInSb;
    reallocParam.dwPicHeightAlloced = dwAllocatedHeightInSB;
    
    if (!m_hcpInterface->IsVp9DfRowstoreCacheEnabled())
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->IsVp9BufferReallocNeeded(
            MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_LINE, 
            &reallocParam));
        if (reallocParam.bNeedBiggerSize || 
            Mos_ResourceIsNull(&resDeblockingFilterLineRowStoreScratchBuffer))
        {
            if (!Mos_ResourceIsNull(&resDeblockingFilterLineRowStoreScratchBuffer))
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &resDeblockingFilterLineRowStoreScratchBuffer);
            }

            // Deblocking Filter Line Row Store Scratch data surface
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->GetVp9BufferSize(
                MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_LINE,
                &hcpBufSizeParam)); 

            CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                &resDeblockingFilterLineRowStoreScratchBuffer,
                hcpBufSizeParam.dwBufferSize,
                "DeblockingLineScratchBuffer"),
                "Failed to allocate deblocking line scratch Buffer.");
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->IsVp9BufferReallocNeeded(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_LINE, 
        &reallocParam));
    if (reallocParam.bNeedBiggerSize || 
        Mos_ResourceIsNull(&resDeblockingFilterTileRowStoreScratchBuffer))
    {
        if (!Mos_ResourceIsNull(&resDeblockingFilterTileRowStoreScratchBuffer))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &resDeblockingFilterTileRowStoreScratchBuffer);
        }

        // Deblocking Filter Tile Row Store Scratch data surface
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->GetVp9BufferSize(
            MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_LINE,
            &hcpBufSizeParam)); 

        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
            &resDeblockingFilterTileRowStoreScratchBuffer,
            hcpBufSizeParam.dwBufferSize,
            "DeblockingTileScratchBuffer"),
            "Failed to allocate deblocking tile scratch Buffer.");
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->IsVp9BufferReallocNeeded(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_COL, 
        &reallocParam));
    if (reallocParam.bNeedBiggerSize || 
        Mos_ResourceIsNull(&resDeblockingFilterColumnRowStoreScratchBuffer))
    {
        if (!Mos_ResourceIsNull(&resDeblockingFilterColumnRowStoreScratchBuffer))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &resDeblockingFilterColumnRowStoreScratchBuffer);
        }
        // Deblocking Filter Column Row Store Scratch data surface
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->GetVp9BufferSize(
            MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_COL,
            &hcpBufSizeParam)); 

        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
            &resDeblockingFilterColumnRowStoreScratchBuffer,
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
            Mos_ResourceIsNull(&resMetadataLineBuffer))
        {
            if (!Mos_ResourceIsNull(&resMetadataLineBuffer))
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &resMetadataLineBuffer);
            }
            
            // Metadata Line buffer
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->GetVp9BufferSize(
                MHW_VDBOX_HCP_INTERNAL_BUFFER_META_LINE,
                &hcpBufSizeParam)); 

            CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                &resMetadataLineBuffer,
                hcpBufSizeParam.dwBufferSize,
                "MetadataLineBuffer"),
                "Failed to allocate meta data line Buffer.");
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->IsVp9BufferReallocNeeded(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_LINE, 
        &reallocParam));
    if (reallocParam.bNeedBiggerSize || 
        Mos_ResourceIsNull(&resMetadataTileLineBuffer))
    {
        if (!Mos_ResourceIsNull(&resMetadataTileLineBuffer))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &resMetadataTileLineBuffer);
        }
        // Metadata Tile Line buffer
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->GetVp9BufferSize(
            MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_LINE,
            &hcpBufSizeParam)); 

        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
            &resMetadataTileLineBuffer,
            hcpBufSizeParam.dwBufferSize,
            "MetadataTileLineBuffer"),
            "Failed to allocate meta data tile line Buffer.");
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->IsVp9BufferReallocNeeded(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_COL, 
        &reallocParam));
    if (reallocParam.bNeedBiggerSize || 
        Mos_ResourceIsNull(&resMetadataTileColumnBuffer))
    {
        if (!Mos_ResourceIsNull(&resMetadataTileColumnBuffer))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &resMetadataTileColumnBuffer);
        }
        // Metadata Tile Column buffer
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->GetVp9BufferSize(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_COL,
        &hcpBufSizeParam)); 

        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
            &resMetadataTileColumnBuffer,
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
            Mos_ResourceIsNull(&resHvcLineRowstoreBuffer))
        {
            if (!Mos_ResourceIsNull(&resHvcLineRowstoreBuffer))
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &resHvcLineRowstoreBuffer);
            }

            // HVC Line Row Store Buffer
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->GetVp9BufferSize(
                MHW_VDBOX_VP9_INTERNAL_BUFFER_HVD_LINE,
                &hcpBufSizeParam)); 

            CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                &resHvcLineRowstoreBuffer,
                hcpBufSizeParam.dwBufferSize,
                "HvcLineRowStoreBuffer"),
                "Failed to allocate Hvc line row store Buffer.");
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->IsVp9BufferReallocNeeded(
        MHW_VDBOX_VP9_INTERNAL_BUFFER_HVD_TILE, 
        &reallocParam));
    if (reallocParam.bNeedBiggerSize || 
        Mos_ResourceIsNull(&resHvcTileRowstoreBuffer))
    {
        if (!Mos_ResourceIsNull(&resHvcTileRowstoreBuffer))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &resHvcTileRowstoreBuffer);
        }
        // HVC Tile Row Store Buffer
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->GetVp9BufferSize(
            MHW_VDBOX_VP9_INTERNAL_BUFFER_HVD_TILE,
            &hcpBufSizeParam)); 

        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
            &resHvcTileRowstoreBuffer,
            hcpBufSizeParam.dwBufferSize,
            "HvcTileRowStoreBuffer"),
            "Failed to allocate Hvc tile row store Buffer.");
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->IsVp9BufferReallocNeeded(
            MHW_VDBOX_VP9_INTERNAL_BUFFER_SEGMENT_ID, 
            &reallocParam));
    if (reallocParam.bNeedBiggerSize || 
        Mos_ResourceIsNull(&resVp9SegmentIdBuffer))
    {
        if (!Mos_ResourceIsNull(&resVp9SegmentIdBuffer))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &resVp9SegmentIdBuffer);
        }
        // VP9 Segment ID buffer
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->GetVp9BufferSize(
            MHW_VDBOX_VP9_INTERNAL_BUFFER_SEGMENT_ID,
            &hcpBufSizeParam)); 

        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
            &resVp9SegmentIdBuffer,
            hcpBufSizeParam.dwBufferSize,
            "Vp9SegmentIdBuffer"),
            "Failed to allocate VP9 segment ID Buffer.");
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->IsVp9BufferReallocNeeded(
        MHW_VDBOX_VP9_INTERNAL_BUFFER_SEGMENT_ID, 
        &reallocParam));
    if (reallocParam.bNeedBiggerSize || Mos_ResourceIsNull(&resSegmentIdBuffReset))
    {
        if (!Mos_ResourceIsNull(&resSegmentIdBuffReset))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &resSegmentIdBuffReset);
        }
        // VP9 Segment ID Reset buffer
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->GetVp9BufferSize(
            MHW_VDBOX_VP9_INTERNAL_BUFFER_SEGMENT_ID,
            &hcpBufSizeParam)); 

        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
            &resSegmentIdBuffReset,
            hcpBufSizeParam.dwBufferSize,
            "SegmentIdBuffreset",
            true,
            0),
            "Failed to allocate segment ID reset Buffer.");
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->IsVp9BufferReallocNeeded(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_CURR_MV_TEMPORAL, 
        &reallocParam));
    if (reallocParam.bNeedBiggerSize || dwMVBufferSize == 0)
    {
        for (uint8_t i = 0; i < CODECHAL_VP9_NUM_MV_BUFFERS; i++)
        {
            if (!Mos_ResourceIsNull(&resVp9MvTemporalBuffer[i]))
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &resVp9MvTemporalBuffer[i]);
            }
        }

        // VP9 MV Temporal buffers
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->GetVp9BufferSize(
            MHW_VDBOX_HCP_INTERNAL_BUFFER_CURR_MV_TEMPORAL,
            &hcpBufSizeParam)); 

        for (uint8_t i = 0; i < CODECHAL_VP9_NUM_MV_BUFFERS; i++)
        {
            CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                &resVp9MvTemporalBuffer[i],
                hcpBufSizeParam.dwBufferSize,
                "MvTemporalBuffer"),
                "Failed to allocate Mv temporal Buffer.");
        }

        dwMVBufferSize = hcpBufSizeParam.dwBufferSize;
    }

    if (m_secureDecoder)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->AllocateResource(this));
    }

    //backup allocated memory size
    dwAllocatedWidthInSB  = widthInSb;
    dwAllocatedHeightInSB = heightInSb;

    return eStatus;
}

MOS_STATUS CodechalDecodeVp9 :: InitializeBeginFrame()
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    m_incompletePicture = false;
    bCopyDataBufferInUse = false;
    dwCopyDataOffset = 0;

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
        &cmdBuffer,         // pCmdBuffer
        &resDataBuffer,     // presSrc
        &resCopyDataBuffer, // presDst
        dwDataSize,         // u32CopyLength
        dwDataOffset,       // u32CopyInputOffset
        dwCopyDataOffset)); // u32CopyOutputOffset

    dwCopyDataOffset += MOS_ALIGN_CEIL(dwDataSize, MHW_CACHELINE_SIZE);
    
    MHW_MI_FLUSH_DW_PARAMS  flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
        &cmdBuffer, 
        &flushDwParams));
    
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddWatchdogTimerStopCmd(
        &cmdBuffer));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
        &cmdBuffer,
        nullptr));
    
    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);
    
    // sync resource
    if (!m_incompletePicture)
    {
        MOS_SYNC_PARAMS syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContext;
        syncParams.presSyncResource = &resSyncObjectVideoContextInUse;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));
        
        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContextForWa;
        syncParams.presSyncResource = &resSyncObjectVideoContextInUse;
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
    if (pVp9SliceParams == nullptr)
    {
        badSliceChopping = 0;
    }
    else
    {
        badSliceChopping = pVp9SliceParams->wBadSliceChopping;
    }

    // No BSBytesInBuffer is sent from App, driver here just give an estimation.
    if (badSliceChopping != 0)
    {
        pVp9PicParams->BSBytesInBuffer = 
            (pVp9PicParams->FrameWidthMinus1 + 1) * (pVp9PicParams->FrameHeightMinus1 + 1) * 6;
    }

    if (m_firstExecuteCall) // first exec call
    {
        if (dwDataSize < pVp9PicParams->BSBytesInBuffer)  // Current bitstream buffer is not big enough
        {
            // Allocate or reallocate the copy data buffer.
            if (dwCopyDataBufferSize < MOS_ALIGN_CEIL(pVp9PicParams->BSBytesInBuffer, 64))
            {
                if (!Mos_ResourceIsNull(&resCopyDataBuffer))
                {
                    m_osInterface->pfnFreeResource(
                        m_osInterface,
                        &resCopyDataBuffer);
                }

                dwCopyDataBufferSize = MOS_ALIGN_CEIL(pVp9PicParams->BSBytesInBuffer, 64);

                CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                    &resCopyDataBuffer,
                    dwCopyDataBufferSize,
                    "Vp9CopyDataBuffer"),
                    "Failed to allocate Vp9 copy data Buffer.");
            }

            // Copy bitstream into the copy buffer
            if (dwDataSize)
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(CopyDataSurface());

                bCopyDataBufferInUse = true;
            }

            m_incompletePicture = true;
        }
    }
    else // second and later exec calls
    {
        CODECHAL_DECODE_CHK_COND_RETURN(
            (dwCopyDataOffset + dwDataSize > dwCopyDataBufferSize),
            "Bitstream size exceeds copy data buffer size!");

        // Copy bitstream into the copy buffer
        if (dwDataSize)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(CopyDataSurface());
        }

        if (dwCopyDataOffset >= pVp9PicParams->BSBytesInBuffer || badSliceChopping == 2)
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

    CODECHAL_DECODE_FUNCTION_ENTER;
    
    CODECHAL_DECODE_CHK_NULL_RETURN(m_decodeParams.m_destSurface);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_decodeParams.m_dataBuffer);
        
    if (m_cencDecoder)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_cencDecoder->SetParamsForDecode(this, m_hwInterface, m_debugInterface, &m_decodeParams));

        CODECHAL_DEBUG_TOOL(
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &resDataBuffer,
                CodechalDbgAttr::attrBitstream,
                "_DEC",
                dwDataSize,
                dwDataOffset,
                CODECHAL_NUM_MEDIA_STATES));
        )
    }
    else
    {
        dwDataSize           = m_decodeParams.m_dataSize;
        dwDataOffset         = m_decodeParams.m_dataOffset;
        pVp9PicParams        = (PCODEC_VP9_PIC_PARAMS)m_decodeParams.m_picParams;
        pVp9SegmentParams    = (PCODEC_VP9_SEGMENT_PARAMS)m_decodeParams.m_iqMatrixBuffer;
        pVp9SliceParams      = (PCODEC_VP9_SLICE_PARAMS)m_decodeParams.m_sliceParams;

        CODECHAL_DECODE_CHK_NULL_RETURN(pVp9SegmentParams);

        sDestSurface         = *(m_decodeParams.m_destSurface);
        resDataBuffer        = *(m_decodeParams.m_dataBuffer);
        if (m_decodeParams.m_coefProbBuffer)        // This is an optional buffer passed from App. To be removed once VP9 FF Decode Driver is mature.
        {
            resCoefProbBuffer    = *(m_decodeParams.m_coefProbBuffer);
        }

        if (m_firstExecuteCall)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(InitializeBeginFrame());
        }

        CODECHAL_DECODE_CHK_STATUS_RETURN(CheckAndCopyBitStream());
    }

    // Bitstream is incomplete, don't do any decoding work.
    if (m_incompletePicture)
    {
        eStatus = MOS_STATUS_SUCCESS;
        return eStatus;
    }

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            bCopyDataBufferInUse ? &resCopyDataBuffer : &resDataBuffer,
            CodechalDbgAttr::attrBitstream,
            "_DEC",
            bCopyDataBufferInUse ? dwCopyDataBufferSize : dwDataSize,
            bCopyDataBufferInUse ? 0 : dwDataOffset,
            CODECHAL_NUM_MEDIA_STATES));
    )

    m_statusReportFeedbackNumber = pVp9PicParams->StatusReportFeedbackNumber;
    m_width                  =
        MOS_MAX(m_width, (uint32_t)(pVp9PicParams->FrameWidthMinus1 + 1));
    m_height                 =
        MOS_MAX(m_height, (uint32_t)(pVp9PicParams->FrameHeightMinus1 + 1));
    usFrameWidthAlignedMinBlk  = 
        MOS_ALIGN_CEIL(pVp9PicParams->FrameWidthMinus1 + 1, CODEC_VP9_MIN_BLOCK_WIDTH);
    usFrameHeightAlignedMinBlk = 
        MOS_ALIGN_CEIL(pVp9PicParams->FrameHeightMinus1 + 1, CODEC_VP9_MIN_BLOCK_WIDTH);

    // Overwrite the actual surface height with the coded height and width of the frame
    // for VP9 since it's possible for a VP9 frame to change size during playback
    sDestSurface.dwWidth     = pVp9PicParams->FrameWidthMinus1 + 1;
    sDestSurface.dwHeight    = pVp9PicParams->FrameHeightMinus1 + 1;
   
    PCODEC_REF_LIST destEntry = pVp9RefList[pVp9PicParams->CurrPic.FrameIdx];

    // Clear FilterLevel Array inside segment data when filter_level inside picparam is zero
    if (m_cencDecoder == nullptr)
    {
        MOS_ZeroMemory(destEntry, sizeof(CODEC_REF_LIST));
        // Clear FilterLevel Array inside segment data when filter_level inside picparam is zero
        if ((!pVp9PicParams->filter_level))
        {
            PCODEC_VP9_SEG_PARAMS     vp9SegData = &pVp9SegmentParams->SegData[0];
            
            for (uint8_t i = 0; i < 8; i++)
            {
                *((uint32_t *)&vp9SegData->FilterLevel[0][0]) = 0;
                *((uint32_t *)&vp9SegData->FilterLevel[2][0]) = 0;
                vp9SegData++;      // Go on to next record.
            }
        }
    }
    destEntry->resRefPic       = sDestSurface.OsResource;
    destEntry->dwFrameWidth    = pVp9PicParams->FrameWidthMinus1 + 1;
    destEntry->dwFrameHeight   = pVp9PicParams->FrameHeightMinus1 + 1;


    if (m_hcpInterface->IsRowStoreCachingSupported() &&
        usFrameWidthAlignedMinBlk != MOS_ALIGN_CEIL(dwPrevFrmWidth, CODEC_VP9_MIN_BLOCK_WIDTH))
    {
        MHW_VDBOX_ROWSTORE_PARAMS rowstoreParams;
        uint8_t usChromaSamplingFormat;
        if (pVp9PicParams->subsampling_x == 1 && pVp9PicParams->subsampling_y == 1)
        {
            usChromaSamplingFormat = HCP_CHROMA_FORMAT_YUV420;
        }
        else if (pVp9PicParams->subsampling_x == 0 && pVp9PicParams->subsampling_y == 0)
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
        rowstoreParams.dwPicWidth   = usFrameWidthAlignedMinBlk;
        rowstoreParams.bMbaff       = false;
        rowstoreParams.Mode         = CODECHAL_DECODE_MODE_VP9VLD;
        rowstoreParams.ucBitDepthMinus8 = pVp9PicParams->BitDepthMinus8;
        rowstoreParams.ucChromaFormat   = usChromaSamplingFormat;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->SetRowstoreCachingOffsets(&rowstoreParams));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(InitializeDecodeMode());
    CODECHAL_DECODE_CHK_STATUS_RETURN(InitSfcState());

    // Allocate internal buffer or reallocate When current resolution is bigger than allocated internal buffer size
    CODECHAL_DECODE_CHK_STATUS_RETURN(AllocateResourcesVariableSizes());
    
    CODECHAL_DECODE_CHK_STATUS_RETURN(DetermineInternalBufferUpdate());

    HcpDecPhase = CodechalHcpDecodePhaseInitialized;

    m_perfType = pVp9PicParams->PicFlags.fields.frame_type ? P_TYPE : I_TYPE;

    m_crrPic = pVp9PicParams->CurrPic;

    if (pVp9PicParams->PicFlags.fields.frame_type == CODEC_VP9_INTER_FRAME &&
        !pVp9PicParams->PicFlags.fields.intra_only)
    {
        ucCurMvTempBufIdx = (ucCurMvTempBufIdx + 1) % CODECHAL_VP9_NUM_MV_BUFFERS;
        ucColMvTempBufIdx = (ucCurMvTempBufIdx < 1) ? (CODECHAL_VP9_NUM_MV_BUFFERS - 1) : (ucCurMvTempBufIdx - 1);
    }

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_NULL_RETURN(m_debugInterface);
        pVp9PicParams->CurrPic.PicFlags            = PICTURE_FRAME;
        m_debugInterface->CurrPic       = m_crrPic;
        m_debugInterface->wFrameType    = m_perfType;
        
        CODECHAL_DECODE_CHK_STATUS_RETURN(DumpDecodePicParams(
            pVp9PicParams));

        if (pVp9SegmentParams)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(DumpDecodeSegmentParams(
                pVp9SegmentParams));
        }
    )

    return eStatus;
}

MOS_STATUS CodechalDecodeVp9 :: DetermineDecodePhase()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;
    
    uint32_t curPhase = HcpDecPhase;
    switch (curPhase)
    {
    case CodechalHcpDecodePhaseInitialized:
        HcpDecPhase = CodechalHcpDecodePhaseLegacyLong;
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

    CODECHAL_DECODE_FUNCTION_ENTER;

    // Reset all pic Mhw Params
    MOS_ZeroMemory(m_picMhwParams.PipeModeSelectParams, sizeof(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS));
    MOS_ZeroMemory(m_picMhwParams.PipeBufAddrParams, sizeof(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS));
    MOS_ZeroMemory(m_picMhwParams.IndObjBaseAddrParams, sizeof(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS));
    MOS_ZeroMemory(m_picMhwParams.Vp9PicState, sizeof(MHW_VDBOX_VP9_PIC_STATE));
    MOS_ZeroMemory(m_picMhwParams.Vp9SegmentState, sizeof(MHW_VDBOX_VP9_SEGMENT_STATE));
    
    PCODEC_PICTURE refFrameList    = &(pVp9PicParams->RefFrameList[0]); 
    uint8_t lastRefPicIndex       = pVp9PicParams->PicFlags.fields.LastRefIdx;
    uint8_t goldenRefPicIndex     = pVp9PicParams->PicFlags.fields.GoldenRefIdx;
    uint8_t altRefPicIndex        = pVp9PicParams->PicFlags.fields.AltRefIdx;
    if(pVp9PicParams->PicFlags.fields.frame_type == CODEC_VP9_KEY_FRAME || 
       pVp9PicParams->PicFlags.fields.intra_only)  
    {
        // reference surface should be nullptr when key_frame == true or intra only frame
        presLastRefSurface   = nullptr;
        presGoldenRefSurface = nullptr;
        presAltRefSurface    = nullptr;
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
        PCODEC_REF_LIST  *vp9RefList = &(pVp9RefList[0]);        
        presLastRefSurface   = &(vp9RefList[refFrameList[lastRefPicIndex].FrameIdx]->resRefPic);
        presGoldenRefSurface = &(vp9RefList[refFrameList[goldenRefPicIndex].FrameIdx]->resRefPic);
        presAltRefSurface    = &(vp9RefList[refFrameList[altRefPicIndex].FrameIdx]->resRefPic);
    }

    uint16_t usChromaSamplingFormat;    
    if (pVp9PicParams->subsampling_x == 1 && pVp9PicParams->subsampling_y == 1)
    {
        usChromaSamplingFormat = HCP_CHROMA_FORMAT_YUV420;
    }
    else if (pVp9PicParams->subsampling_x == 0 && pVp9PicParams->subsampling_y == 0)
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
    m_picMhwParams.SurfaceParams[0]->psSurface          = &sDestSurface;
    m_picMhwParams.SurfaceParams[0]->ChromaType         = (uint8_t)usChromaSamplingFormat;
    m_picMhwParams.SurfaceParams[0]->ucSurfaceStateId   = CODECHAL_HCP_DECODED_SURFACE_ID;
    m_picMhwParams.SurfaceParams[0]->ucBitDepthLumaMinus8   = pVp9PicParams->BitDepthMinus8;
    m_picMhwParams.SurfaceParams[0]->ucBitDepthChromaMinus8 = pVp9PicParams->BitDepthMinus8;
    m_picMhwParams.SurfaceParams[0]->dwUVPlaneAlignment = 8;

    // Populate surface param for reference pictures
    if (pVp9PicParams->PicFlags.fields.frame_type == CODEC_VP9_INTER_FRAME &&
       !pVp9PicParams->PicFlags.fields.intra_only && 
        presLastRefSurface != nullptr && 
        presGoldenRefSurface != nullptr && 
        presAltRefSurface != nullptr)
    {
        //MOS_SURFACE lastRefSurface;
        CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &sLastRefSurface.OsResource, 
            sizeof(MOS_RESOURCE), 
            presLastRefSurface, 
            sizeof(MOS_RESOURCE)));
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
            m_osInterface, 
            &sLastRefSurface));

        //MOS_SURFACE goldenRefSurface;
        CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &sGoldenRefSurface.OsResource, 
            sizeof(MOS_RESOURCE), 
            presGoldenRefSurface, 
            sizeof(MOS_RESOURCE)));
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
            m_osInterface, 
            &sGoldenRefSurface));

        //MOS_SURFACE altRefSurface;
        CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &sAltRefSurface.OsResource, 
            sizeof(MOS_RESOURCE), 
            presAltRefSurface, 
            sizeof(MOS_RESOURCE)));
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
            m_osInterface, 
            &sAltRefSurface));

        for (uint8_t i = 1; i < 4; i++)
        {
            m_picMhwParams.SurfaceParams[i]->Mode               = m_mode;
            m_picMhwParams.SurfaceParams[i]->ChromaType         = (uint8_t)usChromaSamplingFormat;
            m_picMhwParams.SurfaceParams[i]->dwUVPlaneAlignment = 8;

            switch (i)
            {
                case 1:
                    m_picMhwParams.SurfaceParams[i]->psSurface          = &sLastRefSurface;
                    m_picMhwParams.SurfaceParams[i]->ucSurfaceStateId   = CODECHAL_HCP_LAST_SURFACE_ID;
                    break;
                case 2:
                    m_picMhwParams.SurfaceParams[i]->psSurface          = &sGoldenRefSurface;
                    m_picMhwParams.SurfaceParams[i]->ucSurfaceStateId   = CODECHAL_HCP_GOLDEN_SURFACE_ID;
                    break;
                case 3:
                    m_picMhwParams.SurfaceParams[i]->psSurface          = &sAltRefSurface;
                    m_picMhwParams.SurfaceParams[i]->ucSurfaceStateId   = CODECHAL_HCP_ALTREF_SURFACE_ID;
                    break;
            }
        }
    }

    m_picMhwParams.PipeBufAddrParams->Mode                                          = m_mode;
    m_picMhwParams.PipeBufAddrParams->psPreDeblockSurface                           = &sDestSurface;

    m_picMhwParams.PipeBufAddrParams->presReferences[CodechalDecodeLastRef]         = presLastRefSurface;
    m_picMhwParams.PipeBufAddrParams->presReferences[CodechalDecodeGoldenRef]       = presGoldenRefSurface;
    m_picMhwParams.PipeBufAddrParams->presReferences[CodechalDecodeAlternateRef]    = presAltRefSurface;

    // set all ref pic addresses in HCP_PIPE_BUF_ADDR_STATE command to valid addresses for error concealment purpose, set the unused ones to the first used one
    for (uint8_t i = 0; i < CODECHAL_DECODE_VP9_MAX_NUM_REF_FRAME; i++)
    {
        if (!m_picMhwParams.PipeBufAddrParams->presReferences[i])
        {
            m_picMhwParams.PipeBufAddrParams->presReferences[i] = &(sDestSurface.OsResource);
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->SetPipeBufAddr(m_picMhwParams.PipeBufAddrParams));
    
    if (m_streamOutEnabled)
    {
        m_picMhwParams.PipeBufAddrParams->presStreamOutBuffer =
            &(m_streamOutBuffer[m_streamOutCurrBufIdx]);
    }
       
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->CheckReferenceList(m_picMhwParams.PipeBufAddrParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->SetRefrenceSync(m_disableDecodeSyncLock, m_disableLockForTranscode));
    
    m_picMhwParams.PipeBufAddrParams->presMfdDeblockingFilterRowStoreScratchBuffer =
        &resDeblockingFilterLineRowStoreScratchBuffer;
    m_picMhwParams.PipeBufAddrParams->presDeblockingFilterTileRowStoreScratchBuffer     =
        &resDeblockingFilterTileRowStoreScratchBuffer;
    m_picMhwParams.PipeBufAddrParams->presDeblockingFilterColumnRowStoreScratchBuffer   =
        &resDeblockingFilterColumnRowStoreScratchBuffer;

    m_picMhwParams.PipeBufAddrParams->presMetadataLineBuffer       = &resMetadataLineBuffer;
    m_picMhwParams.PipeBufAddrParams->presMetadataTileLineBuffer   = &resMetadataTileLineBuffer;
    m_picMhwParams.PipeBufAddrParams->presMetadataTileColumnBuffer = &resMetadataTileColumnBuffer; 
    m_picMhwParams.PipeBufAddrParams->presVp9ProbBuffer            = &resVp9ProbBuffer[ucFrameCtxIdx];
    m_picMhwParams.PipeBufAddrParams->presVp9SegmentIdBuffer       = &resVp9SegmentIdBuffer;
    m_picMhwParams.PipeBufAddrParams->presHvdLineRowStoreBuffer    = &resHvcLineRowstoreBuffer;
    m_picMhwParams.PipeBufAddrParams->presHvdTileRowStoreBuffer    = &resHvcTileRowstoreBuffer;

    if (pVp9PicParams->PicFlags.fields.frame_type == CODEC_VP9_INTER_FRAME &&
        !pVp9PicParams->PicFlags.fields.intra_only)
    {
        m_picMhwParams.PipeBufAddrParams->presCurMvTempBuffer = &resVp9MvTemporalBuffer[ucCurMvTempBufIdx];

        if (!PrevFrameParams.fields.KeyFrame && !PrevFrameParams.fields.IntraOnly)
        {   
            // For VP9, only index 0 is required to be filled
            m_picMhwParams.PipeBufAddrParams->presColMvTempBuffer[0] = &resVp9MvTemporalBuffer[ucColMvTempBufIdx];
        }
    }
    

    m_picMhwParams.IndObjBaseAddrParams->Mode           = m_mode;
    m_picMhwParams.IndObjBaseAddrParams->dwDataSize     = bCopyDataBufferInUse ? dwCopyDataBufferSize : dwDataSize;
    m_picMhwParams.IndObjBaseAddrParams->dwDataOffset   = bCopyDataBufferInUse ? 0: dwDataOffset;
    m_picMhwParams.IndObjBaseAddrParams->presDataBuffer = bCopyDataBufferInUse ? &resCopyDataBuffer : &resDataBuffer;

    if (m_secureDecoder)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->SetBitstreamBuffer(m_picMhwParams.IndObjBaseAddrParams));
    }

    m_picMhwParams.Vp9PicState->pVp9PicParams           = pVp9PicParams;
    m_picMhwParams.Vp9PicState->ppVp9RefList            = &(pVp9RefList[0]);
    m_picMhwParams.Vp9PicState->PrevFrameParams.value   = PrevFrameParams.value;
    m_picMhwParams.Vp9PicState->dwPrevFrmWidth          = dwPrevFrmWidth;
    m_picMhwParams.Vp9PicState->dwPrevFrmHeight         = dwPrevFrmHeight;

    PrevFrameParams.fields.KeyFrame     = !pVp9PicParams->PicFlags.fields.frame_type;
    PrevFrameParams.fields.IntraOnly    = pVp9PicParams->PicFlags.fields.intra_only;
    PrevFrameParams.fields.Display      = pVp9PicParams->PicFlags.fields.show_frame;
    dwPrevFrmWidth                      = pVp9PicParams->FrameWidthMinus1 + 1;
    dwPrevFrmHeight                     = pVp9PicParams->FrameHeightMinus1 + 1;    

    m_picMhwParams.Vp9SegmentState->Mode                = m_mode;
    m_picMhwParams.Vp9SegmentState->pVp9SegmentParams   = pVp9SegmentParams;    

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
    if (pVp9PicParams->PicFlags.fields.frame_type == CODEC_VP9_INTER_FRAME &&
        !pVp9PicParams->PicFlags.fields.intra_only)
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

    if (m_cencDecoder)
    {
        uint8_t   frameIdx, sliceBatchBufferIdx;

        // pass the correct 2nd level batch buffer index set in CencDecode()
        frameIdx = pVp9PicParams->CurrPic.FrameIdx;
        sliceBatchBufferIdx = pVp9RefList[frameIdx]->ucCencBufIdx[0];

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_cencDecoder->SetBatchBufferForDecode(m_debugInterface, sliceBatchBufferIdx, cmdBuffer));
    }
    else 
    {
        for (uint8_t i = 0; i < CODEC_VP9_MAX_SEGMENTS; i++)
        {
            // Error handling for illegal programming on segmentation fields @ KEY/INTRA_ONLY frames
            PCODEC_VP9_SEG_PARAMS vp9SegData = &(m_picMhwParams.Vp9SegmentState->pVp9SegmentParams->SegData[i]);
            if (vp9SegData->SegmentFlags.fields.SegmentReferenceEnabled &&
                (!pVp9PicParams->PicFlags.fields.frame_type || pVp9PicParams->PicFlags.fields.intra_only))
            {
                vp9SegData->SegmentFlags.fields.SegmentReference = CODECHAL_DECODE_VP9_INTRA_FRAME;
            }

			m_picMhwParams.Vp9SegmentState->ucCurrentSegmentId = i;
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpVp9SegmentStateCmd(
                cmdBuffer, 
                nullptr, 
				m_picMhwParams.Vp9SegmentState));

            if (pVp9PicParams->PicFlags.fields.segmentation_enabled == 0)
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

    if (bResetSegIdBuffer)
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
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->UpdateVP9ProbBufferWithHuc(bFullProbBufferUpdate, this, cmdBuffe));
        }
    }
    else
    {
        if (bFullProbBufferUpdate)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(ProbBufFullUpdatewithDrv());
        }
        else
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(ProbBufferPartialUpdatewithDrv());
        }
    }

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &resVp9SegmentIdBuffer,
            CodechalDbgAttr::attrSegId,
            "SegId_beforeHCP" ,
            (dwAllocatedWidthInSB * dwAllocatedHeightInSB * CODECHAL_CACHELINE_SIZE)));
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &(resVp9ProbBuffer[ucFrameCtxIdx]),
            CodechalDbgAttr::attrCoefProb,
            "PakHwCoeffProbs_beforeHCP",
            CODEC_VP9_PROB_MAX_NUM_ELEM));
    )

    return eStatus;
}



MOS_STATUS CodechalDecodeVp9 :: DecodeStateLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;    

    CODECHAL_DECODE_FUNCTION_ENTER;

    //HCP Decode Phase State Machine
    DetermineDecodePhase();

    if (m_secureDecoder)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->Execute(this));
    }

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(
        m_osInterface, 
        &cmdBuffer,
        0));       

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

    if (m_cencDecoder == nullptr)
    {
        MHW_VDBOX_HCP_BSD_PARAMS bsdParams;
        MOS_ZeroMemory(&bsdParams, sizeof(bsdParams));
        bsdParams.dwBsdDataLength = 
            pVp9PicParams->BSBytesInBuffer - pVp9PicParams->UncompressedHeaderLengthInBytes;
        bsdParams.dwBsdDataStartOffset = pVp9PicParams->UncompressedHeaderLengthInBytes;

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
    syncParams.presSyncResource         = &sDestSurface.OsResource;
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
        decodeStatusReport.m_currDecodedPic     = pVp9PicParams->CurrPic;
        decodeStatusReport.m_currDeblockedPic   = pVp9PicParams->CurrPic;
        decodeStatusReport.m_codecStatus        = CODECHAL_STATUS_UNAVAILABLE;
        decodeStatusReport.m_numMbsAffected     = usFrameWidthAlignedMinBlk * usFrameHeightAlignedMinBlk;
        decodeStatusReport.m_currDecodedPicRes  = pVp9RefList[pVp9PicParams->CurrPic.FrameIdx]->resRefPic;

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

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddWatchdogTimerStopCmd(
        &cmdBuffer));

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
   
        m_mmc->UpdateUserFeatureKey(&sDestSurface);
    )

    bool syncCompleteFrame = bCopyDataBufferInUse;
    
    if (syncCompleteFrame)
    {
        //Sync up complete frame
        MOS_SYNC_PARAMS copyDataSyncParams = g_cInitSyncParams;
        copyDataSyncParams.GpuContext = m_videoContextForWa;
        copyDataSyncParams.presSyncResource = &resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &copyDataSyncParams));

        copyDataSyncParams = g_cInitSyncParams;
        copyDataSyncParams.GpuContext = m_videoContext;
        copyDataSyncParams.presSyncResource = &resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &copyDataSyncParams));
    }

    uint32_t renderingFlags = m_videoContextUsesNullHw;

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
        &resVp9SegmentIdBuffer,
        CodechalDbgAttr::attrSegId,
        "SegId",
        (dwAllocatedWidthInSB * dwAllocatedHeightInSB * CODECHAL_CACHELINE_SIZE)));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
        &(resVp9ProbBuffer[ucFrameCtxIdx]),
        CodechalDbgAttr::attrCoefProb,
        "PakHwCoeffProbs",
        CODEC_VP9_PROB_MAX_NUM_ELEM));
    )

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
    PCODECHAL_SETTINGS          settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(settings);

    CODECHAL_DECODE_CHK_STATUS_RETURN(InitMmcState());

    m_width                      = settings->dwWidth;
    m_height                     = settings->dwHeight;
    if (settings->ucLumaChromaDepth & CODECHAL_LUMA_CHROMA_DEPTH_8_BITS)
        ucVP9DepthIndicator = 0;
    if (settings->ucLumaChromaDepth & CODECHAL_LUMA_CHROMA_DEPTH_10_BITS)
        ucVP9DepthIndicator = 1;
    if (settings->ucLumaChromaDepth & CODECHAL_LUMA_CHROMA_DEPTH_12_BITS)
        ucVP9DepthIndicator = 2;
    ucChromaFormatinProfile      = settings->ucChromaFormat;

    MHW_VDBOX_STATE_CMDSIZE_PARAMS      stateCmdSizeParams;
    MOS_ZeroMemory(&stateCmdSizeParams, sizeof(stateCmdSizeParams));
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

    MOS_ZeroMemory(m_picMhwParams.PipeModeSelectParams, sizeof(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS));
    MOS_ZeroMemory(m_picMhwParams.PipeBufAddrParams, sizeof(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS));
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
