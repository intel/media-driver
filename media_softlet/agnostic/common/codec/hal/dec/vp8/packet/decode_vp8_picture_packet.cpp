/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     decode_vp8_picture_packet.cpp
//! \brief    Defines the interface for vp8 decode picture packet
//!
#include "decode_vp8_picture_packet.h"
#include "codechal_debug.h"
#include "decode_common_feature_defs.h"
#include "decode_vp8_mem_compression.h"

namespace decode
{
    Vp8DecodePicPkt::~Vp8DecodePicPkt()
    {
        FreeResources();
    }

    MOS_STATUS Vp8DecodePicPkt::FreeResources()
    {
        DECODE_FUNC_CALL();

        if (m_allocator != nullptr)
        {
            m_allocator->Destroy(m_resMfdDeblockingFilterRowStoreScratchBuffer);
            m_allocator->Destroy(m_resMfdIntraRowStoreScratchBuffer);
            m_allocator->Destroy(m_resBsdMpcRowStoreScratchBuffer);
            m_allocator->Destroy(m_resMprRowStoreScratchBuffer);
            m_allocator->Destroy(m_resSegmentationIdStreamBuffer);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Vp8DecodePicPkt::Init()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_featureManager);
        DECODE_CHK_NULL(m_hwInterface);
        DECODE_CHK_NULL(m_osInterface);
        DECODE_CHK_NULL(m_miItf);
        DECODE_CHK_NULL(m_vp8Pipeline);

        m_vp8BasicFeature = dynamic_cast<Vp8BasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        DECODE_CHK_NULL(m_vp8BasicFeature);

        m_allocator = m_pipeline->GetDecodeAllocator();
        DECODE_CHK_NULL(m_allocator);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Vp8DecodePicPkt::Prepare()
    {
        DECODE_FUNC_CALL();

        m_vp8PicParams = m_vp8BasicFeature->m_vp8PicParams;
        DECODE_CHK_STATUS(SetRowstoreCachingOffsets());
        DECODE_CHK_STATUS(SetRowStoreScratchBuffer());
        DECODE_CHK_STATUS(SetSegmentationIdStreamBuffer());

    #ifdef _MMC_SUPPORTED
        m_mmcState = m_vp8Pipeline->GetMmcState();
        DECODE_CHK_NULL(m_mmcState);
    #endif

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Vp8DecodePicPkt::SetRowstoreCachingOffsets()
    {
        DECODE_FUNC_CALL();

        if (m_mfxItf->IsRowStoreCachingSupported())
        {
            MHW_VDBOX_ROWSTORE_PARAMS rowstoreParams;
            MOS_ZeroMemory(&rowstoreParams, sizeof(rowstoreParams));
            rowstoreParams.dwPicWidth = m_vp8BasicFeature->m_width;
            rowstoreParams.bMbaff = false;
            rowstoreParams.Mode = CODECHAL_DECODE_MODE_VP8VLD;
            //DECODE_CHK_STATUS(m_hwInterface->SetRowstoreCachingOffsets(&rowstoreParams));
            DECODE_CHK_STATUS(m_mfxItf->GetRowstoreCachingAddrs(&rowstoreParams));

        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Vp8DecodePicPkt::SetRowStoreScratchBuffer()
    {
        DECODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        // MfdDeblockingFilterRowStoreScratchBuffer
        if (m_mfxItf->IsDeblockingFilterRowstoreCacheEnabled() == false)
        {
            // Deblocking Filter Row Store Scratch buffer
            //(Num MacroBlock Width) * (Num Cachlines) * (Cachline size)
            auto buffer_size = (m_vp8PicParams->wFrameWidthInMbsMinus1 + 1) * 2 * CODECHAL_CACHELINE_SIZE;

            if (m_resMfdDeblockingFilterRowStoreScratchBuffer == nullptr)
            {
                m_resMfdDeblockingFilterRowStoreScratchBuffer = m_allocator->AllocateBuffer(
                    buffer_size, "DeblockingScratchBuffer", resourceInternalReadWriteCache, notLockableVideoMem);
                DECODE_CHK_NULL(m_resMfdDeblockingFilterRowStoreScratchBuffer);
            }
            else
            {
                DECODE_CHK_STATUS(m_allocator->Resize(
                    m_resMfdDeblockingFilterRowStoreScratchBuffer, buffer_size, notLockableVideoMem));
            }
        }

        // m_resMfdIntraRowStoreScratchBuffer
        if (m_mfxItf->IsIntraRowstoreCacheEnabled() == false)
        {
            // Intra Row Store Scratch buffer
            // (FrameWidth in MB) * (CacheLine size per MB)
            auto buffer_size = (m_vp8PicParams->wFrameWidthInMbsMinus1 + 1) * 2 * CODECHAL_CACHELINE_SIZE;

            if (m_resMfdIntraRowStoreScratchBuffer == nullptr)
            {
                m_resMfdIntraRowStoreScratchBuffer = m_allocator->AllocateBuffer(
                    buffer_size, "IntraScratchBuffer", resourceInternalReadWriteCache, notLockableVideoMem);
                DECODE_CHK_NULL(m_resMfdIntraRowStoreScratchBuffer);
            }
            else
            {
                DECODE_CHK_STATUS(m_allocator->Resize(
                    m_resMfdIntraRowStoreScratchBuffer, buffer_size, notLockableVideoMem));
            }
        }

        // m_resBsdMpcRowStoreScratchBuffer
        if (m_mfxItf->IsBsdMpcRowstoreCacheEnabled() == false)
        {
            // BSD/MPC Row Store Scratch buffer
            // (FrameWidth in MB) * (2) * (CacheLine size per MB)
            auto buffer_size = (m_vp8PicParams->wFrameWidthInMbsMinus1 + 1) * CODECHAL_CACHELINE_SIZE;

            if (m_resBsdMpcRowStoreScratchBuffer == nullptr)
            {
                m_resBsdMpcRowStoreScratchBuffer = m_allocator->AllocateBuffer(
                    buffer_size, "MpcScratchBuffer", resourceInternalReadWriteCache, notLockableVideoMem);
                DECODE_CHK_NULL(m_resBsdMpcRowStoreScratchBuffer);
            }
            else
            {
                DECODE_CHK_STATUS(m_allocator->Resize(
                    m_resBsdMpcRowStoreScratchBuffer, buffer_size, notLockableVideoMem));
            }
        }

        // m_resMprRowStoreScratchBuffer
        {
            // MPR Row Store Scratch buffer
            // (FrameWidth in MB) * (2) * (CacheLine size per MB)
            auto buffer_size = (m_vp8PicParams->wFrameWidthInMbsMinus1 + 1) * CODECHAL_CACHELINE_SIZE * 22;

            if (m_resMprRowStoreScratchBuffer == nullptr)
            {
                m_resMprRowStoreScratchBuffer = m_allocator->AllocateBuffer(
                    buffer_size, "MprScratchBuffer", resourceInternalReadWriteCache, notLockableVideoMem);
                DECODE_CHK_NULL(m_resMprRowStoreScratchBuffer);
            }
            else
            {
                DECODE_CHK_STATUS(m_allocator->Resize(
                    m_resMprRowStoreScratchBuffer, buffer_size, notLockableVideoMem));
            }
        }

        return eStatus;
    }

    MOS_STATUS Vp8DecodePicPkt::SetSegmentationIdStreamBuffer(){
        DECODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        // m_resSegmentationIdStreamBuffer
        //(Num MacroBlocks) * (Cachline size) * (2 bit)
        uint32_t numMacroblocks   = (m_vp8PicParams->wFrameWidthInMbsMinus1 + 1) * (m_vp8PicParams->wFrameHeightInMbsMinus1 + 1);
        auto buffer_size = MOS_MAX(numMacroblocks * CODECHAL_CACHELINE_SIZE * 2 / 8, 64);

        if (m_resSegmentationIdStreamBuffer == nullptr)
        {
            m_resSegmentationIdStreamBuffer = m_allocator->AllocateBuffer(
                buffer_size, "SegmentationIdStreamBuffer", resourceInternalReadWriteCache, notLockableVideoMem);
            DECODE_CHK_NULL(m_resSegmentationIdStreamBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_resSegmentationIdStreamBuffer, buffer_size, notLockableVideoMem));
        }

        return eStatus;
    }

    MHW_SETPAR_DECL_SRC(MFX_PIPE_MODE_SELECT, Vp8DecodePicPkt)
    {
        params.Mode                  = (CODECHAL_MODE)CODECHAL_DECODE_MODE_VP8VLD;
        params.streamOutEnable     = m_vp8BasicFeature->m_streamOutEnabled;

        params.postDeblockingOutputEnablePostdeblockoutenable = m_vp8BasicFeature->m_deblockingEnabled;
        params.preDeblockingOutputEnablePredeblockoutenable  = !m_vp8BasicFeature->m_deblockingEnabled;
        params.decoderShortFormatMode     = !m_vp8BasicFeature->m_shortFormatInUse;
        params.standardSelect       = 5;    // CODECHAL_VP8
        params.decoderModeSelect    = 0;   //  mfxDecoderModeVld
        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(MFX_SURFACE_STATE, Vp8DecodePicPkt)
    {
        // set cmd function
        MOS_SURFACE *psSurface = &m_vp8BasicFeature->m_destSurface;

    #ifdef _MMC_SUPPORTED
        DECODE_CHK_STATUS(m_mmcState->SetSurfaceMmcState(&(m_vp8BasicFeature->m_destSurface)));
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

    MHW_SETPAR_DECL_SRC(MFX_PIPE_BUF_ADDR_STATE, Vp8DecodePicPkt)
    {
        params.decodeInUse         = true;
        params.Mode                  = (CODECHAL_MODE)CODECHAL_DECODE_MODE_VP8VLD;

        if (m_vp8BasicFeature->m_deblockingEnabled)
        {
            params.psPostDeblockSurface = &(m_vp8BasicFeature->m_destSurface);
        }
        else
        {
            params.psPreDeblockSurface = &(m_vp8BasicFeature->m_destSurface);
        }
    #ifdef _MMC_SUPPORTED
            auto m_mmcEnabled = m_mmcState->IsMmcEnabled();
            Vp8DecodeMemComp *vp8DecodeMemComp = dynamic_cast<Vp8DecodeMemComp *>(m_mmcState);
            DECODE_CHK_NULL(vp8DecodeMemComp);
            vp8DecodeMemComp->m_mmcEnabled = m_mmcEnabled;
            DECODE_CHK_STATUS(vp8DecodeMemComp->SetPipeBufAddr(*m_vp8BasicFeature, params.PostDeblockSurfMmcState, params.PreDeblockSurfMmcState));
    #endif

        params.presReferences[CodechalDecodeLastRef]      = m_vp8BasicFeature->m_LastRefSurface;
        params.presReferences[CodechalDecodeGoldenRef]    = m_vp8BasicFeature->m_GoldenRefSurface;
        params.presReferences[CodechalDecodeAlternateRef] = m_vp8BasicFeature->m_AltRefSurface;

        params.presMfdIntraRowStoreScratchBuffer            = &(m_resMfdIntraRowStoreScratchBuffer->OsResource);
        params.presMfdDeblockingFilterRowStoreScratchBuffer = &(m_resMfdDeblockingFilterRowStoreScratchBuffer->OsResource);

        params.references = params.presReferences;

        if (m_vp8BasicFeature->m_streamOutEnabled)
        {
            params.presStreamOutBuffer = m_vp8BasicFeature->m_streamOutBuffer;
        }

    #ifdef _MMC_SUPPORTED
        DECODE_CHK_STATUS(vp8DecodeMemComp->CheckReferenceList(*m_vp8BasicFeature, params.PostDeblockSurfMmcState, params.PreDeblockSurfMmcState));
    #endif

        CODECHAL_DEBUG_TOOL(DumpResources(params));

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(MFX_IND_OBJ_BASE_ADDR_STATE, Vp8DecodePicPkt)
    {
        params.Mode            = m_vp8BasicFeature->m_mode;
        params.dwDataSize      = m_vp8BasicFeature->m_dataSize;
        params.dwDataOffset    = m_vp8BasicFeature->m_dataOffset;
        params.presDataBuffer  = & (m_vp8BasicFeature->m_resDataBuffer.OsResource);

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(MFX_BSP_BUF_BASE_ADDR_STATE, Vp8DecodePicPkt)
    {
        params.presBsdMpcRowStoreScratchBuffer = &(m_resBsdMpcRowStoreScratchBuffer->OsResource);
        params.presMprRowStoreScratchBuffer    = &(m_resMprRowStoreScratchBuffer->OsResource);

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(MFX_VP8_PIC_STATE, Vp8DecodePicPkt)
    {
        auto vp8PicParams                  = m_vp8BasicFeature->m_vp8PicParams;
        auto vp8IqMatrixParams             = m_vp8BasicFeature->m_vp8IqMatrixParams;
        params.presSegmentationIdStreamBuffer = &(m_resSegmentationIdStreamBuffer->OsResource);
        params.dwCoefProbTableOffset          = 0;
        if (m_vp8BasicFeature->m_bitstreamLockingInUse)
        {
            params.presCoefProbBuffer         = &(m_vp8BasicFeature->m_resCoefProbBufferInternal->OsResource);
        }
        else
        {
            params.presCoefProbBuffer         = &(m_vp8BasicFeature->m_resCoefProbBufferExternal);
        }

        params.FrameWidthMinus1 = vp8PicParams->wFrameWidthInMbsMinus1;
        params.FrameHeightMinus1 = vp8PicParams->wFrameHeightInMbsMinus1;
        params.McFilterSelect = (vp8PicParams->version != 0);
        params.ChromaFullPixelMcFilterMode = (vp8PicParams->version == 3);
        params.Dblkfiltertype = vp8PicParams->filter_type;
        params.Skeyframeflag = vp8PicParams->key_frame;
        params.SegmentationIdStreamoutEnable =
            (vp8PicParams->segmentation_enabled) && (vp8PicParams->update_mb_segmentation_map);
        params.SegmentationIdStreaminEnable =
            (vp8PicParams->segmentation_enabled) && !(vp8PicParams->update_mb_segmentation_map);
        params.SegmentEnableFlag = vp8PicParams->segmentation_enabled;
        params.UpdateMbsegmentMapFlag =
            (vp8PicParams->segmentation_enabled) ? vp8PicParams->update_mb_segmentation_map : 0;
        params.MbNocoeffSkipflag = vp8PicParams->mb_no_coeff_skip;
        params.ModeReferenceLoopFilterDeltaEnabled = vp8PicParams->loop_filter_adj_enable;
        params.GoldenRefPictureMvSignbiasFlag = vp8PicParams->sign_bias_golden;
        params.AlternateRefPicMvSignbiasFlag = vp8PicParams->sign_bias_alternate;
        params.DeblockSharpnessLevel = vp8PicParams->ucSharpnessLevel;
        params.DblkfilterlevelForSegment3 = vp8PicParams->ucLoopFilterLevel[3];
        params.DblkfilterlevelForSegment2 = vp8PicParams->ucLoopFilterLevel[2];
        params.DblkfilterlevelForSegment1 = vp8PicParams->ucLoopFilterLevel[1];
        params.DblkfilterlevelForSegment0 = vp8PicParams->ucLoopFilterLevel[0];

        uint32_t i = 0;
        uint32_t j = 0;
        params.QuantizerValue0Blocktype0Y1Dc = vp8IqMatrixParams->quantization_values[i][j];
        params.QuantizerValue0Blocktype1Y1Ac = vp8IqMatrixParams->quantization_values[i][j + 1];

        i = 0;
        j = 2;
        params.QuantizerValue0Blocktype2Uvdc = vp8IqMatrixParams->quantization_values[i][j];
        params.QuantizerValue0Blocktype3Uvac = vp8IqMatrixParams->quantization_values[i][j + 1];

        i = 0;
        j = 4;
        params.QuantizerValue0Blocktype4Y2Dc = vp8IqMatrixParams->quantization_values[i][j];
        params.QuantizerValue0Blocktype5Y2Ac = vp8IqMatrixParams->quantization_values[i][j + 1];

        i = 1;
        j = 0;
        params.QuantizerValue1Blocktype0Y1Dc = vp8IqMatrixParams->quantization_values[i][j];
        params.QuantizerValue1Blocktype1Y1Ac = vp8IqMatrixParams->quantization_values[i][j + 1];

        i = 1;
        j = 2;
        params.QuantizerValue1Blocktype2Uvdc = vp8IqMatrixParams->quantization_values[i][j];
        params.QuantizerValue1Blocktype3Uvac = vp8IqMatrixParams->quantization_values[i][j + 1];

        i = 1;
        j = 4;
        params.QuantizerValue1Blocktype4Y2Dc = vp8IqMatrixParams->quantization_values[i][j];
        params.QuantizerValue1Blocktype5Y2Ac = vp8IqMatrixParams->quantization_values[i][j + 1];

        i = 2;
        j = 0;
        params.QuantizerValue2Blocktype0Y1Dc = vp8IqMatrixParams->quantization_values[i][j];
        params.QuantizerValue2Blocktype1Y1Ac = vp8IqMatrixParams->quantization_values[i][j + 1];

        i = 2;
        j = 2;
        params.QuantizerValue2Blocktype2Uvdc = vp8IqMatrixParams->quantization_values[i][j];
        params.QuantizerValue2Blocktype3Uvac = vp8IqMatrixParams->quantization_values[i][j + 1];

        i = 2;
        j = 4;
        params.QuantizerValue2Blocktype4Y2Dc = vp8IqMatrixParams->quantization_values[i][j];
        params.QuantizerValue2Blocktype5Y2Ac = vp8IqMatrixParams->quantization_values[i][j + 1];

        i = 3;
        j = 0;
        params.QuantizerValue3Blocktype0Y1Dc = vp8IqMatrixParams->quantization_values[i][j];
        params.QuantizerValue3Blocktype1Y1Ac = vp8IqMatrixParams->quantization_values[i][j + 1];

        i = 3;
        j = 2;
        params.QuantizerValue3Blocktype2Uvdc = vp8IqMatrixParams->quantization_values[i][j];
        params.QuantizerValue3Blocktype3Uvac = vp8IqMatrixParams->quantization_values[i][j + 1];

        i = 3;
        j = 4;
        params.QuantizerValue3Blocktype4Y2Dc = vp8IqMatrixParams->quantization_values[i][j];
        params.QuantizerValue3Blocktype5Y2Ac = vp8IqMatrixParams->quantization_values[i][j + 1];

        params.Mbsegmentidtreeprobs2 = vp8PicParams->cMbSegmentTreeProbs[2];
        params.Mbsegmentidtreeprobs1 = vp8PicParams->cMbSegmentTreeProbs[1];
        params.Mbsegmentidtreeprobs0 = vp8PicParams->cMbSegmentTreeProbs[0];
        params.Mbnocoeffskipfalseprob = vp8PicParams->ucProbSkipFalse;
        params.Intrambprob = vp8PicParams->ucProbIntra;
        params.Interpredfromlastrefprob = vp8PicParams->ucProbLast;
        params.Interpredfromgrefrefprob = vp8PicParams->ucProbGolden;
        params.Ymodeprob3 = vp8PicParams->ucYModeProbs[3];
        params.Ymodeprob2 = vp8PicParams->ucYModeProbs[2];
        params.Ymodeprob1 = vp8PicParams->ucYModeProbs[1];
        params.Ymodeprob0 = vp8PicParams->ucYModeProbs[0];
        params.Uvmodeprob2 = vp8PicParams->ucUvModeProbs[2];
        params.Uvmodeprob1 = vp8PicParams->ucUvModeProbs[1];
        params.Uvmodeprob0 = vp8PicParams->ucUvModeProbs[0];

        i = 0;
        j = 0;
        params.Mvupdateprobs00 = vp8PicParams->ucMvUpdateProb[i][j];
        params.Mvupdateprobs01 = vp8PicParams->ucMvUpdateProb[i][j + 1];
        params.Mvupdateprobs02 = vp8PicParams->ucMvUpdateProb[i][j + 2];
        params.Mvupdateprobs03 = vp8PicParams->ucMvUpdateProb[i][j + 3];

        i = 0;
        j = 4;
        params.Mvupdateprobs04 = vp8PicParams->ucMvUpdateProb[i][j];
        params.Mvupdateprobs05 = vp8PicParams->ucMvUpdateProb[i][j + 1];
        params.Mvupdateprobs06 = vp8PicParams->ucMvUpdateProb[i][j + 2];
        params.Mvupdateprobs07 = vp8PicParams->ucMvUpdateProb[i][j + 3];

        i = 0;
        j = 8;
        params.Mvupdateprobs08 = vp8PicParams->ucMvUpdateProb[i][j];
        params.Mvupdateprobs09 = vp8PicParams->ucMvUpdateProb[i][j + 1];
        params.Mvupdateprobs010 = vp8PicParams->ucMvUpdateProb[i][j + 2];
        params.Mvupdateprobs011 = vp8PicParams->ucMvUpdateProb[i][j + 3];

        i = 0;
        j = 12;
        params.Mvupdateprobs012 = vp8PicParams->ucMvUpdateProb[i][j];
        params.Mvupdateprobs013 = vp8PicParams->ucMvUpdateProb[i][j + 1];
        params.Mvupdateprobs014 = vp8PicParams->ucMvUpdateProb[i][j + 2];
        params.Mvupdateprobs015 = vp8PicParams->ucMvUpdateProb[i][j + 3];

        i = 0;
        j = 16;
        params.Mvupdateprobs016 = vp8PicParams->ucMvUpdateProb[i][j];
        params.Mvupdateprobs017 = vp8PicParams->ucMvUpdateProb[i][j + 1];
        params.Mvupdateprobs018 = vp8PicParams->ucMvUpdateProb[i][j + 2];

        i = 1;
        j = 0;
        params.Mvupdateprobs10 = vp8PicParams->ucMvUpdateProb[i][j];
        params.Mvupdateprobs11 = vp8PicParams->ucMvUpdateProb[i][j + 1];
        params.Mvupdateprobs12 = vp8PicParams->ucMvUpdateProb[i][j + 2];
        params.Mvupdateprobs13 = vp8PicParams->ucMvUpdateProb[i][j + 3];

        i = 1;
        j = 4;
        params.Mvupdateprobs14 = vp8PicParams->ucMvUpdateProb[i][j];
        params.Mvupdateprobs15 = vp8PicParams->ucMvUpdateProb[i][j + 1];
        params.Mvupdateprobs16 = vp8PicParams->ucMvUpdateProb[i][j + 2];
        params.Mvupdateprobs17 = vp8PicParams->ucMvUpdateProb[i][j + 3];

        i = 1;
        j = 8;
        params.Mvupdateprobs18 = vp8PicParams->ucMvUpdateProb[i][j];
        params.Mvupdateprobs19 = vp8PicParams->ucMvUpdateProb[i][j + 1];
        params.Mvupdateprobs110 = vp8PicParams->ucMvUpdateProb[i][j + 2];
        params.Mvupdateprobs111 = vp8PicParams->ucMvUpdateProb[i][j + 3];

        i = 1;
        j = 12;
        params.Mvupdateprobs112 = vp8PicParams->ucMvUpdateProb[i][j];
        params.Mvupdateprobs113 = vp8PicParams->ucMvUpdateProb[i][j + 1];
        params.Mvupdateprobs114 = vp8PicParams->ucMvUpdateProb[i][j + 2];
        params.Mvupdateprobs115 = vp8PicParams->ucMvUpdateProb[i][j + 3];

        i = 1;
        j = 16;
        params.Mvupdateprobs116 = vp8PicParams->ucMvUpdateProb[i][j];
        params.Mvupdateprobs117 = vp8PicParams->ucMvUpdateProb[i][j + 1];
        params.Mvupdateprobs118 = vp8PicParams->ucMvUpdateProb[i][j + 2];

        params.Reflfdelta0ForIntraFrame = vp8PicParams->cRefLfDelta[0];
        params.Reflfdelta1ForLastFrame = vp8PicParams->cRefLfDelta[1];
        params.Reflfdelta2ForGoldenFrame = vp8PicParams->cRefLfDelta[2];
        params.Reflfdelta3ForAltrefFrame = vp8PicParams->cRefLfDelta[3];
        params.Modelfdelta0ForBPredMode = vp8PicParams->cModeLfDelta[0];
        params.Modelfdelta1ForZeromvMode = vp8PicParams->cModeLfDelta[1];
        params.Modelfdelta2ForNearestNearAndNewMode = vp8PicParams->cModeLfDelta[2];
        params.Modelfdelta3ForSplitmvMode = vp8PicParams->cModeLfDelta[3];

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Vp8DecodePicPkt::AddAllCmds_MFX_PIPE_MODE_SELECT(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        auto &par = m_miItf->GETPAR_MFX_WAIT();
        par.iStallVdboxPipeline = true;
        //for gen 12, we need to add MFX wait for both KIN and VRT before and after MFX Pipemode select.
        MHW_MI_CHK_STATUS(m_miItf->ADDCMD_MFX_WAIT(&cmdBuffer, nullptr));

        DECODE_CHK_NULL(m_mfxItf);
        SETPAR_AND_ADDCMD(MFX_PIPE_MODE_SELECT, m_mfxItf, &cmdBuffer);

        //for gen 12, we need to add MFX wait for both KIN and VRT before and after MFX Pipemode select.
        MHW_MI_CHK_STATUS(m_miItf->ADDCMD_MFX_WAIT(&cmdBuffer, nullptr));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Vp8DecodePicPkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
    {
        DECODE_FUNC_CALL();

        commandBufferSize      = m_pictureStatesSize;
        requestedPatchListSize = m_picturePatchListSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Vp8DecodePicPkt::AddMiForceWakeupCmd(MOS_COMMAND_BUFFER& cmdBuffer)
    {
        DECODE_FUNC_CALL();

        auto &par = m_miItf->GETPAR_MI_FORCE_WAKEUP();
        MOS_ZeroMemory(&par, sizeof(par));
        par.bMFXPowerWellControl      = true;
        par.bMFXPowerWellControlMask  = true;
        par.bHEVCPowerWellControl     = false;
        par.bHEVCPowerWellControlMask = true;

        DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_FORCE_WAKEUP(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS Vp8DecodePicPkt::DumpResources(MFX_PIPE_BUF_ADDR_STATE_PAR &pipeBufAddrParams) const
    {
        DECODE_FUNC_CALL();

        CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
        DECODE_CHK_NULL(debugInterface);
        for (uint16_t n = 0; n <= CodechalDecodeAlternateRef; n++)
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

        return MOS_STATUS_SUCCESS;
    }

#endif

}  // namespace decode

