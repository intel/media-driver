/*
* Copyright (c) 2026, Intel Corporation
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
//! \file     decode_vp9_buffer_update_post.cpp
//! \brief    Implements the post-processing buffer update sub pipeline for VP9 decode
//!

#include "decode_vp9_buffer_update_post.h"
#include "decode_basic_feature.h"
#include "decode_vp9_pipeline.h"
#include "decode_resource_auto_lock.h"
#include "decode_huc_packet_creator_base.h"
#include "mos_os_cp_interface_specific.h"

namespace decode
{

DecodeVp9BufferUpdatePost::DecodeVp9BufferUpdatePost(Vp9Pipeline *pipeline, MediaTask *task, uint8_t numVdbox)
    : DecodeSubPipeline(pipeline, task, numVdbox)
{
}

DecodeVp9BufferUpdatePost::~DecodeVp9BufferUpdatePost()
{
    if (m_allocator != nullptr)
    {
        m_allocator->Destroy(m_tempDefaultProbBuffer);
        m_allocator->Destroy(m_tempFrameTypeKeyBuffer);
        m_allocator->Destroy(m_tempFrameTypeNonKeyBuffer);
    }
}

MOS_STATUS DecodeVp9BufferUpdatePost::Init(CodechalSetting &settings)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(m_pipeline);

    CodechalHwInterfaceNext *hwInterface = m_pipeline->GetHwInterface();
    DECODE_CHK_NULL(hwInterface);
    PMOS_INTERFACE osInterface = hwInterface->GetOsInterface();
    DECODE_CHK_NULL(osInterface);
    InitScalabilityPars(osInterface);

    m_allocator = m_pipeline->GetDecodeAllocator();
    DECODE_CHK_NULL(m_allocator);

    MediaFeatureManager *featureManager = m_pipeline->GetFeatureManager();
    DECODE_CHK_NULL(featureManager);
    m_basicFeature = dynamic_cast<Vp9BasicFeature *>(featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_basicFeature);

    HucPacketCreatorBase *hucPostPktCreator = dynamic_cast<HucPacketCreatorBase *>(m_pipeline);
    DECODE_CHK_NULL(hucPostPktCreator);
    m_probbufferResetPostPkt = hucPostPktCreator->CreateHucCopyPkt(m_pipeline, m_task, hwInterface);
    DECODE_CHK_NULL(m_probbufferResetPostPkt);
    MediaPacket *postpacket = dynamic_cast<MediaPacket *>(m_probbufferResetPostPkt);
    DECODE_CHK_NULL(postpacket);
    DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(m_pipeline, hucCopyPacketId), *postpacket));
    DECODE_CHK_STATUS(postpacket->Init());

    AllocateTempBuffer();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeVp9BufferUpdatePost::Prepare(DecodePipelineParams &params)
{
    DECODE_FUNC_CALL();

    if (m_pipeline->IsFirstProcessPipe(params))
    {
        DECODE_CHK_STATUS(Begin());

        if (!m_basicFeature->m_vp9PicParams->PicFlags.fields.frame_type || m_basicFeature->m_vp9PicParams->PicFlags.fields.intra_only)
        {
            HucCopyPktItf::HucCopyParams copyParams;
            copyParams.srcBuffer  = &m_tempDefaultProbBuffer->OsResource;
            copyParams.srcOffset  = CODEC_VP9_INTER_PROB_OFFSET;
            copyParams.destBuffer = &(m_basicFeature->m_resVp9ProbBuffer[m_basicFeature->m_frameCtxIdx]->OsResource);
            copyParams.destOffset = CODEC_VP9_INTER_PROB_OFFSET;
            copyParams.copyLength = CODECHAL_VP9_INTER_PROB_SIZE;
            m_probbufferResetPostPkt->PushCopyParams(copyParams);
            DECODE_CHK_STATUS(ActivatePacket(DecodePacketId(m_pipeline, hucCopyPacketId), true, 0, 0));
        }

        HucCopyPktItf::HucCopyParams copyParams;
        if (!m_basicFeature->m_vp9PicParams->PicFlags.fields.frame_type)
        {
            copyParams.srcBuffer  = &m_tempFrameTypeKeyBuffer->OsResource;
            copyParams.srcOffset  = 0;
            copyParams.destBuffer = &(m_basicFeature->m_resVp9FrameStatusBuffer->OsResource);
            copyParams.destOffset = 0;
            copyParams.copyLength = 1;
        }
        else
        {
            copyParams.srcBuffer  = &m_tempFrameTypeNonKeyBuffer->OsResource;
            copyParams.srcOffset  = 0;
            copyParams.destBuffer = &(m_basicFeature->m_resVp9FrameStatusBuffer->OsResource);
            copyParams.destOffset = 0;
            copyParams.copyLength = 1;
        }
        m_probbufferResetPostPkt->PushCopyParams(copyParams);
        DECODE_CHK_STATUS(ActivatePacket(DecodePacketId(m_pipeline, hucCopyPacketId), true, 0, 0));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeVp9BufferUpdatePost::CtxBufDiffInit(
    uint8_t *ctxBuffer,
    bool     setToKey)
{
    DECODE_FUNC_CALL();

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

MOS_STATUS DecodeVp9BufferUpdatePost::Begin()
{
    DECODE_CHK_STATUS(DecodeSubPipeline::Reset());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeVp9BufferUpdatePost::AllocateTempBuffer()
{
    DECODE_CHK_NULL(m_allocator);

    m_tempDefaultProbBuffer = m_allocator->AllocateBuffer(
        MOS_ALIGN_CEIL(CODEC_VP9_PROB_MAX_NUM_ELEM, CODECHAL_PAGE_SIZE), "tempVp9ResetFullKeyDefaultProbBuffer",
        resourceInternalRead, lockableVideoMem);
    DECODE_CHK_NULL(m_tempDefaultProbBuffer);
    {
        ResourceAutoLock resLock(m_allocator, &(m_tempDefaultProbBuffer->OsResource));
        auto             data = (uint8_t *)resLock.LockResourceForWrite();
        DECODE_CHK_NULL(data);

        MOS_ZeroMemory(data, CODEC_VP9_PROB_MAX_NUM_ELEM);
        DECODE_CHK_STATUS(CtxBufDiffInit(data, false));
    }

    m_tempFrameTypeKeyBuffer = m_allocator->AllocateBuffer(
        1, "tempFrameTypeBuffer", resourceInternalRead, lockableVideoMem);
    DECODE_CHK_NULL(m_tempFrameTypeKeyBuffer);
    {
        ResourceAutoLock resLock(m_allocator, &(m_tempFrameTypeKeyBuffer->OsResource));
        auto             data = (uint8_t *)resLock.LockResourceForWrite();
        DECODE_CHK_NULL(data);
        MOS_ZeroMemory(data, 1);
    }

    m_tempFrameTypeNonKeyBuffer = m_allocator->AllocateBuffer(
        1, "tempFrameTypeBuffer", resourceInternalRead, lockableVideoMem);
    DECODE_CHK_NULL(m_tempFrameTypeNonKeyBuffer);
    {
        ResourceAutoLock resLock(m_allocator, &(m_tempFrameTypeNonKeyBuffer->OsResource));
        auto             data = (uint8_t *)resLock.LockResourceForWrite();
        DECODE_CHK_NULL(data);
        MOS_FillMemory(data, 1, 1);
    }

    return MOS_STATUS_SUCCESS;
}

MediaFunction DecodeVp9BufferUpdatePost::GetMediaFunction()
{
    // WA for Vulkan, currently only support one MOS_GPU_CONTEXT_VIDEO.
    if (m_basicFeature->m_osInterface->pfnIsMismatchOrderProgrammingSupported())
    {
        return VdboxDecodeFunc;
    }

    return VdboxDecodeWaFunc;
}

void DecodeVp9BufferUpdatePost::InitScalabilityPars(PMOS_INTERFACE osInterface)
{
    m_decodeScalabilityPars.disableScalability = true;
    m_decodeScalabilityPars.disableRealTile    = true;
    m_decodeScalabilityPars.enableVE           = MOS_VE_SUPPORTED(osInterface);
    m_decodeScalabilityPars.numVdbox           = m_numVdbox;
}

}  // namespace decode