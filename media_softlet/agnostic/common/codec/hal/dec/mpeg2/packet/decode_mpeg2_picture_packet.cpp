/*
* Copyright (c) 2021-2024, Intel Corporation
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
//! \file     decode_mpeg2_picture_packet.cpp
//! \brief    Defines the interface for mpeg2 decode picture packet
//!
#include "decode_mpeg2_picture_packet.h"
#include "codechal_debug.h"
#include "decode_common_feature_defs.h"
#include "decode_mpeg2_mem_compression.h"

namespace decode{

Mpeg2DecodePicPkt::~Mpeg2DecodePicPkt()
{
    FreeResources();
}

MOS_STATUS Mpeg2DecodePicPkt::FreeResources()
{
    DECODE_FUNC_CALL();

    if (m_allocator != nullptr)
    {
        m_allocator->Destroy(m_resMfdDeblockingFilterRowStoreScratchBuffer);
        m_allocator->Destroy(m_resBsdMpcRowStoreScratchBuffer);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2DecodePicPkt::Init()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(m_featureManager);
    DECODE_CHK_NULL(m_hwInterface);
    DECODE_CHK_NULL(m_osInterface);
    DECODE_CHK_NULL(m_miItf);
    DECODE_CHK_NULL(m_mpeg2Pipeline);
    DECODE_CHK_NULL(m_mfxItf);

    m_mpeg2BasicFeature = dynamic_cast<Mpeg2BasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_mpeg2BasicFeature);

    m_allocator = m_pipeline ->GetDecodeAllocator();
    DECODE_CHK_NULL(m_allocator);

    DECODE_CHK_STATUS(AllocateFixedResources());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2DecodePicPkt::Prepare()
{
    DECODE_FUNC_CALL();

    m_mpeg2PicParams = m_mpeg2BasicFeature->m_mpeg2PicParams;
    DECODE_CHK_NULL(m_mpeg2PicParams);

#ifdef _MMC_SUPPORTED
    m_mmcState = m_mpeg2Pipeline->GetMmcState();
    DECODE_CHK_NULL(m_mmcState);
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2DecodePicPkt::AllocateFixedResources()
{
    DECODE_FUNC_CALL();

    uint16_t picWidthInMb  = m_mpeg2BasicFeature->m_picWidthInMb;
    uint16_t picHeightInMb = m_mpeg2BasicFeature->m_picHeightInMb;

    // Deblocking Filter Row Store Scratch buffer
    // (Num MacroBlock Width) * (Num Cachlines) * (Cachline size)
    m_resMfdDeblockingFilterRowStoreScratchBuffer= m_allocator->AllocateBuffer(
        picWidthInMb * 7 * CODECHAL_CACHELINE_SIZE,
        "DeblockingFilterScratch",
        resourceInternalReadWriteCache,
        notLockableVideoMem);

    // MPR Row Store Scratch buffer
    // (FrameWidth in MB) * (CacheLine size per MB) * 2
    m_resBsdMpcRowStoreScratchBuffer= m_allocator->AllocateBuffer(
        ((uint32_t)(picWidthInMb * CODECHAL_CACHELINE_SIZE)) * 2,
        "MprScratchBuffer",
        resourceInternalReadWriteCache,
        notLockableVideoMem);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2DecodePicPkt::FixMfxPipeBufAddrParams() const
{
    DECODE_FUNC_CALL();

    auto &        par      = m_mfxItf->MHW_GETPAR_F(MFX_PIPE_BUF_ADDR_STATE)();
    PMOS_RESOURCE dummyRef = &(m_mpeg2BasicFeature->m_dummyReference.OsResource);

    // set all ref pic addresses to valid addresses for error concealment purpose
    for (uint32_t i = 0; i < CODEC_MAX_NUM_REF_FRAME_NON_AVC; i++)
    {
        if (m_mpeg2BasicFeature->m_useDummyReference && !m_allocator->ResourceIsNull(dummyRef) &&
            par.presReferences[i] == nullptr)
        {
            par.presReferences[i] = dummyRef;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_PIPE_MODE_SELECT, Mpeg2DecodePicPkt)
{
    params.streamOutEnable                                = m_mpeg2BasicFeature->m_streamOutEnabled;
    params.postDeblockingOutputEnablePostdeblockoutenable = m_mpeg2BasicFeature->m_deblockingEnabled;
    params.preDeblockingOutputEnablePredeblockoutenable   = !m_mpeg2BasicFeature->m_deblockingEnabled;
    params.decoderShortFormatMode                         = true;

    if (CodecHalIsDecodeModeVLD(m_mpeg2BasicFeature->m_mode))
    {
        params.decoderModeSelect = mfxDecoderModeVld;
    }
    else if (CodecHalIsDecodeModeIT(m_mpeg2BasicFeature->m_mode))
    {
        params.decoderModeSelect = mfxDecoderModeIt;
    }

    params.standardSelect = CodecHal_GetStandardFromMode(m_mpeg2BasicFeature->m_mode);

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_SURFACE_STATE, Mpeg2DecodePicPkt)
{
    // set cmd function
    MOS_SURFACE *psSurface = &m_mpeg2BasicFeature->m_destSurface;

#ifdef _MMC_SUPPORTED
    DECODE_CHK_STATUS(m_mmcState->SetSurfaceMmcState(&(m_mpeg2BasicFeature->m_destSurface)));
    DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcState(psSurface, &params.mmcState));
    DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcFormat(psSurface, &params.compressionFormat));
#endif

    params.height           = psSurface->dwHeight - 1;
    params.width            = psSurface->dwWidth - 1;
    params.surfacePitch     = psSurface->dwPitch - 1;
    params.interleaveChroma = 1;
    params.surfaceFormat    = SURFACE_FORMAT_PLANAR4208;

    params.tilemode = m_mfxItf->MosGetHWTileType(psSurface->TileType, psSurface->TileModeGMM, psSurface->bGMMTileEnabled);

    uint32_t uvPlaneAlignment = MHW_VDBOX_MFX_UV_PLANE_ALIGNMENT_LEGACY;
    if (params.surfaceId == CODECHAL_MFX_SRC_SURFACE_ID)
    {
        uvPlaneAlignment = MHW_VDBOX_MFX_RAW_UV_PLANE_ALIGNMENT_GEN9;
    }
    else
    {
        uvPlaneAlignment = MHW_VDBOX_MFX_UV_PLANE_ALIGNMENT_LEGACY;
    }

    if (psSurface->Format == Format_P8)  // monochrome format
    {
        params.interleaveChroma = 0;
    }

    params.yOffsetForUCb = MOS_ALIGN_CEIL((psSurface->UPlaneOffset.iSurfaceOffset - psSurface->dwOffset) /
        psSurface->dwPitch + psSurface->RenderOffset.YUV.U.YOffset, uvPlaneAlignment);
    params.yOffsetForVCr = MOS_ALIGN_CEIL((psSurface->VPlaneOffset.iSurfaceOffset - psSurface->dwOffset) /
        psSurface->dwPitch + psSurface->RenderOffset.YUV.V.YOffset, uvPlaneAlignment);

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_PIPE_BUF_ADDR_STATE, Mpeg2DecodePicPkt)
{
    if (m_mpeg2BasicFeature->m_deblockingEnabled)
    {
        params.psPostDeblockSurface = &(m_mpeg2BasicFeature->m_destSurface);
    }
    else
    {
        params.psPreDeblockSurface = &(m_mpeg2BasicFeature->m_destSurface);
    }
    params.presMfdDeblockingFilterRowStoreScratchBuffer = &m_resMfdDeblockingFilterRowStoreScratchBuffer->OsResource;

    if (m_mpeg2BasicFeature->m_streamOutEnabled)
    {
        params.presStreamOutBuffer = m_mpeg2BasicFeature->m_streamOutBuffer;
    }

    Mpeg2ReferenceFrames &refFrames = m_mpeg2BasicFeature->m_refFrames;

    // when there is not a forward or backward reference,
    // the index is set to the destination frame index
    params.presReferences[CodechalDecodeFwdRefTop] =
        params.presReferences[CodechalDecodeFwdRefBottom] =
            params.presReferences[CodechalDecodeBwdRefTop] =
                params.presReferences[CodechalDecodeBwdRefBottom] =
                    &m_mpeg2BasicFeature->m_destSurface.OsResource;

    if (m_mpeg2BasicFeature->m_fwdRefIdx < CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2)
    {
        params.presReferences[CodechalDecodeFwdRefTop] =
            params.presReferences[CodechalDecodeFwdRefBottom] =
                &refFrames.m_refList[m_mpeg2BasicFeature->m_fwdRefIdx]->resRefPic;
    }
    if (m_mpeg2BasicFeature->m_bwdRefIdx < CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2)
    {
        params.presReferences[CodechalDecodeBwdRefTop] =
            params.presReferences[CodechalDecodeBwdRefBottom] =
                &refFrames.m_refList[m_mpeg2BasicFeature->m_bwdRefIdx]->resRefPic;
    }

    // special case for second fields
    if (m_mpeg2PicParams->m_secondField && m_mpeg2PicParams->m_pictureCodingType == P_TYPE)
    {
        if (m_mpeg2PicParams->m_topFieldFirst)
        {
            params.presReferences[CodechalDecodeFwdRefTop] =
                &m_mpeg2BasicFeature->m_destSurface.OsResource;
        }
        else
        {
            params.presReferences[CodechalDecodeFwdRefBottom] =
                &m_mpeg2BasicFeature->m_destSurface.OsResource;
        }
    }

#ifdef _MMC_SUPPORTED
    DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcState(params.psPreDeblockSurface, &params.PreDeblockSurfMmcState));
#endif

    DECODE_CHK_STATUS(FixMfxPipeBufAddrParams());

    CODECHAL_DEBUG_TOOL(DumpResources(params));

#ifdef _MMC_SUPPORTED
    Mpeg2DecodeMemComp *mpeg2DecodeMemComp = dynamic_cast<Mpeg2DecodeMemComp *>(m_mmcState);
    DECODE_CHK_NULL(mpeg2DecodeMemComp);
    DECODE_CHK_STATUS(mpeg2DecodeMemComp->CheckReferenceList(*m_mpeg2BasicFeature, params.PreDeblockSurfMmcState, params.PostDeblockSurfMmcState));
#endif

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_IND_OBJ_BASE_ADDR_STATE, Mpeg2DecodePicPkt)
{
    params.Mode            = m_mpeg2BasicFeature->m_mode;
    params.dwDataSize      = m_mpeg2BasicFeature->m_copiedDataBufferInUse ?
        m_mpeg2BasicFeature->m_copiedDataBufferSize : m_mpeg2BasicFeature->m_dataSize;
    params.presDataBuffer  = m_mpeg2BasicFeature->m_copiedDataBufferInUse ?
        &(m_mpeg2BasicFeature->m_copiedDataBuf->OsResource) : &(m_mpeg2BasicFeature->m_resDataBuffer.OsResource);

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_BSP_BUF_BASE_ADDR_STATE, Mpeg2DecodePicPkt)
{
    params.presBsdMpcRowStoreScratchBuffer = &m_resBsdMpcRowStoreScratchBuffer->OsResource;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_MPEG2_PIC_STATE, Mpeg2DecodePicPkt)
{
    params.ScanOrder                    = m_mpeg2PicParams->W0.m_scanOrder;
    params.IntraVlcFormat               = m_mpeg2PicParams->W0.m_intraVlcFormat;
    params.QuantizerScaleType           = m_mpeg2PicParams->W0.m_quantizerScaleType;
    params.ConcealmentMotionVectorFlag  = m_mpeg2PicParams->W0.m_concealmentMVFlag;
    params.FramePredictionFrameDct      = m_mpeg2PicParams->W0.m_frameDctPrediction;
    params.TffTopFieldFirst             = (CodecHal_PictureIsFrame(m_mpeg2PicParams->m_currPic)) ?
        m_mpeg2PicParams->W0.m_topFieldFirst : m_mpeg2PicParams->m_topFieldFirst;

    params.PictureStructure  = (CodecHal_PictureIsFrame(m_mpeg2PicParams->m_currPic)) ?
        mpeg2Vc1Frame : (CodecHal_PictureIsTopField(m_mpeg2PicParams->m_currPic)) ?
        mpeg2Vc1TopField : mpeg2Vc1BottomField;
    params.IntraDcPrecision  = m_mpeg2PicParams->W0.m_intraDCPrecision;
    params.FCode00           = m_mpeg2PicParams->W1.m_fcode00;
    params.FCode01           = m_mpeg2PicParams->W1.m_fcode01;
    params.FCode10           = m_mpeg2PicParams->W1.m_fcode10;
    params.FCode11           = m_mpeg2PicParams->W1.m_fcode11;
    params.PictureCodingType = m_mpeg2PicParams->m_pictureCodingType;

    if (m_mpeg2BasicFeature->m_mode == CODECHAL_DECODE_MODE_MPEG2VLD)
    {
        params.ISliceConcealmentMode                                             = m_mpeg2BasicFeature->m_mpeg2ISliceConcealmentMode;
        params.PBSliceConcealmentMode                                            = m_mpeg2BasicFeature->m_mpeg2PbSliceConcealmentMode;
        params.PBSlicePredictedBidirMotionTypeOverrideBiDirectionMvTypeOverride  = m_mpeg2BasicFeature->m_mpeg2PbSlicePredBiDirMvTypeOverride;
        params.PBSlicePredictedMotionVectorOverrideFinalMvValueOverride          = m_mpeg2BasicFeature->m_mpeg2PbSlicePredMvOverride;

        params.SliceConcealmentDisableBit = 1;
    }

    uint16_t widthInMbs =
        (m_mpeg2PicParams->m_horizontalSize + CODECHAL_MACROBLOCK_WIDTH - 1) /
        CODECHAL_MACROBLOCK_WIDTH;

    uint16_t heightInMbs =
        (m_mpeg2PicParams->m_verticalSize + CODECHAL_MACROBLOCK_HEIGHT - 1) /
        CODECHAL_MACROBLOCK_HEIGHT;

    params.Framewidthinmbsminus170PictureWidthInMacroblocks   = widthInMbs - 1;
    params.Frameheightinmbsminus170PictureHeightInMacroblocks = (CodecHal_PictureIsField(m_mpeg2PicParams->m_currPic)) ?
        ((heightInMbs * 2) - 1) : heightInMbs - 1;

    if (m_mpeg2BasicFeature->m_deblockingEnabled)
    {
        params.mfxMpeg2PicStatePar0 = 9;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2DecodePicPkt::AddAllCmds_MFX_QM_STATE(MOS_COMMAND_BUFFER &cmdBuffer)
{
    auto pMpeg2IqMatrix = m_mpeg2BasicFeature->m_mpeg2IqMatrixBuffer;
    MHW_MI_CHK_NULL(pMpeg2IqMatrix);

    auto &params = m_mfxItf->MHW_GETPAR_F(MFX_QM_STATE)();
    params       = {};

    uint8_t *qMatrix  = (uint8_t *)params.quantizermatrix;

    for (auto i = 0; i < 64; i++)
    {
        qMatrix[i] = 0;
    }

    params.qmType = m_mpeg2BasicFeature->Mpeg2QmTypes::mpeg2QmIntra;
    if (pMpeg2IqMatrix->m_loadIntraQuantiserMatrix)
    {
        uint8_t *src = pMpeg2IqMatrix->m_intraQuantiserMatrix;
        for (auto i = 0; i < 64; i++)
        {
            qMatrix[i] = (uint8_t)(src[m_mpeg2BasicFeature->m_mpeg2QuantMatrixScan[i]]);
        }
    }
    else
    {
        for (auto i = 0; i < 64; i++)
        {
            qMatrix[i] = (uint8_t)(m_mpeg2BasicFeature->m_mpeg2DefaultIntraQuantizerMatrix[i]);
        }
    }

    DECODE_CHK_STATUS(m_mfxItf->MHW_ADDCMD_F(MFX_QM_STATE)(&cmdBuffer));

    params.qmType = m_mpeg2BasicFeature->Mpeg2QmTypes::mpeg2QmNonIntra;
    if (pMpeg2IqMatrix->m_loadNonIntraQuantiserMatrix)
    {
        uint8_t *src = pMpeg2IqMatrix->m_nonIntraQuantiserMatrix;
        for (auto i = 0; i < 64; i++)
        {
            qMatrix[i] = (uint8_t)(src[m_mpeg2BasicFeature->m_mpeg2QuantMatrixScan[i]]);
        }
    }
    else
    {
        for (auto i = 0; i < 64; i++)
        {
            qMatrix[i] = (uint8_t)(m_mpeg2BasicFeature->m_mpeg2DefaultNonIntraQuantizerMatrix[i]);
        }
    }

    DECODE_CHK_STATUS(m_mfxItf->MHW_ADDCMD_F(MFX_QM_STATE)(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}



MOS_STATUS Mpeg2DecodePicPkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    DECODE_FUNC_CALL();

    commandBufferSize = m_pictureStatesSize;
    requestedPatchListSize = m_picturePatchListSize;

    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS Mpeg2DecodePicPkt::DumpResources(MFX_PIPE_BUF_ADDR_STATE_PAR &pipeBufAddrParams) const
    {
        DECODE_FUNC_CALL();

        CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
        DECODE_CHK_NULL(debugInterface);

        if (m_mpeg2PicParams->m_pictureCodingType != I_TYPE)
        {
            for (uint16_t n = 0; n <= CodechalDecodeBwdRefBottom; n++)
            {
                if (pipeBufAddrParams.presReferences[n])
                {
                    MOS_SURFACE refSurface;
                    MOS_ZeroMemory(&refSurface, sizeof(MOS_SURFACE));
                    refSurface.OsResource = *(pipeBufAddrParams.presReferences[n]);
                    DECODE_CHK_STATUS(m_allocator->GetSurfaceInfo(&refSurface));

                    debugInterface->m_refIndex = n;
                    std::string refSurfName    = "RefSurf[" + std::to_string(static_cast<uint32_t>(debugInterface->m_refIndex)) + "]";
                    DECODE_CHK_STATUS(debugInterface->DumpYUVSurface(
                        &refSurface,
                        CodechalDbgAttr::attrDecodeReferenceSurfaces,
                        refSurfName.c_str()));
                }
            }
        }

        return MOS_STATUS_SUCCESS;
    }
#endif

MOS_STATUS Mpeg2DecodePicPkt::AddAllCmds_MFX_PIPE_MODE_SELECT(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    //for gen 12, we need to add MFX wait for both KIN and VRT before and after MFX Pipemode select.
    auto &mfxWaitParams               = m_miItf->MHW_GETPAR_F(MFX_WAIT)();
    mfxWaitParams                     = {};
    mfxWaitParams.iStallVdboxPipeline = true;
    DECODE_CHK_STATUS((m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer)));

    DECODE_CHK_NULL(m_mfxItf);
    SETPAR_AND_ADDCMD(MFX_PIPE_MODE_SELECT, m_mfxItf, &cmdBuffer);

    //for gen 12, we need to add MFX wait for both KIN and VRT before and after MFX Pipemode select.
    mfxWaitParams                     = {};
    mfxWaitParams.iStallVdboxPipeline = true;
    DECODE_CHK_STATUS((m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer)));

    return MOS_STATUS_SUCCESS;
}

}  // namespace decode
