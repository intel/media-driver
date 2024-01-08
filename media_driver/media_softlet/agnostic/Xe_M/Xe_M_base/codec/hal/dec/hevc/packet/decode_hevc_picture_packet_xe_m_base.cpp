/*
* Copyright (c) 2019-2022, Intel Corporation
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
//! \file     decode_hevc_picture_packet_xe_m_base.cpp
//! \brief    Defines the interface for hevc decode picture packet
//!
#include "codechal_utilities.h"
#include "decode_hevc_picture_packet_xe_m_base.h"
#include "decode_hevc_phase_real_tile.h"
#include "decode_hevc_phase_front_end.h"
#include "decode_hevc_phase_back_end.h"
#include "decode_common_feature_defs.h"
#include "decode_hevc_mem_compression.h"
#include "decode_resource_auto_lock.h"

namespace decode
{

HevcDecodePicPktXe_M_Base::~HevcDecodePicPktXe_M_Base()
{
    FreeResources();
}

MOS_STATUS HevcDecodePicPktXe_M_Base::FreeResources()
{
    DECODE_FUNC_CALL();

    if (m_allocator != nullptr)
    {
        m_allocator->Destroy(m_resMfdDeblockingFilterRowStoreScratchBuffer);
        m_allocator->Destroy(m_resMfdDeblockingFilterRowStoreScratchBuffer);
        m_allocator->Destroy(m_resDeblockingFilterTileRowStoreScratchBuffer);
        m_allocator->Destroy(m_resDeblockingFilterColumnRowStoreScratchBuffer);
        m_allocator->Destroy(m_resMetadataLineBuffer);
        m_allocator->Destroy(m_resMetadataTileLineBuffer);
        m_allocator->Destroy(m_resMetadataTileColumnBuffer);
        m_allocator->Destroy(m_resSaoLineBuffer);
        m_allocator->Destroy(m_resSaoTileLineBuffer);
        m_allocator->Destroy(m_resSaoTileColumnBuffer);
        m_allocator->Destroy(m_resSliceStateStreamOutBuffer);
        m_allocator->Destroy(m_resMvUpRightColStoreBuffer);
        m_allocator->Destroy(m_resIntraPredUpRightColStoreBuffer);
        m_allocator->Destroy(m_resIntraPredLeftReconColStoreBuffer);
        m_allocator->Destroy(m_resCABACSyntaxStreamOutBuffer);
        m_allocator->Destroy(m_resCABACStreamOutSizeBuffer);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodePicPktXe_M_Base::Init()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(m_featureManager);
    DECODE_CHK_NULL(m_hwInterface);
    DECODE_CHK_NULL(m_osInterface);
    DECODE_CHK_NULL(m_miInterface);
    DECODE_CHK_NULL(m_hevcPipeline);
    DECODE_CHK_NULL(m_hcpInterface);

    m_hevcBasicFeature = dynamic_cast<HevcBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_hevcBasicFeature);

#ifdef _DECODE_PROCESSING_SUPPORTED
    m_downSamplingFeature = dynamic_cast<DecodeDownSamplingFeature*>(m_featureManager->GetFeature(DecodeFeatureIDs::decodeDownSampling));
    DecodeSubPacket* subPacket = m_hevcPipeline->GetSubPacket(DecodePacketId(m_hevcPipeline, downSamplingSubPacketId));
    m_downSamplingPkt = dynamic_cast<DecodeDownSamplingPkt *>(subPacket);
#endif

    m_allocator = m_pipeline ->GetDecodeAllocator();
    DECODE_CHK_NULL(m_allocator);

    DECODE_CHK_STATUS(AllocateFixedResources());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodePicPktXe_M_Base::Prepare()
{
    DECODE_FUNC_CALL();

    m_hevcPicParams      = m_hevcBasicFeature->m_hevcPicParams;
    m_hevcIqMatrixParams = m_hevcBasicFeature->m_hevcIqMatrixParams;
    m_hevcRextPicParams  = m_hevcBasicFeature->m_hevcRextPicParams;
    m_hevcSccPicParams   = m_hevcBasicFeature->m_hevcSccPicParams;

#ifdef _MMC_SUPPORTED
    m_mmcState = m_hevcPipeline->GetMmcState();
    DECODE_CHK_NULL(m_mmcState);
#endif

    DECODE_CHK_STATUS(SetRowstoreCachingOffsets());

    DECODE_CHK_STATUS(AllocateVariableResources());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodePicPktXe_M_Base::SetPhase(DecodePhase *phase)
{
    DECODE_FUNC_CALL();
    m_phase = phase;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodePicPktXe_M_Base::ReportCabacStreamOutSize(MOS_COMMAND_BUFFER &cmdBuffer)
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

bool HevcDecodePicPktXe_M_Base::IsRealTilePhase()
{
    if (m_phase == nullptr)
    {
        return false;
    }
    HevcPhaseRealTile *realtilePhase = dynamic_cast<HevcPhaseRealTile *>(m_phase);
    return (realtilePhase != nullptr);
}

bool HevcDecodePicPktXe_M_Base::IsFrontEndPhase()
{
    if (m_phase == nullptr)
    {
        return false;
    }
    HevcPhaseFrontEnd *frontEndPhase = dynamic_cast<HevcPhaseFrontEnd *>(m_phase);
    return (frontEndPhase != nullptr);
}

bool HevcDecodePicPktXe_M_Base::IsBackEndPhase()
{
    if (m_phase == nullptr)
    {
        return false;
    }
    HevcPhaseBackEnd *backEndPhase = dynamic_cast<HevcPhaseBackEnd *>(m_phase);
    return (backEndPhase != nullptr);
}

MOS_STATUS HevcDecodePicPktXe_M_Base::SetRowstoreCachingOffsets()
{
    if (m_hcpInterface->IsRowStoreCachingSupported())
    {
        MHW_VDBOX_ROWSTORE_PARAMS rowstoreParams;
        rowstoreParams.Mode             = CODECHAL_DECODE_MODE_HEVCVLD;
        rowstoreParams.dwPicWidth       = m_hevcBasicFeature->m_width;
        rowstoreParams.bMbaff           = false;
        rowstoreParams.ucBitDepthMinus8 = (uint8_t)MOS_MAX(m_hevcPicParams->bit_depth_luma_minus8,
                                                           m_hevcPicParams->bit_depth_chroma_minus8);
        rowstoreParams.ucChromaFormat   = m_hevcPicParams->chroma_format_idc;
        rowstoreParams.ucLCUSize        = (uint8_t)m_hevcBasicFeature->m_ctbSize;
        DECODE_CHK_STATUS(m_hwInterface->SetRowstoreCachingOffsets(&rowstoreParams));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodePicPktXe_M_Base::AllocateFixedResources()
{
    DECODE_FUNC_CALL();

    if (m_resSliceStateStreamOutBuffer == nullptr)
    {
        uint32_t sizeOfBuffer = CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6 * sliceStateCachelinesPerSlice * CODECHAL_CACHELINE_SIZE;
        m_resSliceStateStreamOutBuffer = m_allocator->AllocateBuffer(
            sizeOfBuffer,
            "SliceStateStreamOut",
            resourceInternalReadWriteCache,
            notLockableVideoMem);
        DECODE_CHK_NULL(m_resSliceStateStreamOutBuffer);
    }

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

MOS_STATUS HevcDecodePicPktXe_M_Base::AllocateVariableResources()
{
    DECODE_FUNC_CALL();

    MHW_VDBOX_HCP_BUFFER_SIZE_PARAMS hcpBufSizeParam;
    MOS_ZeroMemory(&hcpBufSizeParam, sizeof(hcpBufSizeParam));
    hcpBufSizeParam.ucMaxBitDepth  = m_hevcBasicFeature->m_bitDepth;
    hcpBufSizeParam.ucChromaFormat = m_hevcBasicFeature->m_chromaFormat;
    hcpBufSizeParam.dwCtbLog2SizeY = m_hevcPicParams->log2_diff_max_min_luma_coding_block_size +
                                     m_hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3;
    hcpBufSizeParam.dwPicWidth     = m_hevcBasicFeature->m_width;
    hcpBufSizeParam.dwPicHeight    = m_hevcBasicFeature->m_height;
    hcpBufSizeParam.dwMaxFrameSize = m_hevcBasicFeature->m_dataSize;

    auto AllocateBuffer = [&] (PMOS_BUFFER &buffer, MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE bufferType, const char *bufferName)
    {
        DECODE_CHK_STATUS(m_hcpInterface->GetHevcBufferSize(bufferType, &hcpBufSizeParam));
        if (buffer == nullptr)
        {
            buffer = m_allocator->AllocateBuffer(
                hcpBufSizeParam.dwBufferSize, bufferName, resourceInternalReadWriteCache, notLockableVideoMem);
            DECODE_CHK_NULL(buffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                buffer, hcpBufSizeParam.dwBufferSize, notLockableVideoMem));
        }
        return MOS_STATUS_SUCCESS;
    };

    if (!m_hcpInterface->IsHevcDfRowstoreCacheEnabled())
    {
        // Deblocking Filter Row Store Scratch buffer
        DECODE_CHK_STATUS(AllocateBuffer(
            m_resMfdDeblockingFilterRowStoreScratchBuffer,
            MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_LINE,
            "DeblockingScratchBuffer"));
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

    if (!m_hcpInterface->IsHevcDatRowstoreCacheEnabled())
    {
        // Metadata Line buffer
        DECODE_CHK_STATUS(AllocateBuffer(
            m_resMetadataLineBuffer,
            MHW_VDBOX_HCP_INTERNAL_BUFFER_META_LINE,
            "MetadataLineBuffer"));
    }

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

    if (!m_hcpInterface->IsHevcSaoRowstoreCacheEnabled())
    {
        // SAO Line buffer
        DECODE_CHK_STATUS(AllocateBuffer(
            m_resSaoLineBuffer,
            MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_LINE,
            "SaoLineBuffer"));
    }

    // SAO Tile Line buffer
    DECODE_CHK_STATUS(AllocateBuffer(
        m_resSaoTileLineBuffer,
        MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_TILE_LINE,
        "SaoTileLineBuffer"));

    // SAO Tile Column buffer
    DECODE_CHK_STATUS(AllocateBuffer(
        m_resSaoTileColumnBuffer,
        MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_TILE_COL,
        "SaoTileColumnBuffer"));

    // MV up right column store buffer
    DECODE_CHK_STATUS(AllocateBuffer(
        m_resMvUpRightColStoreBuffer,
        MHW_VDBOX_HCP_INTERNAL_BUFFER_MV_UP_RT_COL,
        "MVUpperRightColumnStore"));

    // Intra prediction up right column store buffer
    DECODE_CHK_STATUS(AllocateBuffer(
        m_resIntraPredUpRightColStoreBuffer,
        MHW_VDBOX_HCP_INTERNAL_BUFFER_INTRA_PRED_UP_RIGHT_COL,
        "MVUpperRightColumnStore"));

    // Intra prediction left recon column store buffer
    DECODE_CHK_STATUS(AllocateBuffer(
        m_resIntraPredLeftReconColStoreBuffer,
        MHW_VDBOX_HCP_INTERNAL_BUFFER_INTRA_PRED_LFT_RECON_COL,
        "IntraPredLeftReconColumnStore"));

    // Cabac stream out buffer
    DECODE_CHK_STATUS(AllocateBuffer(
        m_resCABACSyntaxStreamOutBuffer,
        MHW_VDBOX_HCP_INTERNAL_BUFFER_CABAC_STREAMOUT,
        "CABACStreamOutBuffer"));

    return MOS_STATUS_SUCCESS;
}

void HevcDecodePicPktXe_M_Base::SetHcpPipeModeSelectParams(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS& pipeModeSelectParams)
{
    DECODE_FUNC_CALL();

    pipeModeSelectParams.Mode = m_hevcBasicFeature->m_mode;
    pipeModeSelectParams.bStreamOutEnabled = false;
    pipeModeSelectParams.bShortFormatInUse = m_hevcPipeline->IsShortFormat();
}

MOS_STATUS HevcDecodePicPktXe_M_Base::SetHcpDstSurfaceParams(MHW_VDBOX_SURFACE_PARAMS& dstSurfaceParams)
{
    DECODE_FUNC_CALL();

    dstSurfaceParams.Mode                       = CODECHAL_DECODE_MODE_HEVCVLD;
    dstSurfaceParams.psSurface                  = &m_hevcBasicFeature->m_destSurface;
    dstSurfaceParams.ucSurfaceStateId           = CODECHAL_HCP_DECODED_SURFACE_ID;
    dstSurfaceParams.ChromaType                 = m_hevcPicParams->chroma_format_idc;
    dstSurfaceParams.ucBitDepthLumaMinus8       = m_hevcPicParams->bit_depth_luma_minus8;
    dstSurfaceParams.ucBitDepthChromaMinus8     = m_hevcPicParams->bit_depth_chroma_minus8;
    dstSurfaceParams.dwUVPlaneAlignment         = 1 << (m_hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3);

#ifdef _MMC_SUPPORTED
    DECODE_CHK_STATUS(m_mmcState->SetSurfaceMmcState(dstSurfaceParams.psSurface));
    DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcState(dstSurfaceParams.psSurface, &dstSurfaceParams.mmcState));
    DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcFormat(dstSurfaceParams.psSurface, &dstSurfaceParams.dwCompressionFormat));
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodePicPktXe_M_Base::SetHcpRefSurfaceParams(
    const MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParams, MHW_VDBOX_SURFACE_PARAMS& refSurfaceParams)
{
    DECODE_FUNC_CALL();

    refSurfaceParams.Mode                       = CODECHAL_DECODE_MODE_HEVCVLD;
    refSurfaceParams.psSurface                  = &m_hevcBasicFeature->m_destSurface; // For HEVC decode, reference should be same format as dest surface
    refSurfaceParams.ucSurfaceStateId           = CODECHAL_HCP_REF_SURFACE_ID;
    refSurfaceParams.ChromaType                 = m_hevcPicParams->chroma_format_idc;
    refSurfaceParams.ucBitDepthLumaMinus8       = m_hevcPicParams->bit_depth_luma_minus8;
    refSurfaceParams.ucBitDepthChromaMinus8     = m_hevcPicParams->bit_depth_chroma_minus8;
    refSurfaceParams.dwUVPlaneAlignment         = 1 << (m_hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3);

#ifdef _MMC_SUPPORTED
    HevcDecodeMemComp *hevcDecodeMemComp = dynamic_cast<HevcDecodeMemComp *>(m_mmcState);
    DECODE_CHK_NULL(hevcDecodeMemComp);
    // Set refSurfaceParams mmcState as MOS_MEMCOMP_MC to satisfy MmcEnable in AddHcpSurfaceCmd
    // The actual mmcstate is recorded by refSurfaceParams.mmcSkipMask
    if (m_mmcState->IsMmcEnabled())
    {
        refSurfaceParams.mmcState = MOS_MEMCOMP_MC;
        DECODE_CHK_STATUS(hevcDecodeMemComp->SetRefSurfaceMask(*m_hevcBasicFeature, pipeBufAddrParams.presReferences, refSurfaceParams.mmcSkipMask));
        DECODE_CHK_STATUS(hevcDecodeMemComp->SetRefSurfaceCompressionFormat(*m_hevcBasicFeature, pipeBufAddrParams.presReferences, refSurfaceParams.dwCompressionFormat));
    }
    else
    {
        refSurfaceParams.mmcState            = MOS_MEMCOMP_DISABLED;
        refSurfaceParams.dwCompressionFormat = 0;
    }
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodePicPktXe_M_Base::AddHcpSurfaces(MOS_COMMAND_BUFFER &cmdBuffer, const MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParams)
{
    DECODE_FUNC_CALL();

    MHW_VDBOX_SURFACE_PARAMS dstSurfaceParams;
    MOS_ZeroMemory(&dstSurfaceParams, sizeof(dstSurfaceParams));
    DECODE_CHK_STATUS(SetHcpDstSurfaceParams(dstSurfaceParams));
    DECODE_CHK_STATUS(m_hcpInterface->AddHcpSurfaceCmd(&cmdBuffer, &dstSurfaceParams));

    MHW_VDBOX_SURFACE_PARAMS refSurfaceParams;
    MOS_ZeroMemory(&refSurfaceParams, sizeof(refSurfaceParams));
    SetHcpRefSurfaceParams(pipeBufAddrParams, refSurfaceParams);
    DECODE_CHK_STATUS(m_hcpInterface->AddHcpSurfaceCmd(&cmdBuffer, &refSurfaceParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodePicPktXe_M_Base::FixHcpPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS& pipeBufAddrParams)
{
    if (m_hevcBasicFeature->m_refFrames.m_curIsIntra)
    {
        PMOS_RESOURCE dummyRef = &(m_hevcBasicFeature->m_dummyReference.OsResource);
        if (m_hevcBasicFeature->m_useDummyReference &&
            !m_allocator->ResourceIsNull(dummyRef))
        {
            // set all ref pic addresses to valid addresses for error concealment purpose
            for (uint32_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
            {
                if (pipeBufAddrParams.presReferences[i] == nullptr)
                {
                    pipeBufAddrParams.presReferences[i] = dummyRef;
                    m_hevcBasicFeature->m_dummyReferenceSlot[i] = true;
                }
            }
        }
    }
    else
    {
        PMOS_RESOURCE validRef = m_hevcBasicFeature->m_refFrames.GetValidReference();
        for (uint8_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
        {
            // error concealment for the unset reference addresses and unset mv buffers
            if (pipeBufAddrParams.presReferences[i] == nullptr)
            {
                pipeBufAddrParams.presReferences[i] = validRef;
            }
        }

        PMOS_BUFFER validMvBuf = m_hevcBasicFeature->m_mvBuffers.GetValidBufferForReference(
                                    m_hevcBasicFeature->m_refFrameIndexList);
        for (uint32_t i = 0; i < CODEC_NUM_HEVC_MV_BUFFERS; i++)
        {
            if (pipeBufAddrParams.presColMvTempBuffer[i] == nullptr)
            {
                pipeBufAddrParams.presColMvTempBuffer[i] = &validMvBuf->OsResource;
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodePicPktXe_M_Base::SetHcpPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS& pipeBufAddrParams)
{
    DECODE_FUNC_CALL();

    pipeBufAddrParams.Mode                  = m_hevcBasicFeature->m_mode;

    PMOS_SURFACE destSurface                = &(m_hevcBasicFeature->m_destSurface);
    pipeBufAddrParams.psPreDeblockSurface   = destSurface;

#ifdef _MMC_SUPPORTED
    DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcState(destSurface, &pipeBufAddrParams.PreDeblockSurfMmcState));
#endif

    pipeBufAddrParams.presMfdDeblockingFilterRowStoreScratchBuffer =
        &(m_resMfdDeblockingFilterRowStoreScratchBuffer->OsResource);
    pipeBufAddrParams.presDeblockingFilterTileRowStoreScratchBuffer =
        &(m_resDeblockingFilterTileRowStoreScratchBuffer->OsResource);
    pipeBufAddrParams.presDeblockingFilterColumnRowStoreScratchBuffer =
        &(m_resDeblockingFilterColumnRowStoreScratchBuffer->OsResource);

    pipeBufAddrParams.presMetadataLineBuffer       = &(m_resMetadataLineBuffer->OsResource);
    pipeBufAddrParams.presMetadataTileLineBuffer   = &(m_resMetadataTileLineBuffer->OsResource);
    pipeBufAddrParams.presMetadataTileColumnBuffer = &(m_resMetadataTileColumnBuffer->OsResource);
    pipeBufAddrParams.presSaoLineBuffer            = &(m_resSaoLineBuffer->OsResource);
    pipeBufAddrParams.presSaoTileLineBuffer        = &(m_resSaoTileLineBuffer->OsResource);
    pipeBufAddrParams.presSaoTileColumnBuffer      = &(m_resSaoTileColumnBuffer->OsResource);

    auto mvBuffers = &(m_hevcBasicFeature->m_mvBuffers);
    PMOS_BUFFER curMvBuffer = mvBuffers->GetCurBuffer();
    DECODE_CHK_NULL(curMvBuffer);
    pipeBufAddrParams.presCurMvTempBuffer = &(curMvBuffer->OsResource);

    HevcReferenceFrames &refFrames = m_hevcBasicFeature->m_refFrames;
    const std::vector<uint8_t> & activeRefList = refFrames.GetActiveReferenceList(*m_hevcPicParams);
    if (!refFrames.m_curIsIntra)
    {
        DECODE_ASSERT(activeRefList.size() <= 8);
        for (uint8_t i = 0; i < activeRefList.size(); i++)
        {
            uint8_t frameIdx = activeRefList[i];
            if (frameIdx >= CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC)
            {
                continue;
            }

            pipeBufAddrParams.presReferences[i] = refFrames.GetReferenceByFrameIndex(frameIdx);
            if (pipeBufAddrParams.presReferences[i] == nullptr)
            {
                PCODEC_REF_LIST curFrameInRefList = refFrames.m_refList[m_hevcPicParams->CurrPic.FrameIdx];
                DECODE_CHK_NULL(curFrameInRefList);
                MOS_ZeroMemory(&curFrameInRefList->resRefPic, sizeof(MOS_RESOURCE));
                DECODE_ASSERTMESSAGE("Reference frame for current Frame is not exist, current frame will be skipped. Thus, clear current frame resource in reference list.");
                return MOS_STATUS_INVALID_PARAMETER;
            }

            PMOS_BUFFER mvBuf = mvBuffers->GetBufferByFrameIndex(frameIdx);
            pipeBufAddrParams.presColMvTempBuffer[i] = mvBuf ? (&mvBuf->OsResource) : nullptr;

            // Return error if reference surface's pitch * height is less than dest surface.
            MOS_SURFACE refSurface;
            refSurface.OsResource = *(pipeBufAddrParams.presReferences[i]);
            DECODE_CHK_STATUS(m_allocator->GetSurfaceInfo(&refSurface));
            DECODE_CHK_COND((refSurface.dwPitch * refSurface.dwHeight) < (destSurface->dwPitch * destSurface->dwHeight),
                            "Reference surface's pitch * height is less than Dest surface.");
        }
    }

    DECODE_CHK_STATUS(FixHcpPipeBufAddrParams(pipeBufAddrParams));

    if (m_hevcBasicFeature->m_isSCCIBCMode)
    {
        uint8_t IBCRefIdx = refFrames.m_IBCRefIdx;
        DECODE_CHK_COND(activeRefList.size() <= IBCRefIdx, "Invalid IBC reference index.");

        uint8_t refIdxMask = 0;
        for (uint8_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
        {
            uint8_t IBCFrameIdx = activeRefList[IBCRefIdx];
            if (pipeBufAddrParams.presReferences[i] == refFrames.GetReferenceByFrameIndex(IBCFrameIdx))
            {
                refIdxMask |= (1 << i);
            }
        }
        pipeBufAddrParams.IBCRefIdxMask = refIdxMask;
    }

    CODECHAL_DEBUG_TOOL(DECODE_CHK_STATUS(DumpResources(pipeBufAddrParams, activeRefList.size(), curMvBuffer->size)));

    return MOS_STATUS_SUCCESS;
}

void HevcDecodePicPktXe_M_Base::SetHcpIndObjBaseAddrParams(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS& indObjBaseAddrParams)
{
    DECODE_FUNC_CALL();

    indObjBaseAddrParams.Mode            = m_hevcBasicFeature->m_mode;
    indObjBaseAddrParams.dwDataSize      = m_hevcBasicFeature->m_dataSize;
    indObjBaseAddrParams.dwDataOffset    = m_hevcBasicFeature->m_dataOffset;
    indObjBaseAddrParams.presDataBuffer  = &(m_hevcBasicFeature->m_resDataBuffer.OsResource);
}

MOS_STATUS HevcDecodePicPktXe_M_Base::AddHcpIndObjBaseAddrCmd(MOS_COMMAND_BUFFER  &cmdBuffer)
{
    DECODE_FUNC_CALL();

    MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS indObjBaseAddrParams;
    MOS_ZeroMemory(&indObjBaseAddrParams, sizeof(indObjBaseAddrParams));
    SetHcpIndObjBaseAddrParams(indObjBaseAddrParams);

    DECODE_CHK_STATUS(m_hcpInterface->AddHcpIndObjBaseAddrCmd(&cmdBuffer, &indObjBaseAddrParams));
    return MOS_STATUS_SUCCESS;
}

void HevcDecodePicPktXe_M_Base::SetHcpQmStateParams(MHW_VDBOX_QM_PARAMS& qmParams)
{
    DECODE_FUNC_CALL();

    qmParams.Standard = CODECHAL_HEVC;
    qmParams.pHevcIqMatrix = (PMHW_VDBOX_HEVC_QM_PARAMS)m_hevcIqMatrixParams;
}

MOS_STATUS HevcDecodePicPktXe_M_Base::AddHcpQmStateCmd(MOS_COMMAND_BUFFER  &cmdBuffer)
{
    DECODE_FUNC_CALL();

    MHW_VDBOX_QM_PARAMS qmParams;
    MOS_ZeroMemory(&qmParams, sizeof(qmParams));
    SetHcpQmStateParams(qmParams);

    DECODE_CHK_STATUS(m_hcpInterface->AddHcpQmStateCmd(&cmdBuffer, &qmParams));
    return MOS_STATUS_SUCCESS;
}

void HevcDecodePicPktXe_M_Base::SetHcpPicStateParams(MHW_VDBOX_HEVC_PIC_STATE& picStateParams)
{
    DECODE_FUNC_CALL();
    picStateParams.pHevcPicParams = m_hevcPicParams;
}

void HevcDecodePicPktXe_M_Base::SetHcpTileStateParams(MHW_VDBOX_HEVC_TILE_STATE& tileStateParams)
{
    DECODE_FUNC_CALL();
    tileStateParams.pHevcPicParams = m_hevcPicParams;
    tileStateParams.pTileColWidth  = (uint16_t *)m_hevcBasicFeature->m_tileCoding.GetTileColWidth();
    tileStateParams.pTileRowHeight = (uint16_t *)m_hevcBasicFeature->m_tileCoding.GetTileRowHeight();
}

MOS_STATUS HevcDecodePicPktXe_M_Base::AddHcpTileStateCmd(MOS_COMMAND_BUFFER  &cmdBuffer)
{
    DECODE_FUNC_CALL();

    MHW_VDBOX_HEVC_TILE_STATE tileStateParams;
    MOS_ZeroMemory(&tileStateParams, sizeof(tileStateParams));
    SetHcpTileStateParams(tileStateParams);

    DECODE_CHK_STATUS(m_hcpInterface->AddHcpTileStateCmd(&cmdBuffer, &tileStateParams));
    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS HevcDecodePicPktXe_M_Base::DumpResources(
    MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParams, 
    uint8_t activeRefListSize,
    uint32_t mvBufferSize)
{
    DECODE_FUNC_CALL();

    CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
    DECODE_CHK_NULL(debugInterface);

    for (uint32_t i = 0; i < activeRefListSize; i++)
    {
        if (pipeBufAddrParams.presReferences[i] != nullptr)
        {
            MOS_SURFACE refSurface;
            MOS_ZeroMemory(&refSurface, sizeof(MOS_SURFACE));
            refSurface.OsResource = *(pipeBufAddrParams.presReferences[i]);
            DECODE_CHK_STATUS(CodecHalGetResourceInfo(m_osInterface, &refSurface));

            std::string refSurfDumpName = "RefSurf[" + std::to_string(i) + "]";
            DECODE_CHK_STATUS(debugInterface->DumpYUVSurface(
                &refSurface,
                CodechalDbgAttr::attrDecodeReferenceSurfaces,
                refSurfDumpName.c_str()));
        }

        if (pipeBufAddrParams.presColMvTempBuffer[i] != nullptr)
        {
            std::string mvBufDumpName = "_DEC_" + std::to_string(i);
            DECODE_CHK_STATUS(debugInterface->DumpBuffer(
                pipeBufAddrParams.presColMvTempBuffer[i],
                CodechalDbgAttr::attrMvData,
                mvBufDumpName.c_str(),
                mvBufferSize));
        }
    }

    return MOS_STATUS_SUCCESS;
}
#endif

}
