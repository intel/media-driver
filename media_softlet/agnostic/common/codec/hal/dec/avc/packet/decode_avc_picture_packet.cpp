/*
* Copyright (c) 2020-2024, Intel Corporation
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
//! \file     decode_avc_picture_packet.cpp
//! \brief    Defines the interface for avc decode picture packet
//!
#include "decode_avc_picture_packet.h"
#include "codechal_debug.h"
#include "decode_common_feature_defs.h"
#include "decode_resource_auto_lock.h"

namespace decode
{
AvcDecodePicPkt::~AvcDecodePicPkt()
{
    FreeResources();
}

MOS_STATUS AvcDecodePicPkt::FreeResources()
{
    DECODE_FUNC_CALL();

    if (m_allocator != nullptr)
    {
        m_allocator->Destroy(m_resMfdDeblockingFilterRowStoreScratchBuffer);
        if (!m_mfxItf->IsBsdMpcRowstoreCacheEnabled())
        {
            m_allocator->Destroy(m_resBsdMpcRowStoreScratchBuffer);
        }
        if (!m_mfxItf->IsIntraRowstoreCacheEnabled())
        {
            m_allocator->Destroy(m_resMfdIntraRowStoreScratchBuffer);
        }
        if (!m_mfxItf->IsMprRowstoreCacheEnabled())
        {
            m_allocator->Destroy(m_resMprRowStoreScratchBuffer);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcDecodePicPkt::Init()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(m_featureManager);
    DECODE_CHK_NULL(m_hwInterface);
    DECODE_CHK_NULL(m_osInterface);
    DECODE_CHK_NULL(m_miItf);
    DECODE_CHK_NULL(m_avcPipeline);
    DECODE_CHK_NULL(m_mfxItf);

    m_avcBasicFeature = dynamic_cast<AvcBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_avcBasicFeature);

#ifdef _DECODE_PROCESSING_SUPPORTED
    m_downSamplingFeature      = dynamic_cast<DecodeDownSamplingFeature *>(m_featureManager->GetFeature(DecodeFeatureIDs::decodeDownSampling));
    DecodeSubPacket *subPacket = m_avcPipeline->GetSubPacket(DecodePacketId(m_avcPipeline, downSamplingSubPacketId));
    m_downSamplingPkt          = dynamic_cast<DecodeDownSamplingPkt *>(subPacket);
#endif
    m_allocator = m_pipeline->GetDecodeAllocator();
    DECODE_CHK_NULL(m_allocator);

    DECODE_CHK_STATUS(AllocateFixedResources());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcDecodePicPkt::Prepare()
{
    DECODE_FUNC_CALL();

    m_avcPicParams = m_avcBasicFeature->m_avcPicParams;

#ifdef _MMC_SUPPORTED
    m_mmcState = m_avcPipeline->GetMmcState();
    DECODE_CHK_NULL(m_mmcState);
#endif

    DECODE_CHK_STATUS(SetRowstoreCachingOffsets());

    DECODE_CHK_STATUS(AllocateVariableResources());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcDecodePicPkt::SetRowstoreCachingOffsets()
{
    if (m_mfxItf->IsRowStoreCachingSupported())
    {
        MHW_VDBOX_ROWSTORE_PARAMS rowstoreParams;
        MOS_ZeroMemory(&rowstoreParams, sizeof(rowstoreParams));
        rowstoreParams.dwPicWidth = m_avcBasicFeature->m_width;
        rowstoreParams.bMbaff     = m_avcPicParams->seq_fields.mb_adaptive_frame_field_flag;
        rowstoreParams.Mode       = CODECHAL_DECODE_MODE_AVCVLD;
        rowstoreParams.bIsFrame   = !m_avcPicParams->pic_fields.field_pic_flag;
        DECODE_CHK_STATUS(m_mfxItf->GetRowstoreCachingAddrs(&rowstoreParams));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcDecodePicPkt::AllocateFixedResources()
{
    DECODE_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcDecodePicPkt::AllocateVariableResources()
{
    DECODE_FUNC_CALL();

    uint16_t picWidthInMB   = MOS_MAX(m_picWidthInMbLastMaxAlloced, (m_avcPicParams->pic_width_in_mbs_minus1 + 1));
    uint16_t picHeightInMB  = MOS_MAX(m_picHeightInMbLastMaxAlloced, (m_avcPicParams->pic_height_in_mbs_minus1 + 1));
    uint32_t numMacroblocks = picWidthInMB * picHeightInMB;

    if (m_resMfdDeblockingFilterRowStoreScratchBuffer == nullptr)
    {
        m_resMfdDeblockingFilterRowStoreScratchBuffer = m_allocator->AllocateBuffer(
            picWidthInMB * 4 * CODECHAL_CACHELINE_SIZE,
            "DeblockingScratchBuffer",
            resourceInternalReadWriteCache,
            notLockableVideoMem);
        DECODE_CHK_NULL(m_resMfdDeblockingFilterRowStoreScratchBuffer);
    }
    else
    {
        DECODE_CHK_STATUS(m_allocator->Resize(
            m_resMfdDeblockingFilterRowStoreScratchBuffer,
            picWidthInMB * 4 * CODECHAL_CACHELINE_SIZE,
            notLockableVideoMem));
    }

    if (m_mfxItf->IsBsdMpcRowstoreCacheEnabled() == false)
    {
        if (m_resBsdMpcRowStoreScratchBuffer == nullptr)
        {
            m_resBsdMpcRowStoreScratchBuffer = m_allocator->AllocateBuffer(
                picWidthInMB * 2 * CODECHAL_CACHELINE_SIZE,
                "MpcScratchBuffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_resBsdMpcRowStoreScratchBuffer,
                picWidthInMB * 2 * CODECHAL_CACHELINE_SIZE,
                notLockableVideoMem));
        }
    }

    if (m_mfxItf->IsIntraRowstoreCacheEnabled() == false)
    {
        if (m_resMfdIntraRowStoreScratchBuffer == nullptr)
        {
            m_resMfdIntraRowStoreScratchBuffer = m_allocator->AllocateBuffer(
                picWidthInMB * CODECHAL_CACHELINE_SIZE,
                "IntraScratchBuffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_resMfdIntraRowStoreScratchBuffer,
                picWidthInMB * CODECHAL_CACHELINE_SIZE,
                notLockableVideoMem));
        }
    }

    if (m_mfxItf->IsMprRowstoreCacheEnabled() == false)
    {
        if (m_resMprRowStoreScratchBuffer == nullptr)
        {
            m_resMprRowStoreScratchBuffer = m_allocator->AllocateBuffer(
                picWidthInMB * 2 * CODECHAL_CACHELINE_SIZE,
                "MprScratchBuffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_resMprRowStoreScratchBuffer,
                picWidthInMB * 2 * CODECHAL_CACHELINE_SIZE,
                notLockableVideoMem));
        }
    }

    //record the width and height used for allocation internal resources.
    m_picWidthInMbLastMaxAlloced  = picWidthInMB;
    m_picHeightInMbLastMaxAlloced = picHeightInMB;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcDecodePicPkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    DECODE_FUNC_CALL();

    commandBufferSize      = m_pictureStatesSize;
    requestedPatchListSize = m_picturePatchListSize;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_PIPE_MODE_SELECT, AvcDecodePicPkt)
{
    params.Mode                                           = CODECHAL_DECODE_MODE_AVCVLD;
    params.deblockerStreamOutEnable                       = false;
    params.streamOutEnable                                = m_avcBasicFeature->m_streamOutEnabled;
    params.postDeblockingOutputEnablePostdeblockoutenable = m_avcBasicFeature->m_deblockingEnabled;
    params.preDeblockingOutputEnablePredeblockoutenable   = !m_avcBasicFeature->m_deblockingEnabled;
    params.shortFormatInUse                               = m_avcBasicFeature->m_shortFormatInUse;
    if (params.shortFormatInUse)//Short format Mode
    {
        params.decoderShortFormatMode = shortFormatMode;
    }
    if (!params.shortFormatInUse)//Long format Mode
    {
        params.decoderShortFormatMode = longFormatMode;
    }

    params.codecSelect = 0;
    if (CodecHalIsDecodeModeVLD(params.Mode))
    {
        params.decoderModeSelect = 0;
    }
    else if (CodecHalIsDecodeModeIT(params.Mode))
    {
        params.decoderModeSelect = 1;
    }
    params.standardSelect = CodecHal_GetStandardFromMode(params.Mode);
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_SURFACE_STATE, AvcDecodePicPkt)
{
    uint32_t uvPlaneAlignment = MHW_VDBOX_MFX_RECON_UV_PLANE_ALIGNMENT;
    params.psSurface          = &m_avcBasicFeature->m_destSurface;
    params.tilemode           = m_mfxItf->MosGetHWTileType(params.psSurface->TileType, params.psSurface->TileModeGMM, params.psSurface->bGMMTileEnabled);
    params.height             = params.psSurface->dwHeight - 1;
    params.width              = params.psSurface->dwWidth - 1;
    params.surfacePitch       = params.psSurface->dwPitch - 1;
    if (params.surfaceId == CODECHAL_MFX_SRC_SURFACE_ID)
    {
        uvPlaneAlignment = MHW_VDBOX_MFX_RAW_UV_PLANE_ALIGNMENT_GEN9;
    }
    else if ((params.surfaceId == CODECHAL_MFX_REF_SURFACE_ID) || params.surfaceId == CODECHAL_MFX_DSRECON_SURFACE_ID)
    {
        uvPlaneAlignment = params.uvPlaneAlignment ? params.uvPlaneAlignment : MHW_VDBOX_MFX_RECON_UV_PLANE_ALIGNMENT;
    }
    else
    {
        uvPlaneAlignment = MHW_VDBOX_MFX_UV_PLANE_ALIGNMENT_LEGACY;
    }
    params.surfaceFormat    = SURFACE_FORMAT_PLANAR4208;
    params.interleaveChroma = 1;

    if (params.psSurface->Format == Format_P8)
    {
        params.interleaveChroma = 0;
    }

    params.yOffsetForUCb = MOS_ALIGN_CEIL((params.psSurface->UPlaneOffset.iSurfaceOffset - params.psSurface->dwOffset) /
        params.psSurface->dwPitch + params.psSurface->RenderOffset.YUV.U.YOffset, uvPlaneAlignment);
    params.yOffsetForVCr = MOS_ALIGN_CEIL((params.psSurface->VPlaneOffset.iSurfaceOffset - params.psSurface->dwOffset) /
        params.psSurface->dwPitch + params.psSurface->RenderOffset.YUV.V.YOffset, uvPlaneAlignment);

#ifdef _MMC_SUPPORTED
    DECODE_CHK_STATUS(m_mmcState->SetSurfaceMmcState(&(m_avcBasicFeature->m_destSurface)));
    DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcState(params.psSurface, &params.mmcState));
    DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcFormat(&m_avcBasicFeature->m_destSurface, &params.compressionFormat));
#endif
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_PIPE_BUF_ADDR_STATE, AvcDecodePicPkt)
{
    params.decodeInUse = true;
    params.Mode        = CODECHAL_DECODE_MODE_AVCVLD;
    if (m_avcBasicFeature->m_deblockingEnabled)
    {
        params.psPostDeblockSurface = &(m_avcBasicFeature->m_destSurface);
    }
    else
    {
        params.psPreDeblockSurface = &(m_avcBasicFeature->m_destSurface);
    }
    params.presMfdIntraRowStoreScratchBuffer            = &m_resMfdIntraRowStoreScratchBuffer->OsResource;
    params.presMfdDeblockingFilterRowStoreScratchBuffer = &m_resMfdDeblockingFilterRowStoreScratchBuffer->OsResource;
    if (m_avcBasicFeature->m_streamOutEnabled)
    {
        params.presStreamOutBuffer = m_avcBasicFeature->m_externalStreamOutBuffer;
    }
    AvcReferenceFrames &        refFrames     = m_avcBasicFeature->m_refFrames;
    const std::vector<uint8_t> &activeRefList = refFrames.GetActiveReferenceList(*m_avcPicParams);
    for (uint8_t i = 0; i < activeRefList.size(); i++)
    {
        uint8_t frameIdx               = activeRefList[i];
        uint8_t frameId                = (m_avcBasicFeature->m_picIdRemappingInUse) ? i : refFrames.m_refList[frameIdx]->ucFrameId;
        params.presReferences[frameId] = refFrames.GetReferenceByFrameIndex(frameIdx);

        // Return error if reference surface's width or height is less than dest surface.
        if (params.presReferences[frameId] != nullptr)
        {
            MOS_SURFACE refSurface;
            refSurface.OsResource = *(params.presReferences[frameId]);
            DECODE_CHK_STATUS(m_allocator->GetSurfaceInfo(&refSurface));
            DECODE_CHK_COND(((refSurface.dwWidth < m_avcBasicFeature->m_destSurface.dwWidth)
                || (refSurface.dwHeight < m_avcBasicFeature->m_destSurface.dwHeight)),
                "Reference surface's width or height is less than Dest surface.");
        }
    }
    DECODE_CHK_STATUS(FixMfxPipeBufAddrParams());
    params.references = params.presReferences;
#ifdef _MMC_SUPPORTED
    DECODE_CHK_STATUS(SetSurfaceMmcState());
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcDecodePicPkt::FixMfxPipeBufAddrParams() const
{
    DECODE_FUNC_CALL();
    auto &        par         = m_mfxItf->MHW_GETPAR_F(MFX_PIPE_BUF_ADDR_STATE)();
    PMOS_RESOURCE validRefPic = nullptr;
    PMOS_RESOURCE dummyRef    = &(m_avcBasicFeature->m_dummyReference.OsResource);
    if (m_avcBasicFeature->m_useDummyReference && !m_allocator->ResourceIsNull(dummyRef))
    {
        validRefPic = dummyRef;
    }
    else
    {
        validRefPic = m_avcBasicFeature->m_refFrames.GetValidReference();
        if (validRefPic == nullptr)
        {
            validRefPic = &m_avcBasicFeature->m_destSurface.OsResource;
        }
    }

    for (uint8_t i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
    {
        if (par.presReferences[i] == nullptr)
        {
            par.presReferences[i] = validRefPic;
        }
    }
    return MOS_STATUS_SUCCESS;
}
MHW_SETPAR_DECL_SRC(MFX_IND_OBJ_BASE_ADDR_STATE, AvcDecodePicPkt)
{
    params.Mode           = CODECHAL_DECODE_MODE_AVCVLD;
    params.dwDataSize     = m_avcBasicFeature->m_dataSize;
    params.dwDataOffset   = m_avcBasicFeature->m_dataOffset;
    params.presDataBuffer = &(m_avcBasicFeature->m_resDataBuffer.OsResource);
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_BSP_BUF_BASE_ADDR_STATE, AvcDecodePicPkt)
{
    params.presBsdMpcRowStoreScratchBuffer = &m_resBsdMpcRowStoreScratchBuffer->OsResource;
    params.presMprRowStoreScratchBuffer    = &m_resMprRowStoreScratchBuffer->OsResource;
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_AVC_IMG_STATE, AvcDecodePicPkt)
{
    params.decodeInUse     = true;
    params.avcPicParams    = m_avcPicParams;
    params.mvcExtPicParams = m_avcBasicFeature->m_mvcExtPicParams;
    params.vdencEnabled    = false;

    AvcReferenceFrames &        refFrames      = m_avcBasicFeature->m_refFrames;
    const std::vector<uint8_t> &activeRefList  = refFrames.GetActiveReferenceList(*m_avcPicParams);
    uint8_t                     activeFrameCnt = activeRefList.size();
    params.activeFrameCnt                    = activeFrameCnt;

    params.numMBs =
        (m_avcPicParams->pic_height_in_mbs_minus1 + 1) *
        (m_avcPicParams->pic_width_in_mbs_minus1 + 1);
    params.imgstructImageStructureImgStructure10 = ((m_avcPicParams->CurrPic.PicFlags == PICTURE_FRAME) ? 0 : (CodecHal_PictureIsTopField(m_avcPicParams->CurrPic) ? 1 : 3));
    params.frameSize                             = params.numMBs;
    params.frameHeight                           = m_avcPicParams->pic_height_in_mbs_minus1;
    params.frameWidth                            = m_avcPicParams->pic_width_in_mbs_minus1;
    params.secondChromaQpOffset                  = m_avcPicParams->second_chroma_qp_index_offset;
    params.firstChromaQpOffset                   = m_avcPicParams->chroma_qp_index_offset;
    params.weightedPredFlag                      = m_avcPicParams->pic_fields.weighted_pred_flag;
    params.weightedBipredIdc                     = m_avcPicParams->pic_fields.weighted_bipred_idc;
    params.chromaformatidc                       = m_avcPicParams->seq_fields.chroma_format_idc;
    params.entropycodingflag                     = m_avcPicParams->pic_fields.entropy_coding_mode_flag;
    params.imgdisposableflag                     = !m_avcPicParams->pic_fields.reference_pic_flag;
    params.constrainedipredflag                  = m_avcPicParams->pic_fields.constrained_intra_pred_flag;
    params.direct8X8Infflag                      = m_avcPicParams->seq_fields.direct_8x8_inference_flag;
    params.transform8X8Flag                      = m_avcPicParams->pic_fields.transform_8x8_mode_flag;
    params.framembonlyflag                       = m_avcPicParams->seq_fields.frame_mbs_only_flag;
    params.mbaffflameflag                        = m_avcPicParams->seq_fields.mb_adaptive_frame_field_flag && !m_avcPicParams->pic_fields.field_pic_flag;
    params.fieldpicflag                          = m_avcPicParams->pic_fields.field_pic_flag;
    params.numberOfActiveReferencePicturesFromL0 = m_avcPicParams->num_ref_idx_l0_active_minus1 + 1;
    params.numberOfActiveReferencePicturesFromL1 = m_avcPicParams->num_ref_idx_l1_active_minus1 + 1;
    params.initialQpValue                        = m_avcPicParams->pic_init_qp_minus26;
    params.log2MaxFrameNumMinus4                 = m_avcPicParams->seq_fields.log2_max_frame_num_minus4;
    params.log2MaxPicOrderCntLsbMinus4           = m_avcPicParams->seq_fields.log2_max_pic_order_cnt_lsb_minus4;
    params.numSliceGroupsMinus1                  = m_avcPicParams->num_slice_groups_minus1;
    params.redundantPicCntPresentFlag            = m_avcPicParams->pic_fields.redundant_pic_cnt_present_flag;
    params.picOrderPresentFlag                   = m_avcPicParams->pic_fields.pic_order_present_flag;
    params.sliceGroupMapType                     = m_avcPicParams->slice_group_map_type;
    params.picOrderCntType                       = m_avcPicParams->seq_fields.pic_order_cnt_type;
    params.deblockingFilterControlPresentFlag    = m_avcPicParams->pic_fields.deblocking_filter_control_present_flag;
    params.deltaPicOrderAlwaysZeroFlag           = m_avcPicParams->seq_fields.delta_pic_order_always_zero_flag;
    params.currPicFrameNum                       = m_avcPicParams->frame_num;
    params.sliceGroupChangeRate                  = m_avcPicParams->slice_group_change_rate_minus1;
    params.numberOfReferenceFrames               = params.activeFrameCnt;
    if (params.mvcExtPicParams)
    {
        params.currentFrameViewId    = params.mvcExtPicParams->CurrViewID;
        params.maxViewIdxl0          = params.mvcExtPicParams->NumInterViewRefsL0;
        params.maxViewIdxl1          = params.mvcExtPicParams->NumInterViewRefsL1;
        params.interViewOrderDisable = 0;
    }
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_AVC_DIRECTMODE_STATE, AvcDecodePicPkt)
{
    AvcReferenceFrames &refFrames   = m_avcBasicFeature->m_refFrames;
    auto                mvBuffers   = &(m_avcBasicFeature->m_mvBuffers);
    PMOS_BUFFER         curMvBuffer = mvBuffers->GetCurBuffer();
    DECODE_CHK_NULL(curMvBuffer);

    params.resAvcDmvBuffers[0]      = curMvBuffer->OsResource;
    PMOS_BUFFER curAvailableBuffers = mvBuffers->GetAvailableBuffer();
    DECODE_CHK_NULL(curAvailableBuffers);
    params.resAvcDmvBuffers[CODEC_AVC_NUM_REF_DMV_BUFFERS] = curAvailableBuffers->OsResource;

    const std::vector<uint8_t> &activeRefList = refFrames.GetActiveReferenceList(*m_avcPicParams);

    for (uint8_t i = 0; i < activeRefList.size(); i++)
    {
        uint8_t frameIdx = activeRefList[i];
        if (m_avcBasicFeature->m_secondField && activeRefList.size() > m_avcBasicFeature->m_avcPicParams->frame_num &&
            (frameIdx == m_avcBasicFeature->m_curRenderPic.FrameIdx))
        {
            params.resAvcDmvBuffers[i + 1] = curMvBuffer->OsResource;
        }
        else
        {
            PMOS_BUFFER mvBuf = mvBuffers->GetBufferByFrameIndex(frameIdx);

            // m_resAvcDmvBuffers[0] is used as current mv buffer itself.
            if (mvBuf != nullptr)
            {
                params.resAvcDmvBuffers[i + 1] = mvBuf->OsResource;
            }
            else
            {
                PMOS_BUFFER curAvailableBuf = mvBuffers->GetAvailableBuffer();
                DECODE_CHK_NULL(curAvailableBuf);
                params.resAvcDmvBuffers[i + 1] = curAvailableBuf->OsResource;
            }
        }
        refFrames.m_refList[frameIdx]->ucDMVIdx[0] = i + 1;
    }
    params.CurrPic                 = m_avcPicParams->CurrPic;
    params.uiUsedForReferenceFlags = m_avcPicParams->UsedForReferenceFlags;
    params.ucAvcDmvIdx             = 0;
    params.presAvcDmvBuffers       = params.resAvcDmvBuffers;
    params.pAvcPicIdx              = &(refFrames.m_avcPicIdx[0]);
    params.avcRefList              = (void **)refFrames.m_refList;
    params.bPicIdRemappingInUse    = m_avcBasicFeature->m_picIdRemappingInUse;

    CODECHAL_DEBUG_TOOL(DECODE_CHK_STATUS(DumpResources(curMvBuffer->size)));

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFD_AVC_DPB_STATE, AvcDecodePicPkt)
{
    params.pAvcPicParams        = m_avcPicParams;
    params.pMvcExtPicParams     = m_avcBasicFeature->m_mvcExtPicParams;
    params.ppAvcRefList         = &(m_avcBasicFeature->m_refFrames.m_refList[0]);
    params.bPicIdRemappingInUse = m_avcBasicFeature->m_picIdRemappingInUse;
    auto avcPicParams           = params.pAvcPicParams;
    auto currFrameIdx           = avcPicParams->CurrPic.FrameIdx;
    auto currAvcRefList         = params.ppAvcRefList[currFrameIdx];

    int16_t  refFrameOrder[CODEC_MAX_NUM_REF_FRAME] = {0};
    uint32_t usedForRef                             = 0;
    uint16_t nonExistingFrameFlags                  = 0;
    uint16_t longTermFrame                          = 0;

    for (uint8_t i = 0; i < currAvcRefList->ucNumRef; i++)
    {
        auto picIdx            = currAvcRefList->RefList[i].FrameIdx;
        auto refAvcRefList     = params.ppAvcRefList[picIdx];
        bool longTermFrameFlag = (currAvcRefList->RefList[i].PicFlags == PICTURE_LONG_TERM_REFERENCE);

        uint8_t frameID  = params.bPicIdRemappingInUse ? i : refAvcRefList->ucFrameId;
        int16_t frameNum = refAvcRefList->sFrameNumber;

        refFrameOrder[frameID] = frameNum;
        usedForRef |= (((currAvcRefList->uiUsedForReferenceFlags >> (i * 2)) & 3) << (frameID * 2));
        nonExistingFrameFlags |= (((currAvcRefList->usNonExistingFrameFlags >> i) & 1) << frameID);
        longTermFrame |= (((uint16_t)longTermFrameFlag) << frameID);
    }
    for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME; i++)
    {
        params.refFrameOrder[i] = refFrameOrder[i];
    }

    params.NonExistingframeFlag161Bit = nonExistingFrameFlags;
    params.LongtermframeFlag161Bit    = longTermFrame;
    params.usedForRef                 = usedForRef;
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFD_AVC_PICID_STATE, AvcDecodePicPkt)
{
    AvcReferenceFrames &refFrames    = m_avcBasicFeature->m_refFrames;
    params.bPicIdRemappingInUse      = m_avcBasicFeature->m_picIdRemappingInUse;
    params.pAvcPicIdx                = &(refFrames.m_avcPicIdx[0]);
    params.PictureidRemappingDisable = 1;
    if (params.bPicIdRemappingInUse)
    {
        uint32_t j                       = 0;
        params.PictureidRemappingDisable = 0;
        for (auto i = 0; i < (CODEC_MAX_NUM_REF_FRAME / 2); i++)
        {
            params.Pictureidlist1616Bits[i] = -1;
            if (params.pAvcPicIdx[j++].bValid)
            {
                params.Pictureidlist1616Bits[i] = (params.Pictureidlist1616Bits[i] & 0xffff0000) | (params.pAvcPicIdx[j - 1].ucPicIdx);
            }
            if (params.pAvcPicIdx[j++].bValid)
            {
                params.Pictureidlist1616Bits[i] = (params.Pictureidlist1616Bits[i] & 0x0000ffff) | (params.pAvcPicIdx[j - 1].ucPicIdx << 16);
            }
        }
    }
    else
    {
        for (auto i = 0; i < (CODEC_MAX_NUM_REF_FRAME / 2); i++)
        {
            params.Pictureidlist1616Bits[i] = 0;
        }
    }
    return MOS_STATUS_SUCCESS;
}


MOS_STATUS AvcDecodePicPkt::AddAllCmds_MFX_QM_STATE(PMOS_COMMAND_BUFFER cmdBuffer)
{
    auto pAvcIqMatrix = (PMHW_VDBOX_AVC_QM_PARAMS)m_avcBasicFeature->m_avcIqMatrixParams;
    MHW_MI_CHK_NULL(pAvcIqMatrix);
    auto &params = m_mfxItf->MHW_GETPAR_F(MFX_QM_STATE)();
    params       = {};

    auto     iqMatrix = pAvcIqMatrix;
    uint8_t *qMatrix  = (uint8_t *)params.quantizermatrix;

    for (uint8_t i = 0; i < 16; i++)
    {
        params.quantizermatrix[i] = 0;
    }

    params.qmType = m_avcBasicFeature->avcQmIntra4x4;
    for (auto i = 0; i < 3; i++)
    {
        for (auto ii = 0; ii < 16; ii++)
        {
            qMatrix[i * 16 + ii] = iqMatrix->List4x4[i][ii];
        }
    }
    DECODE_CHK_STATUS(m_mfxItf->MHW_ADDCMD_F(MFX_QM_STATE)(cmdBuffer));

    params.qmType = m_avcBasicFeature->avcQmInter4x4;
    for (auto i = 3; i < 6; i++)
    {
        for (auto ii = 0; ii < 16; ii++)
        {
            qMatrix[(i - 3) * 16 + ii] = iqMatrix->List4x4[i][ii];
        }
    }
    DECODE_CHK_STATUS(m_mfxItf->MHW_ADDCMD_F(MFX_QM_STATE)(cmdBuffer));

    params.qmType = m_avcBasicFeature->avcQmIntra8x8;
    for (auto ii = 0; ii < 64; ii++)
    {
        qMatrix[ii] = iqMatrix->List8x8[0][ii];
    }
    DECODE_CHK_STATUS(m_mfxItf->MHW_ADDCMD_F(MFX_QM_STATE)(cmdBuffer));

    params.qmType = m_avcBasicFeature->avcQmInter8x8;
    for (auto ii = 0; ii < 64; ii++)
    {
        qMatrix[ii] = iqMatrix->List8x8[1][ii];
    }
    DECODE_CHK_STATUS(m_mfxItf->MHW_ADDCMD_F(MFX_QM_STATE)(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS AvcDecodePicPkt::DumpResources(uint32_t mvBufferSize) const
{
    DECODE_FUNC_CALL();

    CodechalDebugInterface *debugInterface = m_avcPipeline->GetDebugInterface();

    auto &par = m_mfxItf->MHW_GETPAR_F(MFX_PIPE_BUF_ADDR_STATE)();
    auto &mvParam = m_mfxItf->MHW_GETPAR_F(MFX_AVC_DIRECTMODE_STATE)();

    for (auto n = 0; n < CODEC_AVC_MAX_NUM_REF_FRAME; n++)
    {
        if (m_avcBasicFeature->m_refFrames.m_avcPicIdx[n].bValid)
        {
            MOS_SURFACE destSurface;
            MOS_ZeroMemory(&destSurface, sizeof(MOS_SURFACE));
            destSurface.OsResource = *(par.presReferences[n]);
            DECODE_CHK_STATUS(m_allocator->GetSurfaceInfo(&destSurface));
            std::string refSurfName = "RefSurf[" + std::to_string(static_cast<uint32_t>(n)) + "]";
            DECODE_CHK_STATUS(debugInterface->DumpYUVSurface(
                &destSurface,
                CodechalDbgAttr::attrDecodeReferenceSurfaces,
                refSurfName.c_str()));

            std::string mvBufDumpName = "_DEC_Ref_MV_" + std::to_string(n);
            DECODE_CHK_STATUS(debugInterface->DumpBuffer(
                &mvParam.presAvcDmvBuffers[n+1],
                CodechalDbgAttr::attrMvData,
                mvBufDumpName.c_str(),
                mvBufferSize));
        }
    }

    DECODE_CHK_STATUS(debugInterface->DumpBuffer(
        &mvParam.presAvcDmvBuffers[0],
        CodechalDbgAttr::attrMvData,
        "DEC_Cur_MV_",
        mvBufferSize));

    return MOS_STATUS_SUCCESS;
}
#endif

MOS_STATUS AvcDecodePicPkt::SetSurfaceMmcState() const
{
    DECODE_FUNC_CALL();
    auto &par = m_mfxItf->MHW_GETPAR_F(MFX_PIPE_BUF_ADDR_STATE)();
    if (m_mmcState->IsMmcEnabled())
    {
        if (m_avcBasicFeature->m_deblockingEnabled)
        {
            DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcState(
                &m_avcBasicFeature->m_destSurface,
                &par.PostDeblockSurfMmcState));
        }
        else
        {
            DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcState(
                &m_avcBasicFeature->m_destSurface,
                &par.PreDeblockSurfMmcState));
        }
    }
    else
    {
        par.PreDeblockSurfMmcState  = MOS_MEMCOMP_DISABLED;
        par.PostDeblockSurfMmcState = MOS_MEMCOMP_DISABLED;
    }

    return MOS_STATUS_SUCCESS;
}

}  // namespace decode
