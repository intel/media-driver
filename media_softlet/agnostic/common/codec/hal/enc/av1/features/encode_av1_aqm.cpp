/*
* Copyright (c) 2021-2024, Intel Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,Av1EncodeTile
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
//! \file     encode_av1_aqm.cpp
//! \brief    Defines the common interface for av1 aqm
//!

#include "encode_av1_aqm.h"
#include "encode_av1_vdenc_feature_manager.h"
#include "encode_av1_basic_feature.h"

namespace encode
{
Av1EncodeAqm::Av1EncodeAqm(MediaFeatureManager *featureManager,
    EncodeAllocator *                           allocator,
    CodechalHwInterfaceNext *                   hwInterface,
    void *                                      constSettings) : EncodeAqmFeature(featureManager, allocator, hwInterface, constSettings)
{
    auto encFeatureManager = dynamic_cast<EncodeAv1VdencFeatureManager *>(featureManager);
    ENCODE_CHK_NULL_NO_STATUS_RETURN(encFeatureManager);

    m_basicFeature = dynamic_cast<EncodeBasicFeature *>(encFeatureManager->GetFeature(Av1FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);
};

MOS_STATUS Av1EncodeAqm::Update(void *params)
{
    auto basicFeature   = dynamic_cast<Av1BasicFeature *>(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(basicFeature);
    if (basicFeature->m_av1PicParams->QualityInfoSupportFlags.fields.enable_frame
        || basicFeature->m_av1PicParams->QualityInfoSupportFlags.fields.enable_block)
    {
        m_enabled = true;
    }

    m_numTiles          = basicFeature->m_av1PicParams->tile_rows * basicFeature->m_av1PicParams->tile_cols;
    m_tileBasedEngine   = m_numTiles > 1 ? true : false;
    for (uint32_t tileIdx = 0; tileIdx < m_numTiles && tileIdx < ENCODE_VDENC_MAX_TILE_NUM; tileIdx++)
    {
        EncodeTileData tileData = {};
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileByIndex, tileData, tileIdx);
        m_tile_width[tileIdx]       = (tileData.tileWidthInMinCbMinus1 + 1) * av1MinBlockWidth;
        m_tile_height[tileIdx]      = (tileData.tileHeightInMinCbMinus1 + 1) * av1MinBlockWidth;
    }
#if USE_CODECHAL_DEBUG_TOOL
    auto displayOrderInSeq = basicFeature->m_ref.GetFrameDisplayOrder();
    m_frameIdxQueue.push(displayOrderInSeq);
#endif
    ENCODE_CHK_STATUS_RETURN(EncodeAqmFeature::Update(params));
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AQM_PIC_STATE, Av1EncodeAqm)
{
    ENCODE_CHK_STATUS_RETURN(EncodeAqmFeature::MHW_SETPAR_F(AQM_PIC_STATE)(params));

    params.lcuSize   = LCU_SIZE_64X64;
    params.codectype = CODECTYPE_AV1;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AVP_PIC_STATE, Av1EncodeAqm)
{

    params.VdaqmEnable = m_enabled;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AQM_TILE_CODING, Av1EncodeAqm)
{
    auto encFeatureManager = dynamic_cast<EncodeAv1VdencFeatureManager *>(m_featureManager);
    ENCODE_CHK_NULL_RETURN(encFeatureManager);

    auto av1Tile = dynamic_cast<Av1EncodeTile *>(encFeatureManager->GetFeature(Av1FeatureIDs::encodeTile));
    ENCODE_CHK_NULL_RETURN(av1Tile);

    Av1TileInfo av1TileInfo;

    av1Tile->GetTileInfo(&av1TileInfo);

    params.tileId               = av1TileInfo.tileId;
    params.tileColPositionInSb  = av1TileInfo.tileColPositionInSb;
    params.tileRowPositionInSb  = av1TileInfo.tileRowPositionInSb;
    params.tileWidthInSbMinus1  = av1TileInfo.tileWidthInSbMinus1;
    params.tileHeightInSbMinus1 = av1TileInfo.tileHeightInSbMinus1;
    params.tileNum              = av1TileInfo.tileId;
    params.tileGroupId          = av1TileInfo.tileGroupId;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AQM_SLICE_STATE, Av1EncodeAqm)
{
    auto encFeatureManager = dynamic_cast<EncodeAv1VdencFeatureManager *>(m_featureManager);
    ENCODE_CHK_NULL_RETURN(encFeatureManager);

    auto av1Tile = dynamic_cast<Av1EncodeTile *>(encFeatureManager->GetFeature(Av1FeatureIDs::encodeTile));
    ENCODE_CHK_NULL_RETURN(av1Tile);

    Av1TileInfo av1TileInfo;

    av1Tile->GetTileInfo(&av1TileInfo);

    params.tileSliceStartLcuMbX     = av1TileInfo.tileStartXInLCU;
    params.tileSliceStartLcuMbY     = av1TileInfo.tileStartYInLCU;
    params.nextTileSliceStartLcuMbX = av1TileInfo.tileEndXInLCU;
    params.nextTileSliceStartLcuMbY = av1TileInfo.tileEndYInLCU;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VD_PIPELINE_FLUSH, Av1EncodeAqm)
{
    ENCODE_FUNC_CALL();
    auto av1BasicFeature = dynamic_cast<Av1BasicFeature *>(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(av1BasicFeature);

    if (av1BasicFeature->m_flushCmd == Av1BasicFeature::waitVdenc && m_enabled)
    {
        params.flushVDAQM    = true;
        params.waitDoneVDAQM = true;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AQM_PIPE_BUF_ADDR_STATE, Av1EncodeAqm)
{
    auto aqmFeature     = dynamic_cast<Av1EncodeAqm*>(m_featureManager->GetFeature(Av1FeatureIDs::av1Aqm));
    ENCODE_CHK_NULL_RETURN(aqmFeature);

    auto basicFeature   = dynamic_cast<Av1BasicFeature*>(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(basicFeature);

    bool mmcEnabled     = basicFeature->m_mmcState ? basicFeature->m_mmcState->IsMmcEnabled() : false;
    if (m_enabled)
    {
        auto bIdx = m_basicFeature->m_currOriginalPic.FrameIdx;

        for (uint8_t index = 0; index < AQM_INDEX; index++)
            params.AqmPipeBufAddrStatePar0[index] = EncodeAqmFeatureMember0[index];

        params.AqmPipeBufAddrStatePar4[0] = nullptr;
        params.AqmPipeBufAddrStatePar4[1] = m_basicFeature->m_recycleBuf->GetBuffer(RecycleResId::VdaqmBuffer1, bIdx);
        params.AqmPipeBufAddrStatePar4[2] = m_basicFeature->m_recycleBuf->GetBuffer(RecycleResId::VdaqmBuffer2, bIdx);
        params.AqmPipeBufAddrStatePar4[3] = m_basicFeature->m_recycleBuf->GetBuffer(RecycleResId::VdaqmBuffer3, bIdx);
        params.AqmPipeBufAddrStatePar4[4] = m_basicFeature->m_recycleBuf->GetBuffer(RecycleResId::VdaqmBuffer4, bIdx);
        params.AqmPipeBufAddrStatePar2    = m_basicFeature->m_recycleBuf->GetBuffer(RecycleResId::VdaqmBuffer0, bIdx);

        if (m_aqmMode)
        {
            params.surfaceRawBuffer     = &m_basicFeature->m_rawSurfaceToEnc->OsResource;
            params.surfaceReconBuffer   = &(&m_basicFeature->m_reconSurface)->OsResource;

            if (mmcEnabled)
            {
                ENCODE_CHK_STATUS_RETURN(basicFeature->m_mmcState->GetSurfaceMmcState(const_cast<PMOS_SURFACE>(&m_basicFeature->m_rawSurface), &params.mmcStateRawSurf));
                ENCODE_CHK_STATUS_RETURN(basicFeature->m_mmcState->GetSurfaceMmcState(const_cast<PMOS_SURFACE>(&m_basicFeature->m_reconSurface), &params.mmcStateReconSurf));
                ENCODE_CHK_STATUS_RETURN(basicFeature->m_mmcState->GetSurfaceMmcFormat(const_cast<PMOS_SURFACE>(m_basicFeature->m_rawSurfaceToEnc), &params.compressionFormat));
            }
            else
            {
                params.mmcStateRawSurf      = MOS_MEMCOMP_DISABLED;
                params.mmcStateReconSurf    = MOS_MEMCOMP_DISABLED;
                params.compressionFormat    = GMM_FORMAT_INVALID;
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AQM_SURFACE_STATE, Av1EncodeAqm)
{
    auto basicFeature = dynamic_cast<Av1BasicFeature*>(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(basicFeature);

    if (m_enabled && m_aqmMode)
    {
        // format
        if (!m_basicFeature->m_is10Bit)
        {
            params.surfaceFormat = mhw::vdbox::aqm::AQM_SURFACE_FORMAT::SURFACE_FORMAT_PLANAR4208;
        }
        else
        {
            if (params.surfaceStateId == srcInputPic)
            {
                params.surfaceFormat = mhw::vdbox::aqm::AQM_SURFACE_FORMAT::SURFACE_FORMAT_P010;
            }
            else
            {
                params.surfaceFormat = mhw::vdbox::aqm::AQM_SURFACE_FORMAT::SURFACE_FORMAT_P010VARIANT;
            }
        }

        switch (params.surfaceStateId)
        {
        case srcInputPic:
            params.pitch    = m_basicFeature->m_rawSurfaceToEnc->dwPitch;
            params.uOffset  = m_basicFeature->m_rawSurfaceToEnc->YoffsetForUplane;
            params.vOffset  = m_basicFeature->m_rawSurfaceToEnc->YoffsetForVplane;
            basicFeature->GetSurfaceMmcInfo(m_basicFeature->m_rawSurfaceToEnc, params.mmcStateRawSurf, params.compressionFormat);
            break;
        case reconPic:
            params.pitch    = m_basicFeature->m_reconSurface.dwPitch;
            params.uOffset  = m_basicFeature->m_reconSurface.YoffsetForUplane;
            params.vOffset  = m_basicFeature->m_reconSurface.YoffsetForVplane;
            basicFeature->GetSurfaceMmcInfo(const_cast<PMOS_SURFACE>(&m_basicFeature->m_reconSurface), params.mmcStateRawSurf, params.compressionFormat);
            break;
        }
    }

    return MOS_STATUS_SUCCESS;
}
}  // namespace encode
