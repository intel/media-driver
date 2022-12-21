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
//! \file     encode_vp9_cqp.cpp
//! \brief    Defines the common interface for vp9 encode cqp features
//!

#include "encode_vp9_cqp.h"
#include "encode_vp9_vdenc_feature_manager.h"
#include "encode_vp9_vdenc_const_settings.h"

namespace encode
{
Vp9EncodeCqp::Vp9EncodeCqp(
    MediaFeatureManager *featureManager,
    EncodeAllocator *    allocator,
    CodechalHwInterfaceNext *hwInterface,
    void *               constSettings) : MediaFeature(constSettings),
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

MOS_STATUS Vp9EncodeCqp::Init(void *settings)
{
    ENCODE_FUNC_CALL();

    m_enabled = true;

    ENCODE_CHK_STATUS_RETURN(AllocateResources());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeCqp::Update(void *params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);

    EncoderParams *encodeParams = (EncoderParams *)params;

    PCODEC_VP9_ENCODE_SEQUENCE_PARAMS vp9SeqParams = static_cast<PCODEC_VP9_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
    ENCODE_CHK_NULL_RETURN(vp9SeqParams);
    PCODEC_VP9_ENCODE_PIC_PARAMS vp9PicParams = static_cast<PCODEC_VP9_ENCODE_PIC_PARAMS>(encodeParams->pPicParams);
    ENCODE_CHK_NULL_RETURN(vp9PicParams);

    if (m_basicFeature->m_newSeq)
    {
        ENCODE_CHK_STATUS_RETURN(SetConstSettings());
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeCqp::AllocateResources()
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

    uint32_t formatMultiFactor = (m_basicFeature->m_chromaFormat == HCP_CHROMA_FORMAT_YUV444) ? 3 : 2;
    formatMultiFactor *= (m_basicFeature->m_bitDepth == 8) ? 1 : 2;
    uint32_t size = (18 * formatMultiFactor) / 2;
    // Deblocking filter line buffer
    size                                = m_basicFeature->m_maxPicWidthInSb * size * CODECHAL_CACHELINE_SIZE;
    allocParamsForBufferLinear.dwBytes  = size;
    allocParamsForBufferLinear.pBufName = "DeblockingFilterLineBuffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ;
    allocatedBuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
    ENCODE_CHK_NULL_RETURN(allocatedBuffer);
    m_resDeblockingFilterLineBuffer = *allocatedBuffer;

    // Deblocking filter tile line buffer
    allocParamsForBufferLinear.dwBytes  = size;
    allocParamsForBufferLinear.pBufName = "DeblockingFilterTileLineBuffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ;
    allocatedBuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
    ENCODE_CHK_NULL_RETURN(allocatedBuffer);
    m_resDeblockingFilterTileLineBuffer = *allocatedBuffer;

    formatMultiFactor = (m_basicFeature->m_chromaFormat == HCP_CHROMA_FORMAT_YUV444) ? 25 : 17;
    size              = formatMultiFactor * ((m_basicFeature->m_bitDepth == 8) ? 1 : 2);
    // Deblocking filter tile column buffer
    size                                = m_basicFeature->m_maxPicHeightInSb * size * CODECHAL_CACHELINE_SIZE;
    allocParamsForBufferLinear.dwBytes  = size;
    allocParamsForBufferLinear.pBufName = "DeblockingFilterTileColumnBuffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ;
    allocatedBuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
    ENCODE_CHK_NULL_RETURN(allocatedBuffer);
    m_resDeblockingFilterTileColumnBuffer = *allocatedBuffer;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_PIPE_MODE_SELECT, Vp9EncodeCqp)
{
    ENCODE_FUNC_CALL();

    params.bRdoqEnable = false;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_PIPE_BUF_ADDR_STATE, Vp9EncodeCqp)
{
    ENCODE_FUNC_CALL();

    params.presMfdDeblockingFilterRowStoreScratchBuffer    = const_cast<PMOS_RESOURCE>(&m_resDeblockingFilterLineBuffer);
    params.presDeblockingFilterTileRowStoreScratchBuffer   = const_cast<PMOS_RESOURCE>(&m_resDeblockingFilterTileLineBuffer);
    params.presDeblockingFilterColumnRowStoreScratchBuffer = const_cast<PMOS_RESOURCE>(&m_resDeblockingFilterTileColumnBuffer);

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
