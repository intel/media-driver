/*
* Copyright (c) 2019-2024, Intel Corporation
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
//! \file     decode_av1_picture_packet_g12_base.cpp
//! \brief    Defines the interface for g12 av1 decode picture packet
//!
#include "codechal_utilities.h"
#include "decode_av1_picture_packet_g12_base.h"
#include "codechal_debug.h"
#include "decode_resource_auto_lock.h"

namespace decode{
    Av1DecodePicPkt_G12_Base::~Av1DecodePicPkt_G12_Base()
    {
        FreeResources();
    }

    MOS_STATUS Av1DecodePicPkt_G12_Base::FreeResources()
    {
        DECODE_FUNC_CALL();

        if (m_allocator != nullptr)
        {
             m_allocator->Destroy(m_intrabcDecodedOutputFrameBuffer);
            if (!m_avpInterface->IsBtdlRowstoreCacheEnabled())
            {
                m_allocator->Destroy(m_bitstreamDecoderEncoderLineRowstoreReadWriteBuffer);
            }
            m_allocator->Destroy(m_bitstreamDecoderEncoderTileLineRowstoreReadWriteBuffer);
            if (!m_avpInterface->IsIpdlRowstoreCacheEnabled())
            {
                m_allocator->Destroy(m_intraPredictionLineRowstoreReadWriteBuffer);
            }
            m_allocator->Destroy(m_intraPredictionTileLineRowstoreReadWriteBuffer);
            if (!m_avpInterface->IsSmvlRowstoreCacheEnabled())
            {
                m_allocator->Destroy(m_spatialMotionVectorLineReadWriteBuffer);
            }
            m_allocator->Destroy(m_spatialMotionVectorCodingTileLineReadWriteBuffer);
            m_allocator->Destroy(m_loopRestorationMetaTileColumnReadWriteBuffer);
            m_allocator->Destroy(m_loopRestorationFilterTileReadWriteLineYBuffer);
            m_allocator->Destroy(m_loopRestorationFilterTileReadWriteLineUBuffer);
            m_allocator->Destroy(m_loopRestorationFilterTileReadWriteLineVBuffer);
            if (!m_avpInterface->IsDflyRowstoreCacheEnabled())
            {
                m_allocator->Destroy(m_deblockerFilterLineReadWriteYBuffer);
            }
            if (!m_avpInterface->IsDfluRowstoreCacheEnabled())
            {
                m_allocator->Destroy(m_deblockerFilterLineReadWriteUBuffer);
            }
            if (!m_avpInterface->IsDflvRowstoreCacheEnabled())
            {
                m_allocator->Destroy(m_deblockerFilterLineReadWriteVBuffer);
            }
            m_allocator->Destroy(m_deblockerFilterTileLineReadWriteYBuffer);
            m_allocator->Destroy(m_deblockerFilterTileLineReadWriteVBuffer);
            m_allocator->Destroy(m_deblockerFilterTileLineReadWriteUBuffer);
            m_allocator->Destroy(m_deblockerFilterTileColumnReadWriteYBuffer);
            m_allocator->Destroy(m_deblockerFilterTileColumnReadWriteUBuffer);
            m_allocator->Destroy(m_deblockerFilterTileColumnReadWriteVBuffer);
            if (!m_avpInterface->IsCdefRowstoreCacheEnabled())
            {
                m_allocator->Destroy(m_cdefFilterLineReadWriteBuffer);
            }
            m_allocator->Destroy(m_cdefFilterTileLineReadWriteBuffer);
            m_allocator->Destroy(m_cdefFilterTileColumnReadWriteBuffer);
            m_allocator->Destroy(m_cdefFilterMetaTileLineReadWriteBuffer);
            m_allocator->Destroy(m_cdefFilterMetaTileColumnReadWriteBuffer);
            m_allocator->Destroy(m_cdefFilterTopLeftCornerReadWriteBuffer);
            m_allocator->Destroy(m_superResTileColumnReadWriteYBuffer);
            m_allocator->Destroy(m_superResTileColumnReadWriteUBuffer);
            m_allocator->Destroy(m_superResTileColumnReadWriteVBuffer);
            m_allocator->Destroy(m_loopRestorationFilterTileColumnReadWriteYBuffer);
            m_allocator->Destroy(m_loopRestorationFilterTileColumnReadWriteUBuffer);
            m_allocator->Destroy(m_loopRestorationFilterTileColumnReadWriteVBuffer);
            m_allocator->Destroy(m_decodedFrameStatusErrorBuffer);
            m_allocator->Destroy(m_decodedBlockDataStreamoutBuffer);
            m_allocator->Destroy(m_curMvBufferForDummyWL);
            m_allocator->Destroy(m_bwdAdaptCdfBufForDummyWL);
            m_allocator->Destroy(m_resDataBufferForDummyWL);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt_G12_Base::Init()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_featureManager);
        DECODE_CHK_NULL(m_hwInterface);
        DECODE_CHK_NULL(m_osInterface);
        DECODE_CHK_NULL(m_miInterface);
        DECODE_CHK_NULL(m_av1Pipeline);
        DECODE_CHK_NULL(m_avpInterface);

        m_av1BasicFeature = dynamic_cast<Av1BasicFeatureG12*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        DECODE_CHK_NULL(m_av1BasicFeature);

        m_allocator = m_pipeline ->GetDecodeAllocator();
        DECODE_CHK_NULL(m_allocator);

        DECODE_CHK_STATUS(AllocateFixedResources());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt_G12_Base::Prepare()
    {
        DECODE_FUNC_CALL();

        m_av1PicParams      = m_av1BasicFeature->m_av1PicParams;

        if (m_av1PicParams->m_seqInfoFlags.m_fields.m_subsamplingX == 1 && m_av1PicParams->m_seqInfoFlags.m_fields.m_subsamplingY == 1)
        {
            chromaSamplingFormat = HCP_CHROMA_FORMAT_YUV420;//Use HCP definitions here, since AVP and HCP will merge together in the future
        }
        else if (m_av1PicParams->m_seqInfoFlags.m_fields.m_subsamplingX == 0 && m_av1PicParams->m_seqInfoFlags.m_fields.m_subsamplingY == 0)
        {
            chromaSamplingFormat = HCP_CHROMA_FORMAT_YUV444;
        }
        else
        {
            DECODE_ASSERTMESSAGE("Invalid Chroma sampling format!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

#ifdef _MMC_SUPPORTED
        m_mmcState = m_av1Pipeline->GetMmcState();
        DECODE_CHK_NULL(m_mmcState);
#endif

        DECODE_CHK_STATUS(SetRowstoreCachingOffsets());

        DECODE_CHK_STATUS(AllocateVariableResources());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt_G12_Base::SetRowstoreCachingOffsets()
    {
        if (m_avpInterface->IsRowStoreCachingSupported() &&
            (m_av1BasicFeature->m_frameWidthAlignedMinBlk != MOS_ALIGN_CEIL(m_prevFrmWidth, av1MinBlockWidth)))
        {
            MHW_VDBOX_ROWSTORE_PARAMS rowstoreParams;
            MOS_ZeroMemory(&rowstoreParams, sizeof(rowstoreParams));
            rowstoreParams.dwPicWidth       = m_av1BasicFeature->m_frameWidthAlignedMinBlk;
            rowstoreParams.bMbaff           = false;
            rowstoreParams.Mode             = CODECHAL_DECODE_MODE_AV1VLD;
            rowstoreParams.ucBitDepthMinus8 = m_av1PicParams->m_bitDepthIdx << 1;
            rowstoreParams.ucChromaFormat   = m_av1BasicFeature->m_chromaFormat;
            DECODE_CHK_STATUS(m_hwInterface->SetRowstoreCachingOffsets(&rowstoreParams));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt_G12_Base::AllocateFixedResources()
    {
        DECODE_FUNC_CALL();

        if (m_av1BasicFeature->m_usingDummyWl == true)
        {
            MhwVdboxAvpBufferSizeParams avpBufSizeParam;
            MOS_ZeroMemory(&avpBufSizeParam, sizeof(avpBufSizeParam));

            avpBufSizeParam.m_bitDepthIdc       = 0;
            avpBufSizeParam.m_picWidth          = 1;
            avpBufSizeParam.m_picHeight         = 1;
            avpBufSizeParam.m_tileWidth         = 16;
            avpBufSizeParam.m_isSb128x128       = false;
            avpBufSizeParam.m_curFrameTileNum   = 1;
            avpBufSizeParam.m_numTileCol        = 1;

            if (m_avpInterface->GetAv1BufferSize(mvTemporalBuf,
                                                &avpBufSizeParam) != MOS_STATUS_SUCCESS)
            {
                DECODE_ASSERTMESSAGE( "Failed to get MvTemporalBuffer size.");
            }
            m_curMvBufferForDummyWL = m_allocator->AllocateBuffer(avpBufSizeParam.m_bufferSize, "MvBuffer",
                resourceInternalReadWriteCache, notLockableVideoMem);
            DECODE_CHK_NULL(m_curMvBufferForDummyWL);

            m_bwdAdaptCdfBufForDummyWL = m_allocator->AllocateBuffer(
                MOS_ALIGN_CEIL(m_av1BasicFeature->m_cdfMaxNumBytes, CODECHAL_PAGE_SIZE), "CdfTableBuffer",
                resourceInternalReadWriteCache, notLockableVideoMem);
            DECODE_CHK_NULL(m_bwdAdaptCdfBufForDummyWL);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt_G12_Base::AllocateVariableResources()
    {
        DECODE_FUNC_CALL();

        int32_t mibSizeLog2 = m_av1PicParams->m_seqInfoFlags.m_fields.m_use128x128Superblock ? av1MaxMibSizeLog2 : av1MinMibSizeLog2;
        int32_t miCols = MOS_ALIGN_CEIL(m_av1PicParams->m_superResUpscaledWidthMinus1 + 1, 8) >> av1MiSizeLog2;
        int32_t miRows = MOS_ALIGN_CEIL(m_av1PicParams->m_superResUpscaledHeightMinus1 + 1, 8) >> av1MiSizeLog2;
        miCols = MOS_ALIGN_CEIL(miCols, 1 << mibSizeLog2);
        miRows = MOS_ALIGN_CEIL(miRows, 1 << mibSizeLog2);

        m_widthInSb = miCols >> mibSizeLog2;
        m_heightInSb = miRows >> mibSizeLog2;
        uint32_t maxTileWidthInSb = MOS_ROUNDUP_DIVIDE(4096, 1 << (mibSizeLog2 + av1MiSizeLog2));
        MhwVdboxAvpBufferSizeParams avpBufSizeParam;
        MOS_ZeroMemory(&avpBufSizeParam, sizeof(avpBufSizeParam));

        avpBufSizeParam.m_bitDepthIdc       = m_av1BasicFeature->m_av1DepthIndicator;
        avpBufSizeParam.m_picWidth          = m_widthInSb;
        avpBufSizeParam.m_picHeight         = m_heightInSb;
        avpBufSizeParam.m_tileWidth         = maxTileWidthInSb;
        avpBufSizeParam.m_isSb128x128       = m_av1PicParams->m_seqInfoFlags.m_fields.m_use128x128Superblock ? true : false;
        avpBufSizeParam.m_curFrameTileNum   = m_av1PicParams->m_tileCols * m_av1PicParams->m_tileRows;
        avpBufSizeParam.m_numTileCol        = m_av1PicParams->m_tileCols;

        // Intrabc Decoded Output Frame Buffer
        if (m_av1PicParams->m_picInfoFlags.m_fields.m_allowIntrabc)
        {
            MOS_SURFACE m_destSurface = m_av1BasicFeature->m_destSurface;
            if (m_intrabcDecodedOutputFrameBuffer == nullptr)
            {
                PMOS_SURFACE surface = nullptr;
                surface = m_allocator->AllocateSurface(
                    m_destSurface.dwWidth,
                    MOS_ALIGN_CEIL(m_destSurface.dwHeight, 8),
                    "Intrabc Decoded Output Frame Buffer",
                    m_destSurface.Format,
                    m_destSurface.bCompressible,
                    resourceInternalReadWriteNoCache,
                    notLockableVideoMem);

                m_intrabcDecodedOutputFrameBuffer = surface;
                DECODE_CHK_NULL(m_intrabcDecodedOutputFrameBuffer);
            }
            else
            {
                DECODE_CHK_STATUS(m_allocator->Resize(
                    m_intrabcDecodedOutputFrameBuffer,
                    m_destSurface.dwWidth,
                    MOS_ALIGN_CEIL(m_destSurface.dwHeight, 8),
                    notLockableVideoMem));
            }
        }

        // Bitstream decode line rowstore buffer
        if (!m_avpInterface->IsBtdlRowstoreCacheEnabled())
        {
            DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
                bsdLineBuf,
                &avpBufSizeParam));
            if (m_bitstreamDecoderEncoderLineRowstoreReadWriteBuffer == nullptr)
            {
                m_bitstreamDecoderEncoderLineRowstoreReadWriteBuffer = m_allocator->AllocateBuffer(
                    avpBufSizeParam.m_bufferSize,
                    "BitstreamDecodeLineBuffer",
                    resourceInternalReadWriteCache,
                    notLockableVideoMem);
                DECODE_CHK_NULL(m_bitstreamDecoderEncoderLineRowstoreReadWriteBuffer);
            }
            else
            {
                DECODE_CHK_STATUS(m_allocator->Resize(
                    m_bitstreamDecoderEncoderLineRowstoreReadWriteBuffer,
                    avpBufSizeParam.m_bufferSize,
                    notLockableVideoMem));
            }
        }

        // Bitstream decode tile line buffer
        DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
            bsdTileLineBuf,
            &avpBufSizeParam));
        if (m_bitstreamDecoderEncoderTileLineRowstoreReadWriteBuffer == nullptr)
        {
            m_bitstreamDecoderEncoderTileLineRowstoreReadWriteBuffer = m_allocator->AllocateBuffer(
                avpBufSizeParam.m_bufferSize,
                "BitstreamDecodeTileLineBuffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_bitstreamDecoderEncoderTileLineRowstoreReadWriteBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_bitstreamDecoderEncoderTileLineRowstoreReadWriteBuffer,
                avpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }

        // Intra Prediction Line Rowstore Read/Write Buffer
        if (!m_avpInterface->IsIpdlRowstoreCacheEnabled())
        {
            DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
                intraPredLine,
                &avpBufSizeParam));
            if (m_intraPredictionLineRowstoreReadWriteBuffer == nullptr)
            {
                m_intraPredictionLineRowstoreReadWriteBuffer = m_allocator->AllocateBuffer(
                    avpBufSizeParam.m_bufferSize,
                    "intraPredictionLineRowstoreBuffer",
                    resourceInternalReadWriteCache,
                    notLockableVideoMem);
                DECODE_CHK_NULL(m_intraPredictionLineRowstoreReadWriteBuffer);
            }
            else
            {
                DECODE_CHK_STATUS(m_allocator->Resize(
                    m_intraPredictionLineRowstoreReadWriteBuffer,
                    avpBufSizeParam.m_bufferSize,
                    notLockableVideoMem));
            }
        }

        // Intra Prediction Tile Line Rowstore Read/Write Buffer
        DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
            intraPredTileLine,
            &avpBufSizeParam));
        if (m_intraPredictionTileLineRowstoreReadWriteBuffer == nullptr)
        {
            m_intraPredictionTileLineRowstoreReadWriteBuffer = m_allocator->AllocateBuffer(
                avpBufSizeParam.m_bufferSize,
                "intraPredictionTileLineRowstoreBuffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_intraPredictionTileLineRowstoreReadWriteBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_intraPredictionTileLineRowstoreReadWriteBuffer,
                avpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }

        // Spatial motion vector Line rowstore buffer
        if (!m_avpInterface->IsSmvlRowstoreCacheEnabled())
        {
            DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
                spatialMvLineBuf,
                &avpBufSizeParam));
            if (m_spatialMotionVectorLineReadWriteBuffer == nullptr)
            {
                m_spatialMotionVectorLineReadWriteBuffer = m_allocator->AllocateBuffer(
                    avpBufSizeParam.m_bufferSize,
                    "SpatialMotionVectorLineRowstoreBuffer",
                    resourceInternalReadWriteCache,
                    notLockableVideoMem);
                DECODE_CHK_NULL(m_spatialMotionVectorLineReadWriteBuffer);
            }
            else
            {
                DECODE_CHK_STATUS(m_allocator->Resize(
                    m_spatialMotionVectorLineReadWriteBuffer,
                    avpBufSizeParam.m_bufferSize,
                    notLockableVideoMem));
            }
        }

        // Spatial motion vector Tile Line Buffer
        DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
            spatialMvTileLineBuf,
            &avpBufSizeParam));

        if (m_spatialMotionVectorCodingTileLineReadWriteBuffer == nullptr)
        {
            m_spatialMotionVectorCodingTileLineReadWriteBuffer = m_allocator->AllocateBuffer(
                avpBufSizeParam.m_bufferSize,
                "SpatialMotionVectorTileLineBuffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_spatialMotionVectorCodingTileLineReadWriteBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_spatialMotionVectorCodingTileLineReadWriteBuffer,
                avpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }

        // Loop Restoration Meta Tile Column Read/Write Buffer
        DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
            lrMetaTileCol,
            &avpBufSizeParam));
        if (m_loopRestorationMetaTileColumnReadWriteBuffer == nullptr)
        {
            m_loopRestorationMetaTileColumnReadWriteBuffer = m_allocator->AllocateBuffer(
                avpBufSizeParam.m_bufferSize,
                "LoopRestorationMetaTileColumnReadWriteBuffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_loopRestorationMetaTileColumnReadWriteBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_loopRestorationMetaTileColumnReadWriteBuffer,
                avpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }

        // Loop Restoration Filter Tile Read/Write Line Y Buffer
        DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
            lrTileLineY,
            &avpBufSizeParam));
        if (m_loopRestorationFilterTileReadWriteLineYBuffer == nullptr)
        {
            m_loopRestorationFilterTileReadWriteLineYBuffer = m_allocator->AllocateBuffer(
                avpBufSizeParam.m_bufferSize,
                "LoopRestorationFilterTileReadWriteLineYBuffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_loopRestorationFilterTileReadWriteLineYBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_loopRestorationFilterTileReadWriteLineYBuffer,
                avpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }

        //Loop Restoration Filter Tile Read/Write Line U Buffer
        DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
            lrTileLineU,
            &avpBufSizeParam));
        if (m_loopRestorationFilterTileReadWriteLineUBuffer == nullptr)
        {
            m_loopRestorationFilterTileReadWriteLineUBuffer = m_allocator->AllocateBuffer(
                avpBufSizeParam.m_bufferSize,
                "LoopRestorationFilterTileReadWriteLineUBuffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_loopRestorationFilterTileReadWriteLineUBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_loopRestorationFilterTileReadWriteLineUBuffer,
                avpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }

        // Loop Restoration Filter Tile Read/Write Line V Buffer
        DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
            lrTileLineV,
            &avpBufSizeParam));
        if (m_loopRestorationFilterTileReadWriteLineVBuffer == nullptr)
        {
            m_loopRestorationFilterTileReadWriteLineVBuffer = m_allocator->AllocateBuffer(
                avpBufSizeParam.m_bufferSize,
                "LoopRestorationFilterTileReadWriteLineVBuffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_loopRestorationFilterTileReadWriteLineVBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_loopRestorationFilterTileReadWriteLineVBuffer,
                avpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }

        if (!m_avpInterface->IsDflyRowstoreCacheEnabled())
        {
            DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
                deblockLineYBuf,
                &avpBufSizeParam));

            if (m_deblockerFilterLineReadWriteYBuffer == nullptr)
            {
                m_deblockerFilterLineReadWriteYBuffer = m_allocator->AllocateBuffer(
                    avpBufSizeParam.m_bufferSize,
                    "DeblockerFilterLineReadWriteYBuffer",
                    resourceInternalReadWriteCache,
                    notLockableVideoMem);
                DECODE_CHK_NULL(m_deblockerFilterLineReadWriteYBuffer);
            }
            else
            {
                DECODE_CHK_STATUS(m_allocator->Resize(
                    m_deblockerFilterLineReadWriteYBuffer,
                    avpBufSizeParam.m_bufferSize,
                    notLockableVideoMem));
            }
        }

        if (!m_avpInterface->IsDfluRowstoreCacheEnabled())
        {
            DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
                deblockLineUBuf,
                &avpBufSizeParam));
            if (m_deblockerFilterLineReadWriteUBuffer == nullptr)
            {
                m_deblockerFilterLineReadWriteUBuffer = m_allocator->AllocateBuffer(
                    avpBufSizeParam.m_bufferSize,
                    "DeblockerFilterLineReadWriteUBuffer",
                    resourceInternalReadWriteCache,
                    notLockableVideoMem);
                DECODE_CHK_NULL(m_deblockerFilterLineReadWriteUBuffer);
            }
            else
            {
                DECODE_CHK_STATUS(m_allocator->Resize(
                    m_deblockerFilterLineReadWriteUBuffer,
                    avpBufSizeParam.m_bufferSize,
                    notLockableVideoMem));
            }
        }

        if (!m_avpInterface->IsDflvRowstoreCacheEnabled())
        {
            DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
                deblockLineVBuf,
                &avpBufSizeParam));
            if (m_deblockerFilterLineReadWriteVBuffer == nullptr)
            {
                m_deblockerFilterLineReadWriteVBuffer = m_allocator->AllocateBuffer(
                    avpBufSizeParam.m_bufferSize,
                    "DeblockerFilterLineReadWriteVBuffer",
                    resourceInternalReadWriteCache,
                    notLockableVideoMem);
                DECODE_CHK_NULL(m_deblockerFilterLineReadWriteVBuffer);
            }
            else
            {
                DECODE_CHK_STATUS(m_allocator->Resize(
                    m_deblockerFilterLineReadWriteVBuffer,
                    avpBufSizeParam.m_bufferSize,
                    notLockableVideoMem));
            }
        }

        // Deblocking Filter Tile Line Y Buffer
        DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
            deblockTileLineYBuf,
            &avpBufSizeParam));

        if (m_deblockerFilterTileLineReadWriteYBuffer == nullptr)
        {
            m_deblockerFilterTileLineReadWriteYBuffer = m_allocator->AllocateBuffer(
                avpBufSizeParam.m_bufferSize,
                "DeblockerFilterTileLineReadWriteYBuffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_deblockerFilterTileLineReadWriteYBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_deblockerFilterTileLineReadWriteYBuffer,
                avpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }

        // Deblocking Filter Tile Line V Buffer
        DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
            deblockTileLineVBuf,
            &avpBufSizeParam));

        if (m_deblockerFilterTileLineReadWriteVBuffer == nullptr)
        {
            m_deblockerFilterTileLineReadWriteVBuffer = m_allocator->AllocateBuffer(
                avpBufSizeParam.m_bufferSize,
                "DeblockerFilterTileLineReadWriteVBuffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_deblockerFilterTileLineReadWriteVBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_deblockerFilterTileLineReadWriteVBuffer,
                avpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }

        // Deblocking Filter Tile Line U Buffer
        DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
            deblockTileLineUBuf,
            &avpBufSizeParam));

        if (m_deblockerFilterTileLineReadWriteUBuffer == nullptr)
        {
            m_deblockerFilterTileLineReadWriteUBuffer = m_allocator->AllocateBuffer(
                avpBufSizeParam.m_bufferSize,
                "DeblockerFilterTileLineReadWriteUBuffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_deblockerFilterTileLineReadWriteUBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_deblockerFilterTileLineReadWriteUBuffer,
                avpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }

        // Deblocking Filter Tile Column Y Buffer
        DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
            deblockTileColYBuf,
            &avpBufSizeParam));

        if (m_deblockerFilterTileColumnReadWriteYBuffer == nullptr)
        {
            m_deblockerFilterTileColumnReadWriteYBuffer = m_allocator->AllocateBuffer(
                avpBufSizeParam.m_bufferSize,
                "DeblockerFilterTileColumnReadWriteYBuffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_deblockerFilterTileColumnReadWriteYBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_deblockerFilterTileColumnReadWriteYBuffer,
                avpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }

        // Deblocking Filter Tile Column U Buffer
        DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
            deblockTileColUBuf,
            &avpBufSizeParam));

        if (m_deblockerFilterTileColumnReadWriteUBuffer == nullptr)
        {
            m_deblockerFilterTileColumnReadWriteUBuffer = m_allocator->AllocateBuffer(
                avpBufSizeParam.m_bufferSize,
                "DeblockerFilterTileColumnReadWriteUBuffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_deblockerFilterTileColumnReadWriteUBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_deblockerFilterTileColumnReadWriteUBuffer,
                avpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }

        // Deblocking Filter Tile Column V Buffer
        DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
            deblockTileColVBuf,
            &avpBufSizeParam));

        if (m_deblockerFilterTileColumnReadWriteVBuffer == nullptr)
        {
            m_deblockerFilterTileColumnReadWriteVBuffer = m_allocator->AllocateBuffer(
                avpBufSizeParam.m_bufferSize,
                "DeblockerFilterTileColumnReadWriteVBuffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_deblockerFilterTileColumnReadWriteVBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_deblockerFilterTileColumnReadWriteVBuffer,
                avpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }

        // CDEF Filter Line Read/Write Buffer
        if (!m_avpInterface->IsCdefRowstoreCacheEnabled())
        {
            DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
                cdefLineBuf,
                &avpBufSizeParam));
            if (m_cdefFilterLineReadWriteBuffer == nullptr)
            {
                m_cdefFilterLineReadWriteBuffer = m_allocator->AllocateBuffer(
                    avpBufSizeParam.m_bufferSize,
                    "CdefFilterLineReadWriteBuffer",
                    resourceInternalReadWriteCache,
                    notLockableVideoMem);
                DECODE_CHK_NULL(m_cdefFilterLineReadWriteBuffer);
            }
            else
            {
                DECODE_CHK_STATUS(m_allocator->Resize(
                    m_cdefFilterLineReadWriteBuffer,
                    avpBufSizeParam.m_bufferSize,
                    notLockableVideoMem));
            }
        }

        // CDEF Filter Tile Line Read/Write Buffer
        DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
            cdefTileLineBuf,
            &avpBufSizeParam));
        if (m_cdefFilterTileLineReadWriteBuffer == nullptr)
        {
            m_cdefFilterTileLineReadWriteBuffer = m_allocator->AllocateBuffer(
                avpBufSizeParam.m_bufferSize,
                "CdefFilterTileLineReadWriteBuffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_cdefFilterTileLineReadWriteBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_cdefFilterTileLineReadWriteBuffer,
                avpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }

        // CDEF Filter Tile Column Read/Write Buffer
        DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
            cdefTileColBuf,
            &avpBufSizeParam));
        if (m_cdefFilterTileColumnReadWriteBuffer == nullptr)
        {
            m_cdefFilterTileColumnReadWriteBuffer = m_allocator->AllocateBuffer(
                avpBufSizeParam.m_bufferSize,
                "CdefFilterTileColumnReadWriteBuffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_cdefFilterTileColumnReadWriteBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_cdefFilterTileColumnReadWriteBuffer,
                avpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }

        // CDEF Filter Meta Tile Line Read Write Buffer
        DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
            cdefMetaTileLine,
            &avpBufSizeParam));
        if (m_cdefFilterMetaTileLineReadWriteBuffer == nullptr)
        {
            m_cdefFilterMetaTileLineReadWriteBuffer = m_allocator->AllocateBuffer(
                avpBufSizeParam.m_bufferSize,
                "CdefFilterMetaTileLineReadWriteBuffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_cdefFilterMetaTileLineReadWriteBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_cdefFilterMetaTileLineReadWriteBuffer,
                avpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }

        // CDEF Filter Meta Tile Column Read Write Buffer
        DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
            cdefMetaTileCol,
            &avpBufSizeParam));
        if (m_cdefFilterMetaTileColumnReadWriteBuffer == nullptr)
        {
            m_cdefFilterMetaTileColumnReadWriteBuffer = m_allocator->AllocateBuffer(
                avpBufSizeParam.m_bufferSize,
                "CdefFilterMetaTileColumnReadWriteBuffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_cdefFilterMetaTileColumnReadWriteBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_cdefFilterMetaTileColumnReadWriteBuffer,
                avpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }

        // Cdef Filter Top Left Corner Buffer
        DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
            cdefTopLeftCornerBuf,
            &avpBufSizeParam));

        if (m_cdefFilterTopLeftCornerReadWriteBuffer == nullptr)
        {
            m_cdefFilterTopLeftCornerReadWriteBuffer = m_allocator->AllocateBuffer(
                avpBufSizeParam.m_bufferSize,
                "CdefFilterTopLeftCornerReadWriteBuffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_cdefFilterTopLeftCornerReadWriteBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_cdefFilterTopLeftCornerReadWriteBuffer,
                avpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }

        // Super-Res Tile Column Y Buffer
        DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
            superResTileColYBuf,
            &avpBufSizeParam));
        if (m_superResTileColumnReadWriteYBuffer == nullptr)
        {
            m_superResTileColumnReadWriteYBuffer = m_allocator->AllocateBuffer(
                avpBufSizeParam.m_bufferSize,
                "SuperResTileColumnReadWriteYBuffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_superResTileColumnReadWriteYBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_superResTileColumnReadWriteYBuffer,
                avpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }

        // Super-Res Tile Column U Buffer
        DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
            superResTileColUBuf,
            &avpBufSizeParam));

        if (m_superResTileColumnReadWriteUBuffer == nullptr)
        {
            m_superResTileColumnReadWriteUBuffer = m_allocator->AllocateBuffer(
                avpBufSizeParam.m_bufferSize,
                "SuperResTileColumnReadWriteUBuffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_superResTileColumnReadWriteUBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_superResTileColumnReadWriteUBuffer,
                avpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }

        // Super-Res Tile Column V Buffer
        DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
            superResTileColVBuf,
            &avpBufSizeParam));

        if (m_superResTileColumnReadWriteVBuffer == nullptr)
        {
            m_superResTileColumnReadWriteVBuffer = m_allocator->AllocateBuffer(
                avpBufSizeParam.m_bufferSize,
                "SuperResTileColumnReadWriteVBuffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_superResTileColumnReadWriteVBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_superResTileColumnReadWriteVBuffer,
                avpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }

        // Loop Restoration Filter Tile Column Y Buffer
        DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
            lrTileColYBuf,
            &avpBufSizeParam));

        if (m_loopRestorationFilterTileColumnReadWriteYBuffer == nullptr)
        {
            m_loopRestorationFilterTileColumnReadWriteYBuffer = m_allocator->AllocateBuffer(
                avpBufSizeParam.m_bufferSize,
                "LoopRestorationFilterTileColumnReadWriteYBuffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_loopRestorationFilterTileColumnReadWriteYBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_loopRestorationFilterTileColumnReadWriteYBuffer,
                avpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }

        // Loop Restoration Filter Tile Column U Buffer
        DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
            lrTileColUBuf,
            &avpBufSizeParam));

        if (m_loopRestorationFilterTileColumnReadWriteUBuffer == nullptr)
        {
            m_loopRestorationFilterTileColumnReadWriteUBuffer = m_allocator->AllocateBuffer(
                avpBufSizeParam.m_bufferSize,
                "LoopRestorationFilterTileColumnReadWriteUBuffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_loopRestorationFilterTileColumnReadWriteUBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_loopRestorationFilterTileColumnReadWriteUBuffer,
                avpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }

        // Loop Restoration Filter Tile Column V Buffer
        DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
            lrTileColVBuf,
            &avpBufSizeParam));

        if (m_loopRestorationFilterTileColumnReadWriteVBuffer == nullptr)
        {
            m_loopRestorationFilterTileColumnReadWriteVBuffer = m_allocator->AllocateBuffer(
                avpBufSizeParam.m_bufferSize,
                "LoopRestorationFilterTileColumnReadWriteVBuffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_loopRestorationFilterTileColumnReadWriteVBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_loopRestorationFilterTileColumnReadWriteVBuffer,
                avpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }

        // Decoded Frame Status Error Buffer
        DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
            frameStatusErrBuf,
            &avpBufSizeParam));

        if (m_decodedFrameStatusErrorBuffer == nullptr)
        {
            m_decodedFrameStatusErrorBuffer = m_allocator->AllocateBuffer(
                avpBufSizeParam.m_bufferSize,
                "DecodedFrameStatusErrorBuffer",
                resourceInternalWrite,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_decodedFrameStatusErrorBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_decodedFrameStatusErrorBuffer,
                avpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }

        // Decoded Block Data Streamout Buffer
        DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
            dbdStreamoutBuf,
            &avpBufSizeParam));

        if (m_decodedBlockDataStreamoutBuffer == nullptr)
        {
            m_decodedBlockDataStreamoutBuffer = m_allocator->AllocateBuffer(
                avpBufSizeParam.m_bufferSize,
                "DecodedBlockDataStreamoutBuffer",
                resourceInternalReadWriteNoCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_decodedBlockDataStreamoutBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_decodedBlockDataStreamoutBuffer,
                avpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt_G12_Base::SetAvpDstSurfaceParams(MHW_VDBOX_SURFACE_PARAMS& dstSurfaceParams)
    {
        DECODE_FUNC_CALL();

        MOS_ZeroMemory(&dstSurfaceParams, sizeof(dstSurfaceParams));
        dstSurfaceParams.Mode                       = CODECHAL_DECODE_MODE_AV1VLD;
        dstSurfaceParams.psSurface                  = &m_av1BasicFeature->m_destSurface;
        dstSurfaceParams.ucSurfaceStateId           = reconPic;
        dstSurfaceParams.ChromaType                 = (uint8_t)chromaSamplingFormat;
        dstSurfaceParams.ucBitDepthLumaMinus8       = m_av1PicParams->m_bitDepthIdx << 1;
        dstSurfaceParams.ucBitDepthChromaMinus8     = m_av1PicParams->m_bitDepthIdx << 1;
        dstSurfaceParams.dwUVPlaneAlignment         = 8;

#ifdef _MMC_SUPPORTED
        DECODE_CHK_STATUS(m_mmcState->SetSurfaceMmcState(&(m_av1BasicFeature->m_destSurface)));
        DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcState(dstSurfaceParams.psSurface, &dstSurfaceParams.mmcState));
        DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcFormat(dstSurfaceParams.psSurface, &dstSurfaceParams.dwCompressionFormat));
#endif
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt_G12_Base::SetAvpRefSurfaceParams(MHW_VDBOX_SURFACE_PARAMS *refSurfaceParams)
    {
        DECODE_FUNC_CALL();

#ifdef _MMC_SUPPORTED
        //Record each reference surface mmc state
        uint8_t  skipMask          = 0;
        uint32_t compressionFormat = 0;
#endif

        if (m_av1PicParams->m_picInfoFlags.m_fields.m_frameType != keyFrame && m_av1PicParams->m_picInfoFlags.m_fields.m_frameType != intraOnlyFrame)
        {
                Av1SurfaceId surfaceId[av1TotalRefsPerFrame] = {
                av1IntraFrame,
                av1LastRef,
                av1Last2Ref,
                av1Last3Ref,
                av1GoldRef,
                av1BwdRef,
                av1AltRef2,
                av1AltRef };

            //set for intra frame
            refSurface[0] = m_av1BasicFeature->m_destSurface;

            Av1ReferenceFramesG12 &refFrames = m_av1BasicFeature->m_refFrames;
            const std::vector<uint8_t> &activeRefList = refFrames.GetActiveReferenceList(*m_av1PicParams, m_av1BasicFeature->m_av1TileParams[m_av1BasicFeature->m_tileCoding.m_curTile]);
            for (uint8_t i = 0; i < activeRefList.size(); i++)
            {
                PMOS_RESOURCE refResource[av1NumInterRefFrames];
                uint8_t frameIdx = activeRefList[i];
                auto refSuf = refFrames.GetReferenceByFrameIndex(frameIdx);
                if (refSuf != nullptr)
                {
                    refSurface[i + 1].OsResource = *refSuf;
                }
            }

            for (auto i = 0; i < av1TotalRefsPerFrame; i++)
            {
                DECODE_CHK_STATUS(m_allocator->GetSurfaceInfo(&refSurface[i]));
                refSurfaceParams[i].Mode                       = CODECHAL_DECODE_MODE_AV1VLD;
                refSurfaceParams[i].ChromaType                 = (uint8_t)chromaSamplingFormat;
                refSurfaceParams[i].ucBitDepthLumaMinus8       = m_av1PicParams->m_bitDepthIdx << 1;
                refSurfaceParams[i].ucBitDepthChromaMinus8     = m_av1PicParams->m_bitDepthIdx << 1;
                refSurfaceParams[i].dwUVPlaneAlignment         = 8;
                refSurfaceParams[i].psSurface                  = &refSurface[i];
                refSurfaceParams[i].ucSurfaceStateId           = surfaceId[i];
            #ifdef _MMC_SUPPORTED
                DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcState(refSurfaceParams[i].psSurface, &refSurfaceParams[i].mmcState));
                DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcFormat(refSurfaceParams[i].psSurface, &refSurfaceParams[i].dwCompressionFormat));
                if (refSurfaceParams[i].mmcState == MOS_MEMCOMP_DISABLED)
                {
                    skipMask |= (1 << i);
                }
                else
                {
                    DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcFormat(refSurfaceParams[i].psSurface, &compressionFormat));
                }
                DECODE_NORMALMESSAGE("AV1 MMC skip mask is %d compression format %d\n", skipMask, compressionFormat);
            #endif
            }

#ifdef _MMC_SUPPORTED
            if (m_mmcState->IsMmcEnabled())
            {
                for (auto i = 0; i < av1TotalRefsPerFrame; i++)
                {
                    // Set each refSurfaceParams mmcState as MOS_MEMCOMP_MC to satisfy MmcEnable in AddAvpSurfaceCmd
                    // Compression type/enable should be the same for all reference surface state
                    // The actual refSurfac mmcstate is recorded by skipMask
                    refSurfaceParams[i].mmcState            = MOS_MEMCOMP_MC;
                    refSurfaceParams[i].mmcSkipMask         = skipMask;
                    refSurfaceParams[i].dwCompressionFormat = compressionFormat;
                }
            }
#endif
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt_G12_Base::SetAvpIntraBCSurfaceParams(MHW_VDBOX_SURFACE_PARAMS &intraBCSurfaceParams)
    {
        DECODE_FUNC_CALL();

        MOS_ZeroMemory(&intraBCSurfaceParams, sizeof(intraBCSurfaceParams));
        DECODE_CHK_STATUS(m_allocator->GetSurfaceInfo(m_intrabcDecodedOutputFrameBuffer));
        intraBCSurfaceParams.Mode                       = CODECHAL_DECODE_MODE_AV1VLD;
        intraBCSurfaceParams.psSurface                  = m_intrabcDecodedOutputFrameBuffer;
        intraBCSurfaceParams.ucSurfaceStateId           = intrabcDecodedFrame;
        intraBCSurfaceParams.ChromaType                 = (uint8_t)chromaSamplingFormat;
        intraBCSurfaceParams.ucBitDepthLumaMinus8       = m_av1PicParams->m_bitDepthIdx << 1;
        intraBCSurfaceParams.ucBitDepthChromaMinus8     = m_av1PicParams->m_bitDepthIdx << 1;
        intraBCSurfaceParams.dwUVPlaneAlignment         = 8;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt_G12_Base::AddAvpSurfacesCmd(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        MHW_VDBOX_SURFACE_PARAMS dstSurfaceParams;
        DECODE_CHK_STATUS(SetAvpDstSurfaceParams(dstSurfaceParams));
        DECODE_CHK_STATUS(m_avpInterface->AddAvpSurfaceCmd(&cmdBuffer, &dstSurfaceParams));

        if (!AV1_KEY_OR_INRA_FRAME(m_av1PicParams->m_picInfoFlags.m_fields.m_frameType))
        {
            MHW_VDBOX_SURFACE_PARAMS refSurfaceParams[av1TotalRefsPerFrame];
            for (uint16_t i = 0; i < av1TotalRefsPerFrame; i++)
            {
                MOS_ZeroMemory(&refSurfaceParams[i], sizeof(MHW_VDBOX_SURFACE_PARAMS));
            }
            DECODE_CHK_STATUS(SetAvpRefSurfaceParams(refSurfaceParams));
            for (uint8_t i = 0; i < av1TotalRefsPerFrame; i++)
            {
                if (m_av1BasicFeature->m_bitDepth == 10 &&
                    m_osInterface->pfnIsMismatchOrderProgrammingSupported())
                {
                    refSurfaceParams[i].psSurface->Format = Format_P010;
                }
                DECODE_CHK_STATUS(m_avpInterface->AddAvpSurfaceCmd(&cmdBuffer, &refSurfaceParams[i]));
            }
        }

        if (m_av1PicParams->m_picInfoFlags.m_fields.m_allowIntrabc)
        {
            MHW_VDBOX_SURFACE_PARAMS intraBCSurfaceParams;
            DECODE_CHK_STATUS(SetAvpIntraBCSurfaceParams(intraBCSurfaceParams));
            DECODE_CHK_STATUS(m_avpInterface->AddAvpSurfaceCmd(&cmdBuffer, &intraBCSurfaceParams));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt_G12_Base::AddAvpSegmentStateCmd(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        MhwVdboxAvpSegmentStateParams segStateParams;
        MOS_ZeroMemory(&segStateParams, sizeof(segStateParams));
        DECODE_CHK_STATUS(SetAvpSegmentStateParams(segStateParams));

        for (uint8_t i = 0; i < av1MaxSegments; i++)
        {
            segStateParams.m_currentSegmentId = i;
            DECODE_CHK_STATUS(m_avpInterface->AddAvpSegmentStateCmd(
                &cmdBuffer,
                &segStateParams));

            if (m_av1PicParams->m_av1SegData.m_enabled == 0)
            {
                break;
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt_G12_Base::SetAvpSegmentStateParams(MhwVdboxAvpSegmentStateParams& segStateParams)
    {
        DECODE_FUNC_CALL();

        segStateParams.m_av1SegmentParams = m_av1BasicFeature->m_segmentParams;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt_G12_Base::SetAvpPipeBufAddrParams(MhwVdboxAvpPipeBufAddrParams& pipeBufAddrParams)
    {
        DECODE_FUNC_CALL();

        pipeBufAddrParams.m_mode = CODECHAL_DECODE_MODE_AV1VLD;

        if (m_av1PicParams->m_picInfoFlags.m_fields.m_allowIntrabc)
        {
            pipeBufAddrParams.m_intrabcDecodedOutputFrameBuffer   = &m_intrabcDecodedOutputFrameBuffer->OsResource;
        }

        pipeBufAddrParams.m_decodedPic                                             = &(m_av1BasicFeature->m_destSurface);
        pipeBufAddrParams.m_bitstreamDecoderEncoderLineRowstoreReadWriteBuffer     = &m_bitstreamDecoderEncoderLineRowstoreReadWriteBuffer->OsResource;
        pipeBufAddrParams.m_bitstreamDecoderEncoderTileLineRowstoreReadWriteBuffer = &m_bitstreamDecoderEncoderTileLineRowstoreReadWriteBuffer->OsResource;
        pipeBufAddrParams.m_intraPredictionLineRowstoreReadWriteBuffer             = &m_intraPredictionLineRowstoreReadWriteBuffer->OsResource;
        pipeBufAddrParams.m_intraPredictionTileLineRowstoreReadWriteBuffer         = &m_intraPredictionTileLineRowstoreReadWriteBuffer->OsResource;
        pipeBufAddrParams.m_spatialMotionVectorLineReadWriteBuffer                 = &m_spatialMotionVectorLineReadWriteBuffer->OsResource;
        pipeBufAddrParams.m_spatialMotionVectorCodingTileLineReadWriteBuffer       = &m_spatialMotionVectorCodingTileLineReadWriteBuffer->OsResource;
        pipeBufAddrParams.m_loopRestorationMetaTileColumnReadWriteBuffer           = &m_loopRestorationMetaTileColumnReadWriteBuffer->OsResource;
        pipeBufAddrParams.m_loopRestorationFilterTileReadWriteLineYBuffer          = &m_loopRestorationFilterTileReadWriteLineYBuffer->OsResource;
        pipeBufAddrParams.m_loopRestorationFilterTileReadWriteLineUBuffer          = &m_loopRestorationFilterTileReadWriteLineUBuffer->OsResource;
        pipeBufAddrParams.m_loopRestorationFilterTileReadWriteLineVBuffer          = &m_loopRestorationFilterTileReadWriteLineVBuffer->OsResource;
        pipeBufAddrParams.m_deblockerFilterLineReadWriteYBuffer                    = &m_deblockerFilterLineReadWriteYBuffer->OsResource;
        pipeBufAddrParams.m_deblockerFilterLineReadWriteUBuffer                    = &m_deblockerFilterLineReadWriteUBuffer->OsResource;
        pipeBufAddrParams.m_deblockerFilterLineReadWriteVBuffer                    = &m_deblockerFilterLineReadWriteVBuffer->OsResource;
        pipeBufAddrParams.m_deblockerFilterTileLineReadWriteYBuffer                = &m_deblockerFilterTileLineReadWriteYBuffer->OsResource;
        pipeBufAddrParams.m_deblockerFilterTileLineReadWriteVBuffer                = &m_deblockerFilterTileLineReadWriteVBuffer->OsResource;
        pipeBufAddrParams.m_deblockerFilterTileLineReadWriteUBuffer                = &m_deblockerFilterTileLineReadWriteUBuffer->OsResource;
        pipeBufAddrParams.m_deblockerFilterTileColumnReadWriteYBuffer              = &m_deblockerFilterTileColumnReadWriteYBuffer->OsResource;
        pipeBufAddrParams.m_deblockerFilterTileColumnReadWriteUBuffer              = &m_deblockerFilterTileColumnReadWriteUBuffer->OsResource;
        pipeBufAddrParams.m_deblockerFilterTileColumnReadWriteVBuffer              = &m_deblockerFilterTileColumnReadWriteVBuffer->OsResource;
        pipeBufAddrParams.m_cdefFilterLineReadWriteBuffer                          = &m_cdefFilterLineReadWriteBuffer->OsResource;
        pipeBufAddrParams.m_cdefFilterTileLineReadWriteBuffer                      = &m_cdefFilterTileLineReadWriteBuffer->OsResource;
        pipeBufAddrParams.m_cdefFilterTileColumnReadWriteBuffer                    = &m_cdefFilterTileColumnReadWriteBuffer->OsResource;
        pipeBufAddrParams.m_cdefFilterMetaTileLineReadWriteBuffer                  = &m_cdefFilterMetaTileLineReadWriteBuffer->OsResource;
        pipeBufAddrParams.m_cdefFilterMetaTileColumnReadWriteBuffer                = &m_cdefFilterMetaTileColumnReadWriteBuffer->OsResource;
        pipeBufAddrParams.m_cdefFilterTopLeftCornerReadWriteBuffer                 = &m_cdefFilterTopLeftCornerReadWriteBuffer->OsResource;
        pipeBufAddrParams.m_superResTileColumnReadWriteYBuffer                     = &m_superResTileColumnReadWriteYBuffer->OsResource;
        pipeBufAddrParams.m_superResTileColumnReadWriteUBuffer                     = &m_superResTileColumnReadWriteUBuffer->OsResource;
        pipeBufAddrParams.m_superResTileColumnReadWriteVBuffer                     = &m_superResTileColumnReadWriteVBuffer->OsResource;
        pipeBufAddrParams.m_loopRestorationFilterTileColumnReadWriteYBuffer        = &m_loopRestorationFilterTileColumnReadWriteYBuffer->OsResource;
        pipeBufAddrParams.m_loopRestorationFilterTileColumnReadWriteUBuffer        = &m_loopRestorationFilterTileColumnReadWriteUBuffer->OsResource;
        pipeBufAddrParams.m_loopRestorationFilterTileColumnReadWriteVBuffer        = &m_loopRestorationFilterTileColumnReadWriteVBuffer->OsResource;
        pipeBufAddrParams.m_decodedFrameStatusErrorBuffer                          = &m_decodedFrameStatusErrorBuffer->OsResource;
        pipeBufAddrParams.m_decodedBlockDataStreamoutBuffer                        = &m_decodedBlockDataStreamoutBuffer->OsResource;

        auto tempBuffers = &(m_av1BasicFeature->m_tempBuffers);
        PMOS_BUFFER curMvBuffer = tempBuffers->GetCurBuffer()->mvBuf;
        DECODE_CHK_NULL(curMvBuffer);
        pipeBufAddrParams.m_curMvTemporalBuffer = &(curMvBuffer->OsResource);

        Av1ReferenceFramesG12 &refFrames = m_av1BasicFeature->m_refFrames;
        uint8_t prevFrameIdx = refFrames.GetPrimaryRefIdx();

        uint32_t refSize = 0;
        if (m_av1PicParams->m_picInfoFlags.m_fields.m_frameType != keyFrame)
        {
            const std::vector<uint8_t> &activeRefList = refFrames.GetActiveReferenceList(*m_av1PicParams, m_av1BasicFeature->m_av1TileParams[m_av1BasicFeature->m_tileCoding.m_curTile]);
            refSize = activeRefList.size();

            //set for INTRA_FRAME
            pipeBufAddrParams.m_references[0] = &m_av1BasicFeature->m_destSurface.OsResource;
            pipeBufAddrParams.m_colMvTemporalBuffer[0] = &(curMvBuffer->OsResource);

            for (uint8_t i = 0; i < activeRefList.size(); i++)
            {
                uint8_t frameIdx = activeRefList[i];
                pipeBufAddrParams.m_references[i + lastFrame] = refFrames.GetReferenceByFrameIndex(frameIdx);
                auto tempBuf = tempBuffers->GetBufferByFrameIndex(frameIdx);
                pipeBufAddrParams.m_colMvTemporalBuffer[i + lastFrame] = tempBuf ? &(tempBuf->mvBuf->OsResource) : nullptr;
            }
        }

        DECODE_CHK_STATUS(FixAvpPipeBufAddrParams(pipeBufAddrParams));

        DECODE_CHK_NULL(tempBuffers->GetCurBuffer()->initCdfBuf);
        PMOS_BUFFER curInitCdfBuffer = tempBuffers->GetCurBuffer()->initCdfBuf->buffer;
        DECODE_CHK_NULL(curInitCdfBuffer);
        pipeBufAddrParams.m_cdfTableInitializationBuffer = &(curInitCdfBuffer->OsResource);

        if (!m_av1PicParams->m_picInfoFlags.m_fields.m_disableFrameEndUpdateCdf)
        {
            PMOS_BUFFER curBwdCdfBuffer = tempBuffers->GetCurBuffer()->bwdAdaptCdfBuf.buffer;
            DECODE_CHK_NULL(curBwdCdfBuffer);
            pipeBufAddrParams.m_cdfTableBwdAdaptationBuffer = &(curBwdCdfBuffer->OsResource);
        }

        if (m_av1PicParams->m_av1SegData.m_enabled && m_av1PicParams->m_av1SegData.m_updateMap)
        {
            PMOS_BUFFER curSegIDWriteBuffer = tempBuffers->GetCurBuffer()->segIdWriteBuf.buffer;
            DECODE_CHK_NULL(curSegIDWriteBuffer);
            pipeBufAddrParams.m_segmentIdWriteBuffer = &(curSegIDWriteBuffer->OsResource);
        }

        if (m_av1PicParams->m_av1SegData.m_enabled)
        {
            bool useSegMapFromPrevFrame = m_av1PicParams->m_av1SegData.m_temporalUpdate ||
                !m_av1PicParams->m_av1SegData.m_updateMap;
            if (useSegMapFromPrevFrame && refFrames.CheckSegForPrimFrame(*m_av1PicParams))
            {
                auto tempBuf = tempBuffers->GetBufferByFrameIndex(prevFrameIdx);
                auto segIdBuf = tempBuf ? tempBuf->segIdBuf : nullptr;
                auto buf = segIdBuf ? segIdBuf->buffer : nullptr;
                pipeBufAddrParams.m_segmentIdReadBuffer = buf ? &(buf->OsResource) : nullptr;
            }
        }

        CODECHAL_DEBUG_TOOL(DumpResources(pipeBufAddrParams, refSize));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt_G12_Base::FixAvpPipeBufAddrParams(MhwVdboxAvpPipeBufAddrParams& pipeBufAddrParams)
    {
        DECODE_FUNC_CALL();

        if (m_av1PicParams->m_picInfoFlags.m_fields.m_frameType == keyFrame)
        {
            for (auto i = 0; i < av1TotalRefsPerFrame; i++)
            {
                pipeBufAddrParams.m_references[i] = nullptr;
            }
        }
        else
        {
            PMOS_RESOURCE validRefPic = m_av1BasicFeature->m_refFrames.GetValidReference();
            if (validRefPic == nullptr)
            {
                validRefPic = &m_av1BasicFeature->m_destSurface.OsResource;
            }
            for (uint8_t i = 0; i < av1TotalRefsPerFrame; i++)
            {
                // error concealment for the unset reference addresses and unset mv buffers
                if (pipeBufAddrParams.m_references[i] == nullptr)
                {
                    pipeBufAddrParams.m_references[i] = validRefPic;
                }
            }

            PMOS_BUFFER validMvBuf = m_av1BasicFeature->m_tempBuffers.GetValidBufferForReference(
                                        m_av1BasicFeature->m_refFrameIndexList)->mvBuf;
            for (uint32_t i = 0; i < CODEC_NUM_AV1_TEMP_BUFFERS; i++)
            {
                if (pipeBufAddrParams.m_colMvTemporalBuffer[i] == nullptr)
                {
                    pipeBufAddrParams.m_colMvTemporalBuffer[i] = &validMvBuf->OsResource;
                }
            }
        }

        PMOS_RESOURCE dummyRef = &(m_av1BasicFeature->m_dummyReference.OsResource);
        if (m_av1BasicFeature->m_useDummyReference &&
            !m_allocator->ResourceIsNull(dummyRef))
        {
            // set all ref pic addresses to valid addresses for error concealment purpose
            for (uint32_t i = 0; i < av1TotalRefsPerFrame; i++)
            {
                if (pipeBufAddrParams.m_references[i] == nullptr)
                {
                    pipeBufAddrParams.m_references[i] = dummyRef;
                }
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    void Av1DecodePicPkt_G12_Base::SetAvpIndObjBaseAddrParams(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS& indObjBaseAddrParams)
    {
        DECODE_FUNC_CALL();

        MOS_ZeroMemory(&indObjBaseAddrParams, sizeof(indObjBaseAddrParams));
        indObjBaseAddrParams.Mode            = CODECHAL_DECODE_MODE_AV1VLD;
        indObjBaseAddrParams.dwDataSize      = m_av1BasicFeature->m_dataSize;
        indObjBaseAddrParams.dwDataOffset    = m_av1BasicFeature->m_dataOffset;
        indObjBaseAddrParams.presDataBuffer  = &(m_av1BasicFeature->m_resDataBuffer.OsResource);
    }

    MOS_STATUS Av1DecodePicPkt_G12_Base::AddAvpIndObjBaseAddrCmd(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS indObjBaseAddrParams;
        SetAvpIndObjBaseAddrParams(indObjBaseAddrParams);
        DECODE_CHK_STATUS(m_avpInterface->AddAvpIndObjBaseAddrCmd(&cmdBuffer, &indObjBaseAddrParams));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt_G12_Base::SetAvpPipeBufAddrParamsForDummyWL(MhwVdboxAvpPipeBufAddrParams& pipeBufAddrParams)
    {
        DECODE_FUNC_CALL();

        pipeBufAddrParams.m_mode = CODECHAL_DECODE_MODE_AV1VLD;

        pipeBufAddrParams.m_decodedPic                                             = m_av1BasicFeature->m_destSurfaceForDummyWL;

        pipeBufAddrParams.m_bitstreamDecoderEncoderLineRowstoreReadWriteBuffer     = &m_bitstreamDecoderEncoderLineRowstoreReadWriteBuffer->OsResource;
        pipeBufAddrParams.m_bitstreamDecoderEncoderTileLineRowstoreReadWriteBuffer = &m_bitstreamDecoderEncoderTileLineRowstoreReadWriteBuffer->OsResource;
        pipeBufAddrParams.m_intraPredictionLineRowstoreReadWriteBuffer             = &m_intraPredictionLineRowstoreReadWriteBuffer->OsResource;
        pipeBufAddrParams.m_intraPredictionTileLineRowstoreReadWriteBuffer         = &m_intraPredictionTileLineRowstoreReadWriteBuffer->OsResource;
        pipeBufAddrParams.m_spatialMotionVectorLineReadWriteBuffer                 = &m_spatialMotionVectorLineReadWriteBuffer->OsResource;
        pipeBufAddrParams.m_spatialMotionVectorCodingTileLineReadWriteBuffer       = &m_spatialMotionVectorCodingTileLineReadWriteBuffer->OsResource;
        pipeBufAddrParams.m_loopRestorationMetaTileColumnReadWriteBuffer           = &m_loopRestorationMetaTileColumnReadWriteBuffer->OsResource;
        pipeBufAddrParams.m_loopRestorationFilterTileReadWriteLineYBuffer          = &m_loopRestorationFilterTileReadWriteLineYBuffer->OsResource;
        pipeBufAddrParams.m_loopRestorationFilterTileReadWriteLineUBuffer          = &m_loopRestorationFilterTileReadWriteLineUBuffer->OsResource;
        pipeBufAddrParams.m_loopRestorationFilterTileReadWriteLineVBuffer          = &m_loopRestorationFilterTileReadWriteLineVBuffer->OsResource;
        pipeBufAddrParams.m_deblockerFilterLineReadWriteYBuffer                    = &m_deblockerFilterLineReadWriteYBuffer->OsResource;
        pipeBufAddrParams.m_deblockerFilterLineReadWriteUBuffer                    = &m_deblockerFilterLineReadWriteUBuffer->OsResource;
        pipeBufAddrParams.m_deblockerFilterLineReadWriteVBuffer                    = &m_deblockerFilterLineReadWriteVBuffer->OsResource;
        pipeBufAddrParams.m_deblockerFilterTileLineReadWriteYBuffer                = &m_deblockerFilterTileLineReadWriteYBuffer->OsResource;
        pipeBufAddrParams.m_deblockerFilterTileLineReadWriteVBuffer                = &m_deblockerFilterTileLineReadWriteVBuffer->OsResource;
        pipeBufAddrParams.m_deblockerFilterTileLineReadWriteUBuffer                = &m_deblockerFilterTileLineReadWriteUBuffer->OsResource;
        pipeBufAddrParams.m_deblockerFilterTileColumnReadWriteYBuffer              = &m_deblockerFilterTileColumnReadWriteYBuffer->OsResource;
        pipeBufAddrParams.m_deblockerFilterTileColumnReadWriteUBuffer              = &m_deblockerFilterTileColumnReadWriteUBuffer->OsResource;
        pipeBufAddrParams.m_deblockerFilterTileColumnReadWriteVBuffer              = &m_deblockerFilterTileColumnReadWriteVBuffer->OsResource;
        pipeBufAddrParams.m_cdefFilterLineReadWriteBuffer                          = &m_cdefFilterLineReadWriteBuffer->OsResource;
        pipeBufAddrParams.m_cdefFilterTileLineReadWriteBuffer                      = &m_cdefFilterTileLineReadWriteBuffer->OsResource;
        pipeBufAddrParams.m_cdefFilterTileColumnReadWriteBuffer                    = &m_cdefFilterTileColumnReadWriteBuffer->OsResource;
        pipeBufAddrParams.m_cdefFilterMetaTileLineReadWriteBuffer                  = &m_cdefFilterMetaTileLineReadWriteBuffer->OsResource;
        pipeBufAddrParams.m_cdefFilterMetaTileColumnReadWriteBuffer                = &m_cdefFilterMetaTileColumnReadWriteBuffer->OsResource;
        pipeBufAddrParams.m_cdefFilterTopLeftCornerReadWriteBuffer                 = &m_cdefFilterTopLeftCornerReadWriteBuffer->OsResource;
        pipeBufAddrParams.m_superResTileColumnReadWriteYBuffer                     = &m_superResTileColumnReadWriteYBuffer->OsResource;
        pipeBufAddrParams.m_superResTileColumnReadWriteUBuffer                     = &m_superResTileColumnReadWriteUBuffer->OsResource;
        pipeBufAddrParams.m_superResTileColumnReadWriteVBuffer                     = &m_superResTileColumnReadWriteVBuffer->OsResource;
        pipeBufAddrParams.m_loopRestorationFilterTileColumnReadWriteYBuffer        = &m_loopRestorationFilterTileColumnReadWriteYBuffer->OsResource;
        pipeBufAddrParams.m_loopRestorationFilterTileColumnReadWriteUBuffer        = &m_loopRestorationFilterTileColumnReadWriteUBuffer->OsResource;
        pipeBufAddrParams.m_loopRestorationFilterTileColumnReadWriteVBuffer        = &m_loopRestorationFilterTileColumnReadWriteVBuffer->OsResource;
        pipeBufAddrParams.m_decodedFrameStatusErrorBuffer                          = &m_decodedFrameStatusErrorBuffer->OsResource;
        pipeBufAddrParams.m_decodedBlockDataStreamoutBuffer                        = &m_decodedBlockDataStreamoutBuffer->OsResource;

        pipeBufAddrParams.m_curMvTemporalBuffer          = &m_curMvBufferForDummyWL->OsResource;
        pipeBufAddrParams.m_cdfTableInitializationBuffer = &m_av1BasicFeature->m_defaultCdfBuffers[3]->OsResource;
        pipeBufAddrParams.m_cdfTableBwdAdaptationBuffer  = &m_bwdAdaptCdfBufForDummyWL->OsResource;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt_G12_Base::UpdatePipeBufAddrForDummyWL(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        MhwVdboxAvpPipeBufAddrParams pipeBufAddrParams = {};
        DECODE_CHK_STATUS(SetAvpPipeBufAddrParamsForDummyWL(pipeBufAddrParams));
#ifdef _MMC_SUPPORTED
        pipeBufAddrParams.m_preDeblockSurfMmcState = MOS_MEMCOMP_DISABLED;
#endif
        DECODE_CHK_STATUS(m_avpInterface->AddAvpPipeBufAddrCmd(&cmdBuffer, &pipeBufAddrParams));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt_G12_Base::UpdateIndObjAddrForDummyWL(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        if (!m_dummyBsBufInited)
        {
            m_resDataBufferForDummyWL= m_allocator->AllocateBuffer(
                140, "BsBuffer for inserted Dummy WL", resourceInputBitstream, lockableVideoMem); //140 Bytes
            DECODE_CHK_NULL(m_resDataBufferForDummyWL);
            auto data = (uint8_t *)m_allocator->LockResourceForWrite(&m_resDataBufferForDummyWL->OsResource);
            DECODE_CHK_NULL(data);

            uint32_t bsBuffer[] =
            {
                0x3004260a, 0x1d95985a, 0x8311a32e, 0x4957f9a8,
                0x16000832, 0x00000100, 0x00900000, 0x88040000,
                0x797797f7, 0x346907ae, 0x00106332, 0x00010000,
                0x01c02a07, 0x9a165a76, 0x13041816, 0x92fd0c00,
                0x02fc1aad, 0xf94923f3, 0x88ce7a1b, 0xc440d4fb,
                0xcf5940bb, 0xe3a4fcff, 0x6c13cd19, 0x8e7b22b1,
                0x83f20193, 0x41c12b17, 0x7a266ac8, 0x9b0b871e,
                0x956345f8, 0x538c1c25, 0x2302b41f, 0x1e0587a8,
                0x182a0ec1, 0x3672822a, 0x5003db7b
            };

            MOS_SecureMemcpy(data, sizeof(bsBuffer), bsBuffer, sizeof(bsBuffer));
            m_dummyBsBufInited = true;
        }
        MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS indObjBaseAddrParams;

        MOS_ZeroMemory(&indObjBaseAddrParams, sizeof(indObjBaseAddrParams));
        indObjBaseAddrParams.Mode            = CODECHAL_DECODE_MODE_AV1VLD;
        indObjBaseAddrParams.dwDataSize      = 140;
        indObjBaseAddrParams.dwDataOffset    = 0;
        indObjBaseAddrParams.presDataBuffer  = &(m_resDataBufferForDummyWL->OsResource);

        DECODE_CHK_STATUS(m_avpInterface->AddAvpIndObjBaseAddrCmd(&cmdBuffer, &indObjBaseAddrParams));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt_G12_Base::SetAvpPicStateParams(MhwVdboxAvpPicStateParams& picStateParams)
    {
        DECODE_FUNC_CALL();

        MOS_ZeroMemory(&picStateParams, sizeof(picStateParams));
        picStateParams.m_picParams = m_av1PicParams;
        picStateParams.m_refList   = &(m_av1BasicFeature->m_refFrames.m_refList[0]);

        DECODE_CHK_STATUS(SetupSkipModeFrames(picStateParams));
        DECODE_CHK_STATUS(SetupFrameSignBias(picStateParams));

        memset(&m_av1PicParams->m_refFrameSide, 0, sizeof(m_av1PicParams->m_refFrameSide));
        if(m_av1PicParams->m_seqInfoFlags.m_fields.m_enableOrderHint &&
          !AV1_KEY_OR_INRA_FRAME(m_av1PicParams->m_picInfoFlags.m_fields.m_frameType))
        {
            for (auto ref = 0; ref < av1NumInterRefFrames; ref++)
            {
                picStateParams.m_refOrderHints[ref] = m_av1BasicFeature->m_refFrames.m_currRefList->m_refOrderHint[ref];

                if (m_av1BasicFeature->m_refFrames.GetRelativeDist(*m_av1PicParams, picStateParams.m_refOrderHints[ref], m_av1PicParams->m_orderHint) > 0 ||
                picStateParams.m_refOrderHints[ref] == m_av1PicParams->m_orderHint)
                {
                    m_av1PicParams->m_refFrameSide[ref + lastFrame] = 1;
                }
            }
        }

        DECODE_CHK_STATUS(m_av1BasicFeature->m_refFrames.GetValidReferenceIndex(&picStateParams.m_validRefPicIdx));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt_G12_Base::SetAvpInterPredStateParams(MhwVdboxAvpPicStateParams& picStateParams)
    {
        DECODE_FUNC_CALL();

        MOS_ZeroMemory(&picStateParams, sizeof(picStateParams));
        picStateParams.m_picParams = m_av1PicParams;

        if(!AV1_KEY_OR_INRA_FRAME(m_av1PicParams->m_picInfoFlags.m_fields.m_frameType) &&
            m_av1PicParams->m_seqInfoFlags.m_fields.m_enableOrderHint)
        {
            for (auto ref = 0; ref < av1NumInterRefFrames; ref++)
            {
                uint8_t refPicIndex = m_av1PicParams->m_refFrameIdx[ref];
                if (!CodecHal_PictureIsInvalid(m_av1PicParams->m_refFrameMap[refPicIndex]))
                {
                    uint8_t refFrameIdx = m_av1PicParams->m_refFrameMap[refPicIndex].FrameIdx;
                    for (auto i = 0; i < 7; i++)
                    {
                       picStateParams.m_savedRefOrderHints[ref][i] = m_av1BasicFeature->m_refFrames.m_refList[refFrameIdx]->m_refOrderHint[i];
                    }
                }
            }

            DECODE_CHK_STATUS(m_av1BasicFeature->m_refFrames.SetupMotionFieldProjection(*m_av1PicParams));

            picStateParams.m_refMaskMfProj = m_av1PicParams->m_activeRefBitMaskMfmv[0] |
                                                (m_av1PicParams->m_activeRefBitMaskMfmv[1] << 1) |
                                                (m_av1PicParams->m_activeRefBitMaskMfmv[2] << 2) |
                                                (m_av1PicParams->m_activeRefBitMaskMfmv[3] << 3) |
                                                (m_av1PicParams->m_activeRefBitMaskMfmv[4] << 4) |
                                                (m_av1PicParams->m_activeRefBitMaskMfmv[5] << 5) |
                                                (m_av1PicParams->m_activeRefBitMaskMfmv[6] << 6);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt_G12_Base::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
    {
        DECODE_FUNC_CALL();

        commandBufferSize = m_pictureStatesSize;
        requestedPatchListSize = m_picturePatchListSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt_G12_Base::SetupSkipModeFrames(MhwVdboxAvpPicStateParams& picStateParams)
    {
        DECODE_FUNC_CALL();

        if (!m_av1PicParams->m_seqInfoFlags.m_fields.m_enableOrderHint ||
            AV1_KEY_OR_INRA_FRAME(m_av1PicParams->m_picInfoFlags.m_fields.m_frameType) ||
            m_av1PicParams->m_modeControlFlags.m_fields.m_referenceMode == singleReference)
        {
            picStateParams.m_skipModeFrame[0] = -1;
            picStateParams.m_skipModeFrame[1] = -1;
            return MOS_STATUS_SUCCESS;
        }

        int32_t curFrameOffset = m_av1PicParams->m_orderHint;
        int32_t refFrameOffset[2] = { -1, 0x7fffffff };
        int32_t refIdx[2] = { -1, -1 };
        Av1ReferenceFramesG12 &refFrames = m_av1BasicFeature->m_refFrames;
        DECODE_CHK_STATUS(refFrames.Identify1stNearRef(*m_av1PicParams, curFrameOffset, refFrameOffset, refIdx));

        if (refIdx[0] != -1 && refIdx[1] != -1)
        {
            // == Bi-directional prediction ==
            //cm->is_skip_mode_allowed = 1;
            picStateParams.m_skipModeFrame[0] = AOMMIN(refIdx[0], refIdx[1]);
            picStateParams.m_skipModeFrame[1] = AOMMAX(refIdx[0], refIdx[1]);
        }
        else if (refIdx[0] != -1 && refIdx[1] == -1)
        {
            DECODE_CHK_STATUS(refFrames.Identify2ndNearRef(*m_av1PicParams, curFrameOffset, refFrameOffset, refIdx));
            if (refFrameOffset[1] != -1)
            {
                picStateParams.m_skipModeFrame[0] = AOMMIN(refIdx[0], refIdx[1]);
                picStateParams.m_skipModeFrame[1] = AOMMAX(refIdx[0], refIdx[1]);
            }
        }
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt_G12_Base::SetupFrameSignBias(MhwVdboxAvpPicStateParams& picStateParams)
    {
        DECODE_FUNC_CALL();

        for (auto refFrame = (uint32_t)lastFrame; refFrame <= (uint32_t)altRefFrame; refFrame++)//no bias for intra frame
        {
            if (m_av1PicParams->m_seqInfoFlags.m_fields.m_enableOrderHint &&
                !AV1_KEY_OR_INRA_FRAME(m_av1PicParams->m_picInfoFlags.m_fields.m_frameType))
            {
                int32_t refFrameOffset = -1;

                uint8_t refPicIndex = m_av1PicParams->m_refFrameIdx[refFrame - lastFrame];//0 corresponds to LAST_FRAME
                PCODEC_PICTURE refFrameList = &(m_av1PicParams->m_refFrameMap[0]);

                if (!CodecHal_PictureIsInvalid(refFrameList[refPicIndex]))
                {
                    uint8_t refFrameIdx = refFrameList[refPicIndex].FrameIdx;
                    refFrameOffset = m_av1BasicFeature->m_refFrames.m_refList[refFrameIdx]->m_orderHint;
                }

                int32_t frameOffset = (int32_t)m_av1PicParams->m_orderHint;
                picStateParams.m_referenceFrameSignBias[refFrame] =
                    (m_av1BasicFeature->m_refFrames.GetRelativeDist(*m_av1PicParams, refFrameOffset, frameOffset) <= 0) ? 0 : 1;
            }
            else
            {
                picStateParams.m_referenceFrameSignBias[refFrame] = 0;
            }
        }

        return MOS_STATUS_SUCCESS;
    }

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS Av1DecodePicPkt_G12_Base::DumpResources(MhwVdboxAvpPipeBufAddrParams &pipeBufAddrParams, uint32_t refSize)
    {
        DECODE_FUNC_CALL();

        CodechalDebugInterface *debugInterface = m_av1Pipeline->GetDebugInterface();
        debugInterface->m_frameType            = m_av1PicParams->m_picInfoFlags.m_fields.m_frameType ? P_TYPE : I_TYPE;
        m_av1PicParams->m_currPic.PicFlags     = PICTURE_FRAME;
        debugInterface->m_currPic              = m_av1PicParams->m_currPic;
        debugInterface->m_bufferDumpFrameNum   = m_av1BasicFeature->m_frameNum;

        if (m_av1PicParams->m_picInfoFlags.m_fields.m_frameType != keyFrame)
        {
            for (uint32_t n = 0; n < refSize; n++)
            {
                MOS_SURFACE refSurface;
                MOS_ZeroMemory(&refSurface, sizeof(MOS_SURFACE));
                refSurface.OsResource = *(pipeBufAddrParams.m_references[n + lastFrame]);
                DECODE_CHK_STATUS(m_allocator->GetSurfaceInfo(&refSurface));
                std::string refSurfName = "RefSurf[" + std::to_string(static_cast<uint32_t>(n)) + "]";
                DECODE_CHK_STATUS(debugInterface->DumpYUVSurface(
                    &refSurface,
                    CodechalDbgAttr::attrDecodeReferenceSurfaces,
                    refSurfName.c_str()));
            }
        }

        //For multi-tiles per frame case, only need dump these resources once.
        if (m_av1BasicFeature->m_tileCoding.m_curTile == 0)
        {
            if (pipeBufAddrParams.m_segmentIdReadBuffer != nullptr &&
                !m_allocator->ResourceIsNull(pipeBufAddrParams.m_segmentIdReadBuffer))
            {
                DECODE_CHK_STATUS(debugInterface->DumpBuffer(
                    pipeBufAddrParams.m_segmentIdReadBuffer,
                    CodechalDbgAttr::attrSegId,
                    "SegIdReadBuffer",
                    (m_widthInSb * m_heightInSb * CODECHAL_CACHELINE_SIZE),
                    CODECHAL_NUM_MEDIA_STATES));
            }

            DECODE_CHK_STATUS(debugInterface->DumpBuffer(
                pipeBufAddrParams.m_cdfTableInitializationBuffer,
                CodechalDbgAttr::attrCoefProb,
                "CdfTableInitialization",
                m_av1BasicFeature->m_cdfMaxNumBytes,
                CODECHAL_NUM_MEDIA_STATES));
        }

        return MOS_STATUS_SUCCESS;
    }
#endif

}
