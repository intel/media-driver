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
//! \file     decode_av1_picture_packet.cpp
//! \brief    Defines the interface for av1 decode picture packet
//!
#include "decode_av1_picture_packet.h"
#include "codechal_debug.h"

namespace decode{
    Av1DecodePicPkt::~Av1DecodePicPkt()
    {
        FreeResources();
    }

    MOS_STATUS Av1DecodePicPkt::FreeResources()
    {
        DECODE_FUNC_CALL();

        if (m_allocator != nullptr)
        {
             m_allocator->Destroy(m_intrabcDecodedOutputFrameBuffer);
            if (!m_avpItf->IsBufferRowstoreCacheEnabled(bsdLineBuffer))
            {
                m_allocator->Destroy(m_bitstreamDecoderEncoderLineRowstoreReadWriteBuffer);
            }
            m_allocator->Destroy(m_bitstreamDecoderEncoderTileLineRowstoreReadWriteBuffer);
            if (!m_avpItf->IsBufferRowstoreCacheEnabled(intraPredLineBuffer))
            {
                m_allocator->Destroy(m_intraPredictionLineRowstoreReadWriteBuffer);
            }
            m_allocator->Destroy(m_intraPredictionTileLineRowstoreReadWriteBuffer);
            if (!m_avpItf->IsBufferRowstoreCacheEnabled(spatialMvLineBuffer))
            {
                m_allocator->Destroy(m_spatialMotionVectorLineReadWriteBuffer);
            }
            m_allocator->Destroy(m_spatialMotionVectorCodingTileLineReadWriteBuffer);
            m_allocator->Destroy(m_loopRestorationMetaTileColumnReadWriteBuffer);
            m_allocator->Destroy(m_loopRestorationFilterTileReadWriteLineYBuffer);
            m_allocator->Destroy(m_loopRestorationFilterTileReadWriteLineUBuffer);
            m_allocator->Destroy(m_loopRestorationFilterTileReadWriteLineVBuffer);
            if (!m_avpItf->IsBufferRowstoreCacheEnabled(deblockLineYBuffer))
            {
                m_allocator->Destroy(m_deblockerFilterLineReadWriteYBuffer);
            }
            if (!m_avpItf->IsBufferRowstoreCacheEnabled(deblockLineUBuffer))
            {
                m_allocator->Destroy(m_deblockerFilterLineReadWriteUBuffer);
            }
            if (!m_avpItf->IsBufferRowstoreCacheEnabled(deblockLineVBuffer))
            {
                m_allocator->Destroy(m_deblockerFilterLineReadWriteVBuffer);
            }
            m_allocator->Destroy(m_deblockerFilterTileLineReadWriteYBuffer);
            m_allocator->Destroy(m_deblockerFilterTileLineReadWriteVBuffer);
            m_allocator->Destroy(m_deblockerFilterTileLineReadWriteUBuffer);
            m_allocator->Destroy(m_deblockerFilterTileColumnReadWriteYBuffer);
            m_allocator->Destroy(m_deblockerFilterTileColumnReadWriteUBuffer);
            m_allocator->Destroy(m_deblockerFilterTileColumnReadWriteVBuffer);
            if (!m_avpItf->IsBufferRowstoreCacheEnabled(cdefLineBuffer))
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
            m_allocator->Destroy(m_filmGrainSampleTemplateBuf);
            m_allocator->Destroy(m_filmGrainTileColumnDataBuf);
            m_allocator->Destroy(m_loopRestorationFilterTileColumnAlignmentBuf);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt::Init()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_featureManager);
        DECODE_CHK_NULL(m_hwInterface);
        DECODE_CHK_NULL(m_osInterface);
        DECODE_CHK_NULL(m_av1Pipeline);

        m_av1BasicFeature = dynamic_cast<Av1BasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        DECODE_CHK_NULL(m_av1BasicFeature);

        m_allocator = m_pipeline ->GetDecodeAllocator();
        DECODE_CHK_NULL(m_allocator);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt::Prepare()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_STATUS(GetChromaFormat());

#ifdef _MMC_SUPPORTED
        m_mmcState = m_av1Pipeline->GetMmcState();
        DECODE_CHK_NULL(m_mmcState);
#endif

        DECODE_CHK_STATUS(SetRowstoreCachingOffsets());

        DECODE_CHK_STATUS(AllocateVariableResources());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt::GetChromaFormat()
    {
        DECODE_FUNC_CALL();

        m_av1PicParams = m_av1BasicFeature->m_av1PicParams;

        if (m_av1PicParams->m_seqInfoFlags.m_fields.m_subsamplingX == 1 && m_av1PicParams->m_seqInfoFlags.m_fields.m_subsamplingY == 1)
        {
            chromaSamplingFormat = av1ChromaFormatYuv420;
        }
        else
        {
            DECODE_ASSERTMESSAGE("Invalid Chroma sampling format!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt::SetRowstoreCachingOffsets()
    {
        if (m_avpItf->IsRowStoreCachingSupported() &&
            (m_av1BasicFeature->m_frameWidthAlignedMinBlk != MOS_ALIGN_CEIL(m_prevFrmWidth, av1MinBlockWidth)))
        {
            MHW_VDBOX_ROWSTORE_PARAMS rowstoreParams;
            MOS_ZeroMemory(&rowstoreParams, sizeof(rowstoreParams));
            rowstoreParams.dwPicWidth       = m_av1BasicFeature->m_frameWidthAlignedMinBlk;
            rowstoreParams.bMbaff           = false;
            rowstoreParams.Mode             = CODECHAL_DECODE_MODE_AV1VLD;
            rowstoreParams.ucBitDepthMinus8 = m_av1PicParams->m_bitDepthIdx << 1;
            rowstoreParams.ucChromaFormat   = static_cast<uint8_t>(chromaSamplingFormat);
            DECODE_CHK_STATUS(m_hwInterface->SetRowstoreCachingOffsets(&rowstoreParams));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt::AllocateVariableResources()
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
        AvpBufferSizePar avpBufSizeParam;
        MOS_ZeroMemory(&avpBufSizeParam, sizeof(avpBufSizeParam));

        avpBufSizeParam.bitDepthIdc     = m_av1BasicFeature->m_av1DepthIndicator;
        avpBufSizeParam.width           = m_widthInSb;
        avpBufSizeParam.height          = m_heightInSb;
        avpBufSizeParam.tileWidth       = maxTileWidthInSb;
        avpBufSizeParam.isSb128x128     = m_av1PicParams->m_seqInfoFlags.m_fields.m_use128x128Superblock ? true : false;
        avpBufSizeParam.curFrameTileNum = m_av1PicParams->m_tileCols * m_av1PicParams->m_tileRows;
        avpBufSizeParam.numTileCol      = m_av1PicParams->m_tileCols;
        avpBufSizeParam.chromaFormat    = chromaSamplingFormat;

        // Lamda expression
        auto AllocateBuffer = [&] (PMOS_BUFFER &buffer, AvpBufferType bufferType, const char *bufferName)
        {
            DECODE_CHK_STATUS(m_avpItf->GetAvpBufSize(bufferType, &avpBufSizeParam));
            if (buffer == nullptr)
            {
                buffer = m_allocator->AllocateBuffer(
                    avpBufSizeParam.bufferSize, bufferName, resourceInternalReadWriteCache, notLockableVideoMem);
                DECODE_CHK_NULL(buffer);
            }
            else
            {
                DECODE_CHK_STATUS(m_allocator->Resize(buffer, avpBufSizeParam.bufferSize, notLockableVideoMem));
            }
            return MOS_STATUS_SUCCESS;
        };

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
        if (!m_avpItf->IsBufferRowstoreCacheEnabled(bsdLineBuffer))
        {
            DECODE_CHK_STATUS(AllocateBuffer(
                m_bitstreamDecoderEncoderLineRowstoreReadWriteBuffer,
                bsdLineBuffer,
                "BitstreamDecodeLineBuffer"));
        }

        // Bitstream decode tile line buffer
        DECODE_CHK_STATUS(AllocateBuffer(
            m_bitstreamDecoderEncoderTileLineRowstoreReadWriteBuffer,
            bsdTileLineBuffer,
            "BitstreamDecodeTileLineBuffer"));

        // Intra Prediction Line Rowstore Read/Write Buffer
        if (!m_avpItf->IsBufferRowstoreCacheEnabled(intraPredLineBuffer))
        {
            DECODE_CHK_STATUS(AllocateBuffer(
                m_intraPredictionLineRowstoreReadWriteBuffer,
                intraPredLineBuffer,
                "intraPredictionLineRowstoreBuffer"));
        }

        // Intra Prediction Tile Line Rowstore Read/Write Buffer
        DECODE_CHK_STATUS(AllocateBuffer(
            m_intraPredictionTileLineRowstoreReadWriteBuffer,
            intraPredTileLineBuffer,
            "intraPredictionTileLineRowstoreBuffer"));

        // Spatial motion vector Line rowstore buffer
        if (!m_avpItf->IsBufferRowstoreCacheEnabled(spatialMvLineBuffer))
        {
            DECODE_CHK_STATUS(AllocateBuffer(
                m_spatialMotionVectorLineReadWriteBuffer,
                spatialMvLineBuffer,
                "SpatialMotionVectorLineRowstoreBuffer"));
        }

        // Spatial motion vector Tile Line Buffer
        DECODE_CHK_STATUS(AllocateBuffer(
            m_spatialMotionVectorCodingTileLineReadWriteBuffer,
            spatialMvTileLineBuffer,
            "SpatialMotionVectorTileLineBuffer"));

        // Loop Restoration Meta Tile Column Read/Write Buffer
        DECODE_CHK_STATUS(AllocateBuffer(
            m_loopRestorationMetaTileColumnReadWriteBuffer,
            lrMetaTileColBuffer,
            "LoopRestorationMetaTileColumnReadWriteBuffer"));

        // Loop Restoration Filter Tile Read/Write Line Y Buffer
        DECODE_CHK_STATUS(AllocateBuffer(
            m_loopRestorationFilterTileReadWriteLineYBuffer,
            lrTileLineYBuffer,
            "LoopRestorationFilterTileReadWriteLineYBuffer"));

        //Loop Restoration Filter Tile Read/Write Line U Buffer
        DECODE_CHK_STATUS(AllocateBuffer(
            m_loopRestorationFilterTileReadWriteLineUBuffer,
            lrTileLineUBuffer,
            "LoopRestorationFilterTileReadWriteLineUBuffer"));

        // Loop Restoration Filter Tile Read/Write Line V Buffer
        DECODE_CHK_STATUS(AllocateBuffer(
            m_loopRestorationFilterTileReadWriteLineVBuffer,
            lrTileLineVBuffer,
            "LoopRestorationFilterTileReadWriteLineVBuffer"));

        if (!m_avpItf->IsBufferRowstoreCacheEnabled(deblockLineYBuffer))
        {
            DECODE_CHK_STATUS(AllocateBuffer(
                m_deblockerFilterLineReadWriteYBuffer,
                deblockLineYBuffer,
                "DeblockerFilterLineReadWriteYBuffer"));
        }

        if (!m_avpItf->IsBufferRowstoreCacheEnabled(deblockLineUBuffer))
        {
            DECODE_CHK_STATUS(AllocateBuffer(
                m_deblockerFilterLineReadWriteUBuffer,
                deblockLineUBuffer,
                "DeblockerFilterLineReadWriteUBuffer"));
        }

        if (!m_avpItf->IsBufferRowstoreCacheEnabled(deblockLineVBuffer))
        {
            DECODE_CHK_STATUS(AllocateBuffer(
                m_deblockerFilterLineReadWriteVBuffer,
                deblockLineVBuffer,
                "DeblockerFilterLineReadWriteVBuffer"));
        }

        // Deblocking Filter Tile Line Y Buffer
        DECODE_CHK_STATUS(AllocateBuffer(
            m_deblockerFilterTileLineReadWriteYBuffer,
            deblockTileLineYBuffer,
            "DeblockerFilterTileLineReadWriteYBuffer"));

        // Deblocking Filter Tile Line V Buffer
        DECODE_CHK_STATUS(AllocateBuffer(
            m_deblockerFilterTileLineReadWriteVBuffer,
            deblockTileLineVBuffer,
            "DeblockerFilterTileLineReadWriteVBuffer"));

        // Deblocking Filter Tile Line U Buffer
        DECODE_CHK_STATUS(AllocateBuffer(
            m_deblockerFilterTileLineReadWriteUBuffer,
            deblockTileLineUBuffer,
            "DeblockerFilterTileLineReadWriteUBuffer"));

        // Deblocking Filter Tile Column Y Buffer
        DECODE_CHK_STATUS(AllocateBuffer(
            m_deblockerFilterTileColumnReadWriteYBuffer,
            deblockTileColYBuffer,
            "DeblockerFilterTileColumnReadWriteYBuffer"));

        // Deblocking Filter Tile Column U Buffer
        DECODE_CHK_STATUS(AllocateBuffer(
            m_deblockerFilterTileColumnReadWriteUBuffer,
            deblockTileColUBuffer,
            "DeblockerFilterTileColumnReadWriteUBuffer"));

        // Deblocking Filter Tile Column V Buffer
        DECODE_CHK_STATUS(AllocateBuffer(
            m_deblockerFilterTileColumnReadWriteVBuffer,
            deblockTileColVBuffer,
            "DeblockerFilterTileColumnReadWriteVBuffer"));

        // CDEF Filter Line Read/Write Buffer
        if (!m_avpItf->IsBufferRowstoreCacheEnabled(cdefLineBuffer))
        {
            DECODE_CHK_STATUS(AllocateBuffer(
                m_cdefFilterLineReadWriteBuffer,
                cdefLineBuffer,
                "CdefFilterLineReadWriteBuffer"));
        }

        // CDEF Filter Tile Line Read/Write Buffer
        DECODE_CHK_STATUS(AllocateBuffer(
            m_cdefFilterTileLineReadWriteBuffer,
            cdefTileLineBuffer,
            "CdefFilterTileLineReadWriteBuffer"));

        // CDEF Filter Tile Column Read/Write Buffer
        DECODE_CHK_STATUS(AllocateBuffer(
            m_cdefFilterTileColumnReadWriteBuffer,
            cdefTileColBuffer,
            "CdefFilterTileColumnReadWriteBuffer"));

        // CDEF Filter Meta Tile Line Read Write Buffer
        DECODE_CHK_STATUS(AllocateBuffer(
            m_cdefFilterMetaTileLineReadWriteBuffer,
            cdefMetaTileLineBuffer,
            "CdefFilterMetaTileLineReadWriteBuffer"));

        // CDEF Filter Meta Tile Column Read Write Buffer
        DECODE_CHK_STATUS(AllocateBuffer(
            m_cdefFilterMetaTileColumnReadWriteBuffer,
            cdefMetaTileColBuffer,
            "CdefFilterMetaTileColumnReadWriteBuffer"));

        // Cdef Filter Top Left Corner Buffer
        DECODE_CHK_STATUS(AllocateBuffer(
            m_cdefFilterTopLeftCornerReadWriteBuffer,
            cdefTopLeftCornerBuffer,
            "CdefFilterTopLeftCornerReadWriteBuffer"));

        // Super-Res Tile Column Y Buffer
        DECODE_CHK_STATUS(AllocateBuffer(
            m_superResTileColumnReadWriteYBuffer,
            superResTileColYBuffer,
            "SuperResTileColumnReadWriteYBuffer"));

        // Super-Res Tile Column U Buffer
        DECODE_CHK_STATUS(AllocateBuffer(
            m_superResTileColumnReadWriteUBuffer,
            superResTileColUBuffer,
            "SuperResTileColumnReadWriteUBuffer"));

        // Super-Res Tile Column V Buffer
        DECODE_CHK_STATUS(AllocateBuffer(
            m_superResTileColumnReadWriteVBuffer,
            superResTileColVBuffer,
            "SuperResTileColumnReadWriteVBuffer"));

        // Loop Restoration Filter Tile Column Y Buffer
        DECODE_CHK_STATUS(AllocateBuffer(
            m_loopRestorationFilterTileColumnReadWriteYBuffer,
            lrTileColYBuffer,
            "LoopRestorationFilterTileColumnReadWriteYBuffer"));

        // Loop Restoration Filter Tile Column U Buffer
        DECODE_CHK_STATUS(AllocateBuffer(
            m_loopRestorationFilterTileColumnReadWriteUBuffer,
            lrTileColUBuffer,
            "LoopRestorationFilterTileColumnReadWriteUBuffer"));

        // Loop Restoration Filter Tile Column V Buffer
        DECODE_CHK_STATUS(AllocateBuffer(
            m_loopRestorationFilterTileColumnReadWriteVBuffer,
            lrTileColVBuffer,
            "LoopRestorationFilterTileColumnReadWriteVBuffer"));

        // Decoded Frame Status Error Buffer
        DECODE_CHK_STATUS(AllocateBuffer(
            m_decodedFrameStatusErrorBuffer,
            frameStatusErrBuffer,
            "DecodedFrameStatusErrorBuffer"));

        // Decoded Block Data Streamout Buffer
        DECODE_CHK_STATUS(AllocateBuffer(
            m_decodedBlockDataStreamoutBuffer,
            dbdStreamoutBuffer,
            "DecodedBlockDataStreamoutBuffer"));

        // Film Grain sample template buffer
        DECODE_CHK_STATUS(AllocateBuffer(
            m_filmGrainSampleTemplateBuf,
            fgSampleTmpBuffer,
            "FilmGrainSampleTemplateBuf"));

        // Film Grain tile column data read/write buffer
        DECODE_CHK_STATUS(AllocateBuffer(
            m_filmGrainTileColumnDataBuf,
            fgTileColBuffer,
            "FilmGrainTileColumnBuf"));

        // Loop restoration filter tile column alignment read/write buffer
        DECODE_CHK_STATUS(AllocateBuffer(
            m_loopRestorationFilterTileColumnAlignmentBuf,
            lrTileColAlignBuffer,
            "LoopRestorationFilterTileColumnAlignmentBuf"));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
    {
        DECODE_FUNC_CALL();

        commandBufferSize = m_pictureStatesSize;
        requestedPatchListSize = m_picturePatchListSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt::AddAllCmds_AVP_PIPE_MODE_SELECT(MOS_COMMAND_BUFFER& cmdBuffer)
    {
        DECODE_FUNC_CALL();

        // for Gen11+, we need to add MFX wait for both KIN and VRT before and after AVP Pipemode select.
        auto &mfxWaitParams               = m_miItf->MHW_GETPAR_F(MFX_WAIT)();
        mfxWaitParams                     = {};
        mfxWaitParams.iStallVdboxPipeline = true;
        DECODE_CHK_STATUS((m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer)));
        DECODE_CHK_NULL(m_avpItf);

        SETPAR_AND_ADDCMD(AVP_PIPE_MODE_SELECT, m_avpItf, &cmdBuffer);

        // for Gen11+, we need to add MFX wait for both KIN and VRT before and after AVP Pipemode select.
        mfxWaitParams                     = {};
        mfxWaitParams.iStallVdboxPipeline = true;
        DECODE_CHK_STATUS((m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer)));

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_PIPE_MODE_SELECT, Av1DecodePicPkt)
    {
        params.codecStandardSelect = 2;
        params.codecSelect         = 0;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_IND_OBJ_BASE_ADDR_STATE, Av1DecodePicPkt)
    {
        params.Mode           = CODECHAL_DECODE_MODE_AV1VLD;
        params.dataSize       = m_av1BasicFeature->m_dataSize;
        params.dataOffset     = m_av1BasicFeature->m_dataOffset;
        params.dataBuffer     = &(m_av1BasicFeature->m_resDataBuffer.OsResource);

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_PIC_STATE, Av1DecodePicPkt)
    {
        params.frameWidthMinus1  = m_av1PicParams->m_frameWidthMinus1;
        params.frameHeightMinus1 = m_av1PicParams->m_frameHeightMinus1;

        if (m_av1PicParams->m_seqInfoFlags.m_fields.m_subsamplingX == 1 && m_av1PicParams->m_seqInfoFlags.m_fields.m_subsamplingY == 1)
        {
            if (!m_av1PicParams->m_seqInfoFlags.m_fields.m_monoChrome &&
                (m_av1PicParams->m_bitDepthIdx == 0 || m_av1PicParams->m_bitDepthIdx == 1))
            {
                //4:2:0
                params.chromaFormat = av1ChromaFormatYuv420;
            }
            else
            {
                return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
            }
        }

        params.bitDepthIdc              = m_av1PicParams->m_bitDepthIdx;
        params.superblockSizeUsed       = m_av1PicParams->m_seqInfoFlags.m_fields.m_use128x128Superblock;
        params.enableOrderHint          = m_av1PicParams->m_seqInfoFlags.m_fields.m_enableOrderHint;
        params.orderHintBitsMinus1      = (m_av1PicParams->m_seqInfoFlags.m_fields.m_enableOrderHint)? m_av1PicParams->m_orderHintBitsMinus1 : 0;
        params.enableFilterIntra        = m_av1PicParams->m_seqInfoFlags.m_fields.m_enableFilterIntra;
        params.enableIntraEdgeFilter    = m_av1PicParams->m_seqInfoFlags.m_fields.m_enableIntraEdgeFilter;
        params.enableDualFilter         = m_av1PicParams->m_seqInfoFlags.m_fields.m_enableDualFilter;
        params.enableInterIntraCompound = m_av1PicParams->m_seqInfoFlags.m_fields.m_enableInterintraCompound;
        params.enableMaskedCompound     = m_av1PicParams->m_seqInfoFlags.m_fields.m_enableMaskedCompound;
        params.enableJointCompound      = m_av1PicParams->m_seqInfoFlags.m_fields.m_enableJntComp;

        params.allowScreenContentTools  = m_av1PicParams->m_picInfoFlags.m_fields.m_allowScreenContentTools;
        params.forceIntegerMv           = m_av1PicParams->m_picInfoFlags.m_fields.m_forceIntegerMv;
        params.allowWarpedMotion        = m_av1PicParams->m_picInfoFlags.m_fields.m_allowWarpedMotion;
        params.enableCDEF               = !(m_av1PicParams->m_losslessMode || m_av1PicParams->m_picInfoFlags.m_fields.m_allowIntrabc
                                           || !m_av1PicParams->m_seqInfoFlags.m_fields.m_enableCdef); // coded lossless is used here
        params.enableSuperres           = m_av1PicParams->m_picInfoFlags.m_fields.m_useSuperres;
        params.enableRestoration        = m_av1PicParams->m_loopRestorationFlags.m_fields.m_yframeRestorationType != 0 ||
                                           m_av1PicParams->m_loopRestorationFlags.m_fields.m_cbframeRestorationType != 0 ||
                                           m_av1PicParams->m_loopRestorationFlags.m_fields.m_crframeRestorationType != 0;
        params.enableLargeScaleTile     = m_av1PicParams->m_picInfoFlags.m_fields.m_largeScaleTile;
        params.frameType                = m_av1PicParams->m_picInfoFlags.m_fields.m_frameType;
        params.errorResilientMode       = m_av1PicParams->m_picInfoFlags.m_fields.m_errorResilientMode;
        params.allowIntraBC             = m_av1PicParams->m_picInfoFlags.m_fields.m_allowIntrabc;
        params.primaryRefFrame          = m_av1PicParams->m_primaryRefFrame;
        params.segmentParams            = m_av1PicParams->m_av1SegData;
        params.deltaQPresentFlag        = m_av1PicParams->m_modeControlFlags.m_fields.m_deltaQPresentFlag;
        params.log2DeltaQRes            = m_av1PicParams->m_modeControlFlags.m_fields.m_log2DeltaQRes;
        params.codedLossless            = m_av1PicParams->m_losslessMode;
        params.baseQindex               = m_av1PicParams->m_baseQindex;
        params.yDcDeltaQ                = m_av1PicParams->m_yDcDeltaQ;

        params.uDcDeltaQ                = m_av1PicParams->m_uDcDeltaQ;
        params.uAcDeltaQ                = m_av1PicParams->m_uAcDeltaQ;
        params.vDcDeltaQ                = m_av1PicParams->m_vDcDeltaQ;
        params.vAcDeltaQ                = m_av1PicParams->m_vAcDeltaQ;

        params.allowHighPrecisionMV     = m_av1PicParams->m_picInfoFlags.m_fields.m_allowHighPrecisionMv;
        params.referenceSelect          = !(m_av1PicParams->m_modeControlFlags.m_fields.m_referenceMode == singleReference);
        params.interpFilter             = m_av1PicParams->m_interpFilter;
        params.motionModeSwitchable     = m_av1PicParams->m_picInfoFlags.m_fields.m_isMotionModeSwitchable;
        params.useReferenceFrameMvSet   = m_av1PicParams->m_picInfoFlags.m_fields.m_useRefFrameMvs;
        params.currentOrderHint         = m_av1PicParams->m_orderHint;
        params.reducedTxSetUsed         = m_av1PicParams->m_modeControlFlags.m_fields.m_reducedTxSetUsed;
        params.txMode                   = m_av1PicParams->m_modeControlFlags.m_fields.m_txMode;
        params.skipModePresent          = m_av1PicParams->m_modeControlFlags.m_fields.m_skipModePresent;
        params.applyFilmGrainFlag       = m_av1PicParams->m_filmGrainParams.m_filmGrainInfoFlags.m_fields.m_applyGrain ? 1: 0;

        DECODE_CHK_STATUS(SetRefPicStateParam());
        DECODE_CHK_STATUS(SetSkipModeFrameParam());

        uint8_t idx = 0;
        for (uint32_t frame = (uint32_t)lastFrame; frame <= (uint32_t)altRefFrame; frame++)
        {
            params.warpParamsArrayProjection[idx++] = CAT2SHORTS(m_av1PicParams->m_wm[frame - lastFrame].m_wmmat[0], m_av1PicParams->m_wm[frame - lastFrame].m_wmmat[1]);
            params.warpParamsArrayProjection[idx++] = CAT2SHORTS(m_av1PicParams->m_wm[frame - lastFrame].m_wmmat[2], m_av1PicParams->m_wm[frame - lastFrame].m_wmmat[3]);
            params.warpParamsArrayProjection[idx++] = CAT2SHORTS(m_av1PicParams->m_wm[frame - lastFrame].m_wmmat[4], m_av1PicParams->m_wm[frame - lastFrame].m_wmmat[5]);
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_INTER_PRED_STATE, Av1DecodePicPkt)
    {
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
                       params.savedRefOrderHints[ref][i] = m_av1BasicFeature->m_refFrames.m_refList[refFrameIdx]->m_refOrderHint[i];
                    }
                }
            }

            DECODE_CHK_STATUS(m_av1BasicFeature->m_refFrames.SetupMotionFieldProjection(*m_av1PicParams));

            params.refMaskMfProj = m_av1PicParams->m_activeRefBitMaskMfmv[0] |
                                    (m_av1PicParams->m_activeRefBitMaskMfmv[1] << 1) |
                                    (m_av1PicParams->m_activeRefBitMaskMfmv[2] << 2) |
                                    (m_av1PicParams->m_activeRefBitMaskMfmv[3] << 3) |
                                    (m_av1PicParams->m_activeRefBitMaskMfmv[4] << 4) |
                                    (m_av1PicParams->m_activeRefBitMaskMfmv[5] << 5) |
                                    (m_av1PicParams->m_activeRefBitMaskMfmv[6] << 6);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt::AddAllCmds_AVP_SEGMENT_STATE(MOS_COMMAND_BUFFER& cmdBuffer)
    {
        DECODE_FUNC_CALL();

        auto &par = m_avpItf->MHW_GETPAR_F(AVP_SEGMENT_STATE)();
        par = {};

        MOS_SecureMemcpy(
            &par.av1SegmentParams,
            sizeof(par.av1SegmentParams),
            m_av1BasicFeature->m_segmentParams,
            sizeof(par.av1SegmentParams));

        for (uint8_t i = 0; i < av1MaxSegments; i++)
        {
            par.currentSegmentId = i;
            DECODE_CHK_STATUS(m_avpItf->MHW_ADDCMD_F(AVP_SEGMENT_STATE)(&cmdBuffer));

            if (m_av1PicParams->m_av1SegData.m_enabled == 0)
            {
                break;
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt::AddAllCmds_AVP_SURFACE_STATE(MOS_COMMAND_BUFFER& cmdBuffer)
    {
        DECODE_FUNC_CALL();

        m_curAvpSurfStateId = reconPic;
        SETPAR_AND_ADDCMD(AVP_SURFACE_STATE, m_avpItf, &cmdBuffer);
        if (!AV1_KEY_OR_INRA_FRAME(m_av1PicParams->m_picInfoFlags.m_fields.m_frameType))
        {
            for (uint8_t i = 0; i < av1TotalRefsPerFrame; i++)
            {
                m_curAvpSurfStateId = i + av1IntraFrame;

                //set for intra frame
                m_refSurface[0] = m_av1BasicFeature->m_destSurface;
                GetSurfaceMmcInfo(const_cast<PMOS_SURFACE>(&m_refSurface[0]), m_refMmcState[0], m_refCompressionFormat);
                Av1ReferenceFrames &refFrames = m_av1BasicFeature->m_refFrames;
                const std::vector<uint8_t> &activeRefList = refFrames.GetActiveReferenceList(*m_av1PicParams,
                    m_av1BasicFeature->m_av1TileParams[m_av1BasicFeature->m_tileCoding.m_curTile]);

                for (uint8_t i = 0; i < activeRefList.size(); i++)
                {
                    PMOS_RESOURCE refResource[av1NumInterRefFrames];
                    uint8_t frameIdx = activeRefList[i];
                    auto refSuf = refFrames.GetReferenceByFrameIndex(frameIdx);
                    if (refSuf != nullptr)
                    {
                        m_refSurface[i + 1].OsResource = *refSuf;
                        GetSurfaceMmcInfo(const_cast<PMOS_SURFACE>(&m_refSurface[i + 1]), m_refMmcState[i + 1], m_refCompressionFormat);
                    }
                }

                SETPAR_AND_ADDCMD(AVP_SURFACE_STATE, m_avpItf, &cmdBuffer);
            }
        }

        if (m_av1PicParams->m_picInfoFlags.m_fields.m_allowIntrabc)
        {
            DECODE_CHK_STATUS(m_allocator->GetSurfaceInfo(m_intrabcDecodedOutputFrameBuffer));
            m_curAvpSurfStateId = intrabcDecodedFrame;
            SETPAR_AND_ADDCMD(AVP_SURFACE_STATE, m_avpItf, &cmdBuffer);
        }

        if (m_av1PicParams->m_filmGrainParams.m_filmGrainInfoFlags.m_fields.m_applyGrain)
        {
            m_curAvpSurfStateId = filmGrainPic;
            SETPAR_AND_ADDCMD(AVP_SURFACE_STATE, m_avpItf, &cmdBuffer);
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_SURFACE_STATE, Av1DecodePicPkt)
    {
        MOS_MEMCOMP_STATE mmcState = {};
        params.surfaceStateId = m_curAvpSurfStateId;
        params.bitDepthLumaMinus8 = m_av1PicParams->m_bitDepthIdx << 1;

        if (params.bitDepthLumaMinus8 == 0)
        {
            params.srcFormat = mhw::vdbox::avp::SURFACE_FORMAT::SURFACE_FORMAT_PLANAR4208;
        }
        else
        {
            params.srcFormat = mhw::vdbox::avp::SURFACE_FORMAT::SURFACE_FORMAT_P010;
        }

        switch (params.surfaceStateId)
        {
            case reconPic:
                params.pitch = m_av1BasicFeature->m_destSurface.dwPitch;
                params.uOffset = m_av1BasicFeature->m_destSurface.YoffsetForUplane;
                params.vOffset = 0;
                DECODE_CHK_STATUS(GetSurfaceMmcInfo(&m_av1BasicFeature->m_destSurface,
                   mmcState, params.compressionFormat));
                std::fill(std::begin(params.mmcState), std::end(params.mmcState), mmcState);
                break;
            case intrabcDecodedFrame:
                DECODE_CHK_STATUS(m_allocator->GetSurfaceInfo(m_intrabcDecodedOutputFrameBuffer));
                params.pitch = m_intrabcDecodedOutputFrameBuffer->dwPitch;
                params.uOffset = m_intrabcDecodedOutputFrameBuffer->YoffsetForUplane;
                params.vOffset = 0;
                break;
            case av1IntraFrame:
            case av1LastRef:
            case av1Last2Ref:
            case av1Last3Ref:
            case av1GoldRef:
            case av1BwdRef:
            case av1AltRef2:
            case av1AltRef:
                DECODE_CHK_STATUS(m_allocator->GetSurfaceInfo(const_cast<PMOS_SURFACE>(&m_refSurface[params.surfaceStateId - av1IntraFrame])));
                params.pitch = m_refSurface[params.surfaceStateId - av1IntraFrame].dwPitch;
                params.uOffset = m_refSurface[params.surfaceStateId - av1IntraFrame].YoffsetForUplane;
                params.vOffset = 0;
                std::copy(std::begin(m_refMmcState), std::end(m_refMmcState), params.mmcState);
                params.compressionFormat = m_refCompressionFormat;
                break;
            case filmGrainPic:
                DECODE_CHK_STATUS(m_allocator->GetSurfaceInfo(m_av1BasicFeature->m_filmGrainProcParams->m_outputSurface));
                params.pitch =  m_av1BasicFeature->m_filmGrainProcParams->m_outputSurface->dwPitch;
                params.uOffset =  m_av1BasicFeature->m_filmGrainProcParams->m_outputSurface->YoffsetForUplane;
                params.vOffset = 0;
                DECODE_CHK_STATUS(GetSurfaceMmcInfo(m_av1BasicFeature->m_filmGrainProcParams->m_outputSurface,
                mmcState, params.compressionFormat));
                std::fill(std::begin(params.mmcState), std::end(params.mmcState), mmcState);
                break;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_PIPE_BUF_ADDR_STATE, Av1DecodePicPkt)
    {
        if (m_av1PicParams->m_picInfoFlags.m_fields.m_allowIntrabc)
        {
            params.intrabcDecodedOutputFrameBuffer = &m_intrabcDecodedOutputFrameBuffer->OsResource;
        }

        params.decodedPic                      = &(m_av1BasicFeature->m_destSurface);
        params.bsLineRowstoreBuffer            = &m_bitstreamDecoderEncoderLineRowstoreReadWriteBuffer->OsResource;
        params.bsTileLineRowstoreBuffer        = &m_bitstreamDecoderEncoderTileLineRowstoreReadWriteBuffer->OsResource;
        params.intraPredLineRowstoreBuffer     = &m_intraPredictionLineRowstoreReadWriteBuffer->OsResource;
        params.intraPredTileLineRowstoreBuffer = &m_intraPredictionTileLineRowstoreReadWriteBuffer->OsResource;
        params.spatialMVLineBuffer             = &m_spatialMotionVectorLineReadWriteBuffer->OsResource;
        params.spatialMVCodingTileLineBuffer   = &m_spatialMotionVectorCodingTileLineReadWriteBuffer->OsResource;
        params.lrMetaTileColumnBuffer          = &m_loopRestorationMetaTileColumnReadWriteBuffer->OsResource;
        params.lrTileLineYBuffer               = &m_loopRestorationFilterTileReadWriteLineYBuffer->OsResource;
        params.lrTileLineUBuffer               = &m_loopRestorationFilterTileReadWriteLineUBuffer->OsResource;
        params.lrTileLineVBuffer               = &m_loopRestorationFilterTileReadWriteLineVBuffer->OsResource;
        params.deblockLineYBuffer              = &m_deblockerFilterLineReadWriteYBuffer->OsResource;
        params.deblockLineUBuffer              = &m_deblockerFilterLineReadWriteUBuffer->OsResource;
        params.deblockLineVBuffer              = &m_deblockerFilterLineReadWriteVBuffer->OsResource;
        params.deblockTileLineYBuffer          = &m_deblockerFilterTileLineReadWriteYBuffer->OsResource;
        params.deblockTileLineUBuffer          = &m_deblockerFilterTileLineReadWriteUBuffer->OsResource;
        params.deblockTileLineVBuffer          = &m_deblockerFilterTileLineReadWriteVBuffer->OsResource;
        params.deblockTileColumnYBuffer        = &m_deblockerFilterTileColumnReadWriteYBuffer->OsResource;
        params.deblockTileColumnUBuffer        = &m_deblockerFilterTileColumnReadWriteUBuffer->OsResource;
        params.deblockTileColumnVBuffer        = &m_deblockerFilterTileColumnReadWriteVBuffer->OsResource;
        params.cdefLineBuffer                  = &m_cdefFilterLineReadWriteBuffer->OsResource;
        params.cdefTileLineBuffer              = &m_cdefFilterTileLineReadWriteBuffer->OsResource;
        params.cdefTileColumnBuffer            = &m_cdefFilterTileColumnReadWriteBuffer->OsResource;
        params.cdefMetaTileLineBuffer          = &m_cdefFilterMetaTileLineReadWriteBuffer->OsResource;
        params.cdefMetaTileColumnBuffer        = &m_cdefFilterMetaTileColumnReadWriteBuffer->OsResource;
        params.cdefTopLeftCornerBuffer         = &m_cdefFilterTopLeftCornerReadWriteBuffer->OsResource;
        params.superResTileColumnYBuffer       = &m_superResTileColumnReadWriteYBuffer->OsResource;
        params.superResTileColumnUBuffer       = &m_superResTileColumnReadWriteUBuffer->OsResource;
        params.superResTileColumnVBuffer       = &m_superResTileColumnReadWriteVBuffer->OsResource;
        params.lrTileColumnYBuffer             = &m_loopRestorationFilterTileColumnReadWriteYBuffer->OsResource;
        params.lrTileColumnUBuffer             = &m_loopRestorationFilterTileColumnReadWriteUBuffer->OsResource;
        params.lrTileColumnVBuffer             = &m_loopRestorationFilterTileColumnReadWriteVBuffer->OsResource;
        params.decodedFrameStatusErrorBuffer   = &m_decodedFrameStatusErrorBuffer->OsResource;
        params.decodedBlockDataStreamoutBuffer = &m_decodedBlockDataStreamoutBuffer->OsResource;
        params.filmGrainTileColumnDataBuffer   = &m_filmGrainTileColumnDataBuf->OsResource;
        params.filmGrainSampleTemplateBuffer   = &m_filmGrainSampleTemplateBuf->OsResource;
        params.lrTileColumnAlignBuffer         = &m_loopRestorationFilterTileColumnAlignmentBuf->OsResource;

        if (m_av1PicParams->m_filmGrainParams.m_filmGrainInfoFlags.m_fields.m_applyGrain)
        {
            params.filmGrainOutputSurface = &(m_av1BasicFeature->m_filmGrainProcParams->m_outputSurface->OsResource);
        }

        auto tempBuffers = &(m_av1BasicFeature->m_tempBuffers);
        PMOS_BUFFER curMvBuffer = tempBuffers->GetCurBuffer()->mvBuf;
        DECODE_CHK_NULL(curMvBuffer);
        params.curMvTempBuffer = &(curMvBuffer->OsResource);

        Av1ReferenceFrames &refFrames = m_av1BasicFeature->m_refFrames;
        uint8_t prevFrameIdx = refFrames.GetPrimaryRefIdx();

        uint32_t refSize = 0;
        if (m_av1PicParams->m_picInfoFlags.m_fields.m_frameType != keyFrame)
        {
            const std::vector<uint8_t> &activeRefList = refFrames.GetActiveReferenceList(
                *m_av1PicParams, m_av1BasicFeature->m_av1TileParams[m_av1BasicFeature->m_tileCoding.m_curTile]);
            refSize = activeRefList.size();

            //set for INTRA_FRAME
            params.refs[0] = &m_av1BasicFeature->m_destSurface.OsResource;
            params.colMvTempBuffer[0] = &(curMvBuffer->OsResource);

            for (uint8_t i = 0; i < activeRefList.size(); i++)
            {
                uint8_t frameIdx = activeRefList[i];
                params.refs[i + lastFrame] = refFrames.GetReferenceByFrameIndex(frameIdx);
                auto tempBuf = tempBuffers->GetBufferByFrameIndex(frameIdx);
                params.colMvTempBuffer[i + lastFrame] = tempBuf ? &(tempBuf->mvBuf->OsResource) : nullptr;
            }
        }

        DECODE_CHK_STATUS(RefAddrErrorConcel());

        DECODE_CHK_NULL(tempBuffers->GetCurBuffer()->initCdfBuf);
        PMOS_BUFFER curInitCdfBuffer = tempBuffers->GetCurBuffer()->initCdfBuf->buffer;
        DECODE_CHK_NULL(curInitCdfBuffer);
        params.cdfTableInitBuffer = &(curInitCdfBuffer->OsResource);

        if (!m_av1PicParams->m_picInfoFlags.m_fields.m_disableFrameEndUpdateCdf)
        {
            PMOS_BUFFER curBwdCdfBuffer = tempBuffers->GetCurBuffer()->bwdAdaptCdfBuf.buffer;
            DECODE_CHK_NULL(curBwdCdfBuffer);
            params.cdfTableBwdAdaptBuffer = &(curBwdCdfBuffer->OsResource);
        }

        if (m_av1PicParams->m_av1SegData.m_enabled && m_av1PicParams->m_av1SegData.m_updateMap)
        {
            PMOS_BUFFER curSegIDWriteBuffer = tempBuffers->GetCurBuffer()->segIdWriteBuf.buffer;
            DECODE_CHK_NULL(curSegIDWriteBuffer);
            params.segmentIdWriteBuffer = &(curSegIDWriteBuffer->OsResource);
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
                params.segmentIdReadBuffer = buf ? &(buf->OsResource) : nullptr;
            }
        }

#ifdef _MMC_SUPPORTED
        if (m_mmcState && m_mmcState->IsMmcEnabled())
        {
            DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcState(const_cast<PMOS_SURFACE>(&m_av1BasicFeature->m_destSurface), &params.mmcStatePreDeblock));
        };
#endif

#if USE_CODECHAL_DEBUG_TOOL
        DECODE_CHK_STATUS(DumpResources(refSize));
#endif

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt::RefAddrErrorConcel() const
    {
        DECODE_FUNC_CALL();

        auto &par = m_avpItf->MHW_GETPAR_F(AVP_PIPE_BUF_ADDR_STATE)();

        if (m_av1PicParams->m_picInfoFlags.m_fields.m_frameType == keyFrame)
        {
            for (auto i = 0; i < av1TotalRefsPerFrame; i++)
            {
                par.refs[i] = nullptr;
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
                if (par.refs[i] == nullptr)
                {
                    par.refs[i] = validRefPic;
                }
            }

            PMOS_BUFFER validMvBuf = m_av1BasicFeature->m_tempBuffers.GetValidBufferForReference(
                                     m_av1BasicFeature->m_refFrameIndexList)->mvBuf;
            for (uint32_t i = 0; i < CODEC_NUM_AV1_TEMP_BUFFERS; i++)
            {
                if (par.colMvTempBuffer[i] == nullptr)
                {
                    par.colMvTempBuffer[i] = &validMvBuf->OsResource;
                }
            }
        }

        PMOS_RESOURCE dummyRef = &(m_av1BasicFeature->m_dummyReference.OsResource);
        if (m_av1BasicFeature->m_useDummyReference && !m_allocator->ResourceIsNull(dummyRef))
        {
            // set all ref pic addresses to valid addresses for error concealment purpose
            for (uint32_t i = 0; i < av1TotalRefsPerFrame; i++)
            {
                if (par.refs[i] == nullptr)
                {
                    par.refs[i] = dummyRef;
                }
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt::SetSkipModeFrameParam() const
    {
        DECODE_FUNC_CALL();

        auto &par = m_avpItf->MHW_GETPAR_F(AVP_PIC_STATE)();

        if (!m_av1PicParams->m_seqInfoFlags.m_fields.m_enableOrderHint ||
            AV1_KEY_OR_INRA_FRAME(m_av1PicParams->m_picInfoFlags.m_fields.m_frameType) ||
            m_av1PicParams->m_modeControlFlags.m_fields.m_referenceMode == singleReference)
        {
            par.skipModeFrame[0] = intraFrame;
            par.skipModeFrame[1] = intraFrame;
        }
        else
        {
            int32_t curFrameOffset = m_av1PicParams->m_orderHint;
            int32_t refFrameOffset[2] = { -1, 0x7fffffff };
            int32_t refIdx[2] = { -1, -1 };
            Av1ReferenceFrames &refFrames = m_av1BasicFeature->m_refFrames;
            DECODE_CHK_STATUS(refFrames.Identify1stNearRef(*m_av1PicParams, curFrameOffset, refFrameOffset, refIdx));

            if (refIdx[0] != -1 && refIdx[1] != -1)
            {
                // == Bi-directional prediction ==
                // cm->is_skip_mode_allowed = 1;
                par.skipModeFrame[0] = AOMMIN(refIdx[0], refIdx[1]);
                par.skipModeFrame[1] = AOMMAX(refIdx[0], refIdx[1]);
            }
            else if (refIdx[0] != -1 && refIdx[1] == -1)
            {
                DECODE_CHK_STATUS(refFrames.Identify2ndNearRef(*m_av1PicParams, curFrameOffset, refFrameOffset, refIdx));
                if (refFrameOffset[1] != -1)
                {
                    par.skipModeFrame[0] = AOMMIN(refIdx[0], refIdx[1]);
                    par.skipModeFrame[1] = AOMMAX(refIdx[0], refIdx[1]);
                }
            }

            par.skipModeFrame[0] += lastFrame;
            par.skipModeFrame[1] += lastFrame;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt::SetRefPicStateParam() const
    {
        DECODE_FUNC_CALL();

        auto &par = m_avpItf->MHW_GETPAR_F(AVP_PIC_STATE)();

        uint8_t refFrameSignBias[8]        = {};
        auto curRefList                    = m_av1BasicFeature->m_refFrames.m_currRefList;
        PCODEC_PICTURE  refFrameList       = &(m_av1PicParams->m_refFrameMap[0]);
        PCODEC_REF_LIST_AV1 *m_refList     = &(m_av1BasicFeature->m_refFrames.m_refList[0]);

        for (auto refFrame = (uint32_t)lastFrame; refFrame <= (uint32_t)altRefFrame; refFrame++) //no bias for intra frame
        {
            if (m_av1PicParams->m_seqInfoFlags.m_fields.m_enableOrderHint &&
                !AV1_KEY_OR_INRA_FRAME(m_av1PicParams->m_picInfoFlags.m_fields.m_frameType))
            {
                int32_t refFrameOffset = -1;

                uint8_t refPicIndex = m_av1PicParams->m_refFrameIdx[refFrame - lastFrame]; //0 corresponds to LAST_FRAME
                PCODEC_PICTURE refFrameList = &(m_av1PicParams->m_refFrameMap[0]);

                if (!CodecHal_PictureIsInvalid(refFrameList[refPicIndex]))
                {
                    uint8_t refFrameIdx = refFrameList[refPicIndex].FrameIdx;
                    refFrameOffset = m_av1BasicFeature->m_refFrames.m_refList[refFrameIdx]->m_orderHint;
                }

                int32_t frameOffset = (int32_t)m_av1PicParams->m_orderHint;
                refFrameSignBias[refFrame] = (m_av1BasicFeature->m_refFrames.GetRelativeDist(*m_av1PicParams, refFrameOffset, frameOffset) <= 0) ? 0 : 1;
            }
            else
            {
                refFrameSignBias[refFrame] = 0;
            }
        }

        par.refFrameRes[intraFrame]    = CAT2SHORTS(m_av1PicParams->m_frameWidthMinus1, m_av1PicParams->m_frameHeightMinus1);
        par.refScaleFactor[intraFrame] = CAT2SHORTS(m_av1ScalingFactor, m_av1ScalingFactor);
        par.refOrderHints[intraFrame]  = m_av1PicParams->m_seqInfoFlags.m_fields.m_enableOrderHint ? curRefList->m_orderHint : 0;
        par.refFrameIdx[0]             = intraFrame;
        par.refFrameSide               = 0;
        uint32_t horizontalScaleFactor, verticalScaleFactor;

        for (auto i = 0; i < av1NumInterRefFrames; i++)
        {
            uint32_t curFrameWidth  = m_av1PicParams->m_frameWidthMinus1 + 1;
            uint32_t curFrameHeight = m_av1PicParams->m_frameHeightMinus1 + 1;

            par.refFrameBiasFlag                   |= (refFrameSignBias[i + lastFrame] << (i + lastFrame));
            par.frameLevelGlobalMotionInvalidFlags |= (m_av1PicParams->m_wm[i].m_invalid << (i + lastFrame));
            par.globalMotionType[i]                 = m_av1PicParams->m_wm[i].m_wmtype;
            par.refFrameIdx[i + lastFrame]          = i + lastFrame;

            if (!AV1_KEY_OR_INRA_FRAME(m_av1PicParams->m_picInfoFlags.m_fields.m_frameType))
            {
                uint8_t refPicIndex = 0xFF;
                if (m_av1PicParams->m_refFrameIdx[i] < av1TotalRefsPerFrame &&
                    refFrameList[m_av1PicParams->m_refFrameIdx[i]].FrameIdx < CODECHAL_MAX_DPB_NUM_AV1)
                {
                    refPicIndex = refFrameList[m_av1PicParams->m_refFrameIdx[i]].FrameIdx;
                }
                else
                {
                    MOS_STATUS hr = m_av1BasicFeature->m_refFrames.GetValidReferenceIndex(&refPicIndex);
                }

                horizontalScaleFactor = (m_refList[refPicIndex]->m_frameWidth * m_av1ScalingFactor + (curFrameWidth >> 1)) / curFrameWidth;
                verticalScaleFactor   = (m_refList[refPicIndex]->m_frameHeight * m_av1ScalingFactor + (curFrameHeight >> 1)) / curFrameHeight;

                par.refFrameRes[i + lastFrame]    = CAT2SHORTS(m_refList[refPicIndex]->m_frameWidth - 1, m_refList[refPicIndex]->m_frameHeight - 1);
                par.refScaleFactor[i + lastFrame] = CAT2SHORTS(verticalScaleFactor, horizontalScaleFactor);
                par.refOrderHints[i + lastFrame]  = m_av1PicParams->m_seqInfoFlags.m_fields.m_enableOrderHint ? curRefList->m_refOrderHint[i] : 0;
            }
            else
            {
                par.refFrameRes[i + lastFrame]    = par.refFrameRes[intraFrame];
                par.refScaleFactor[i + lastFrame] = par.refScaleFactor[intraFrame];
                par.refOrderHints[i + lastFrame]  = par.refOrderHints[intraFrame];
            }

            if ((m_av1BasicFeature->m_refFrames.GetRelativeDist(*m_av1PicParams, par.refOrderHints[i + lastFrame], curRefList->m_orderHint) > 0 ||
                par.refOrderHints[i + lastFrame] == curRefList->m_orderHint) && m_av1PicParams->m_seqInfoFlags.m_fields.m_enableOrderHint)
            {
                par.refFrameSide |= 1 << (i + lastFrame);
            }
        }

        if (AV1_KEY_OR_INRA_FRAME(m_av1PicParams->m_picInfoFlags.m_fields.m_frameType))
        {
            MOS_ZeroMemory(par.refFrameRes, sizeof(par.refFrameRes));
            MOS_ZeroMemory(par.refScaleFactor, sizeof(par.refScaleFactor));
            par.refFrameSide = 0;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPkt::GetSurfaceMmcInfo(PMOS_SURFACE surface, MOS_MEMCOMP_STATE& mmcState, uint32_t& compressionFormat) const
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(surface);
#ifdef _MMC_SUPPORTED
        DECODE_CHK_NULL(m_mmcState);

        if (m_mmcState->IsMmcEnabled())
        {
            DECODE_CHK_STATUS(m_mmcState->SetSurfaceMmcState(surface));
            DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcState(surface, &mmcState));
            DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcFormat(surface, &compressionFormat));
        }
        else
#endif
        {
            mmcState = MOS_MEMCOMP_DISABLED;
        }

        return MOS_STATUS_SUCCESS;
    }

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS Av1DecodePicPkt::DumpResources(uint32_t refSize) const
    {
        DECODE_FUNC_CALL();

        CodechalDebugInterface *debugInterface = m_av1Pipeline->GetDebugInterface();
        debugInterface->m_frameType = m_av1PicParams->m_picInfoFlags.m_fields.m_frameType ? P_TYPE : I_TYPE;
        m_av1PicParams->m_currPic.PicFlags  = PICTURE_FRAME;
        debugInterface->m_currPic      = m_av1PicParams->m_currPic;
        debugInterface->m_bufferDumpFrameNum = m_av1BasicFeature->m_frameNum;

        auto &par = m_avpItf->MHW_GETPAR_F(AVP_PIPE_BUF_ADDR_STATE)();

        if (m_av1PicParams->m_picInfoFlags.m_fields.m_frameType != keyFrame)
        {
            for (uint32_t n = 0; n < refSize; n++)
            {
                MOS_SURFACE refSurface;
                MOS_ZeroMemory(&refSurface, sizeof(MOS_SURFACE));
                refSurface.OsResource = *(par.refs[n + lastFrame]);
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
            if (par.segmentIdReadBuffer != nullptr &&
                !m_allocator->ResourceIsNull(par.segmentIdReadBuffer))
            {
                DECODE_CHK_STATUS(debugInterface->DumpBuffer(
                    par.segmentIdReadBuffer,
                    CodechalDbgAttr::attrSegId,
                    "SegIdReadBuffer",
                    (m_widthInSb * m_heightInSb * CODECHAL_CACHELINE_SIZE),
                    CODECHAL_NUM_MEDIA_STATES));
            } 

            DECODE_CHK_STATUS(debugInterface->DumpBuffer(
                par.cdfTableInitBuffer,
                CodechalDbgAttr::attrCoefProb,
                "CdfTableInitialization",
                m_av1BasicFeature->m_cdfMaxNumBytes,
                CODECHAL_NUM_MEDIA_STATES));
        }

        return MOS_STATUS_SUCCESS;
    }
#endif

}
