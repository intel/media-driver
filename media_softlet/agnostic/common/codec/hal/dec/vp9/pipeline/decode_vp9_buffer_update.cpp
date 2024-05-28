/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     decode_vp9_buffer_update.cpp
//! \brief    Defines the interface for vp9 buffer update
//! \details  Defines the interface to handle the vp9 buffer update for
//!           segment id buffer and probability buffer.
//!
#include "decode_vp9_buffer_update.h"
#include "decode_basic_feature.h"
#include "decode_vp9_pipeline.h"
#include "decode_resource_auto_lock.h"
#include "decode_huc_packet_creator_base.h"
#include "mos_os_cp_interface_specific.h"

namespace decode
{
DecodeVp9BufferUpdate::DecodeVp9BufferUpdate(Vp9Pipeline *pipeline, MediaTask *task, uint8_t numVdbox)
    : DecodeSubPipeline(pipeline, task, numVdbox)
{
}

DecodeVp9BufferUpdate::~DecodeVp9BufferUpdate()
{
    if (m_allocator != nullptr)
    {
        m_allocator->Destroy(m_segmentInitBuffer);
    }
}

MOS_STATUS DecodeVp9BufferUpdate::Init(CodechalSetting &settings)
{
    DECODE_CHK_NULL(m_pipeline);

    CodechalHwInterfaceNext *hwInterface = m_pipeline->GetHwInterface();
    DECODE_CHK_NULL(hwInterface);
    PMOS_INTERFACE osInterface = hwInterface->GetOsInterface();
    DECODE_CHK_NULL(osInterface);
    InitScalabilityPars(osInterface);

    m_allocator     = m_pipeline->GetDecodeAllocator();
    DECODE_CHK_NULL(m_allocator);

    MediaFeatureManager *featureManager = m_pipeline->GetFeatureManager();
    DECODE_CHK_NULL(featureManager);
    m_basicFeature = dynamic_cast<Vp9BasicFeature *>(featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_basicFeature);

    HucPacketCreatorBase *hucPktCreator = dynamic_cast<HucPacketCreatorBase *>(m_pipeline);
    DECODE_CHK_NULL(hucPktCreator);
    m_sgementbufferResetPkt = hucPktCreator->CreateHucCopyPkt(m_pipeline, m_task, hwInterface);
    DECODE_CHK_NULL(m_sgementbufferResetPkt);
    MediaPacket *packet     = dynamic_cast<MediaPacket *>(m_sgementbufferResetPkt);
    DECODE_CHK_NULL(packet);
    DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(m_pipeline, hucCopyPacketId), *packet));
    DECODE_CHK_STATUS(packet->Init());

    HucPacketCreatorBase *probUpdateCreator = dynamic_cast<HucPacketCreatorBase *>(m_pipeline);
    DECODE_CHK_NULL(probUpdateCreator);
    auto probUpdatePkt = probUpdateCreator->CreateProbUpdatePkt(m_pipeline, m_task, hwInterface);
    DECODE_CHK_NULL(probUpdatePkt);
    DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(this, HucVp9ProbUpdatePktId), *probUpdatePkt));
    DECODE_CHK_STATUS(probUpdatePkt->Init());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeVp9BufferUpdate::Prepare(DecodePipelineParams &params)
{
    if (m_pipeline->IsFirstProcessPipe(params))
    {
        DECODE_CHK_STATUS(Begin());

        if (m_basicFeature->m_resetSegIdBuffer)
        {
            DECODE_CHK_NULL(m_basicFeature->m_resVp9SegmentIdBuffer);
            uint32_t allocSize = m_basicFeature->m_resVp9SegmentIdBuffer->size;
            DECODE_CHK_STATUS(AllocateSegmentInitBuffer(allocSize));

            HucCopyPktItf::HucCopyParams copyParams;
            copyParams.srcBuffer  = &m_segmentInitBuffer->OsResource;
            copyParams.srcOffset  = 0;
            copyParams.destBuffer = &(m_basicFeature->m_resVp9SegmentIdBuffer->OsResource);
            copyParams.destOffset = 0;
            copyParams.copyLength = allocSize;
            m_sgementbufferResetPkt->PushCopyParams(copyParams);

            DECODE_CHK_STATUS(ActivatePacket(DecodePacketId(m_pipeline, hucCopyPacketId), true, 0, 0));
        }

        if (m_basicFeature->m_osInterface->osCpInterface->IsHMEnabled())
        {
            DECODE_CHK_STATUS(ActivatePacket(DecodePacketId(this, HucVp9ProbUpdatePktId), true, 0, 0));
        }
        else
        {
            if (m_basicFeature->m_fullProbBufferUpdate)
            {
                DECODE_CHK_STATUS(ProbBufFullUpdatewithDrv());
            }
            else
            {
                DECODE_CHK_STATUS(ProbBufferPartialUpdatewithDrv());
            }
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeVp9BufferUpdate ::ProbBufFullUpdatewithDrv()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    DECODE_FUNC_CALL();

    ResourceAutoLock resLock(m_allocator, &(m_basicFeature->m_resVp9ProbBuffer[m_basicFeature->m_frameCtxIdx]->OsResource));
    auto             data = (uint8_t *)resLock.LockResourceForWrite();
    DECODE_CHK_NULL(data);

    DECODE_CHK_STATUS(ContextBufferInit(
        data, (m_basicFeature->m_probUpdateFlags.bResetKeyDefault ? true : false)));
    DECODE_CHK_STATUS(MOS_SecureMemcpy(
        (data + CODEC_VP9_SEG_PROB_OFFSET),
        7,
        m_basicFeature->m_probUpdateFlags.SegTreeProbs,
        7));
    DECODE_CHK_STATUS(MOS_SecureMemcpy(
        (data + CODEC_VP9_SEG_PROB_OFFSET + 7),
        3,
        m_basicFeature->m_probUpdateFlags.SegPredProbs,
        3));

    return eStatus;
}

MOS_STATUS DecodeVp9BufferUpdate ::ProbBufferPartialUpdatewithDrv()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    DECODE_FUNC_CALL();

    if (m_basicFeature->m_probUpdateFlags.bSegProbCopy ||
        m_basicFeature->m_probUpdateFlags.bProbSave ||
        m_basicFeature->m_probUpdateFlags.bProbReset ||
        m_basicFeature->m_probUpdateFlags.bProbRestore)
    {
        ResourceAutoLock resLock(m_allocator, &(m_basicFeature->m_resVp9ProbBuffer[m_basicFeature->m_frameCtxIdx]->OsResource));
        auto             data = (uint8_t *)resLock.LockResourceForWrite();

        DECODE_CHK_NULL(data);

        if (m_basicFeature->m_probUpdateFlags.bSegProbCopy)
        {
            DECODE_CHK_STATUS(MOS_SecureMemcpy(
                (data + CODEC_VP9_SEG_PROB_OFFSET),
                7,
                m_basicFeature->m_probUpdateFlags.SegTreeProbs,
                7));
            DECODE_CHK_STATUS(MOS_SecureMemcpy(
                (data + CODEC_VP9_SEG_PROB_OFFSET + 7),
                3,
                m_basicFeature->m_probUpdateFlags.SegPredProbs,
                3));
        }

        if (m_basicFeature->m_probUpdateFlags.bProbSave)
        {
            DECODE_CHK_STATUS(MOS_SecureMemcpy(
                m_basicFeature->m_interProbSaved,
                CODECHAL_VP9_INTER_PROB_SIZE,
                data + CODEC_VP9_INTER_PROB_OFFSET,
                CODECHAL_VP9_INTER_PROB_SIZE));
        }

        if (m_basicFeature->m_probUpdateFlags.bProbReset)
        {
            if (m_basicFeature->m_probUpdateFlags.bResetFull)
            {
                DECODE_CHK_STATUS(ContextBufferInit(
                    data, (m_basicFeature->m_probUpdateFlags.bResetKeyDefault ? true : false)));
            }
            else
            {
                DECODE_CHK_STATUS(CtxBufDiffInit(
                    data, (m_basicFeature->m_probUpdateFlags.bResetKeyDefault ? true : false)));
            }
        }

        if (m_basicFeature->m_probUpdateFlags.bProbRestore)
        {
            DECODE_CHK_STATUS(MOS_SecureMemcpy(
                data + CODEC_VP9_INTER_PROB_OFFSET,
                CODECHAL_VP9_INTER_PROB_SIZE,
                m_basicFeature->m_interProbSaved,
                CODECHAL_VP9_INTER_PROB_SIZE));
        }
    }

    return eStatus;
}

MOS_STATUS DecodeVp9BufferUpdate::ContextBufferInit(
    uint8_t *ctxBuffer,
    bool     setToKey)
{
    MOS_ZeroMemory(ctxBuffer, CODEC_VP9_SEG_PROB_OFFSET);

    int32_t  i, j;
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

    uint8_t blocktype          = 0;
    uint8_t reftype            = 0;
    uint8_t coeffbands         = 0;
    uint8_t unConstrainedNodes = 0;
    uint8_t prevCoefCtx        = 0;
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

MOS_STATUS DecodeVp9BufferUpdate::CtxBufDiffInit(
    uint8_t *ctxBuffer,
    bool     setToKey)
{
    int32_t  i, j;
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

MOS_STATUS DecodeVp9BufferUpdate::Begin()
{
    DECODE_CHK_STATUS(DecodeSubPipeline::Reset());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeVp9BufferUpdate::AllocateSegmentInitBuffer(uint32_t allocSize)
{
    DECODE_CHK_NULL(m_allocator);

    if (m_segmentInitBuffer == nullptr)
    {
        m_segmentInitBuffer = m_allocator->AllocateBuffer(
            allocSize, "Vp9SegmentIdInitBuffer", resourceInternalRead, lockableVideoMem, true, 0);
        DECODE_CHK_NULL(m_segmentInitBuffer);
        return MOS_STATUS_SUCCESS;
    }

    if (allocSize > m_segmentInitBuffer->size)
    {
        DECODE_CHK_STATUS(m_allocator->Resize(m_segmentInitBuffer, allocSize, lockableVideoMem, true, 0));
    }

    return MOS_STATUS_SUCCESS;
}

MediaFunction DecodeVp9BufferUpdate::GetMediaFunction()
{
    return VdboxDecodeWaFunc;
}

void DecodeVp9BufferUpdate::InitScalabilityPars(PMOS_INTERFACE osInterface)
{
    m_decodeScalabilityPars.disableScalability = true;
    m_decodeScalabilityPars.disableRealTile    = true;
    m_decodeScalabilityPars.enableVE           = MOS_VE_SUPPORTED(osInterface);
    m_decodeScalabilityPars.numVdbox           = m_numVdbox;
}

}  // namespace decode
