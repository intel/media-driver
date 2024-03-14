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
//! \file     encode_hevc_cqp.cpp
//! \brief    Defines the common interface for hevc encode cqp features
//!

#include "encode_hevc_basic_feature.h"
#include "encode_hevc_cqp.h"
#include "encode_hevc_vdenc_feature_manager.h"
#include "encode_hevc_vdenc_const_settings.h"

using namespace mhw::vdbox;
namespace encode
{
HevcEncodeCqp::HevcEncodeCqp(
    MediaFeatureManager *featureManager,
    EncodeAllocator     *allocator,
    CodechalHwInterfaceNext *hwInterface,
    void                *constSettings) :
    MediaFeature(constSettings, hwInterface ? hwInterface->GetOsInterface():nullptr),
    m_allocator(allocator)
{
    m_featureManager = featureManager;
    ENCODE_CHK_NULL_NO_STATUS_RETURN(hwInterface);
    m_mosCtx = hwInterface->GetOsInterface()->pOsContext;
    // can be optimized after move encode parameter to feature manager.
    auto encFeatureManager = dynamic_cast<EncodeHevcVdencFeatureManager *>(featureManager);
    ENCODE_CHK_NULL_NO_STATUS_RETURN(encFeatureManager);

    m_basicFeature = dynamic_cast<EncodeBasicFeature *>(encFeatureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);

    m_hcpItf = std::static_pointer_cast<mhw::vdbox::hcp::Itf>(hwInterface->GetHcpInterfaceNext());
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_hcpItf);
}

MOS_STATUS HevcEncodeCqp::Init(void *settings)
{
    ENCODE_FUNC_CALL();

#if (_DEBUG || _RELEASE_INTERNAL)
    MediaUserSetting::Value outValue;
    ReadUserSetting(
        m_userSettingPtr,
        outValue,
        "HEVC RDOQ Enable",
        MediaUserSetting::Group::Sequence);
    m_rdoqEnable = outValue.Get<bool>();
#else
    m_rdoqEnable = true;
#endif

    ENCODE_CHK_STATUS_RETURN(AllocateResources());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcEncodeCqp::Update(void *params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);

    EncoderParams *encodeParams = (EncoderParams *)params;

    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams =
        static_cast<PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
    ENCODE_CHK_NULL_RETURN(hevcSeqParams);
    PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams =
        static_cast<PCODEC_HEVC_ENCODE_PICTURE_PARAMS>(encodeParams->pPicParams);
    ENCODE_CHK_NULL_RETURN(hevcPicParams);

    ////Livia: Legacy driver won't care about PPS parameters for deblocking, it only care about slice parameter.
    ////As it set pps_deblocking_filter_disabled_flag=0, it's ok for enable/disable all slices DB
    ////But it maybe a potential issue with App set this as 1 and set enable/disable DB differently for each slices.
    ////hevcPictureParams->deblocking_filter_override_enabled_flag;
    ////hevcPictureParams->pps_deblocking_filter_disabled_flag;

    if (m_basicFeature->m_newSeq)
    {
        ENCODE_CHK_STATUS_RETURN(SetConstSettings());
    }
    m_picQPY              = hevcPicParams->QpY;
    m_transformSkipEnable = hevcPicParams->transform_skip_enabled_flag;

    m_saoEnable = hevcSeqParams->SAO_enabled_flag;
    if (m_saoEnable)
    {
        ENCODE_CHK_STATUS_RETURN(VerifySliceSAOState());
    }

    UpdateRDOQCfg();

#if (_DEBUG || _RELEASE_INTERNAL)
    ReportUserSettingForDebug(
        m_userSettingPtr,
        "HEVC RDOQ Enable",
        m_rdoqEnable,
        MediaUserSetting::Group::Sequence);
#endif

    return MOS_STATUS_SUCCESS;
}

void HevcEncodeCqp::UpdateRDOQCfg()
{
    ENCODE_FUNC_CALL();

    // RDOQ disable for SCC palette mode
    auto hevcFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
    ENCODE_CHK_NULL_NO_STATUS_RETURN(hevcFeature);

    if (hevcFeature->m_hevcSeqParams->palette_mode_enabled_flag)
    {
        m_rdoqEnable = false;
    }

    m_rdoqIntraTuThreshold = 0;
    if (m_rdoqEnable)
    {
        if (1 == m_basicFeature->m_targetUsage || 2 == m_basicFeature->m_targetUsage || 4 == m_basicFeature->m_targetUsage || 6 == m_basicFeature->m_targetUsage)
        {
            m_rdoqIntraTuThreshold = 0xffff;
        }
        else if (7 == m_basicFeature->m_targetUsage)
        {
            uint32_t frameSize = m_basicFeature->m_oriFrameWidth * m_basicFeature->m_oriFrameHeight;
            m_rdoqIntraTuThreshold = MOS_MIN(((frameSize * 30) / 100) >> 8, 0xffff);
        }
    }
}

MOS_STATUS HevcEncodeCqp::SetConstSettings()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(m_constSettings);
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    auto       setting = static_cast<HevcVdencFeatureSettings *>(m_constSettings);
    ENCODE_CHK_NULL_RETURN(setting);

    m_rdoqEnable = m_rdoqEnable ? (setting->rdoqEnable[m_basicFeature->m_targetUsage]) : m_rdoqEnable;

    return eStatus;
}

MOS_STATUS HevcEncodeCqp::AllocateResources()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    MOS_STATUS                       eStatus = MOS_STATUS_SUCCESS;

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format   = Format_Buffer;

    uint32_t              bufSize       = 0;
    hcp::HcpBufferSizePar hcpBufSizePar = {};
    hcpBufSizePar.ucMaxBitDepth  = m_basicFeature->m_bitDepth;
    hcpBufSizePar.ucChromaFormat = m_basicFeature->m_chromaFormat;
    // We should move the buffer allocation to picture level if the size is dependent on LCU size
    hcpBufSizePar.dwCtbLog2SizeY = 6;  //assume Max LCU size
    hcpBufSizePar.dwPicWidth     = MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, ((HevcBasicFeature *)m_basicFeature)->m_maxLCUSize);
    hcpBufSizePar.dwPicHeight    = MOS_ALIGN_CEIL(m_basicFeature->m_frameHeight, ((HevcBasicFeature *)m_basicFeature)->m_maxLCUSize);

    auto AllocateResource = [&](PMOS_RESOURCE &res, const hcp::HCP_INTERNAL_BUFFER_TYPE bufferType, const char *bufferName, bool usecache) {
        hcpBufSizePar.bufferType     = bufferType;
        eStatus                      = m_hcpItf->GetHcpBufSize(hcpBufSizePar, bufSize);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            ENCODE_ASSERTMESSAGE("Failed to get the size for Deblocking Filter Buffer.");
            return eStatus;
        }
        allocParamsForBufferLinear.dwBytes  = bufSize;
        allocParamsForBufferLinear.pBufName = bufferName;
        if (usecache)
        {
            allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        }
        else
        {
            allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
        }
        res = m_allocator->AllocateResource(allocParamsForBufferLinear, false);
        return MOS_STATUS_SUCCESS;
    };

    // Deblocking Filter Row Store Scratch data surface
    ENCODE_CHK_STATUS_RETURN(AllocateResource(m_resDeblockingFilterRowStoreScratchBuffer, hcp::HCP_INTERNAL_BUFFER_TYPE::DBLK_LINE, "DeblockingScratchBuffer", true));
    // Deblocking Filter Tile Row Store Scratch data surface
    ENCODE_CHK_STATUS_RETURN(AllocateResource(m_resDeblockingFilterTileRowStoreScratchBuffer, hcp::HCP_INTERNAL_BUFFER_TYPE::DBLK_TILE_LINE, "DeblockingTileRowScratchBuffer", true));
    // Deblocking Filter Column Row Store Scratch data surface
    ENCODE_CHK_STATUS_RETURN(AllocateResource(m_resDeblockingFilterColumnRowStoreScratchBuffer, hcp::HCP_INTERNAL_BUFFER_TYPE::DBLK_TILE_COL, "DeblockingColumnScratchBuffer", true));

    // SAO Line buffer
    ENCODE_CHK_STATUS_RETURN(AllocateResource(m_resSAOLineBuffer, hcp::HCP_INTERNAL_BUFFER_TYPE::SAO_LINE, "SaoLineBuffer", false));
    // SAO Tile Line buffer
    ENCODE_CHK_STATUS_RETURN(AllocateResource(m_resSAOTileLineBuffer, hcp::HCP_INTERNAL_BUFFER_TYPE::SAO_TILE_LINE, "SaoTileLineBuffer", false));

    // SAO Tile Column buffer
    ENCODE_CHK_STATUS_RETURN(AllocateResource(m_resSAOTileColumnBuffer, hcp::HCP_INTERNAL_BUFFER_TYPE::SAO_TILE_COL, "SaoTileColumnBuffer", false));

    // SAO StreamOut buffer
    uint32_t size = MOS_ALIGN_CEIL(((HevcBasicFeature *)m_basicFeature)->m_picWidthInMinLCU, 4) * m_hevcSAOStreamoutSizePerLCU;
    //extra added size to cover tile enabled case, per tile width aligned to 4.  20: max tile column No.
    size += 3 * 20 * m_hevcSAOStreamoutSizePerLCU;
    allocParamsForBufferLinear.dwBytes  = size;
    allocParamsForBufferLinear.pBufName = "SaoStreamOutBuffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ;
    m_resSAOStreamOutBuffer             = m_allocator->AllocateResource(allocParamsForBufferLinear, false);

    const uint32_t minLCUSize        = 16;
    const uint32_t picWidthInMinLCU  = MOS_ROUNDUP_DIVIDE(m_basicFeature->m_frameWidth, minLCUSize);   //assume smallest LCU to get max width
    const uint32_t picHeightInMinLCU = MOS_ROUNDUP_DIVIDE(m_basicFeature->m_frameHeight, minLCUSize);  //assume smallest LCU to get max height
    // Aligned to 4 for each tile column
    uint32_t maxTileColumn              = MOS_ROUNDUP_DIVIDE(m_basicFeature->m_frameWidth, CODECHAL_HEVC_MIN_TILE_SIZE);
    allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(picWidthInMinLCU + 3 * maxTileColumn, 4) * 16;
    allocParamsForBufferLinear.pBufName = "SaoRowStoreBuffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ;
    MOS_RESOURCE *allocatedresource = m_allocator->AllocateResource(allocParamsForBufferLinear, false);
    ENCODE_CHK_NULL_RETURN(allocatedresource);
    m_vdencSAORowStoreBuffer = *allocatedresource;
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        ENCODE_ASSERTMESSAGE("Failed to allocate SAO row store Buffer.");
        return eStatus;
    }

    return eStatus;
}

MOS_STATUS HevcEncodeCqp::VerifySliceSAOState()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_basicFeature);

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (m_saoEnable)
    {
        auto hevcFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(hevcFeature);

        PCODEC_HEVC_ENCODE_SLICE_PARAMS hevcSliceParams = hevcFeature->m_hevcSliceParams;
        ENCODE_CHK_NULL_RETURN(hevcSliceParams);

        uint32_t slcSaoLumaCount = 0, slcSaoChromaCount = 0;

        for (uint32_t slcCount = 0; slcCount < m_basicFeature->m_numSlices; slcCount++, hevcSliceParams++)
        {
            slcSaoLumaCount += hevcSliceParams->slice_sao_luma_flag;
            slcSaoChromaCount += hevcSliceParams->slice_sao_chroma_flag;
        }

        // For HCP_SLICE_STATE command, slices must have the same SAO setting within a picture for encoder.
        // And SAO should be disabled in HCP_SLICE_STATE command if luma and chroma sao disabled.
        if (((slcSaoLumaCount > 0) && (slcSaoLumaCount != m_basicFeature->m_numSlices)) ||
            ((slcSaoChromaCount > 0) && (slcSaoChromaCount != m_basicFeature->m_numSlices)) ||
            ((slcSaoLumaCount == 0) && (slcSaoChromaCount == 0)))
        {
            m_saoEnable = false;
            ENCODE_ASSERTMESSAGE("Invalid SAO parameters in slice. All slices must have the same SAO setting within a picture.");
        }
    }

    return eStatus;
}

MHW_SETPAR_DECL_SRC(HCP_PIC_STATE, HevcEncodeCqp)
{
    auto hevcFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(hevcFeature);

    PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams = hevcFeature->m_hevcPicParams;
    ENCODE_CHK_NULL_RETURN(hevcPicParams);

    params.transformSkipEnabled         = m_transformSkipEnable;
    params.sampleAdaptiveOffsetEnabled  = m_saoEnable;
    params.rdoqEnable                   = m_rdoqEnable;
    params.rhodomainframelevelqp        = params.rhodomainRateControlEnable ? hevcPicParams->QpY : 0;
    params.intratucountbasedrdoqdisable = m_rdoqEnable && (7 == m_basicFeature->m_targetUsage);
    params.rdoqintratuthreshold         = (uint16_t)m_rdoqIntraTuThreshold;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_PIPE_MODE_SELECT, HevcEncodeCqp)
{
    ENCODE_FUNC_CALL();

    params.bRdoqEnable   = m_rdoqEnable;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_PIPE_BUF_ADDR_STATE, HevcEncodeCqp)
{
    ENCODE_FUNC_CALL();

    params.presDeblockingFilterTileRowStoreScratchBuffer   = m_resDeblockingFilterTileRowStoreScratchBuffer;
    params.presDeblockingFilterColumnRowStoreScratchBuffer = m_resDeblockingFilterColumnRowStoreScratchBuffer;
    params.presMfdDeblockingFilterRowStoreScratchBuffer    = m_resDeblockingFilterRowStoreScratchBuffer;

    params.presSaoLineBuffer       = m_resSAOLineBuffer;
    params.presSaoTileLineBuffer   = m_resSAOTileLineBuffer;
    params.presSaoTileColumnBuffer = m_resSAOTileColumnBuffer;
    params.presSaoStreamOutBuffer  = m_resSAOStreamOutBuffer;
    params.presSaoRowStoreBuffer   = const_cast<PMOS_RESOURCE>(&m_vdencSAORowStoreBuffer);

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_SLICE_STATE, HevcEncodeCqp)
{
    auto hevcFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(hevcFeature);

    PCODEC_HEVC_ENCODE_SLICE_PARAMS hevcSliceParams = hevcFeature->m_hevcSliceParams;
    ENCODE_CHK_NULL_RETURN(hevcSliceParams);
    PCODEC_HEVC_ENCODE_SLICE_PARAMS pEncodeHevcSliceParams = (CODEC_HEVC_ENCODE_SLICE_PARAMS *)&hevcSliceParams[hevcFeature->m_curNumSlices];
    params.deblockingFilterDisable = pEncodeHevcSliceParams->slice_deblocking_filter_disable_flag;
    params.tcOffsetDiv2            = pEncodeHevcSliceParams->tc_offset_div2;
    params.betaOffsetDiv2          = pEncodeHevcSliceParams->beta_offset_div2;

    //SAO
    params.saoLumaFlag   = (m_saoEnable) ? pEncodeHevcSliceParams->slice_sao_luma_flag : 0;
    params.saoChromaFlag = (m_saoEnable) ? pEncodeHevcSliceParams->slice_sao_chroma_flag : 0;

    if (m_transformSkipEnable)
    {
        int slcQP = m_picQPY + pEncodeHevcSliceParams->slice_qp_delta;
        ENCODE_ASSERT(slcQP >= 0 && slcQP < HevcBasicFeature::m_qpNum);

        int qpIdx                    = 0;
        if (slcQP <= 22)
        {
            qpIdx = 0;
        }
        else if (slcQP <= 27)
        {
            qpIdx = 1;
        }
        else if (slcQP <= 32)
        {
            qpIdx = 2;
        }
        else
        {
            qpIdx = 3;
        }

        auto setting = static_cast<HevcVdencFeatureSettings *>(m_constSettings);
        ENCODE_CHK_NULL_RETURN(setting);

        params.transformskiplambda = setting->transformSkipLambdaTable[slcQP];

        if (m_basicFeature->m_pictureCodingType == I_TYPE)
        {
            params.transformskipNumzerocoeffsFactor0    = setting->transformSkipCoeffsTable[qpIdx][0][0][0][0];
            params.transformskipNumzerocoeffsFactor1    = setting->transformSkipCoeffsTable[qpIdx][0][0][1][0];
            params.transformskipNumnonzerocoeffsFactor0 = setting->transformSkipCoeffsTable[qpIdx][0][0][0][1] + 32;
            params.transformskipNumnonzerocoeffsFactor1 = setting->transformSkipCoeffsTable[qpIdx][0][0][1][1] + 32;
        }
        else
        {
            params.transformskipNumzerocoeffsFactor0    = setting->transformSkipCoeffsTable[qpIdx][1][0][0][0];
            params.transformskipNumzerocoeffsFactor1    = setting->transformSkipCoeffsTable[qpIdx][1][0][1][0];
            params.transformskipNumnonzerocoeffsFactor0 = setting->transformSkipCoeffsTable[qpIdx][1][0][0][1] + 32;
            params.transformskipNumnonzerocoeffsFactor1 = setting->transformSkipCoeffsTable[qpIdx][1][0][1][1] + 32;
        }
    }

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
