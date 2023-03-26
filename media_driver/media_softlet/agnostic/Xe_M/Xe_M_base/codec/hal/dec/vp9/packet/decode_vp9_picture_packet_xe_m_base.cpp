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
//! \file     decode_vp9_picture_packet_xe_m_base.cpp
//! \brief    Defines the interface for vp9 decode picture packet
//!
#include "codechal_utilities.h"
#include "decode_vp9_picture_packet_xe_m_base.h"
#include "codechal_debug.h"
#include "decode_vp9_mem_compression.h"
#include "decode_common_feature_defs.h"
#include "decode_vp9_phase_front_end.h"
#include "decode_vp9_phase_back_end.h"
#include "decode_resource_auto_lock.h"

namespace decode
{
Vp9DecodePicPktXe_M_Base::~Vp9DecodePicPktXe_M_Base()
{
    FreeResources();
}

//????need check which buffer need free for VP9
MOS_STATUS Vp9DecodePicPktXe_M_Base::FreeResources()
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

MOS_STATUS Vp9DecodePicPktXe_M_Base::Init()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(m_featureManager);
    DECODE_CHK_NULL(m_hwInterface);
    DECODE_CHK_NULL(m_osInterface);
    DECODE_CHK_NULL(m_miInterface);
    DECODE_CHK_NULL(m_vp9Pipeline);
    DECODE_CHK_NULL(m_hcpInterface);

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

MOS_STATUS Vp9DecodePicPktXe_M_Base::Prepare()
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

MOS_STATUS Vp9DecodePicPktXe_M_Base::SetPhase(DecodePhase *phase)
{
    DECODE_FUNC_CALL();
    m_phase = phase;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DecodePicPktXe_M_Base::ReportCabacStreamOutSize(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(m_resCABACStreamOutSizeBuffer);

    auto mmioRegistersHcp = m_hwInterface->GetHcpInterface()->GetMmioRegisters(MHW_VDBOX_NODE_1);

    MHW_MI_STORE_REGISTER_MEM_PARAMS params;
    MOS_ZeroMemory(&params, sizeof(MHW_MI_STORE_REGISTER_MEM_PARAMS));
    params.presStoreBuffer = &m_resCABACStreamOutSizeBuffer->OsResource;
    params.dwOffset        = 0;
    params.dwRegister      = mmioRegistersHcp->hcpDebugFEStreamOutSizeRegOffset;

    DECODE_CHK_STATUS(m_miInterface->AddMiStoreRegisterMemCmd(
        &cmdBuffer,
        &params));

    return MOS_STATUS_SUCCESS;
}

bool Vp9DecodePicPktXe_M_Base::IsFrontEndPhase()
{
    if (m_phase == nullptr)
    {
        return false;
    }
    Vp9PhaseFrontEnd *frontEndPhase = dynamic_cast<Vp9PhaseFrontEnd *>(m_phase);
    return (frontEndPhase != nullptr);
}

bool Vp9DecodePicPktXe_M_Base::IsBackEndPhase()
{
    if (m_phase == nullptr)
    {
        return false;
    }
    Vp9PhaseBackEnd *backEndPhase = dynamic_cast<Vp9PhaseBackEnd *>(m_phase);
    return (backEndPhase != nullptr);
}

MOS_STATUS Vp9DecodePicPktXe_M_Base::SetRowstoreCachingOffsets()
{
    if (m_hcpInterface->IsRowStoreCachingSupported() &&
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

MOS_STATUS Vp9DecodePicPktXe_M_Base::AllocateFixedResources()
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

MOS_STATUS Vp9DecodePicPktXe_M_Base::AllocateVariableResources()
{
    DECODE_FUNC_CALL();

    uint8_t maxBitDepth = 8 + m_vp9BasicFeature->m_vp9DepthIndicator * 2;
    m_widthInSb         = MOS_ROUNDUP_DIVIDE(m_vp9BasicFeature->m_width, CODEC_VP9_SUPER_BLOCK_WIDTH);
    m_heightInSb        = MOS_ROUNDUP_DIVIDE(m_vp9BasicFeature->m_height, CODEC_VP9_SUPER_BLOCK_HEIGHT);

    MHW_VDBOX_HCP_BUFFER_SIZE_PARAMS hcpBufSizeParam;
    MOS_ZeroMemory(&hcpBufSizeParam, sizeof(hcpBufSizeParam));
    hcpBufSizeParam.ucMaxBitDepth  = maxBitDepth;
    hcpBufSizeParam.ucChromaFormat = m_vp9BasicFeature->m_chromaFormat;
    hcpBufSizeParam.dwPicWidth     = m_widthInSb;
    hcpBufSizeParam.dwPicHeight    = m_heightInSb;
    hcpBufSizeParam.dwMaxFrameSize = m_vp9BasicFeature->m_dataSize;

     auto AllocateBuffer = [&](PMOS_BUFFER &buffer, MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE bufferType, const char *bufferName)
     {
        DECODE_CHK_STATUS(m_hcpInterface->GetVp9BufferSize(bufferType, &hcpBufSizeParam));
        if (buffer == nullptr)
        {
            buffer = m_allocator->AllocateBuffer(
                hcpBufSizeParam.dwBufferSize, bufferName, resourceInternalReadWriteCache, notLockableVideoMem);
            DECODE_CHK_NULL(buffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(buffer, hcpBufSizeParam.dwBufferSize, notLockableVideoMem));
        }
        return MOS_STATUS_SUCCESS;
    };


    if (!m_hcpInterface->IsVp9DfRowstoreCacheEnabled())
    {
        // Deblocking Filter Line Row Store Scratch data surface
        DECODE_CHK_STATUS(AllocateBuffer(
            m_resDeblockingFilterLineRowStoreScratchBuffer,
            MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_LINE,
            "BitstreamDecodeLineBuffer"));       
    }

    // Deblocking Filter Tile Row Store Scratch data surface
    DECODE_CHK_STATUS(AllocateBuffer(
        m_resDeblockingFilterTileRowStoreScratchBuffer,
        MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_LINE,
        "DeblockingTileScratchBuffer"));   

    // Deblocking Filter Column Row Store Scratch data surface
    DECODE_CHK_STATUS(AllocateBuffer(
        m_resDeblockingFilterColumnRowStoreScratchBuffer,
        MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_COL,
        "DeblockingColumnScratchBuffer"));   

    // Metadata Line buffer
    DECODE_CHK_STATUS(AllocateBuffer(
        m_resMetadataLineBuffer,
        MHW_VDBOX_HCP_INTERNAL_BUFFER_META_LINE,
        "MetadataLineBuffer"));   

    // Metadata Tile Line buffer
    DECODE_CHK_STATUS(AllocateBuffer(
        m_resMetadataTileLineBuffer,
        MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_LINE,
        "MetadataTileLineBuffer"));  

    // Metadata Tile Column buffer
    DECODE_CHK_STATUS(AllocateBuffer(
        m_resMetadataTileColumnBuffer,
        MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_COL,
        "MetadataTileColumnBuffer"));

    // HVC Line Row Store Buffer
    DECODE_CHK_STATUS(AllocateBuffer(
        m_resHvcLineRowstoreBuffer,
        MHW_VDBOX_VP9_INTERNAL_BUFFER_HVD_LINE,
        "HvcLineRowStoreBuffer"));

    // HVC Tile Row Store Buffer
    DECODE_CHK_STATUS(AllocateBuffer(
        m_resHvcTileRowstoreBuffer,
        MHW_VDBOX_VP9_INTERNAL_BUFFER_HVD_TILE,
        "HvcTileRowStoreBuffer"));

   // Cabac stream out buffer
    DECODE_CHK_STATUS(AllocateBuffer(
        m_resCABACSyntaxStreamOutBuffer,
        MHW_VDBOX_HCP_INTERNAL_BUFFER_CABAC_STREAMOUT,
        "CABACStreamOutBuffer"));

   // Intra prediction up right column store buffer
    DECODE_CHK_STATUS(AllocateBuffer(
        m_resIntraPredUpRightColStoreBuffer,
        MHW_VDBOX_HCP_INTERNAL_BUFFER_INTRA_PRED_UP_RIGHT_COL,
        "IntraPredUpperRightColumnStore"));

   // Intra prediction left recon column store buffer
    DECODE_CHK_STATUS(AllocateBuffer(
        m_resIntraPredLeftReconColStoreBuffer,
        MHW_VDBOX_HCP_INTERNAL_BUFFER_INTRA_PRED_LFT_RECON_COL,
        "IntraPredLeftReconColumnStore"));

    return MOS_STATUS_SUCCESS;
}

void Vp9DecodePicPktXe_M_Base::SetHcpPipeModeSelectParams(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS &pipeModeSelectParams)
{
    DECODE_FUNC_CALL();

    pipeModeSelectParams.Mode              = CODECHAL_DECODE_MODE_VP9VLD;
    pipeModeSelectParams.bStreamOutEnabled = false;
}

MOS_STATUS Vp9DecodePicPktXe_M_Base::SetHcpDstSurfaceParams(MHW_VDBOX_SURFACE_PARAMS &dstSurfaceParams)
{
    DECODE_FUNC_CALL();

    MOS_ZeroMemory(&dstSurfaceParams, sizeof(dstSurfaceParams));
    dstSurfaceParams.Mode                   = CODECHAL_DECODE_MODE_VP9VLD;
    dstSurfaceParams.psSurface              = &m_vp9BasicFeature->m_destSurface;
    dstSurfaceParams.ucSurfaceStateId       = CODECHAL_HCP_DECODED_SURFACE_ID;
    dstSurfaceParams.ChromaType             = (uint8_t)chromaSamplingFormat;
    dstSurfaceParams.ucBitDepthLumaMinus8   = m_vp9PicParams->BitDepthMinus8;
    dstSurfaceParams.ucBitDepthChromaMinus8 = m_vp9PicParams->BitDepthMinus8;
    dstSurfaceParams.dwUVPlaneAlignment     = 8;

#ifdef _MMC_SUPPORTED
    DECODE_CHK_STATUS(m_mmcState->SetSurfaceMmcState(&(m_vp9BasicFeature->m_destSurface)));
    DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcState(dstSurfaceParams.psSurface, &dstSurfaceParams.mmcState));
    DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcFormat(dstSurfaceParams.psSurface, &dstSurfaceParams.dwCompressionFormat));
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DecodePicPktXe_M_Base::SetHcpRefSurfaceParams(MHW_VDBOX_SURFACE_PARAMS refSurfaceParams[])
{
    DECODE_FUNC_CALL();

    for (uint8_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_VP9; i++)
    {
        refSurfaceParams[i].Mode               = CODECHAL_DECODE_MODE_VP9VLD;
        refSurfaceParams[i].ChromaType         = (uint8_t)chromaSamplingFormat;
        refSurfaceParams[i].dwUVPlaneAlignment = 8;

        switch (i)
        {
        case 0:
            refSurfaceParams[i].psSurface        = &(m_vp9BasicFeature->m_lastRefSurface);
            refSurfaceParams[i].ucSurfaceStateId = CODECHAL_HCP_LAST_SURFACE_ID;
            break;
        case 1:
            refSurfaceParams[i].psSurface        = &(m_vp9BasicFeature->m_goldenRefSurface);
            refSurfaceParams[i].ucSurfaceStateId = CODECHAL_HCP_GOLDEN_SURFACE_ID;
            break;
        case 2:
            refSurfaceParams[i].psSurface        = &(m_vp9BasicFeature->m_altRefSurface);
            refSurfaceParams[i].ucSurfaceStateId = CODECHAL_HCP_ALTREF_SURFACE_ID;
            break;
        }

#ifdef _MMC_SUPPORTED
    DECODE_CHK_STATUS(m_mmcState->SetSurfaceMmcState(refSurfaceParams[i].psSurface));
    DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcState(refSurfaceParams[i].psSurface, &refSurfaceParams[i].mmcState));
    DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcFormat(refSurfaceParams[i].psSurface, &refSurfaceParams[i].dwCompressionFormat));
#endif
    }
#ifdef _MMC_SUPPORTED
    if (m_mmcState->IsMmcEnabled())
    {
        Vp9DecodeMemComp *vp9DecodeMemComp = dynamic_cast<Vp9DecodeMemComp *>(m_mmcState);
        DECODE_CHK_STATUS(vp9DecodeMemComp->SetRefSurfaceMask(*m_vp9BasicFeature, refSurfaceParams));
        DECODE_CHK_STATUS(vp9DecodeMemComp->SetRefSurfaceCompressionFormat(*m_vp9BasicFeature, refSurfaceParams));
    }
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DecodePicPktXe_M_Base::AddHcpSurfacesCmd(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    //destsurface surface cmd set
    MHW_VDBOX_SURFACE_PARAMS dstSurfaceParams;
    DECODE_CHK_STATUS(SetHcpDstSurfaceParams(dstSurfaceParams));
    DECODE_CHK_STATUS(m_hcpInterface->AddHcpSurfaceCmd(&cmdBuffer, &dstSurfaceParams));
     
    // For non-key frame, send extra surface commands for reference pictures
    if (m_vp9PicParams->PicFlags.fields.frame_type == CODEC_VP9_INTER_FRAME &&
        !m_vp9PicParams->PicFlags.fields.intra_only)
    {
        MHW_VDBOX_SURFACE_PARAMS refSurfaceParams[3];

        MOS_ZeroMemory(refSurfaceParams, sizeof(refSurfaceParams));

        SetHcpRefSurfaceParams(refSurfaceParams);

        for (uint8_t i = 0; i < 3; i++)
        {
            DECODE_CHK_STATUS(m_hcpInterface->AddHcpSurfaceCmd(
                &cmdBuffer,
                &refSurfaceParams[i]));
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DecodePicPktXe_M_Base::AddHcpSegmentStateCmd(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    MHW_VDBOX_VP9_SEGMENT_STATE vp9SegmentState;
    MOS_ZeroMemory(&vp9SegmentState, sizeof(MHW_VDBOX_VP9_SEGMENT_STATE));
    DECODE_CHK_STATUS(SetHcpSegmentStateParams(vp9SegmentState));

    for (uint8_t i = 0; i < CODEC_VP9_MAX_SEGMENTS; i++)
    {
        // Error handling for illegal programming on segmentation fields @ KEY/INTRA_ONLY frames
        PCODEC_VP9_SEG_PARAMS vp9SegData = &(vp9SegmentState.pVp9SegmentParams->SegData[i]);

        if (vp9SegData->SegmentFlags.fields.SegmentReferenceEnabled &&
            (!m_vp9PicParams->PicFlags.fields.frame_type || m_vp9PicParams->PicFlags.fields.intra_only))
        {
            vp9SegData->SegmentFlags.fields.SegmentReference = CODECHAL_DECODE_VP9_INTRA_FRAME;
        }

        vp9SegmentState.ucCurrentSegmentId = i;
        DECODE_CHK_STATUS(m_hcpInterface->AddHcpVp9SegmentStateCmd(
            &cmdBuffer,
            nullptr,
            &vp9SegmentState));

        if (m_vp9PicParams->PicFlags.fields.segmentation_enabled == 0)
        {
            break;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DecodePicPktXe_M_Base::SetHcpSegmentStateParams(MHW_VDBOX_VP9_SEGMENT_STATE &vp9SegmentState)
{
    DECODE_FUNC_CALL();
    vp9SegmentState.Mode              = CODECHAL_DECODE_MODE_VP9VLD;
    vp9SegmentState.pVp9SegmentParams = m_vp9BasicFeature->m_vp9SegmentParams;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DecodePicPktXe_M_Base::SetHcpPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParams)
{
    DECODE_FUNC_CALL();

    pipeBufAddrParams.Mode                                       = (CODECHAL_MODE)CODECHAL_DECODE_MODE_VP9VLD;
    pipeBufAddrParams.psPreDeblockSurface                        = &(m_vp9BasicFeature->m_destSurface);
    pipeBufAddrParams.presReferences[CodechalDecodeLastRef]      = m_vp9BasicFeature->m_presLastRefSurface;
    pipeBufAddrParams.presReferences[CodechalDecodeGoldenRef]    = m_vp9BasicFeature->m_presGoldenRefSurface;
    pipeBufAddrParams.presReferences[CodechalDecodeAlternateRef] = m_vp9BasicFeature->m_presAltRefSurface;

    pipeBufAddrParams.presMfdDeblockingFilterRowStoreScratchBuffer    = &(m_resDeblockingFilterLineRowStoreScratchBuffer->OsResource);
    pipeBufAddrParams.presDeblockingFilterTileRowStoreScratchBuffer   = &(m_resDeblockingFilterTileRowStoreScratchBuffer->OsResource);
    pipeBufAddrParams.presDeblockingFilterColumnRowStoreScratchBuffer = &(m_resDeblockingFilterColumnRowStoreScratchBuffer->OsResource);

    pipeBufAddrParams.presMetadataLineBuffer       = &(m_resMetadataLineBuffer->OsResource);
    pipeBufAddrParams.presMetadataTileLineBuffer   = &(m_resMetadataTileLineBuffer->OsResource);
    pipeBufAddrParams.presMetadataTileColumnBuffer = &(m_resMetadataTileColumnBuffer->OsResource);

    pipeBufAddrParams.presHvdLineRowStoreBuffer = &(m_resHvcLineRowstoreBuffer->OsResource);
    pipeBufAddrParams.presHvdTileRowStoreBuffer = &(m_resHvcTileRowstoreBuffer->OsResource);

#ifdef _MMC_SUPPORTED
    DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcState(pipeBufAddrParams.psPreDeblockSurface, &pipeBufAddrParams.PreDeblockSurfMmcState));
#endif

     
    pipeBufAddrParams.presVp9SegmentIdBuffer = &(m_vp9BasicFeature->m_resVp9SegmentIdBuffer->OsResource);
    pipeBufAddrParams.presVp9ProbBuffer      = &(m_vp9BasicFeature->m_resVp9ProbBuffer[m_vp9BasicFeature->m_frameCtxIdx]->OsResource);

    Vp9ReferenceFrames &        refFrames     = m_vp9BasicFeature->m_refFrames;
    const std::vector<uint8_t> &activeRefList = refFrames.GetActiveReferenceList(*m_vp9PicParams);

    if (m_vp9PicParams->PicFlags.fields.frame_type == CODEC_VP9_INTER_FRAME &&
        !m_vp9PicParams->PicFlags.fields.intra_only)
    {
        pipeBufAddrParams.presCurMvTempBuffer = &(m_vp9BasicFeature->m_resVp9MvTemporalBuffer[m_vp9BasicFeature->m_curMvTempBufIdx]->OsResource);
        
        if (!m_vp9BasicFeature->m_prevFrameParams.fields.KeyFrame && !m_vp9PicParams->PicFlags.fields.intra_only)
        {
            pipeBufAddrParams.presColMvTempBuffer[0] = &(m_vp9BasicFeature->m_resVp9MvTemporalBuffer[m_vp9BasicFeature->m_colMvTempBufIdx]->OsResource);
        }
    }

    DECODE_CHK_STATUS(FixHcpPipeBufAddrParams(pipeBufAddrParams));    

    CODECHAL_DEBUG_TOOL(DumpResources(pipeBufAddrParams, activeRefList.size(), m_vp9BasicFeature->m_resVp9MvTemporalBuffer[0]->size));
   
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DecodePicPktXe_M_Base::FixHcpPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParams)
{
    DECODE_FUNC_CALL();

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
        if (!pipeBufAddrParams.presReferences[i])
        {
            pipeBufAddrParams.presReferences[i] = dummyRef;
        }
    }
    
    return MOS_STATUS_SUCCESS;
}

void Vp9DecodePicPktXe_M_Base::SetHcpIndObjBaseAddrParams(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS &indObjBaseAddrParams)
{
    DECODE_FUNC_CALL();

    MOS_ZeroMemory(&indObjBaseAddrParams, sizeof(indObjBaseAddrParams));
    indObjBaseAddrParams.Mode           = (CODECHAL_MODE)CODECHAL_DECODE_MODE_VP9VLD;
    indObjBaseAddrParams.dwDataSize     = m_vp9BasicFeature->m_dataSize;
    indObjBaseAddrParams.dwDataOffset   = m_vp9BasicFeature->m_dataOffset;
    indObjBaseAddrParams.presDataBuffer = &(m_vp9BasicFeature->m_resDataBuffer.OsResource);
}

MOS_STATUS Vp9DecodePicPktXe_M_Base::AddHcpIndObjBaseAddrCmd(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS indObjBaseAddrParams;
    SetHcpIndObjBaseAddrParams(indObjBaseAddrParams);
    DECODE_CHK_STATUS(m_hcpInterface->AddHcpIndObjBaseAddrCmd(&cmdBuffer, &indObjBaseAddrParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DecodePicPktXe_M_Base::SetHcpPicStateParams(MHW_VDBOX_VP9_PIC_STATE &picStateParams)
{
    DECODE_FUNC_CALL();

    MOS_ZeroMemory(&picStateParams, sizeof(picStateParams));

    picStateParams.pVp9PicParams         = m_vp9PicParams;
    picStateParams.ppVp9RefList          = &(m_vp9BasicFeature->m_refFrames.m_vp9RefList[0]);
    picStateParams.PrevFrameParams.value = m_vp9BasicFeature->m_prevFrameParams.value;
    picStateParams.dwPrevFrmWidth        = m_vp9BasicFeature->m_prevFrmWidth;
    picStateParams.dwPrevFrmHeight       = m_vp9BasicFeature->m_prevFrmHeight;

    m_vp9BasicFeature->m_prevFrmWidth  = m_vp9PicParams->FrameWidthMinus1 + 1;
    m_vp9BasicFeature->m_prevFrmHeight = m_vp9PicParams->FrameHeightMinus1 + 1;

    //update preframe field here, as pre frame keyframe was used for presColMvTempBuffer in SetHcpPipeBufAddrParams
    //make sure the value was not updated before SetHcpPipeBufAddrParams
    m_vp9BasicFeature->m_prevFrameParams.fields.KeyFrame  = !m_vp9PicParams->PicFlags.fields.frame_type;
    m_vp9BasicFeature->m_prevFrameParams.fields.IntraOnly = m_vp9PicParams->PicFlags.fields.intra_only;
    m_vp9BasicFeature->m_prevFrameParams.fields.Display   = m_vp9PicParams->PicFlags.fields.show_frame;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DecodePicPktXe_M_Base::SetHcpBsdObjectParams(MHW_VDBOX_HCP_BSD_PARAMS &bsdParams)
{
    
    bsdParams.dwBsdDataLength =
        m_vp9PicParams->BSBytesInBuffer - m_vp9PicParams->UncompressedHeaderLengthInBytes;
    bsdParams.dwBsdDataStartOffset = m_vp9PicParams->UncompressedHeaderLengthInBytes;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DecodePicPktXe_M_Base::AddHcpBsdObjectCmd(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    MHW_VDBOX_HCP_BSD_PARAMS bsdParams;
    MOS_ZeroMemory(&bsdParams, sizeof(bsdParams));
    DECODE_CHK_STATUS(SetHcpBsdObjectParams(bsdParams));
    DECODE_CHK_STATUS(m_hcpInterface->AddHcpBsdObjectCmd(&cmdBuffer, &bsdParams));

    return MOS_STATUS_SUCCESS;
}


MOS_STATUS Vp9DecodePicPktXe_M_Base::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    DECODE_FUNC_CALL();

    //m_pictureStatesSize will get with GetHcpxxx in packet_g12.cpp
    commandBufferSize      = m_pictureStatesSize;
    requestedPatchListSize = m_picturePatchListSize;

    return MOS_STATUS_SUCCESS;
}

//dump reference 
#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS Vp9DecodePicPktXe_M_Base::DumpResources(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &params, uint32_t refSize, uint32_t mvSize)
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
                DECODE_CHK_STATUS(CodecHalGetResourceInfo(
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
            mvSize));
    };

    if (params.presCurMvTempBuffer)
    {
        // dump mvdata
        DECODE_CHK_STATUS(debugInterface->DumpBuffer(
            params.presCurMvTempBuffer,
            CodechalDbgAttr::attrMvData,
            "DEC_Cur_MV_",
            mvSize));
    };

   return MOS_STATUS_SUCCESS;
}
#endif

}  // namespace decode

