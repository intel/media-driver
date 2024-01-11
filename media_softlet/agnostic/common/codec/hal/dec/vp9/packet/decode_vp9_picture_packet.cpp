/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     decode_vp9_picture_packet.cpp
//! \brief    Defines the interface for vp9 decode picture packet
//!
#include "codec_utilities_next.h"
#include "decode_vp9_picture_packet.h"
#include "codechal_debug.h"
#include "decode_vp9_mem_compression.h"
#include "decode_common_feature_defs.h"
#include "decode_vp9_phase_front_end.h"
#include "decode_vp9_phase_back_end.h"

namespace decode
{
Vp9DecodePicPkt::~Vp9DecodePicPkt()
{
    DECODE_FUNC_CALL();

    FreeResources();
}

MOS_STATUS Vp9DecodePicPkt::FreeResources()
{
    DECODE_FUNC_CALL();

    if (m_allocator != nullptr)
    {
        m_allocator->Destroy(m_resMfdDeblockingFilterRowStoreScratchBuffer);
        m_allocator->Destroy(m_resDeblockingFilterTileRowStoreScratchBuffer);
        m_allocator->Destroy(m_resDeblockingFilterColumnRowStoreScratchBuffer);
        m_allocator->Destroy(m_resMetadataLineBuffer);
        m_allocator->Destroy(m_resMetadataTileLineBuffer);
        m_allocator->Destroy(m_resMetadataTileColumnBuffer);
        m_allocator->Destroy(m_resSaoLineBuffer);
        m_allocator->Destroy(m_resSaoTileLineBuffer);
        m_allocator->Destroy(m_resSaoTileColumnBuffer);
        m_allocator->Destroy(m_resDeblockingFilterLineRowStoreScratchBuffer);
        m_allocator->Destroy(m_resHvcLineRowstoreBuffer);
        m_allocator->Destroy(m_resHvcTileRowstoreBuffer);
        m_allocator->Destroy(m_resIntraPredUpRightColStoreBuffer);
        m_allocator->Destroy(m_resIntraPredLeftReconColStoreBuffer);
        m_allocator->Destroy(m_resCABACSyntaxStreamOutBuffer);
        m_allocator->Destroy(m_resCABACStreamOutSizeBuffer);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DecodePicPkt::Init()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(m_featureManager);
    DECODE_CHK_NULL(m_hwInterface);
    DECODE_CHK_NULL(m_osInterface);
    DECODE_CHK_NULL(m_miItf);
    DECODE_CHK_NULL(m_vp9Pipeline);
    DECODE_CHK_NULL(m_hcpItf);

    m_vp9BasicFeature = dynamic_cast<Vp9BasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_vp9BasicFeature);

#ifdef _DECODE_PROCESSING_SUPPORTED
    m_downSamplingFeature      = dynamic_cast<DecodeDownSamplingFeature *>(m_featureManager->GetFeature(DecodeFeatureIDs::decodeDownSampling));
    DecodeSubPacket *subPacket = m_vp9Pipeline->GetSubPacket(DecodePacketId(m_vp9Pipeline, downSamplingSubPacketId));
    m_downSamplingPkt          = dynamic_cast<DecodeDownSamplingPkt *>(subPacket);
#endif

    m_allocator = m_pipeline->GetDecodeAllocator();
    DECODE_CHK_NULL(m_allocator);

    // DECODE_CHK_STATUS(m_statusReport->RegistObserver(this));

    DECODE_CHK_STATUS(AllocateFixedResources());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DecodePicPkt::Prepare()
{
    DECODE_FUNC_CALL();

    m_vp9PicParams = m_vp9BasicFeature->m_vp9PicParams;

    if (m_vp9PicParams->subsampling_x == 1 && m_vp9PicParams->subsampling_y == 1)
    {
        chromaSamplingFormat = HCP_CHROMA_FORMAT_YUV420;  
    }
    else if (m_vp9PicParams->subsampling_x == 0 && m_vp9PicParams->subsampling_y == 0)
    {
        chromaSamplingFormat = HCP_CHROMA_FORMAT_YUV444;
    }
    else
    {
        DECODE_ASSERTMESSAGE("Invalid Chroma sampling format!");
        return MOS_STATUS_INVALID_PARAMETER;
    }

#ifdef _MMC_SUPPORTED
    m_mmcState = m_vp9Pipeline->GetMmcState();
    DECODE_CHK_NULL(m_mmcState);
#endif

    DECODE_CHK_STATUS(SetRowstoreCachingOffsets());

    DECODE_CHK_STATUS(AllocateVariableResources());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DecodePicPkt::SetPhase(DecodePhase *phase)
{
    DECODE_FUNC_CALL();
    m_phase = phase;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DecodePicPkt::ReportCabacStreamOutSize(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(m_resCABACStreamOutSizeBuffer);

    auto mmioRegistersHcp = m_hwInterface->GetHcpInterfaceNext()->GetMmioRegisters(MHW_VDBOX_NODE_1);

    auto &par = m_miItf->GETPAR_MI_STORE_REGISTER_MEM();
    par       = {};
    par.presStoreBuffer    = &m_resCABACStreamOutSizeBuffer->OsResource;
    par.dwOffset           = 0;
    par.dwRegister         = mmioRegistersHcp->hcpDebugFEStreamOutSizeRegOffset;

    DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_STORE_REGISTER_MEM(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

bool Vp9DecodePicPkt::IsFrontEndPhase()
{
    DECODE_FUNC_CALL();

    if (m_phase == nullptr)
    {
        return false;
    }
    Vp9PhaseFrontEnd *frontEndPhase = dynamic_cast<Vp9PhaseFrontEnd *>(m_phase);
    return (frontEndPhase != nullptr);
}

bool Vp9DecodePicPkt::IsBackEndPhase()
{
    DECODE_FUNC_CALL();

    if (m_phase == nullptr)
    {
        return false;
    }
    Vp9PhaseBackEnd *backEndPhase = dynamic_cast<Vp9PhaseBackEnd *>(m_phase);
    return (backEndPhase != nullptr);
}

MOS_STATUS Vp9DecodePicPkt::SetRowstoreCachingOffsets()
{
    DECODE_FUNC_CALL();

    if (m_hcpItf->IsRowStoreCachingSupported() &&
        (m_vp9BasicFeature->m_frameWidthAlignedMinBlk != MOS_ALIGN_CEIL(m_vp9BasicFeature->m_prevFrmWidth, CODEC_VP9_MIN_BLOCK_WIDTH)))
    {
        MHW_VDBOX_ROWSTORE_PARAMS rowstoreParams;
        MOS_ZeroMemory(&rowstoreParams, sizeof(rowstoreParams));

        MOS_ZeroMemory(&rowstoreParams, sizeof(rowstoreParams));
        rowstoreParams.dwPicWidth       = m_vp9BasicFeature->m_frameWidthAlignedMinBlk;
        rowstoreParams.bMbaff           = false;
        rowstoreParams.Mode             = CODECHAL_DECODE_MODE_VP9VLD;
        rowstoreParams.ucBitDepthMinus8 = m_vp9PicParams->BitDepthMinus8;
        rowstoreParams.ucChromaFormat = (uint8_t)chromaSamplingFormat;
        DECODE_CHK_STATUS(m_hwInterface->SetRowstoreCachingOffsets(&rowstoreParams));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DecodePicPkt::AllocateFixedResources()
{
    DECODE_FUNC_CALL();

    if (m_resCABACStreamOutSizeBuffer == nullptr)
    {
        m_resCABACStreamOutSizeBuffer = m_allocator->AllocateBuffer(
            sizeof(uint64_t),
            "CABACStreamOutSizeBuffer",
            resourceInternalReadWriteCache,
            notLockableVideoMem);
        DECODE_CHK_NULL(m_resCABACStreamOutSizeBuffer);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DecodePicPkt::AllocateVariableResources()
{
    DECODE_FUNC_CALL();

    uint8_t maxBitDepth = 8 + m_vp9BasicFeature->m_vp9DepthIndicator * 2;
    m_widthInSb         = MOS_ROUNDUP_DIVIDE(m_vp9BasicFeature->m_width, CODEC_VP9_SUPER_BLOCK_WIDTH);
    m_heightInSb        = MOS_ROUNDUP_DIVIDE(m_vp9BasicFeature->m_height, CODEC_VP9_SUPER_BLOCK_HEIGHT);

    HcpBufferSizePar vp9BufSizeParams;
    MOS_ZeroMemory(&vp9BufSizeParams, sizeof(HcpBufferSizePar));
    vp9BufSizeParams.ucMaxBitDepth  = maxBitDepth;
    vp9BufSizeParams.ucChromaFormat = m_vp9BasicFeature->m_chromaFormat;
    vp9BufSizeParams.dwPicWidth     = m_widthInSb;
    vp9BufSizeParams.dwPicHeight    = m_heightInSb;
    vp9BufSizeParams.dwMaxFrameSize = m_vp9BasicFeature->m_dataSize;

    auto AllocateBuffer = [&](PMOS_BUFFER &buffer, HCP_INTERNAL_BUFFER_TYPE bufferType, const char *bufferName) {
        uint32_t bufSize = 0;
        vp9BufSizeParams.bufferType = bufferType;
        DECODE_CHK_STATUS(m_hcpItf->GetVP9BufSize(vp9BufSizeParams, bufSize));
        if (buffer == nullptr)
        {
            buffer = m_allocator->AllocateBuffer(
                bufSize, bufferName, resourceInternalReadWriteCache, notLockableVideoMem);
            DECODE_CHK_NULL(buffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(buffer, bufSize, notLockableVideoMem));
        }
        return MOS_STATUS_SUCCESS;
    };


    if (!m_hcpItf->IsVp9DfRowstoreCacheEnabled())
    {
        // Deblocking Filter Line Row Store Scratch data surface
        DECODE_CHK_STATUS(AllocateBuffer(m_resDeblockingFilterLineRowStoreScratchBuffer, HCP_INTERNAL_BUFFER_TYPE::DBLK_LINE, "BitstreamDecodeLineBuffer"));       
    }

    // Deblocking Filter Tile Row Store Scratch data surface
    DECODE_CHK_STATUS(AllocateBuffer(m_resDeblockingFilterTileRowStoreScratchBuffer, HCP_INTERNAL_BUFFER_TYPE::DBLK_TILE_LINE, "DeblockingTileScratchBuffer"));   

    // Deblocking Filter Column Row Store Scratch data surface
    DECODE_CHK_STATUS(AllocateBuffer(m_resDeblockingFilterColumnRowStoreScratchBuffer, HCP_INTERNAL_BUFFER_TYPE::DBLK_TILE_COL, "DeblockingColumnScratchBuffer"));   

    // Metadata Line buffer
    DECODE_CHK_STATUS(AllocateBuffer(m_resMetadataLineBuffer, HCP_INTERNAL_BUFFER_TYPE::META_LINE, "MetadataLineBuffer"));   

    // Metadata Tile Line buffer
    DECODE_CHK_STATUS(AllocateBuffer(m_resMetadataTileLineBuffer, HCP_INTERNAL_BUFFER_TYPE::META_TILE_LINE, "MetadataTileLineBuffer"));  

    // Metadata Tile Column buffer
    DECODE_CHK_STATUS(AllocateBuffer(m_resMetadataTileColumnBuffer, HCP_INTERNAL_BUFFER_TYPE::META_TILE_COL, "MetadataTileColumnBuffer"));

    // HVC Line Row Store Buffer
    DECODE_CHK_STATUS(AllocateBuffer(m_resHvcLineRowstoreBuffer, HCP_INTERNAL_BUFFER_TYPE::HVD_LINE, "HvcLineRowStoreBuffer"));

    // HVC Tile Row Store Buffer
    DECODE_CHK_STATUS(AllocateBuffer(m_resHvcTileRowstoreBuffer, HCP_INTERNAL_BUFFER_TYPE::HVD_TILE, "HvcTileRowStoreBuffer"));

   // Cabac stream out buffer
    DECODE_CHK_STATUS(AllocateBuffer(m_resCABACSyntaxStreamOutBuffer, HCP_INTERNAL_BUFFER_TYPE::CABAC_STREAMOUT, "CABACStreamOutBuffer"));

   // Intra prediction up right column store buffer
    DECODE_CHK_STATUS(AllocateBuffer(m_resIntraPredUpRightColStoreBuffer, HCP_INTERNAL_BUFFER_TYPE::INTRA_PRED_UP_RIGHT_COL, "IntraPredUpperRightColumnStore"));

   // Intra prediction left recon column store buffer
    DECODE_CHK_STATUS(AllocateBuffer(m_resIntraPredLeftReconColStoreBuffer, HCP_INTERNAL_BUFFER_TYPE::INTRA_PRED_LFT_RECON_COL, "IntraPredLeftReconColumnStore"));

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_PIPE_MODE_SELECT, Vp9DecodePicPkt)
{
    DECODE_FUNC_CALL();

    params.codecSelect         = 0; // CODEC_SELECT_DECODE
    params.codecStandardSelect = CodecHal_GetStandardFromMode(m_vp9BasicFeature->m_mode) - CODECHAL_HCP_BASE;
    params.bStreamOutEnabled   = false;

    auto cpInterface           = m_hwInterface->GetCpInterface();
    DECODE_CHK_NULL(cpInterface);

    bool twoPassScalable         = params.multiEngineMode != MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY && !params.bTileBasedReplayMode;
    params.setProtectionSettings = [=](uint32_t *data) { return cpInterface->SetProtectionSettingsForHcpPipeModeSelect(data, twoPassScalable); };
    
    params.mediaSoftResetCounterPer1000Clocks = 0;
#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_osInterface->bSoftReset)
    {
        params.mediaSoftResetCounterPer1000Clocks = 500;
    }
#endif

    auto waTable = m_osInterface->pfnGetWaTable(m_osInterface);
    DECODE_CHK_NULL(waTable);

    if (MEDIA_IS_WA(waTable, Wa_14012254246))
    {
        auto userSettingPtr    = m_osInterface->pfnGetUserSettingInstance(m_osInterface);
        params.prefetchDisable = ReadUserFeature(userSettingPtr, "DisableTlbPrefetch", MediaUserSetting::Group::Sequence).Get<bool>();
    }

    return MOS_STATUS_SUCCESS;
}

#ifdef _MMC_SUPPORTED
MOS_STATUS Vp9DecodePicPkt::SetRefMmcStatus(uint8_t surfaceID, PMOS_SURFACE pSurface)
{
    MOS_MEMCOMP_STATE mmcState = MOS_MEMCOMP_DISABLED;
    DECODE_CHK_STATUS(m_mmcState->SetSurfaceMmcState(pSurface));
    DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcState(pSurface, &mmcState));
    m_refsMmcEnable |= (mmcState == MOS_MEMCOMP_RC || mmcState == MOS_MEMCOMP_MC) ? (1 << (surfaceID - 2)) : 0;
    m_refsMmcType |= (mmcState == MOS_MEMCOMP_RC) ? (1 << (surfaceID - 2)) : 0;
    if (m_mmcState->IsMmcEnabled())
    {
        DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcFormat(pSurface, &m_mmcFormat));
    }
    return MOS_STATUS_SUCCESS;
}
#endif

MHW_SETPAR_DECL_SRC(HCP_SURFACE_STATE, Vp9DecodePicPkt)
{
    DECODE_FUNC_CALL();

    uint8_t  chromaType         = (uint8_t)chromaSamplingFormat;
    uint32_t dwUVPlaneAlignment = 8;
    uint8_t  ucBitDepthLumaMinus8   = (m_curHcpSurfStateId == CODECHAL_HCP_DECODED_SURFACE_ID) ? m_vp9PicParams->BitDepthMinus8 : 0;
    uint8_t  ucBitDepthChromaMinus8 = (m_curHcpSurfStateId == CODECHAL_HCP_DECODED_SURFACE_ID) ? m_vp9PicParams->BitDepthMinus8 : 0;

    DECODE_CHK_NULL(psSurface);

#ifdef _MMC_SUPPORTED
    if (m_curHcpSurfStateId == CODECHAL_HCP_DECODED_SURFACE_ID)
    {
        DECODE_CHK_STATUS(m_mmcState->SetSurfaceMmcState(&(m_vp9BasicFeature->m_destSurface)));
        DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcState(psSurface, &params.mmcState));
        DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcFormat(psSurface, &params.dwCompressionFormat));
    }
#endif

    uint32_t uvPlaneAlignment = m_uvPlaneAlignmentLegacy;
    params.surfaceStateId     = m_curHcpSurfStateId;
    params.surfacePitchMinus1 = psSurface->dwPitch - 1;

    if (ucBitDepthLumaMinus8 == 0 && ucBitDepthChromaMinus8 == 0)
    {
        if (chromaType == HCP_CHROMA_FORMAT_YUV420 && psSurface->Format == Format_NV12) // 4:2:0 8bit surface
        {
            params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_PLANAR4208;
        }
        else if (chromaType == HCP_CHROMA_FORMAT_YUV420 && psSurface->Format == Format_P010) // 4:2:0 10bit surface
        {
            params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_P010;
        }
        else if (chromaType == HCP_CHROMA_FORMAT_YUV422 && psSurface->Format == Format_YUY2) // 4:2:2 8bit surface
        {
            params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_YUY2FORMAT;
        }
        else if (chromaType == HCP_CHROMA_FORMAT_YUV422 && psSurface->Format == Format_Y210) // 4:2:2 10bit surface
        {
            params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_Y216Y210FORMAT;
        }
        else if (chromaType == HCP_CHROMA_FORMAT_YUV444 && psSurface->Format == Format_AYUV) // 4:4:4 8bit surface
        {
            params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_AYUV4444FORMAT;
        }
        else if (chromaType == HCP_CHROMA_FORMAT_YUV444 && psSurface->Format == Format_Y410) // 4:4:4 10bit surface
        {
            params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_Y410FORMAT;
        }
        else if (chromaType == HCP_CHROMA_FORMAT_YUV420 && psSurface->Format == Format_P016) // 4:2:0 16bit surface
        {
            params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_P016;
        }
        else if (chromaType == HCP_CHROMA_FORMAT_YUV422 && psSurface->Format == Format_Y216) // 4:2:2 16bit surface
        {
            params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_Y216Y210FORMAT;
        }
        else if (chromaType == HCP_CHROMA_FORMAT_YUV444 && psSurface->Format == Format_Y416) // 4:4:4
        {
            params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_Y416FORMAT;
        }
        else
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }
    else if ((ucBitDepthLumaMinus8 <= 2) && (ucBitDepthChromaMinus8 <= 2)) // only support bitdepth <= 10bit
    {
        if (chromaType == HCP_CHROMA_FORMAT_YUV420 && psSurface->Format == Format_P010) // 4:2:0
        {
            params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_P010;
        }
        else if (chromaType == HCP_CHROMA_FORMAT_YUV420 && psSurface->Format == Format_P016) // 4:2:0
        {
            params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_P016;
        }
        else if (chromaType == HCP_CHROMA_FORMAT_YUV422 && psSurface->Format == Format_Y210) // 4:2:2
        {
            params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_Y216Y210FORMAT;
        }
        else if (chromaType == HCP_CHROMA_FORMAT_YUV422 && psSurface->Format == Format_Y216) // 4:2:2
        {
            params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_Y216Y210FORMAT;
        }
        else if (chromaType == HCP_CHROMA_FORMAT_YUV444 && psSurface->Format == Format_Y410) // 4:4:4
        {
            params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_Y410FORMAT;
        }
        else if (chromaType == HCP_CHROMA_FORMAT_YUV444 && psSurface->Format == Format_Y416) // 4:4:4
        {
            params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_Y416FORMAT;
        }
        else
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }
    else  // 12bit
    {
        if (chromaType == HCP_CHROMA_FORMAT_YUV420 && psSurface->Format == Format_P016) // 4:2:0
        {
            params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_P016;
        }
        else if (chromaType == HCP_CHROMA_FORMAT_YUV422 && psSurface->Format == Format_Y216) // 4:2:2
        {
            params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_Y216Y210FORMAT;
        }
        else if (chromaType == HCP_CHROMA_FORMAT_YUV444 && psSurface->Format == Format_Y416) // 4:4:4
        {
            params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_Y416FORMAT;
        }
        else
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }

    uvPlaneAlignment = dwUVPlaneAlignment;

    params.yOffsetForUCbInPixel =
        MOS_ALIGN_CEIL((psSurface->UPlaneOffset.iSurfaceOffset - psSurface->dwOffset) / psSurface->dwPitch + psSurface->RenderOffset.YUV.U.YOffset, uvPlaneAlignment);

    if ((ucBitDepthLumaMinus8 == 4) || (ucBitDepthChromaMinus8 == 4)) // 12 bit
    {
        params.defaultAlphaValue = 0xfff0;
    }
    else
    {
        params.defaultAlphaValue = 0xffff;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DecodePicPkt::AddAllCmds_HCP_SURFACE_STATE(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    auto &params = m_hcpItf->MHW_GETPAR_F(HCP_SURFACE_STATE)();
    params       = {};

    // destsurface surface cmd set
    m_curHcpSurfStateId = CODECHAL_HCP_DECODED_SURFACE_ID;
    psSurface           = &m_vp9BasicFeature->m_destSurface;
    SETPAR_AND_ADDCMD(HCP_SURFACE_STATE, m_hcpItf, &cmdBuffer);

    // For non-key frame, send extra surface commands for reference pictures
    if (m_vp9PicParams->PicFlags.fields.frame_type == CODEC_VP9_INTER_FRAME &&
        !m_vp9PicParams->PicFlags.fields.intra_only)
    {
        mhw::vdbox::hcp::HCP_SURFACE_STATE_PAR refSurfaceParams[3] = {m_hcpItf->MHW_GETPAR_F(HCP_SURFACE_STATE)()};
        MOS_ZeroMemory(refSurfaceParams, sizeof(refSurfaceParams));

        for (uint8_t i = 0; i < 3; i++)
        {
            switch (i)
            {
            case 0:
                psSurface           = &(m_vp9BasicFeature->m_lastRefSurface);
                m_curHcpSurfStateId = CODECHAL_HCP_LAST_SURFACE_ID;
                break;
            case 1:
                psSurface           = &(m_vp9BasicFeature->m_goldenRefSurface);
                m_curHcpSurfStateId = CODECHAL_HCP_GOLDEN_SURFACE_ID;
                break;
            case 2:
                psSurface           = &(m_vp9BasicFeature->m_altRefSurface);
                m_curHcpSurfStateId = CODECHAL_HCP_ALTREF_SURFACE_ID;
                break;
            }
#ifdef _MMC_SUPPORTED            
            DECODE_CHK_STATUS(m_mmcState->SetSurfaceMmcState(psSurface));
            DECODE_CHK_STATUS(SetRefMmcStatus(m_curHcpSurfStateId, psSurface));
#endif
            MHW_SETPAR_F(HCP_SURFACE_STATE)(refSurfaceParams[i]);
        }

        for (uint8_t i = 0; i < 3; i++)
        {
            params = refSurfaceParams[i];
#ifdef _MMC_SUPPORTED 
            params.refsMmcEnable = m_refsMmcEnable;
            params.refsMmcType   = m_refsMmcType;
            params.dwCompressionFormat = m_mmcFormat;
#endif
            DECODE_CHK_STATUS(m_hcpItf->MHW_ADDCMD_F(HCP_SURFACE_STATE)(&cmdBuffer));
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DecodePicPkt::AddAllCmds_HCP_VP9_SEGMENT_STATE(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    auto &params = m_hcpItf->MHW_GETPAR_F(HCP_VP9_SEGMENT_STATE)();
    params       = {};

    PCODEC_VP9_SEGMENT_PARAMS pVp9SegmentParams = m_vp9BasicFeature->m_vp9SegmentParams;

    for (uint8_t i = 0; i < CODEC_VP9_MAX_SEGMENTS; i++)
    {
        // Error handling for illegal programming on segmentation fields @ KEY/INTRA_ONLY frames
        PCODEC_VP9_SEG_PARAMS vp9SegData = &(pVp9SegmentParams->SegData[i]);

        if (vp9SegData->SegmentFlags.fields.SegmentReferenceEnabled &&
            (!m_vp9PicParams->PicFlags.fields.frame_type || m_vp9PicParams->PicFlags.fields.intra_only))
        {
            vp9SegData->SegmentFlags.fields.SegmentReference = CODECHAL_DECODE_VP9_INTRA_FRAME;
        }

        params.segmentId = i;
        params.segmentSkipped                   = vp9SegData->SegmentFlags.fields.SegmentReferenceSkipped;
        params.segmentReference                 = vp9SegData->SegmentFlags.fields.SegmentReference;
        params.segmentReferenceEnabled          = vp9SegData->SegmentFlags.fields.SegmentReferenceEnabled;
        params.filterLevelRef0Mode0             = vp9SegData->FilterLevel[0][0];
        params.filterLevelRef0Mode1             = vp9SegData->FilterLevel[0][1];
        params.filterLevelRef1Mode0             = vp9SegData->FilterLevel[1][0];
        params.filterLevelRef1Mode1             = vp9SegData->FilterLevel[1][1];
        params.filterLevelRef2Mode0             = vp9SegData->FilterLevel[2][0];
        params.filterLevelRef2Mode1             = vp9SegData->FilterLevel[2][1];
        params.filterLevelRef3Mode0             = vp9SegData->FilterLevel[3][0];
        params.filterLevelRef3Mode1             = vp9SegData->FilterLevel[3][1];
        params.lumaDcQuantScaleDecodeModeOnly   = vp9SegData->LumaDCQuantScale;
        params.lumaAcQuantScaleDecodeModeOnly   = vp9SegData->LumaACQuantScale;
        params.chromaDcQuantScaleDecodeModeOnly = vp9SegData->ChromaDCQuantScale;
        params.chromaAcQuantScaleDecodeModeOnly = vp9SegData->ChromaACQuantScale;

        DECODE_CHK_STATUS(m_hcpItf->MHW_ADDCMD_F(HCP_VP9_SEGMENT_STATE)(&cmdBuffer));

        if (m_vp9PicParams->PicFlags.fields.segmentation_enabled == 0)
        {
            break;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_PIPE_BUF_ADDR_STATE, Vp9DecodePicPkt)
{
    DECODE_FUNC_CALL();

    params.Mode                                       = (CODECHAL_MODE)CODECHAL_DECODE_MODE_VP9VLD;
    params.psPreDeblockSurface                        = &(m_vp9BasicFeature->m_destSurface);
    params.presReferences[CodechalDecodeLastRef]      = m_vp9BasicFeature->m_presLastRefSurface;
    params.presReferences[CodechalDecodeGoldenRef]    = m_vp9BasicFeature->m_presGoldenRefSurface;
    params.presReferences[CodechalDecodeAlternateRef] = m_vp9BasicFeature->m_presAltRefSurface;

    params.presMfdDeblockingFilterRowStoreScratchBuffer    = &(m_resDeblockingFilterLineRowStoreScratchBuffer->OsResource);
    params.presDeblockingFilterTileRowStoreScratchBuffer   = &(m_resDeblockingFilterTileRowStoreScratchBuffer->OsResource);
    params.presDeblockingFilterColumnRowStoreScratchBuffer = &(m_resDeblockingFilterColumnRowStoreScratchBuffer->OsResource);

    params.presMetadataLineBuffer       = &(m_resMetadataLineBuffer->OsResource);
    params.presMetadataTileLineBuffer   = &(m_resMetadataTileLineBuffer->OsResource);
    params.presMetadataTileColumnBuffer = &(m_resMetadataTileColumnBuffer->OsResource);

    params.presHvdLineRowStoreBuffer = &(m_resHvcLineRowstoreBuffer->OsResource);
    params.presHvdTileRowStoreBuffer = &(m_resHvcTileRowstoreBuffer->OsResource);

#ifdef _MMC_SUPPORTED
    DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcState(params.psPreDeblockSurface, &params.PreDeblockSurfMmcState));
#endif

    params.presVp9SegmentIdBuffer = &(m_vp9BasicFeature->m_resVp9SegmentIdBuffer->OsResource);
    params.presVp9ProbBuffer      = &(m_vp9BasicFeature->m_resVp9ProbBuffer[m_vp9BasicFeature->m_frameCtxIdx]->OsResource);

    Vp9ReferenceFrames         &refFrames     = m_vp9BasicFeature->m_refFrames;
    const std::vector<uint8_t> &activeRefList = refFrames.GetActiveReferenceList(*m_vp9PicParams);

    if ((m_vp9PicParams->PicFlags.fields.frame_type == CODEC_VP9_INTER_FRAME) && !m_vp9PicParams->PicFlags.fields.intra_only)
    {
        params.presCurMvTempBuffer = &(m_vp9BasicFeature->m_resVp9MvTemporalBuffer[m_vp9BasicFeature->m_curMvTempBufIdx]->OsResource);

        if (!m_vp9BasicFeature->m_prevFrameParams.fields.KeyFrame && !m_vp9PicParams->PicFlags.fields.intra_only)
        {
            params.presColMvTempBuffer[0] = &(m_vp9BasicFeature->m_resVp9MvTemporalBuffer[m_vp9BasicFeature->m_colMvTempBufIdx]->OsResource);
        }
    }

    DECODE_CHK_STATUS(FixHcpPipeBufAddrParams());

    CODECHAL_DEBUG_TOOL(DumpResources(params, activeRefList.size(), m_vp9BasicFeature->m_resVp9MvTemporalBuffer[0]->size));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DecodePicPkt::FixHcpPipeBufAddrParams() const
{
    DECODE_FUNC_CALL();
    auto &params = m_hcpItf->MHW_GETPAR_F(HCP_PIPE_BUF_ADDR_STATE)();

    PMOS_RESOURCE dummyRef = nullptr ;

    if (m_vp9BasicFeature->m_dummyReferenceStatus &&
        !m_allocator->ResourceIsNull(&(m_vp9BasicFeature->m_dummyReference.OsResource)))
    {
        dummyRef = &(m_vp9BasicFeature->m_dummyReference.OsResource);
    }
    else
    {
        dummyRef = &(m_vp9BasicFeature->m_destSurface.OsResource);
    }

    for (uint8_t i = 0; i < CODECHAL_DECODE_VP9_MAX_NUM_REF_FRAME; i++)
    {
        if (!params.presReferences[i])
        {
            params.presReferences[i] = dummyRef;
        }
    }
    
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_IND_OBJ_BASE_ADDR_STATE, Vp9DecodePicPkt)
{
    DECODE_FUNC_CALL();
    params = {};

    params.bDecodeInUse   = true;
    params.dwDataSize     = m_vp9BasicFeature->m_dataSize;
    params.dwDataOffset   = m_vp9BasicFeature->m_dataOffset;
    params.presDataBuffer = &(m_vp9BasicFeature->m_resDataBuffer.OsResource);

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_VP9_PIC_STATE, Vp9DecodePicPkt)
{
    DECODE_FUNC_CALL();

    params.bDecodeInUse = true;

    auto            vp9RefList   = &(m_vp9BasicFeature->m_refFrames.m_vp9RefList[0]);
    PrevFrameParams prevFramePar = {};
    prevFramePar.value           = m_vp9BasicFeature->m_prevFrameParams.value;
    uint32_t dwPrevFrmWidth      = m_vp9BasicFeature->m_prevFrmWidth;
    uint32_t dwPrevFrmHeight     = m_vp9BasicFeature->m_prevFrmHeight;

    m_vp9BasicFeature->m_prevFrmWidth  = m_vp9PicParams->FrameWidthMinus1 + 1;
    m_vp9BasicFeature->m_prevFrmHeight = m_vp9PicParams->FrameHeightMinus1 + 1;

    // Update preframe field here, as pre frame keyframe was used for presColMvTempBuffer in SetHcpPipeBufAddrParams
    // Make sure the value was not updated before SetHcpPipeBufAddrParams
    m_vp9BasicFeature->m_prevFrameParams.fields.KeyFrame  = !m_vp9PicParams->PicFlags.fields.frame_type;
    m_vp9BasicFeature->m_prevFrameParams.fields.IntraOnly = m_vp9PicParams->PicFlags.fields.intra_only;
    m_vp9BasicFeature->m_prevFrameParams.fields.Display   = m_vp9PicParams->PicFlags.fields.show_frame;

    uint32_t curFrameWidth  = m_vp9PicParams->FrameWidthMinus1 + 1;
    uint32_t curFrameHeight = m_vp9PicParams->FrameHeightMinus1 + 1;
    bool     isScaling      = (curFrameWidth == dwPrevFrmWidth) && (curFrameHeight == dwPrevFrmHeight) ? false : true;

    params.frameWidthInPixelsMinus1  = MOS_ALIGN_CEIL(curFrameWidth, CODEC_VP9_MIN_BLOCK_WIDTH)  - 1;
    params.frameHeightInPixelsMinus1 = MOS_ALIGN_CEIL(curFrameHeight, CODEC_VP9_MIN_BLOCK_WIDTH) - 1;
    params.frameType                 = m_vp9PicParams->PicFlags.fields.frame_type;
    params.adaptProbabilitiesFlag    = !m_vp9PicParams->PicFlags.fields.error_resilient_mode && !m_vp9PicParams->PicFlags.fields.frame_parallel_decoding_mode;
    params.intraOnlyFlag             = m_vp9PicParams->PicFlags.fields.intra_only;
    params.refreshFrameContext       = m_vp9PicParams->PicFlags.fields.refresh_frame_context;
    params.errorResilientMode        = m_vp9PicParams->PicFlags.fields.error_resilient_mode;
    params.frameParallelDecodingMode = m_vp9PicParams->PicFlags.fields.frame_parallel_decoding_mode;
    params.filterLevel               = m_vp9PicParams->filter_level;
    params.sharpnessLevel            = m_vp9PicParams->sharpness_level;
    params.segmentationEnabled       = m_vp9PicParams->PicFlags.fields.segmentation_enabled;
    params.segmentationUpdateMap     = params.segmentationEnabled && m_vp9PicParams->PicFlags.fields.segmentation_update_map;
    params.losslessMode              = m_vp9PicParams->PicFlags.fields.LosslessFlag;
    params.segmentIdStreamOutEnable  = params.segmentationUpdateMap;

    uint8_t segmentIDStreaminEnable = 0;
    if (m_vp9PicParams->PicFlags.fields.intra_only ||
        (m_vp9PicParams->PicFlags.fields.frame_type == CODEC_VP9_KEY_FRAME))
    {
        segmentIDStreaminEnable = 1;
    }
    else if (m_vp9PicParams->PicFlags.fields.segmentation_enabled)
    {
        if (!m_vp9PicParams->PicFlags.fields.segmentation_update_map)
        {
            segmentIDStreaminEnable = 1;
        }
        else if (m_vp9PicParams->PicFlags.fields.segmentation_temporal_update)
        {
            segmentIDStreaminEnable = 1;
        }
    }
    if (m_vp9PicParams->PicFlags.fields.error_resilient_mode)
    {
        segmentIDStreaminEnable = 1;
    }
    // Resolution change will reset the segment ID buffer
    if (isScaling)
    {
        segmentIDStreaminEnable = 1;
    }
    params.segmentIdStreamInEnable = segmentIDStreaminEnable;

    params.log2TileRow    = m_vp9PicParams->log2_tile_rows;    // No need to minus 1 here.
    params.log2TileColumn = m_vp9PicParams->log2_tile_columns; // No need to minus 1 here.
    if (m_vp9PicParams->subsampling_x == 1 && m_vp9PicParams->subsampling_y == 1)
    {
        // 4:2:0
        params.chromaSamplingFormat = 0;
    }
    else if (m_vp9PicParams->subsampling_x == 1 && m_vp9PicParams->subsampling_y == 0)
    {
        // 4:2:2
        params.chromaSamplingFormat = 1;
    }
    else if (m_vp9PicParams->subsampling_x == 0 && m_vp9PicParams->subsampling_y == 0)
    {
        // 4:4:4
        params.chromaSamplingFormat = 2;
    }
    params.bitdepthMinus8 = m_vp9PicParams->BitDepthMinus8;
    params.profileLevel   = m_vp9PicParams->profile;

    params.uncompressedHeaderLengthInBytes70 = m_vp9PicParams->UncompressedHeaderLengthInBytes;
    params.firstPartitionSizeInBytes150      = m_vp9PicParams->FirstPartitionSize;

    if (m_vp9PicParams->PicFlags.fields.frame_type && !m_vp9PicParams->PicFlags.fields.intra_only)
    {
        PCODEC_PICTURE refFrameList = &(m_vp9PicParams->RefFrameList[0]);

        uint8_t  lastRefPicIndex    = refFrameList[m_vp9PicParams->PicFlags.fields.LastRefIdx].FrameIdx;
        uint32_t lastRefFrameWidth  = vp9RefList[lastRefPicIndex]->dwFrameWidth;
        uint32_t lastRefFrameHeight = vp9RefList[lastRefPicIndex]->dwFrameHeight;

        uint8_t  goldenRefPicIndex    = refFrameList[m_vp9PicParams->PicFlags.fields.GoldenRefIdx].FrameIdx;
        uint32_t goldenRefFrameWidth  = vp9RefList[goldenRefPicIndex]->dwFrameWidth;
        uint32_t goldenRefFrameHeight = vp9RefList[goldenRefPicIndex]->dwFrameHeight;

        uint8_t  altRefPicIndex    = refFrameList[m_vp9PicParams->PicFlags.fields.AltRefIdx].FrameIdx;
        uint32_t altRefFrameWidth  = vp9RefList[altRefPicIndex]->dwFrameWidth;
        uint32_t altRefFrameHeight = vp9RefList[altRefPicIndex]->dwFrameHeight;

        params.allowHiPrecisionMv         = m_vp9PicParams->PicFlags.fields.allow_high_precision_mv;
        params.mcompFilterType            = m_vp9PicParams->PicFlags.fields.mcomp_filter_type;
        params.segmentationTemporalUpdate = params.segmentationUpdateMap && m_vp9PicParams->PicFlags.fields.segmentation_temporal_update;

        params.refFrameSignBias02 = m_vp9PicParams->PicFlags.fields.LastRefSignBias |
                                    (m_vp9PicParams->PicFlags.fields.GoldenRefSignBias << 1) |
                                    (m_vp9PicParams->PicFlags.fields.AltRefSignBias << 2);

        params.lastFrameType = !prevFramePar.fields.KeyFrame;

        // Reset UsePrevInFindMvReferences to zero if last picture has a different size,
        // Current picture is error-resilient mode, Last picture was intra_only or keyframe,
        // Last picture was not a displayed picture.
        params.usePrevInFindMvReferences =
            !(m_vp9PicParams->PicFlags.fields.error_resilient_mode ||
            prevFramePar.fields.KeyFrame  ||
            prevFramePar.fields.IntraOnly ||
            !prevFramePar.fields.Display);

        // Reset UsePrevInFindMvReferences in case of resolution change on inter frames
        if (isScaling)
        {
            params.usePrevInFindMvReferences = 0;
        }

        params.horizontalScaleFactorForLast    = (lastRefFrameWidth * m_vp9ScalingFactor) / curFrameWidth;
        params.verticalScaleFactorForLast      = (lastRefFrameHeight * m_vp9ScalingFactor) / curFrameHeight;
        params.horizontalScaleFactorForGolden  = (goldenRefFrameWidth * m_vp9ScalingFactor) / curFrameWidth;
        params.verticalScaleFactorForGolden    = (goldenRefFrameHeight * m_vp9ScalingFactor) / curFrameHeight;
        params.horizontalScaleFactorForAltref  = (altRefFrameWidth * m_vp9ScalingFactor) / curFrameWidth;
        params.verticalScaleFactorForAltref    = (altRefFrameHeight * m_vp9ScalingFactor) / curFrameHeight;
        params.lastFrameWidthInPixelsMinus1    = lastRefFrameWidth - 1;
        params.lastFrameHeightInPixelsMinus1   = lastRefFrameHeight - 1;
        params.goldenFrameWidthInPixelsMinus1  = goldenRefFrameWidth - 1;
        params.goldenFrameHeightInPixelsMinus1 = goldenRefFrameHeight - 1;
        params.altrefFrameWidthInPixelsMinus1  = altRefFrameWidth - 1;
        params.altrefFrameHeightInPixelsMinus1 = altRefFrameHeight - 1;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_BSD_OBJECT, Vp9DecodePicPkt)
{
    DECODE_FUNC_CALL();

    params.bsdDataLength      = m_vp9PicParams->BSBytesInBuffer - m_vp9PicParams->UncompressedHeaderLengthInBytes;
    params.bsdDataStartOffset = m_vp9PicParams->UncompressedHeaderLengthInBytes; // already defined in HEVC patch

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DecodePicPkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    DECODE_FUNC_CALL();

    commandBufferSize      = m_pictureStatesSize;
    requestedPatchListSize = m_picturePatchListSize;

    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS Vp9DecodePicPkt::DumpResources(HCP_PIPE_BUF_ADDR_STATE_PAR &params, uint32_t refSize, uint32_t mvBufferSize) const
{
    DECODE_FUNC_CALL();

    CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
    DECODE_CHK_NULL(debugInterface);

    if (m_vp9PicParams->PicFlags.fields.frame_type == CODEC_VP9_INTER_FRAME)
    {
        for (uint16_t n = 0; n < refSize; n++)
        {
            if (params.presReferences[n])
            {
                MOS_SURFACE refSurface;
                MOS_ZeroMemory(&refSurface, sizeof(MOS_SURFACE));
                refSurface.OsResource = *(params.presReferences[n]);
                DECODE_CHK_STATUS(CodecUtilities::CodecHalGetResourceInfo(
                    m_osInterface,
                    &refSurface));

                debugInterface->m_refIndex = n;
                std::string refSurfName    = "RefSurf[" + std::to_string(static_cast<uint32_t>(debugInterface->m_refIndex)) + "]";
                DECODE_CHK_STATUS(debugInterface->DumpYUVSurface(
                    &refSurface,
                    CodechalDbgAttr::attrDecodeReferenceSurfaces,
                    refSurfName.c_str()));
            }
        }
    }

    if (params.presColMvTempBuffer[0])
    {
        // dump mvdata
        DECODE_CHK_STATUS(debugInterface->DumpBuffer(
            params.presColMvTempBuffer[0],
            CodechalDbgAttr::attrMvData,
            "DEC_Col_MV_",
            mvBufferSize));
    };

    if (params.presCurMvTempBuffer)
    {
        // dump mvdata
        DECODE_CHK_STATUS(debugInterface->DumpBuffer(
            params.presCurMvTempBuffer,
            CodechalDbgAttr::attrMvData,
            "DEC_Cur_MV_",
            mvBufferSize));
    };

   return MOS_STATUS_SUCCESS;
}
#endif

} // namespace decode

