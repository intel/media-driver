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
//! \file     encode_vp9_hpu.cpp
//! \brief    Defines the common interface for vp9 encode hpu (header probability update) features
//!

#include "encode_vp9_hpu.h"
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

}  // namespace encode
