/*
* Copyright (c) 2020-2023, Intel Corporation
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
//! \file     encode_vp9_hpu.cpp
//! \brief    Defines the common interface for vp9 encode hpu (header probability update) features
//!

#include "encode_vp9_hpu.h"
#include "encode_vp9_pak.h"
#include "encode_vp9_vdenc_feature_manager.h"
#include "encode_vp9_vdenc_const_settings.h"
#include "media_vp9_packet_defs.h"

namespace encode
{
const uint32_t Vp9EncodeHpu::m_probDmem[320] = {
    0x00000000, 0x00000000, 0x00000000, 0x00000003, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000100, 0x00000000, 0x00000000,
    0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000001,
    0x00000004, 0x00000004, 0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x01000000, 0x0000FF00, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000001, 0x00000000, 0x00000000, 0x00000001,
    0x00540049, 0x00000060, 0x00000072, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x02B80078, 0x00000001, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
};

Vp9EncodeHpu::Vp9EncodeHpu(
    MediaFeatureManager *featureManager,
    EncodeAllocator *    allocator,
    CodechalHwInterfaceNext *hwInterface,
    void *               constSettings)
    : MediaFeature(constSettings),
      m_allocator(allocator)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_NO_STATUS_RETURN(featureManager);

    m_featureManager = featureManager;
    auto encFeatureManager = dynamic_cast<EncodeVp9VdencFeatureManager *>(featureManager);
    ENCODE_CHK_NULL_NO_STATUS_RETURN(encFeatureManager);

    m_basicFeature = dynamic_cast<Vp9BasicFeature *>(encFeatureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);
}

MOS_STATUS Vp9EncodeHpu::Init(void *settings)
{
    ENCODE_FUNC_CALL();

    m_enabled = true;

    ENCODE_CHK_STATUS_RETURN(AllocateResources());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeHpu::Update(void *params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);

    EncoderParams *encodeParams = (EncoderParams *)params;

    auto vp9SeqParams = static_cast<PCODEC_VP9_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
    ENCODE_CHK_NULL_RETURN(vp9SeqParams);
    auto vp9PicParams = static_cast<PCODEC_VP9_ENCODE_PIC_PARAMS>(encodeParams->pPicParams);
    ENCODE_CHK_NULL_RETURN(vp9PicParams);

    if (m_basicFeature->m_newSeq)
    {
        ENCODE_CHK_STATUS_RETURN(SetConstSettings());
    }

    // Initialize huc prob dmem buffers in the first pass.
    for (auto i = 0; i < 3; ++i)
    {
        auto dmem = (HucProbDmem *)m_allocator->LockResourceForWrite(
            &m_resHucProbDmemBuffer[m_basicFeature->m_currRecycledBufIdx][i]);
        ENCODE_CHK_NULL_RETURN(dmem);

        MOS_SecureMemcpy(dmem, sizeof(HucProbDmem), m_probDmem, sizeof(HucProbDmem));

        ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(
            &m_resHucProbDmemBuffer[m_basicFeature->m_currRecycledBufIdx][i]));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeHpu::SetRegions(mhw::vdbox::huc::HUC_VIRTUAL_ADDR_STATE_PAR &params) const
{
    ENCODE_FUNC_CALL();

    auto frameCtxtIdx = m_basicFeature->m_vp9PicParams->PicFlags.fields.frame_context_idx;

    // Input regions: 0,1

    params.regionParams[0].presRegion = const_cast<MOS_RESOURCE *>(&m_resProbBuffer[frameCtxtIdx]);
    params.regionParams[0].isWritable = true;  // Region 0 is both read and write for HuC.
                                               // Has input probabilities before running HuC and updated
                                               // probabilities after running HuC, which will then be input to next pass
    params.regionParams[1].presRegion = const_cast<MOS_RESOURCE *>(&m_resProbabilityCounterBuffer);
    params.regionParams[1].dwOffset   = 0;

    // Output regions: 2,3
    // Final probability output from HuC after each pass
    params.regionParams[2].presRegion  = const_cast<MOS_RESOURCE *>(&m_resHucProbOutputBuffer);
    params.regionParams[2].isWritable  = true;
    params.regionParams[3].presRegion  = const_cast<MOS_RESOURCE *>(&m_resProbabilityDeltaBuffer);
    params.regionParams[3].isWritable  = true;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeHpu::GetProbabilityBuffer(uint32_t idx, PMOS_RESOURCE &buffer)
{
    ENCODE_FUNC_CALL();

    if (idx >= CODEC_VP9_NUM_CONTEXTS)
    {
        ENCODE_ASSERTMESSAGE("Index exceeds the max number, when try to get resProbBuffer");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    buffer = &m_resProbBuffer[idx];

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeHpu::GetHucProbDmemBuffer(uint32_t idx, PMOS_RESOURCE &buffer)
{
    ENCODE_FUNC_CALL();

    if (idx >= 3)
    {
        ENCODE_ASSERTMESSAGE("Index exceeds the max number, when try to get resHucProbDmemBuffer");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    buffer = &m_resHucProbDmemBuffer[m_basicFeature->m_currRecycledBufIdx][idx];

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeHpu::AllocateResources()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_basicFeature);

    MOS_RESOURCE *allocatedBuffer = nullptr;

    // Initiate allocation parameters and lock flags
    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format   = Format_Buffer;

    // Probability delta buffer
    allocParamsForBufferLinear.dwBytes  = 29 * CODECHAL_CACHELINE_SIZE;
    allocParamsForBufferLinear.pBufName = "ProbabilityDeltaBuffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ;
    allocatedBuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
    ENCODE_CHK_NULL_RETURN(allocatedBuffer);
    m_resProbabilityDeltaBuffer = *allocatedBuffer;

    // Probability counter buffer
    allocParamsForBufferLinear.dwBytes  = m_probabilityCounterBufferSize * m_basicFeature->m_maxTileNumber;
    allocParamsForBufferLinear.pBufName = "ProbabilityCounterBuffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    allocatedBuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
    ENCODE_CHK_NULL_RETURN(allocatedBuffer);
    m_resProbabilityCounterBuffer = *allocatedBuffer;

    // Huc Prob DMEM buffer
    allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(MOS_MAX(sizeof(HucProbDmem), sizeof(HucProbDmem)), CODECHAL_CACHELINE_SIZE);
    allocParamsForBufferLinear.pBufName = "HucProbDmemBuffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
    for (auto i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; ++i)
    {
        for (auto j = 0; j < 3; ++j)
        {
            allocatedBuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
            ENCODE_CHK_NULL_RETURN(allocatedBuffer);
            m_resHucProbDmemBuffer[i][j] = *allocatedBuffer;
        }
    }

    // Huc probability output buffer
    allocParamsForBufferLinear.dwBytes  = 32 * CODECHAL_CACHELINE_SIZE;
    allocParamsForBufferLinear.pBufName = "HucProbabilityOutputBuffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ;
    allocatedBuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
    ENCODE_CHK_NULL_RETURN(allocatedBuffer);
    m_resHucProbOutputBuffer = *allocatedBuffer;

    // Probability buffers
    allocParamsForBufferLinear.dwBytes  = 32 * CODECHAL_CACHELINE_SIZE;
    allocParamsForBufferLinear.pBufName = "ProbabilityBuffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    for (auto i = 0; i < CODEC_VP9_NUM_CONTEXTS; ++i)
    {
        allocatedBuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
        ENCODE_CHK_NULL_RETURN(allocatedBuffer);
        m_resProbBuffer[i] = *allocatedBuffer;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeHpu::SetIsLastPass(bool isLastPass)
{
    m_isLastPass = isLastPass;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_IND_OBJ_BASE_ADDR_STATE, Vp9EncodeHpu)
{
    ENCODE_FUNC_CALL();

    auto basicFeature = dynamic_cast<Vp9BasicFeature *>(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(basicFeature);

    params.presProbabilityDeltaBuffer = const_cast<PMOS_RESOURCE>(&m_resProbabilityDeltaBuffer);
    params.dwProbabilityDeltaSize     = 29 * CODECHAL_CACHELINE_SIZE;

    if (!basicFeature->m_scalableMode)
    {
        params.presProbabilityCounterBuffer = const_cast<PMOS_RESOURCE>(&m_resProbabilityCounterBuffer);
        params.dwProbabilityCounterOffset   = 0;
        params.dwProbabilityCounterSize     = 193 * CODECHAL_CACHELINE_SIZE;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_PIPE_BUF_ADDR_STATE, Vp9EncodeHpu)
{
    ENCODE_FUNC_CALL();

    // Huc first pass doesn't write probabilities to output prob region but only updates to the input region.
    // HuC run before repak writes to the ouput region.
    if (m_basicFeature->m_hucEnabled && m_isLastPass)
    {
        params.presVp9ProbBuffer = const_cast<PMOS_RESOURCE>(&m_resHucProbOutputBuffer);
    }
    else
    {
        auto frameCtxIdx = m_basicFeature->m_vp9PicParams->PicFlags.fields.frame_context_idx;
        ENCODE_ASSERT(frameCtxIdx < CODEC_VP9_NUM_CONTEXTS);
        params.presVp9ProbBuffer = const_cast<PMOS_RESOURCE>(&m_resProbBuffer[frameCtxIdx]);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeHpu::SetDefaultTxProbs(uint8_t *ctxBuffer, uint32_t &byteCnt) const
{
    ENCODE_FUNC_CALL();

    int32_t i, j;
    // TX probs
    for (i = 0; i < CODEC_VP9_TX_SIZE_CONTEXTS; ++i)
    {
        for (j = 0; j < CODEC_VP9_TX_SIZES - 3; ++j)
        {
            ctxBuffer[byteCnt++] = DefaultTxProbs.p8x8[i][j];
        }
    }
    for (i = 0; i < CODEC_VP9_TX_SIZE_CONTEXTS; ++i)
    {
        for (j = 0; j < CODEC_VP9_TX_SIZES - 2; ++j)
        {
            ctxBuffer[byteCnt++] = DefaultTxProbs.p16x16[i][j];
        }
    }
    for (i = 0; i < CODEC_VP9_TX_SIZE_CONTEXTS; ++i)
    {
        for (j = 0; j < CODEC_VP9_TX_SIZES - 1; ++j)
        {
            ctxBuffer[byteCnt++] = DefaultTxProbs.p32x32[i][j];
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeHpu::SetDefaultCoeffProbs(uint8_t *ctxBuffer, uint32_t &byteCnt) const
{
    ENCODE_FUNC_CALL();

    uint8_t blocktype          = 0;
    uint8_t reftype            = 0;
    uint8_t coeffbands         = 0;
    uint8_t unConstrainedNodes = 0;
    uint8_t prevCoefCtx        = 0;
    // Coeff probs
    for (blocktype = 0; blocktype < CODEC_VP9_BLOCK_TYPES; ++blocktype)
    {
        for (reftype = 0; reftype < CODEC_VP9_REF_TYPES; ++reftype)
        {
            for (coeffbands = 0; coeffbands < CODEC_VP9_COEF_BANDS; ++coeffbands)
            {
                uint8_t numPrevCoeffCtxts = (coeffbands == 0) ? 3 : CODEC_VP9_PREV_COEF_CONTEXTS;
                for (prevCoefCtx = 0; prevCoefCtx < numPrevCoeffCtxts; ++prevCoefCtx)
                {
                    for (unConstrainedNodes = 0; unConstrainedNodes < CODEC_VP9_UNCONSTRAINED_NODES; ++unConstrainedNodes)
                    {
                        ctxBuffer[byteCnt++] = DefaultCoefProbs4x4[blocktype][reftype][coeffbands][prevCoefCtx][unConstrainedNodes];
                    }
                }
            }
        }
    }

    for (blocktype = 0; blocktype < CODEC_VP9_BLOCK_TYPES; ++blocktype)
    {
        for (reftype = 0; reftype < CODEC_VP9_REF_TYPES; ++reftype)
        {
            for (coeffbands = 0; coeffbands < CODEC_VP9_COEF_BANDS; ++coeffbands)
            {
                uint8_t numPrevCoeffCtxts = (coeffbands == 0) ? 3 : CODEC_VP9_PREV_COEF_CONTEXTS;
                for (prevCoefCtx = 0; prevCoefCtx < numPrevCoeffCtxts; ++prevCoefCtx)
                {
                    for (unConstrainedNodes = 0; unConstrainedNodes < CODEC_VP9_UNCONSTRAINED_NODES; ++unConstrainedNodes)
                    {
                        ctxBuffer[byteCnt++] = DefaultCoefPprobs8x8[blocktype][reftype][coeffbands][prevCoefCtx][unConstrainedNodes];
                    }
                }
            }
        }
    }

    for (blocktype = 0; blocktype < CODEC_VP9_BLOCK_TYPES; ++blocktype)
    {
        for (reftype = 0; reftype < CODEC_VP9_REF_TYPES; ++reftype)
        {
            for (coeffbands = 0; coeffbands < CODEC_VP9_COEF_BANDS; ++coeffbands)
            {
                uint8_t numPrevCoeffCtxts = (coeffbands == 0) ? 3 : CODEC_VP9_PREV_COEF_CONTEXTS;
                for (prevCoefCtx = 0; prevCoefCtx < numPrevCoeffCtxts; ++prevCoefCtx)
                {
                    for (unConstrainedNodes = 0; unConstrainedNodes < CODEC_VP9_UNCONSTRAINED_NODES; ++unConstrainedNodes)
                    {
                        ctxBuffer[byteCnt++] = DefaultCoefProbs16x16[blocktype][reftype][coeffbands][prevCoefCtx][unConstrainedNodes];
                    }
                }
            }
        }
    }

    for (blocktype = 0; blocktype < CODEC_VP9_BLOCK_TYPES; ++blocktype)
    {
        for (reftype = 0; reftype < CODEC_VP9_REF_TYPES; ++reftype)
        {
            for (coeffbands = 0; coeffbands < CODEC_VP9_COEF_BANDS; ++coeffbands)
            {
                uint8_t numPrevCoeffCtxts = (coeffbands == 0) ? 3 : CODEC_VP9_PREV_COEF_CONTEXTS;
                for (prevCoefCtx = 0; prevCoefCtx < numPrevCoeffCtxts; ++prevCoefCtx)
                {
                    for (unConstrainedNodes = 0; unConstrainedNodes < CODEC_VP9_UNCONSTRAINED_NODES; ++unConstrainedNodes)
                    {
                        ctxBuffer[byteCnt++] = DefaultCoefProbs32x32[blocktype][reftype][coeffbands][prevCoefCtx][unConstrainedNodes];
                    }
                }
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeHpu::SetDefaultMbskipProbs(uint8_t *ctxBuffer, uint32_t &byteCnt) const
{
    ENCODE_FUNC_CALL();

    // mb skip probs
    for (auto i = 0; i < CODEC_VP9_MBSKIP_CONTEXTS; ++i)
    {
        ctxBuffer[byteCnt++] = DefaultMbskipProbs[i];
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeHpu::SetDefaultInterModeProbs(uint8_t *ctxBuffer, uint32_t &byteCnt, bool setToKey) const
{
    ENCODE_FUNC_CALL();

    int32_t i, j;

    // Inter mode probs
    for (i = 0; i < CODEC_VP9_INTER_MODE_CONTEXTS; ++i)
    {
        for (j = 0; j < CODEC_VP9_INTER_MODES - 1; ++j)
        {
            if (!setToKey)
            {
                ctxBuffer[byteCnt++] = DefaultInterModeProbs[i][j];
            }
            else
            {
                // Zeros for key frame
                byteCnt++;
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeHpu::SetDefaultSwitchableInterpProb(uint8_t *ctxBuffer, uint32_t &byteCnt, bool setToKey) const
{
    ENCODE_FUNC_CALL();

    int32_t i, j;

    // Switchable interprediction probs
    for (i = 0; i < CODEC_VP9_SWITCHABLE_FILTERS + 1; ++i)
    {
        for (j = 0; j < CODEC_VP9_SWITCHABLE_FILTERS - 1; ++j)
        {
            if (!setToKey)
            {
                ctxBuffer[byteCnt++] = DefaultSwitchableInterpProb[i][j];
            }
            else
            {
                // Zeros for key frame
                byteCnt++;
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeHpu::SetDefaultIntraInterProb(uint8_t *ctxBuffer, uint32_t &byteCnt, bool setToKey) const
{
    ENCODE_FUNC_CALL();

    // Intra inter probs
    for (auto i = 0; i < CODEC_VP9_INTRA_INTER_CONTEXTS; ++i)
    {
        if (!setToKey)
        {
            ctxBuffer[byteCnt++] = DefaultIntraInterProb[i];
        }
        else
        {
            // Zeros for key frame
            byteCnt++;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeHpu::SetDefaultCompInterProb(uint8_t *ctxBuffer, uint32_t &byteCnt, bool setToKey) const
{
    ENCODE_FUNC_CALL();

    // Comp inter probs
    for (auto i = 0; i < CODEC_VP9_COMP_INTER_CONTEXTS; ++i)
    {
        if (!setToKey)
        {
            ctxBuffer[byteCnt++] = DefaultCompInterProb[i];
        }
        else
        {
            // Zeros for key frame
            byteCnt++;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeHpu::SetDefaultSingleRefProb(uint8_t *ctxBuffer, uint32_t &byteCnt, bool setToKey) const
{
    ENCODE_FUNC_CALL();

    int32_t i, j;

    // Single ref probs
    for (i = 0; i < CODEC_VP9_REF_CONTEXTS; ++i)
    {
        for (j = 0; j < 2; ++j)
        {
            if (!setToKey)
            {
                ctxBuffer[byteCnt++] = DefaultSingleRefProb[i][j];
            }
            else
            {
                // Zeros for key frame
                byteCnt++;
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeHpu::SetDefaultCompRefProb(uint8_t *ctxBuffer, uint32_t &byteCnt, bool setToKey) const
{
    ENCODE_FUNC_CALL();

    // Comp ref probs
    for (auto i = 0; i < CODEC_VP9_REF_CONTEXTS; ++i)
    {
        if (!setToKey)
        {
            ctxBuffer[byteCnt++] = DefaultCompRefProb[i];
        }
        else
        {
            // Zeros for key frame
            byteCnt++;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeHpu::SetDefaultYModeProb(uint8_t *ctxBuffer, uint32_t &byteCnt, bool setToKey) const
{
    ENCODE_FUNC_CALL();

    int32_t i, j;

    // y mode probs
    for (i = 0; i < CODEC_VP9_BLOCK_SIZE_GROUPS; ++i)
    {
        for (j = 0; j < CODEC_VP9_INTRA_MODES - 1; ++j)
        {
            if (!setToKey)
            {
                ctxBuffer[byteCnt++] = DefaultIFYProb[i][j];
            }
            else
            {
                // Zeros for key frame, since HW will not use this buffer, but default right buffer
                byteCnt++;
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeHpu::SetDefaultPartitionProb(uint8_t *ctxBuffer, uint32_t &byteCnt, bool setToKey) const
{
    ENCODE_FUNC_CALL();

    int32_t i, j;

    // Partition probs, key & intra-only frames use key type, other inter frames use inter type
    for (i = 0; i < CODECHAL_VP9_PARTITION_CONTEXTS; ++i)
    {
        for (j = 0; j < CODEC_VP9_PARTITION_TYPES - 1; ++j)
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

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeHpu::SetDefaultNmvContext(uint8_t *ctxBuffer, uint32_t &byteCnt, bool setToKey) const
{
    ENCODE_FUNC_CALL();

    int32_t i, j;

    for (i = 0; i < (CODEC_VP9_MV_JOINTS - 1); ++i)
    {
        if (!setToKey)
        {
            ctxBuffer[byteCnt++] = DefaultNmvContext.joints[i];
        }
        else
        {
            byteCnt++;
        }
    }

    for (i = 0; i < 2; ++i)
    {
        if (!setToKey)
        {
            ctxBuffer[byteCnt++] = DefaultNmvContext.comps[i].sign;
            for (j = 0; j < (CODEC_VP9_MV_CLASSES - 1); ++j)
            {
                ctxBuffer[byteCnt++] = DefaultNmvContext.comps[i].classes[j];
            }
            for (j = 0; j < (CODECHAL_VP9_CLASS0_SIZE - 1); ++j)
            {
                ctxBuffer[byteCnt++] = DefaultNmvContext.comps[i].class0[j];
            }
            for (j = 0; j < CODECHAL_VP9_MV_OFFSET_BITS; ++j)
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
    for (i = 0; i < 2; ++i)
    {
        if (!setToKey)
        {
            for (j = 0; j < CODECHAL_VP9_CLASS0_SIZE; ++j)
            {
                for (int32_t k = 0; k < (CODEC_VP9_MV_FP_SIZE - 1); ++k)
                {
                    ctxBuffer[byteCnt++] = DefaultNmvContext.comps[i].class0_fp[j][k];
                }
            }
            for (j = 0; j < (CODEC_VP9_MV_FP_SIZE - 1); ++j)
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
    for (i = 0; i < 2; ++i)
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

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeHpu::SetDefaultUVModeProbs(uint8_t *ctxBuffer, uint32_t &byteCnt, bool setToKey) const
{
    ENCODE_FUNC_CALL();

    int32_t i, j;

    // uv mode probs
    for (i = 0; i < CODEC_VP9_INTRA_MODES; ++i)
    {
        for (j = 0; j < CODEC_VP9_INTRA_MODES - 1; ++j)
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

MOS_STATUS Vp9EncodeHpu::CtxBufDiffInit(uint8_t *ctxBuffer, bool setToKey) const
{
    ENCODE_FUNC_CALL();

    uint32_t byteCnt = CODEC_VP9_INTER_PROB_OFFSET;

    ENCODE_CHK_STATUS_RETURN(SetDefaultInterModeProbs(ctxBuffer, byteCnt, setToKey));

    ENCODE_CHK_STATUS_RETURN(SetDefaultSwitchableInterpProb(ctxBuffer, byteCnt, setToKey));

    ENCODE_CHK_STATUS_RETURN(SetDefaultIntraInterProb(ctxBuffer, byteCnt, setToKey));

    ENCODE_CHK_STATUS_RETURN(SetDefaultCompInterProb(ctxBuffer, byteCnt, setToKey));

    ENCODE_CHK_STATUS_RETURN(SetDefaultSingleRefProb(ctxBuffer, byteCnt, setToKey));

    ENCODE_CHK_STATUS_RETURN(SetDefaultCompRefProb(ctxBuffer, byteCnt, setToKey));

    ENCODE_CHK_STATUS_RETURN(SetDefaultYModeProb(ctxBuffer, byteCnt, setToKey));

    ENCODE_CHK_STATUS_RETURN(SetDefaultPartitionProb(ctxBuffer, byteCnt, setToKey));

    ENCODE_CHK_STATUS_RETURN(SetDefaultNmvContext(ctxBuffer, byteCnt, setToKey));

    // 47 bytes of zeros
    byteCnt += 47;

    ENCODE_CHK_STATUS_RETURN(SetDefaultUVModeProbs(ctxBuffer, byteCnt, setToKey));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeHpu::ContextBufferInit(uint8_t *ctxBuffer, bool setToKey) const
{
    ENCODE_FUNC_CALL();

    MOS_ZeroMemory(ctxBuffer, CODEC_VP9_SEG_PROB_OFFSET);

    uint32_t byteCnt = 0;

    ENCODE_CHK_STATUS_RETURN(SetDefaultTxProbs(ctxBuffer, byteCnt));

    // 52 bytes of zeros
    byteCnt += 52;

    ENCODE_CHK_STATUS_RETURN(SetDefaultCoeffProbs(ctxBuffer, byteCnt));

    // 16 bytes of zeros
    byteCnt += 16;

    ENCODE_CHK_STATUS_RETURN(SetDefaultMbskipProbs(ctxBuffer, byteCnt));

    // Populate prob values which are different between Key and Non-Key frame
    CtxBufDiffInit(ctxBuffer, setToKey);

    // Skip Seg tree/pred probs, updating not done in this function
    byteCnt = CODEC_VP9_SEG_PROB_OFFSET;
    byteCnt += 7;
    byteCnt += 3;

    // 28 bytes of zeros
    for (auto i = 0; i < 28; i++)
    {
        ctxBuffer[byteCnt++] = 0;
    }

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

void Vp9EncodeHpu::PutDataForCompressedHdr(
    CompressedHeader *compressedHdr,
    uint32_t          bit,
    uint32_t          prob,
    uint32_t          binIdx)
{
    compressedHdr[binIdx].fields.valid        = 1;
    compressedHdr[binIdx].fields.bin_probdiff = 1;
    compressedHdr[binIdx].fields.bin          = bit;
    compressedHdr[binIdx].fields.prob         = (prob == 128) ? 0 : 1;
}

MOS_STATUS Vp9EncodeHpu::RefreshFrameInternalBuffers()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_vp9PicParams);

    auto hpuFeature = dynamic_cast<Vp9EncodeHpu *>(m_featureManager->GetFeature(Vp9FeatureIDs::vp9HpuFeature));
    ENCODE_CHK_NULL_RETURN(hpuFeature);
    auto pakFeature = dynamic_cast<Vp9EncodePak *>(m_featureManager->GetFeature(Vp9FeatureIDs::vp9PakFeature));
    ENCODE_CHK_NULL_RETURN(pakFeature);

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    auto &picFields = m_basicFeature->m_vp9PicParams->PicFlags.fields;
    ENCODE_ASSERT(picFields.refresh_frame_context == 0);

    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;

    bool keyFrame      = !picFields.frame_type;
    bool isScaling     = (m_basicFeature->m_oriFrameWidth == m_basicFeature->m_prevFrameInfo.FrameWidth) &&
                             (m_basicFeature->m_oriFrameHeight == m_basicFeature->m_prevFrameInfo.FrameHeight)
                             ? false
                             : true;
    bool resetSegIdBuf = keyFrame || isScaling ||
                         picFields.error_resilient_mode ||
                         picFields.intra_only;

    if (resetSegIdBuf)
    {
        uint8_t *data = (uint8_t *)m_allocator->LockResourceForWrite(m_basicFeature->m_resSegmentIdBuffer);
        ENCODE_CHK_NULL_RETURN(data);

        MOS_ZeroMemory(data, m_basicFeature->m_picSizeInSb * CODECHAL_CACHELINE_SIZE);

        ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(m_basicFeature->m_resSegmentIdBuffer));
    }

    //refresh inter probs in needed frame context buffers
    bool clearAll = (keyFrame || picFields.error_resilient_mode ||
                     (picFields.reset_frame_context == 3 && picFields.intra_only));

    bool clearSpecified = (picFields.reset_frame_context == 2 &&
                           picFields.intra_only);

    for (auto i = 0; i < CODEC_VP9_NUM_CONTEXTS; i++)
    {
        PMOS_RESOURCE resProbBuffer = nullptr;
        ENCODE_CHK_STATUS_RETURN(hpuFeature->GetProbabilityBuffer(i, resProbBuffer));
        ENCODE_CHK_NULL_RETURN(resProbBuffer);

        if (clearAll || (clearSpecified && i == picFields.frame_context_idx))
        {
            uint8_t *data = (uint8_t *)m_allocator->LockResourceForWrite(resProbBuffer);
            ENCODE_CHK_NULL_RETURN(data);

            eStatus = ContextBufferInit(data, keyFrame || picFields.intra_only);

            ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(resProbBuffer));
            ENCODE_CHK_STATUS_RETURN(eStatus);

            m_clearAllToKey[i] = keyFrame || picFields.intra_only;
            if (i == 0)  //reset this flag when Ctx buffer 0 is cleared.
            {
                m_isPreCtx0InterProbSaved = false;
            }
        }
        else if (m_clearAllToKey[i])  // this buffer is inside inter frame, but its interProb has not been init to default inter type data.
        {
            uint8_t *data = (uint8_t *)m_allocator->LockResourceForWrite(resProbBuffer);
            ENCODE_CHK_NULL_RETURN(data);

            if (picFields.intra_only && i == 0)  // this buffer is used as intra_only context, do not need to set interprob to be inter type.
            {
                eStatus = CtxBufDiffInit(data, true);
            }
            else  // set interprob to be inter type.
            {
                eStatus            = CtxBufDiffInit(data, false);
                m_clearAllToKey[i] = false;
            }

            ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(resProbBuffer));
            ENCODE_CHK_STATUS_RETURN(eStatus);
        }
        else if (i == 0)  // this buffer do not need to clear in current frame, also it has not been cleared to key type in previous frame.
        {                 // in this case, only context buffer 0 will be temporally overwritten.
            if (picFields.intra_only)
            {
                uint8_t *data = (uint8_t *)m_allocator->LockResourceForWrite(resProbBuffer);
                ENCODE_CHK_NULL_RETURN(data);

                if (!m_isPreCtx0InterProbSaved)  // only when non intra-only -> intra-only need save InterProb, otherwise leave saved InterProb unchanged.
                {
                    //save current interprob
                    ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(m_preCtx0InterProbSaved, CODECHAL_VP9_INTER_PROB_SIZE, data + CODEC_VP9_INTER_PROB_OFFSET, CODECHAL_VP9_INTER_PROB_SIZE));
                    m_isPreCtx0InterProbSaved = true;
                }
                eStatus = CtxBufDiffInit(data, true);
                ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(resProbBuffer));
                ENCODE_CHK_STATUS_RETURN(eStatus);
            }
            else if (m_isPreCtx0InterProbSaved)
            {
                uint8_t *data = (uint8_t *)m_allocator->LockResourceForWrite(resProbBuffer);
                ENCODE_CHK_NULL_RETURN(data);

                //reload former interprob
                ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(data + CODEC_VP9_INTER_PROB_OFFSET, CODECHAL_VP9_INTER_PROB_SIZE, m_preCtx0InterProbSaved, CODECHAL_VP9_INTER_PROB_SIZE));

                ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(resProbBuffer));
                m_isPreCtx0InterProbSaved = false;
            }
        }
    }

    // compressed header
    uint32_t          index         = 0;
    auto              txMode        = m_basicFeature->m_txMode;
    CompressedHeader *compressedHdr = (CompressedHeader *)MOS_AllocAndZeroMemory(sizeof(CompressedHeader) * (PAK_COMPRESSED_HDR_SYNTAX_ELEMS + 1));
    ENCODE_CHK_NULL_RETURN(compressedHdr);

    if (!picFields.LosslessFlag)
    {
        if (txMode == CODEC_VP9_TX_SELECTABLE)
        {
            PutDataForCompressedHdr(compressedHdr, 1, 128, PAK_TX_MODE_IDX);
            PutDataForCompressedHdr(compressedHdr, 1, 128, PAK_TX_MODE_IDX + 1);
            PutDataForCompressedHdr(compressedHdr, 1, 128, PAK_TX_MODE_SELECT_IDX);
        }
        else if (txMode == CODEC_VP9_TX_32X32)
        {
            PutDataForCompressedHdr(compressedHdr, 1, 128, PAK_TX_MODE_IDX);
            PutDataForCompressedHdr(compressedHdr, 1, 128, PAK_TX_MODE_IDX + 1);
            PutDataForCompressedHdr(compressedHdr, 0, 128, PAK_TX_MODE_SELECT_IDX);
        }
        else
        {
            PutDataForCompressedHdr(compressedHdr, (txMode & 0x02) >> 1, 128, PAK_TX_MODE_IDX);
            PutDataForCompressedHdr(compressedHdr, (txMode & 0x01), 128, PAK_TX_MODE_IDX + 1);
        }

        if (txMode == CODEC_VP9_TX_SELECTABLE)
        {
            index = PAK_TX_8x8_PROB_IDX;
            for (auto i = 0; i < 2; i++)
            {
                PutDataForCompressedHdr(compressedHdr, 0, 252, index);
                index += 2;
            }

            index = PAK_TX_16x16_PROB_IDX;
            for (auto i = 0; i < 2; i++)
            {
                for (auto j = 0; j < 2; j++)
                {
                    PutDataForCompressedHdr(compressedHdr, 0, 252, index);
                    index += 2;
                }
            }

            index = PAK_TX_32x32_PROB_IDX;
            for (auto i = 0; i < 2; i++)
            {
                for (auto j = 0; j < 3; j++)
                {
                    PutDataForCompressedHdr(compressedHdr, 0, 252, index);
                    index += 2;
                }
            }
        }
    }

    for (auto coeffSize = 0; coeffSize < 4; coeffSize++)
    {
        if (coeffSize > txMode)
        {
            continue;
        }

        switch (coeffSize)
        {
        case 0:
            index = PAK_TX_4x4_COEFF_PROB_IDX;
            break;
        case 1:
            index = PAK_TX_8x8_COEFF_PROB_IDX;
            break;
        case 2:
            index = PAK_TX_16x16_COEFF_PROB_IDX;
            break;
        case 3:
            index = PAK_TX_32x32_COEFF_PROB_IDX;
            break;
        }

        PutDataForCompressedHdr(compressedHdr, 0, 128, index);
    }

    PutDataForCompressedHdr(compressedHdr, 0, 252, PAK_SKIP_CONTEXT_IDX);
    PutDataForCompressedHdr(compressedHdr, 0, 252, PAK_SKIP_CONTEXT_IDX + 2);
    PutDataForCompressedHdr(compressedHdr, 0, 252, PAK_SKIP_CONTEXT_IDX + 4);

    if (picFields.frame_type != 0 && !picFields.intra_only)
    {
        index = PAK_INTER_MODE_CTX_IDX;
        for (auto i = 0; i < 7; i++)
        {
            for (auto j = 0; j < 3; j++)
            {
                PutDataForCompressedHdr(compressedHdr, 0, 252, index);
                index += 2;
            }
        }

        if (picFields.mcomp_filter_type == CODEC_VP9_SWITCHABLE_FILTERS)
        {
            index = PAK_SWITCHABLE_FILTER_CTX_IDX;
            for (auto i = 0; i < 4; i++)
            {
                for (auto j = 0; j < 2; j++)
                {
                    PutDataForCompressedHdr(compressedHdr, 0, 252, index);
                    index += 2;
                }
            }
        }

        index = PAK_INTRA_INTER_CTX_IDX;
        for (auto i = 0; i < 4; i++)
        {
            PutDataForCompressedHdr(compressedHdr, 0, 252, index);
            index += 2;
        }

        auto &picRefFields = m_basicFeature->m_vp9PicParams->RefFlags.fields;
        bool  allowComp    = !(
            (picRefFields.LastRefSignBias && picRefFields.GoldenRefSignBias && picRefFields.AltRefSignBias) ||
            (!picRefFields.LastRefSignBias && !picRefFields.GoldenRefSignBias && !picRefFields.AltRefSignBias));

        if (allowComp)
        {
            if (picFields.comp_prediction_mode == PRED_MODE_HYBRID)
            {
                PutDataForCompressedHdr(compressedHdr, 1, 128, PAK_COMPOUND_PRED_MODE_IDX);
                PutDataForCompressedHdr(compressedHdr, 1, 128, PAK_COMPOUND_PRED_MODE_IDX + 1);
                index = PAK_HYBRID_PRED_CTX_IDX;
                for (auto i = 0; i < 5; i++)
                {
                    PutDataForCompressedHdr(compressedHdr, 0, 252, index);
                    index += 2;
                }
            }
            else if (picFields.comp_prediction_mode == PRED_MODE_COMPOUND)
            {
                PutDataForCompressedHdr(compressedHdr, 1, 128, PAK_COMPOUND_PRED_MODE_IDX);
                PutDataForCompressedHdr(compressedHdr, 0, 128, PAK_COMPOUND_PRED_MODE_IDX + 1);
            }
            else
            {
                PutDataForCompressedHdr(compressedHdr, 0, 128, PAK_COMPOUND_PRED_MODE_IDX);
            }
        }

        if (picFields.comp_prediction_mode != PRED_MODE_COMPOUND)
        {
            index = PAK_SINGLE_REF_PRED_CTX_IDX;
            for (auto i = 0; i < 5; i++)
            {
                for (auto j = 0; j < 2; j++)
                {
                    PutDataForCompressedHdr(compressedHdr, 0, 252, index);
                    index += 2;
                }
            }
        }

        if (picFields.comp_prediction_mode != PRED_MODE_SINGLE)
        {
            index = PAK_CMPUND_PRED_CTX_IDX;
            for (auto i = 0; i < 5; i++)
            {
                PutDataForCompressedHdr(compressedHdr, 0, 252, index);
                index += 2;
            }
        }

        index = PAK_INTRA_MODE_PROB_CTX_IDX;
        for (auto i = 0; i < 4; i++)
        {
            for (auto j = 0; j < 9; j++)
            {
                PutDataForCompressedHdr(compressedHdr, 0, 252, index);
                index += 2;
            }
        }

        index = PAK_PARTITION_PROB_IDX;
        for (auto i = 0; i < 16; i++)
        {
            for (auto j = 0; j < 3; j++)
            {
                PutDataForCompressedHdr(compressedHdr, 0, 252, index);
                index += 2;
            }
        }

        index = PAK_MVJOINTS_PROB_IDX;
        for (auto i = 0; i < 3; i++)
        {
            PutDataForCompressedHdr(compressedHdr, 0, 252, index);
            index += 8;
        }

        for (auto d = 0; d < 2; d++)
        {
            index = (d == 0) ? PAK_MVCOMP0_IDX : PAK_MVCOMP1_IDX;
            PutDataForCompressedHdr(compressedHdr, 0, 252, index);
            index += 8;
            for (auto i = 0; i < 10; i++)
            {
                PutDataForCompressedHdr(compressedHdr, 0, 252, index);
                index += 8;
            }
            PutDataForCompressedHdr(compressedHdr, 0, 252, index);
            index += 8;
            for (auto i = 0; i < 10; i++)
            {
                PutDataForCompressedHdr(compressedHdr, 0, 252, index);
                index += 8;
            }
        }

        for (auto d = 0; d < 2; d++)
        {
            index = (d == 0) ? PAK_MVFRAC_COMP0_IDX : PAK_MVFRAC_COMP1_IDX;
            for (auto i = 0; i < 3; i++)
            {
                PutDataForCompressedHdr(compressedHdr, 0, 252, index);
                index += 8;
            }
            for (auto i = 0; i < 3; i++)
            {
                PutDataForCompressedHdr(compressedHdr, 0, 252, index);
                index += 8;
            }
            for (auto i = 0; i < 3; i++)
            {
                PutDataForCompressedHdr(compressedHdr, 0, 252, index);
                index += 8;
            }
        }

        if (picFields.allow_high_precision_mv)
        {
            for (auto d = 0; d < 2; d++)
            {
                index = (d == 0) ? PAK_MVHP_COMP0_IDX : PAK_MVHP_COMP1_IDX;
                PutDataForCompressedHdr(compressedHdr, 0, 252, index);
                index += 8;
                PutDataForCompressedHdr(compressedHdr, 0, 252, index);
            }
        }
    }

    PMOS_RESOURCE comprHeaderBuffer = pakFeature->GetCompressedHeaderBuffer();
    ENCODE_CHK_NULL_RETURN(comprHeaderBuffer);

    uint8_t *data = (uint8_t *)m_allocator->LockResourceForWrite(comprHeaderBuffer);
    if (data == nullptr)
    {
        MOS_FreeMemory(compressedHdr);
        ENCODE_CHK_NULL_RETURN(nullptr);
    }

    for (uint32_t i = 0; i < PAK_COMPRESSED_HDR_SYNTAX_ELEMS; i += 2)
    {
        data[i >> 1] = (compressedHdr[i + 1].value << 0x04) | (compressedHdr[i].value);
    }

    eStatus = m_allocator->UnLock(comprHeaderBuffer);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_FreeMemory(compressedHdr);
        ENCODE_CHK_STATUS_RETURN(eStatus);
    }

    MOS_FreeMemory(compressedHdr);
    return eStatus;
}

}  // namespace encode
