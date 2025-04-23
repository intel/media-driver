/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     decode_vvc_picture_packet.cpp
//! \brief    Defines the interface for vvc decode picture packet
//!
#include "decode_vvc_picture_packet.h"
#include "codechal_debug.h"
#include "decode_resource_auto_lock.h"

namespace decode{
    VvcDecodePicPkt::~VvcDecodePicPkt()
    {
        FreeResources();
    }

    MOS_STATUS VvcDecodePicPkt::FreeResources()
    {
        DECODE_FUNC_CALL();

        if (m_allocator != nullptr)
        {
            //sw written buffer
            m_allocator->Destroy(m_alfBufferArray);
            m_allocator->Destroy(m_scalingListBufferArray);
            m_allocator->Destroy(m_chromaQpBufferArray);

            //Tile (boundary) Storage
            m_allocator->Destroy(m_vclfYTileRowBuffer);
            m_allocator->Destroy(m_vclfYTileColumnBuffer);
            m_allocator->Destroy(m_vclfUTileRowBuffer);
            m_allocator->Destroy(m_vclfUTileColumnBuffer);
            m_allocator->Destroy(m_vclfVTileRowBuffer);
            m_allocator->Destroy(m_vclfVTileColumnBuffer);
            m_allocator->Destroy(m_vcSaoYTileRowBuffer);
            m_allocator->Destroy(m_vcSaoYTileColumnBuffer);
            m_allocator->Destroy(m_vcSaoUTileRowBuffer);
            m_allocator->Destroy(m_vcSaoUTileColumnBuffer);
            m_allocator->Destroy(m_vcSaoVTileRowBuffer);
            m_allocator->Destroy(m_vcSaoVTileColumnBuffer);
            m_allocator->Destroy(m_vcAlfTileRowBuffer);
            m_allocator->Destroy(m_vcAlfYTileColumnBuffer);
            m_allocator->Destroy(m_vcAlfUTileColumnBuffer);
            m_allocator->Destroy(m_vcAlfVTileColumnBuffer);

            //rowstore
            if (!m_vvcpItf->IsBufferRowstoreCacheEnabled(vcedLineBuffer))
            {
                m_allocator->Destroy(m_vcedLineBuffer);
            }
            if (!m_vvcpItf->IsBufferRowstoreCacheEnabled(vcmvLineBuffer))
            {
                m_allocator->Destroy(m_vcmvLineBuffer);
            }
            if (!m_vvcpItf->IsBufferRowstoreCacheEnabled(vcprLineBuffer))
            {
                m_allocator->Destroy(m_vcprLineBuffer);
            }
            if (!m_vvcpItf->IsBufferRowstoreCacheEnabled(vclfYLineBuffer))
            {
                m_allocator->Destroy(m_vclfYLineBuffer);
            }
            if (!m_vvcpItf->IsBufferRowstoreCacheEnabled(vclfULineBuffer))
            {
                m_allocator->Destroy(m_vclfULineBuffer);
            }
            if (!m_vvcpItf->IsBufferRowstoreCacheEnabled(vclfVLineBuffer))
            {
                m_allocator->Destroy(m_vclfVLineBuffer);
            }
            if (!m_vvcpItf->IsBufferRowstoreCacheEnabled(vcSaoYLineBuffer))
            {
                m_allocator->Destroy(m_vcSaoYLineBuffer);
            }
            if (!m_vvcpItf->IsBufferRowstoreCacheEnabled(vcSaoULineBuffer))
            {
                m_allocator->Destroy(m_vcSaoULineBuffer);
            }
            if (!m_vvcpItf->IsBufferRowstoreCacheEnabled(vcSaoVLineBuffer))
            {
                m_allocator->Destroy(m_vcSaoVLineBuffer);
            }
            if (!m_vvcpItf->IsBufferRowstoreCacheEnabled(vcAlfLineBuffer))
            {
                m_allocator->Destroy(m_vcAlfLineBuffer);
            }

            // Pic Lvl BB Array
            if (m_picLevelBBArray)
            {
                DECODE_CHK_STATUS(m_allocator->Destroy(m_picLevelBBArray));
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodePicPkt::Init()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_featureManager);
        DECODE_CHK_NULL(m_hwInterface);
        DECODE_CHK_NULL(m_osInterface);
        DECODE_CHK_NULL(m_vvcPipeline);

        m_vvcBasicFeature = dynamic_cast<VvcBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        DECODE_CHK_NULL(m_vvcBasicFeature);

        m_allocator = m_pipeline ->GetDecodeAllocator();
        DECODE_CHK_NULL(m_allocator);

        DECODE_CHK_STATUS(AllocateFixedResources());
        DECODE_CHK_STATUS(CalculatePictureStateCommandSize());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodePicPkt::AllocatePicLvlBB()
    {
        DECODE_FUNC_CALL();
        // Pic Lvl Batch Buffer needed to be lockable and program by driver
        if (m_picLevelBBArray == nullptr)
        {
            m_picLevelBBArray = m_allocator->AllocateBatchBufferArray(
                m_pictureStatesSize, 1, CODEC_VVC_BUFFER_ARRAY_SIZE, true, lockableVideoMem);
            DECODE_CHK_NULL(m_picLevelBBArray);
            m_curPicLvlBatchBuffer = m_picLevelBBArray->Fetch();
            DECODE_CHK_NULL(m_curPicLvlBatchBuffer);
        }
        else
        {
            m_curPicLvlBatchBuffer = m_picLevelBBArray->Fetch();
            DECODE_CHK_NULL(m_curPicLvlBatchBuffer);
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_curPicLvlBatchBuffer, m_pictureStatesSize, lockableVideoMem));
        }
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodePicPkt::Prepare()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_vvcBasicFeature);
        m_vvcPicParams      = m_vvcBasicFeature->m_vvcPicParams;
        DECODE_CHK_NULL(m_vvcPicParams);

#ifdef _MMC_SUPPORTED
        m_mmcState = m_vvcPipeline->GetMmcState();
        DECODE_CHK_NULL(m_mmcState);
#endif
        DECODE_CHK_STATUS(SetRowstoreCachingOffsets());
        DECODE_CHK_STATUS(AllocateVariableResources());
        
        DECODE_CHK_STATUS(AllocatePicLvlBB());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodePicPkt::SetRowstoreCachingOffsets()
    {
        if (m_vvcpItf->IsRowStoreCachingSupported() &&
            (m_vvcBasicFeature->m_frameWidthAlignedMinBlk != MOS_ALIGN_CEIL(m_prevFrmWidth, vvcMinBlockWidth)))
        {
            MHW_VDBOX_ROWSTORE_PARAMS rowstoreParams;
            MOS_ZeroMemory(&rowstoreParams, sizeof(rowstoreParams));
            rowstoreParams.dwPicWidth       = m_vvcBasicFeature->m_frameWidthAlignedMinBlk;
            rowstoreParams.bMbaff           = false;
            rowstoreParams.Mode             = CODECHAL_DECODE_MODE_VVCVLD;
            rowstoreParams.ucBitDepthMinus8 = m_vvcPicParams->m_spsBitdepthMinus8;
            rowstoreParams.ucChromaFormat   = m_vvcBasicFeature->m_chromaFormat;
            rowstoreParams.ucLCUSize        = 1 << (m_vvcPicParams->m_spsLog2CtuSizeMinus5 + 5);
            DECODE_CHK_STATUS(m_hwInterface->SetRowstoreCachingOffsets(&rowstoreParams));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodePicPkt::Execute(MOS_COMMAND_BUFFER& cmdBuffer)
    {
        DECODE_FUNC_CALL()

        PMOS_RESOURCE validRefPic = m_vvcBasicFeature->m_refFrames.GetValidReference();
        if (nullptr == validRefPic)
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }

        DECODE_CHK_STATUS(AddAllCmds_VVCP_SURFACE_STATE(cmdBuffer));

        // temp solution: add here to avoid compile error
        // APS scaling List Data Buffer
        // Scaling List data buffer
        DECODE_CHK_NULL(m_scalingListBufferArray);
        m_apsScalingListBuffer = m_scalingListBufferArray->Fetch();
        DECODE_CHK_NULL(m_apsScalingListBuffer);
        // ALF APS data buffer
        DECODE_CHK_NULL(m_alfBufferArray);
        m_apsAlfBuffer = m_alfBufferArray->Fetch();
        DECODE_CHK_NULL(m_apsAlfBuffer);
        // Chroma QP
        DECODE_CHK_NULL(m_chromaQpBufferArray);
        m_chromaQpBuffer = m_chromaQpBufferArray->Fetch();
        DECODE_CHK_NULL(m_chromaQpBuffer);

        SETPAR_AND_ADDCMD(VVCP_PIPE_BUF_ADDR_STATE, m_vvcpItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(VVCP_IND_OBJ_BASE_ADDR_STATE, m_vvcpItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(VVCP_DPB_STATE, m_vvcpItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(VVCP_PIC_STATE, m_vvcpItf, &cmdBuffer);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodePicPkt::InitVvcState(MOS_COMMAND_BUFFER& cmdBuffer)
    {
        DECODE_FUNC_CALL()

        // Send VD_CONTROL_STATE Pipe Initialization
        SETPAR_AND_ADDCMD(VVCP_VD_CONTROL_STATE, m_vvcpItf, &cmdBuffer);
        DECODE_CHK_STATUS(AddAllCmds_VVCP_PIPE_MODE_SELECT(cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodePicPkt::AllocateFixedResources()
    {
        DECODE_FUNC_CALL();

        // ALF APS buffers
        uint32_t allocSize = MOS_ALIGN_CEIL(m_alfBufferSize, CODECHAL_PAGE_SIZE);
        if (m_alfBufferArray == nullptr)
        {
            m_alfBufferArray= m_allocator->AllocateBufferArray(
                allocSize,
                "ALF APS Data Buffer",
                CODEC_VVC_BUFFER_ARRAY_SIZE,
                resourceInternalReadWriteCache,
                lockableVideoMem);
            DECODE_CHK_NULL(m_alfBufferArray);
        }

        // Scaling List APS buffers
        allocSize = MOS_ALIGN_CEIL(m_scalingListBufferSize, CODECHAL_PAGE_SIZE);
        if (m_scalingListBufferArray == nullptr)
        {
            m_scalingListBufferArray= m_allocator->AllocateBufferArray(
                allocSize,
                "ScalingList APS Data Buffer",
                CODEC_VVC_BUFFER_ARRAY_SIZE,
                resourceInternalReadWriteCache,
                lockableVideoMem);
            DECODE_CHK_NULL(m_scalingListBufferArray);
        }

        //ChromaQp table
        allocSize = MOS_ALIGN_CEIL(m_chromaQpBufferSize, CODECHAL_PAGE_SIZE);
        if (m_chromaQpBufferArray == nullptr)
        {
            m_chromaQpBufferArray= m_allocator->AllocateBufferArray(
                allocSize,
                "ChromaQP Table Buffer",
                CODEC_VVC_BUFFER_ARRAY_SIZE,
                resourceInternalReadWriteCache,
                lockableVideoMem);
            DECODE_CHK_NULL(m_chromaQpBufferArray);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodePicPkt::AllocateVariableResources()
    {
        DECODE_FUNC_CALL();

        VvcpBufferSizePar vvcpBufSizeParam;
        MOS_ZeroMemory(&vvcpBufSizeParam, sizeof(vvcpBufSizeParam));

        vvcpBufSizeParam.m_picWidth             = m_vvcPicParams->m_ppsPicWidthInLumaSamples;
        vvcpBufSizeParam.m_picHeight            = m_vvcPicParams->m_ppsPicHeightInLumaSamples;
        vvcpBufSizeParam.m_maxTileWidthInCtus   = m_vvcBasicFeature->m_maxTileWidth;
        vvcpBufSizeParam.m_bitDepthIdc          = m_vvcPicParams->m_spsBitdepthMinus8;
        vvcpBufSizeParam.m_spsChromaFormatIdc   = m_vvcPicParams->m_spsChromaFormatIdc;
        vvcpBufSizeParam.m_spsLog2CtuSizeMinus5 = m_vvcPicParams->m_spsLog2CtuSizeMinus5;

        // Tile (boundary) Storage
        // VCLF Y Tile Row Buffer (LFYTR)
        DECODE_CHK_STATUS(m_vvcpItf->GetVvcpBufSize(
            vclfYTileRowBuffer,
            &vvcpBufSizeParam));
        if (m_vclfYTileRowBuffer == nullptr)
        {
            m_vclfYTileRowBuffer = m_allocator->AllocateBuffer(
                vvcpBufSizeParam.m_bufferSize,
                "VCLF Y Tile Row Buffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_vclfYTileRowBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_vclfYTileRowBuffer,
                vvcpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }
        // VCLF Y Tile Column Buffer (LFYTC)
        DECODE_CHK_STATUS(m_vvcpItf->GetVvcpBufSize(
            vclfYTileColumnBuffer,
            &vvcpBufSizeParam));
        if (m_vclfYTileColumnBuffer == nullptr)
        {
            m_vclfYTileColumnBuffer = m_allocator->AllocateBuffer(
                vvcpBufSizeParam.m_bufferSize,
                "VCLF Y Tile Column Buffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_vclfYTileColumnBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_vclfYTileColumnBuffer,
                vvcpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }
        // VCLF U Tile Row Buffer (LFUTR)
        DECODE_CHK_STATUS(m_vvcpItf->GetVvcpBufSize(
            vclfUTileRowBuffer,
            &vvcpBufSizeParam));
        if (m_vclfUTileRowBuffer == nullptr)
        {
            m_vclfUTileRowBuffer = m_allocator->AllocateBuffer(
                vvcpBufSizeParam.m_bufferSize,
                "VCLF U Tile Row Buffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_vclfUTileRowBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_vclfUTileRowBuffer,
                vvcpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }
        // VCLF U Tile Column Buffer (LFUTC)
        DECODE_CHK_STATUS(m_vvcpItf->GetVvcpBufSize(
            vclfUTileColumnBuffer,
            &vvcpBufSizeParam));
        if (m_vclfUTileColumnBuffer == nullptr)
        {
            m_vclfUTileColumnBuffer = m_allocator->AllocateBuffer(
                vvcpBufSizeParam.m_bufferSize,
                "VCLF U Tile Column Buffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_vclfUTileColumnBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_vclfUTileColumnBuffer,
                vvcpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }
        // VCLF V Tile Row Buffer (LFVTR)
        DECODE_CHK_STATUS(m_vvcpItf->GetVvcpBufSize(
            vclfVTileRowBuffer,
            &vvcpBufSizeParam));
        if (m_vclfVTileRowBuffer == nullptr)
        {
            m_vclfVTileRowBuffer = m_allocator->AllocateBuffer(
                vvcpBufSizeParam.m_bufferSize,
                "VCLF V Tile Row Buffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_vclfVTileRowBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_vclfVTileRowBuffer,
                vvcpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }
        // VCLF V Tile Column Buffer (LFVTC)
        DECODE_CHK_STATUS(m_vvcpItf->GetVvcpBufSize(
            vclfVTileColumnBuffer,
            &vvcpBufSizeParam));
        if (m_vclfVTileColumnBuffer == nullptr)
        {
            m_vclfVTileColumnBuffer = m_allocator->AllocateBuffer(
                vvcpBufSizeParam.m_bufferSize,
                "VCLF V Tile Column Buffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_vclfVTileColumnBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_vclfVTileColumnBuffer,
                vvcpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }
        // VCSAO Y Tile Row Buffer (SAYTR)
        DECODE_CHK_STATUS(m_vvcpItf->GetVvcpBufSize(
            vcSaoYTileRowBuffer,
            &vvcpBufSizeParam));
        if (m_vcSaoYTileRowBuffer == nullptr)
        {
            m_vcSaoYTileRowBuffer = m_allocator->AllocateBuffer(
                vvcpBufSizeParam.m_bufferSize,
                "VCSAO Y Tile Row Buffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_vcSaoYTileRowBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_vcSaoYTileRowBuffer,
                vvcpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }
        // VCSAO Y Tile Column Buffer (SAYTC)
        DECODE_CHK_STATUS(m_vvcpItf->GetVvcpBufSize(
            vcSaoYTileColumnBuffer,
            &vvcpBufSizeParam));
        if (m_vcSaoYTileColumnBuffer == nullptr)
        {
            m_vcSaoYTileColumnBuffer = m_allocator->AllocateBuffer(
                vvcpBufSizeParam.m_bufferSize,
                "VCSAO Y Tile Column Buffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_vcSaoYTileColumnBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_vcSaoYTileColumnBuffer,
                vvcpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }
        // VCSAO U Tile Row Buffer (SAUTR)
        DECODE_CHK_STATUS(m_vvcpItf->GetVvcpBufSize(
            vcSaoUTileRowBuffer,
            &vvcpBufSizeParam));
        if (m_vcSaoUTileRowBuffer == nullptr)
        {
            m_vcSaoUTileRowBuffer = m_allocator->AllocateBuffer(
                vvcpBufSizeParam.m_bufferSize,
                "VCSAO U Tile Row Buffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_vcSaoUTileRowBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_vcSaoUTileRowBuffer,
                vvcpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }
        // VCSAO U Tile Column Buffer (SAUTC)
        DECODE_CHK_STATUS(m_vvcpItf->GetVvcpBufSize(
            vcSaoUTileColumnBuffer,
            &vvcpBufSizeParam));
        if (m_vcSaoUTileColumnBuffer == nullptr)
        {
            m_vcSaoUTileColumnBuffer = m_allocator->AllocateBuffer(
                vvcpBufSizeParam.m_bufferSize,
                "VCSAO U Tile Column Buffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_vcSaoUTileColumnBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_vcSaoUTileColumnBuffer,
                vvcpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }
        // VCSAO V Tile Row Buffer (SAVTR)
        DECODE_CHK_STATUS(m_vvcpItf->GetVvcpBufSize(
            vcSaoVTileRowBuffer,
            &vvcpBufSizeParam));
        if (m_vcSaoVTileRowBuffer == nullptr)
        {
            m_vcSaoVTileRowBuffer = m_allocator->AllocateBuffer(
                vvcpBufSizeParam.m_bufferSize,
                "VCSAO V Tile Row Buffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_vcSaoVTileRowBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_vcSaoVTileRowBuffer,
                vvcpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }
        // VCSAO V Tile Column Buffer (SAVTC)
        DECODE_CHK_STATUS(m_vvcpItf->GetVvcpBufSize(
            vcSaoVTileColumnBuffer,
            &vvcpBufSizeParam));
        if (m_vcSaoVTileColumnBuffer == nullptr)
        {
            m_vcSaoVTileColumnBuffer = m_allocator->AllocateBuffer(
                vvcpBufSizeParam.m_bufferSize,
                "VCSAO V Tile Column Buffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_vcSaoVTileColumnBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_vcSaoVTileColumnBuffer,
                vvcpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }
        // VCALF Tile Row Buffer (ALFTR)
        DECODE_CHK_STATUS(m_vvcpItf->GetVvcpBufSize(
            vcAlfTileRowBuffer,
            &vvcpBufSizeParam));
        if (m_vcAlfTileRowBuffer == nullptr)
        {
            m_vcAlfTileRowBuffer = m_allocator->AllocateBuffer(
                vvcpBufSizeParam.m_bufferSize,
                "VCALF Tile Row Buffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_vcAlfTileRowBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_vcAlfTileRowBuffer,
                vvcpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }
        // VCALF Tile Row Buffer (ALFTR)
        DECODE_CHK_STATUS(m_vvcpItf->GetVvcpBufSize(
            vcAlfYTileColumnBuffer,
            &vvcpBufSizeParam));
        if (m_vcAlfYTileColumnBuffer == nullptr)
        {
            m_vcAlfYTileColumnBuffer = m_allocator->AllocateBuffer(
                vvcpBufSizeParam.m_bufferSize,
                "VCALF Y Tile Column Buffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_vcAlfYTileColumnBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_vcAlfYTileColumnBuffer,
                vvcpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }
        // VCALF U Tile Column Buffer (ALUTC)
        DECODE_CHK_STATUS(m_vvcpItf->GetVvcpBufSize(
            vcAlfUTileColumnBuffer,
            &vvcpBufSizeParam));
        if (m_vcAlfUTileColumnBuffer == nullptr)
        {
            m_vcAlfUTileColumnBuffer = m_allocator->AllocateBuffer(
                vvcpBufSizeParam.m_bufferSize,
                "VCALF U Tile Column Buffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_vcAlfUTileColumnBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_vcAlfUTileColumnBuffer,
                vvcpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }
        // VCALF V Tile Column Buffer (ALVTC)
        DECODE_CHK_STATUS(m_vvcpItf->GetVvcpBufSize(
            vcAlfVTileColumnBuffer,
            &vvcpBufSizeParam));
        if (m_vcAlfVTileColumnBuffer == nullptr)
        {
            m_vcAlfVTileColumnBuffer = m_allocator->AllocateBuffer(
                vvcpBufSizeParam.m_bufferSize,
                "VCALF V Tile Column Buffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_vcAlfVTileColumnBuffer);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(
                m_vcAlfVTileColumnBuffer,
                vvcpBufSizeParam.m_bufferSize,
                notLockableVideoMem));
        }

        // Rowstore buffers
        // VCED Line Buffer (EDLB)
        if (!m_vvcpItf->IsBufferRowstoreCacheEnabled(vcedLineBuffer))
        {
            DECODE_CHK_STATUS(m_vvcpItf->GetVvcpBufSize(
                vcedLineBuffer,
                &vvcpBufSizeParam));

            if (m_vcedLineBuffer == nullptr)
            {
                m_vcedLineBuffer = m_allocator->AllocateBuffer(
                    vvcpBufSizeParam.m_bufferSize,
                    "VCED Line Buffer (EDLB)",
                    resourceInternalReadWriteCache,
                    notLockableVideoMem);
                DECODE_CHK_NULL(m_vcedLineBuffer);
            }
            else
            {
                DECODE_CHK_STATUS(m_allocator->Resize(
                    m_vcedLineBuffer,
                    vvcpBufSizeParam.m_bufferSize,
                    notLockableVideoMem));
            }
        }
        // VCMV Line Buffer (MVLB)
        if (!m_vvcpItf->IsBufferRowstoreCacheEnabled(vcmvLineBuffer))
        {
            DECODE_CHK_STATUS(m_vvcpItf->GetVvcpBufSize(
                vcmvLineBuffer,
                &vvcpBufSizeParam));

            if (m_vcmvLineBuffer == nullptr)
            {
                m_vcmvLineBuffer = m_allocator->AllocateBuffer(
                    vvcpBufSizeParam.m_bufferSize,
                    "VCMV Line Buffer (MVLB)",
                    resourceInternalReadWriteCache,
                    notLockableVideoMem);
                DECODE_CHK_NULL(m_vcmvLineBuffer);
            }
            else
            {
                DECODE_CHK_STATUS(m_allocator->Resize(
                    m_vcmvLineBuffer,
                    vvcpBufSizeParam.m_bufferSize,
                    notLockableVideoMem));
            }
        }
        // VCPR Line Buffer (PRLB)
        if (!m_vvcpItf->IsBufferRowstoreCacheEnabled(vcprLineBuffer))
        {
            DECODE_CHK_STATUS(m_vvcpItf->GetVvcpBufSize(
                vcprLineBuffer,
                &vvcpBufSizeParam));

            if (m_vcprLineBuffer == nullptr)
            {
                m_vcprLineBuffer = m_allocator->AllocateBuffer(
                    vvcpBufSizeParam.m_bufferSize,
                    "VCPR Line Buffer (PRLB)",
                    resourceInternalReadWriteCache,
                    notLockableVideoMem);
                DECODE_CHK_NULL(m_vcprLineBuffer);
            }
            else
            {
                DECODE_CHK_STATUS(m_allocator->Resize(
                    m_vcprLineBuffer,
                    vvcpBufSizeParam.m_bufferSize,
                    notLockableVideoMem));
            }
        }
        // VCLF Y Line Buffer (LFYLB)
        if (!m_vvcpItf->IsBufferRowstoreCacheEnabled(vclfYLineBuffer))
        {
            DECODE_CHK_STATUS(m_vvcpItf->GetVvcpBufSize(
                vclfYLineBuffer,
                &vvcpBufSizeParam));

            if (m_vclfYLineBuffer == nullptr)
            {
                m_vclfYLineBuffer = m_allocator->AllocateBuffer(
                    vvcpBufSizeParam.m_bufferSize,
                    "VCPR Line Buffer (PRLB)",
                    resourceInternalReadWriteCache,
                    notLockableVideoMem);
                DECODE_CHK_NULL(m_vclfYLineBuffer);
            }
            else
            {
                DECODE_CHK_STATUS(m_allocator->Resize(
                    m_vclfYLineBuffer,
                    vvcpBufSizeParam.m_bufferSize,
                    notLockableVideoMem));
            }
        }
        // VCLF U Line Buffer (LFULB)
        if (!m_vvcpItf->IsBufferRowstoreCacheEnabled(vclfULineBuffer))
        {
            DECODE_CHK_STATUS(m_vvcpItf->GetVvcpBufSize(
                vclfULineBuffer,
                &vvcpBufSizeParam));

            if (m_vclfULineBuffer == nullptr)
            {
                m_vclfULineBuffer = m_allocator->AllocateBuffer(
                    vvcpBufSizeParam.m_bufferSize,
                    "VCPR Line Buffer (PRLB)",
                    resourceInternalReadWriteCache,
                    notLockableVideoMem);
                DECODE_CHK_NULL(m_vclfULineBuffer);
            }
            else
            {
                DECODE_CHK_STATUS(m_allocator->Resize(
                    m_vclfULineBuffer,
                    vvcpBufSizeParam.m_bufferSize,
                    notLockableVideoMem));
            }
        }
        // VCLF V Line Buffer (LFVLB)
        if (!m_vvcpItf->IsBufferRowstoreCacheEnabled(vclfVLineBuffer))
        {
            DECODE_CHK_STATUS(m_vvcpItf->GetVvcpBufSize(
                vclfVLineBuffer,
                &vvcpBufSizeParam));

            if (m_vclfVLineBuffer == nullptr)
            {
                m_vclfVLineBuffer = m_allocator->AllocateBuffer(
                    vvcpBufSizeParam.m_bufferSize,
                    "VCLF V Line Buffer (LFVLB)",
                    resourceInternalReadWriteCache,
                    notLockableVideoMem);
                DECODE_CHK_NULL(m_vclfVLineBuffer);
            }
            else
            {
                DECODE_CHK_STATUS(m_allocator->Resize(
                    m_vclfVLineBuffer,
                    vvcpBufSizeParam.m_bufferSize,
                    notLockableVideoMem));
            }
        }
        // VCSAO Y Line Buffer (SAYLB)
        if (!m_vvcpItf->IsBufferRowstoreCacheEnabled(vcSaoYLineBuffer))
        {
            DECODE_CHK_STATUS(m_vvcpItf->GetVvcpBufSize(
                vcSaoYLineBuffer,
                &vvcpBufSizeParam));

            if (m_vcSaoYLineBuffer == nullptr)
            {
                m_vcSaoYLineBuffer = m_allocator->AllocateBuffer(
                    vvcpBufSizeParam.m_bufferSize,
                    "VCSAO Y Line Buffer (SAYLB)",
                    resourceInternalReadWriteCache,
                    notLockableVideoMem);
                DECODE_CHK_NULL(m_vcSaoYLineBuffer);
            }
            else
            {
                DECODE_CHK_STATUS(m_allocator->Resize(
                    m_vcSaoYLineBuffer,
                    vvcpBufSizeParam.m_bufferSize,
                    notLockableVideoMem));
            }
        }
        // VCSAO U Line Buffer (SAULB)
        if (!m_vvcpItf->IsBufferRowstoreCacheEnabled(vcSaoULineBuffer))
        {
            DECODE_CHK_STATUS(m_vvcpItf->GetVvcpBufSize(
                vcSaoULineBuffer,
                &vvcpBufSizeParam));

            if (m_vcSaoULineBuffer == nullptr)
            {
                m_vcSaoULineBuffer = m_allocator->AllocateBuffer(
                    vvcpBufSizeParam.m_bufferSize,
                    "VCSAO U Line Buffer (SAULB)",
                    resourceInternalReadWriteCache,
                    notLockableVideoMem);
                DECODE_CHK_NULL(m_vcSaoULineBuffer);
            }
            else
            {
                DECODE_CHK_STATUS(m_allocator->Resize(
                    m_vcSaoULineBuffer,
                    vvcpBufSizeParam.m_bufferSize,
                    notLockableVideoMem));
            }
        }
        // VCSAO V Line Buffer (SAVLB)
        if (!m_vvcpItf->IsBufferRowstoreCacheEnabled(vcSaoVLineBuffer))
        {
            DECODE_CHK_STATUS(m_vvcpItf->GetVvcpBufSize(
                vcSaoVLineBuffer,
                &vvcpBufSizeParam));

            if (m_vcSaoVLineBuffer == nullptr)
            {
                m_vcSaoVLineBuffer = m_allocator->AllocateBuffer(
                    vvcpBufSizeParam.m_bufferSize,
                    "VCSAO V Line Buffer (SAVLB)",
                    resourceInternalReadWriteCache,
                    notLockableVideoMem);
                DECODE_CHK_NULL(m_vcSaoVLineBuffer);
            }
            else
            {
                DECODE_CHK_STATUS(m_allocator->Resize(
                    m_vcSaoVLineBuffer,
                    vvcpBufSizeParam.m_bufferSize,
                    notLockableVideoMem));
            }
        }
        // VCALF Line Buffer (ALFLB)
        if (!m_vvcpItf->IsBufferRowstoreCacheEnabled(vcAlfLineBuffer))
        {
            DECODE_CHK_STATUS(m_vvcpItf->GetVvcpBufSize(
                vcAlfLineBuffer,
                &vvcpBufSizeParam));

            if (m_vcAlfLineBuffer == nullptr)
            {
                m_vcAlfLineBuffer = m_allocator->AllocateBuffer(
                    vvcpBufSizeParam.m_bufferSize,
                    "VCALF Line Buffer (ALFLB)",
                    resourceInternalReadWriteCache,
                    notLockableVideoMem);
                DECODE_CHK_NULL(m_vcAlfLineBuffer);
            }
            else
            {
                DECODE_CHK_STATUS(m_allocator->Resize(
                    m_vcAlfLineBuffer,
                    vvcpBufSizeParam.m_bufferSize,
                    notLockableVideoMem));
            }
        }


        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodePicPkt::FixVvcpPipeBufAddrParams() const
    {
        DECODE_FUNC_CALL();

        uint8_t validRefFrameIdx = m_vvcBasicFeature->m_refFrames.GetValidReferenceFrameIdx();
        DECODE_ASSERT(validRefFrameIdx < CODEC_MAX_DPB_NUM_VVC);

        auto &params = m_vvcpItf->MHW_GETPAR_F(VVCP_PIPE_BUF_ADDR_STATE)();
        PMOS_RESOURCE validRefPic = m_vvcBasicFeature->m_refFrames.GetReferenceByFrameIndex(validRefFrameIdx);
        for (uint8_t i = 0; i < vvcMaxNumRefFrame; i++)
        {
            // error concealment for the unset reference addresses and unset mv buffers
            if (params.references[i] == nullptr)
            {
                params.references[i] = validRefPic;
            }
        }

        PMOS_BUFFER validMvBuf = m_vvcBasicFeature->m_mvBuffers.GetValidBufferForReference(
                                    m_vvcBasicFeature->m_refFrameIndexList);
        for (uint32_t i = 0; i < vvcMaxNumRefFrame; i++)
        {
            if (params.colMvTemporalBuffer[i] == nullptr)
            {
                params.colMvTemporalBuffer[i] = &validMvBuf->OsResource;
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodePicPkt::SetScalingListDataBuffer(uint8_t *buffer, uint32_t size) const
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_NULL(buffer);

        uint8_t apsId = m_vvcPicParams->m_phScalingListApsId;
        DECODE_CHK_COND((m_vvcBasicFeature->m_activeScalingListMask & (1 << apsId)) == 0, "Invalid scaling list id.");

        const CodecVvcQmData &scalingList = m_vvcBasicFeature->m_scalingListArray[apsId];
        MOS_SecureMemcpy(buffer, size, &scalingList, sizeof(scalingList));
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodePicPkt::SetDataBuffers() const
    {
        DECODE_FUNC_CALL();

        if (m_vvcBasicFeature->m_activeAlfMask != 0)
        {
            uint32_t alfTempBuffer[m_alfBufferSize];
            // Set ALF Data buffers with accumulated ALF tables
            DECODE_CHK_STATUS(m_vvcpItf->SetAlfApsDataBuffer(alfTempBuffer, m_vvcBasicFeature->m_alfApsArray, m_vvcBasicFeature->m_activeAlfMask));
            ResourceAutoLock resLock(m_allocator, &m_apsAlfBuffer->OsResource);
            uint32_t* data = static_cast<uint32_t*>(resLock.LockResourceForWrite());
            DECODE_CHK_NULL(data);
            MOS_SecureMemcpy(data, m_alfBufferSize, alfTempBuffer, m_alfBufferSize);
        }

        if (m_vvcPicParams->m_spsFlags2.m_fields.m_spsExplicitScalingListEnabledFlag &&
            m_vvcPicParams->m_phFlags.m_fields.m_phExplicitScalingListEnabledFlag)
        {
            DECODE_CHK_COND(m_vvcBasicFeature->m_activeScalingListMask == 0, "Invalid scaling list enable flag.");
            ResourceAutoLock resLock(m_allocator, &m_apsScalingListBuffer->OsResource);
            uint8_t* data = static_cast<uint8_t*>(resLock.LockResourceForWrite());
            // Set Scaling List APS Data buffers with accumulated Scaling List tables
            DECODE_CHK_STATUS(SetScalingListDataBuffer(data, m_apsScalingListBuffer->size));
        }

        ResourceAutoLock resLock(m_allocator, &m_chromaQpBuffer->OsResource);
        uint8_t* data = static_cast<uint8_t*>(resLock.LockResourceForWrite());
        DECODE_CHK_NULL(data);
        //change to copy for 3 times because ddi parse m_chromaQpTable[3][112] while hw support m_chromaQpTable[3][76]
        MOS_SecureMemcpy(data, 76 * sizeof(int8_t), m_vvcPicParams->m_chromaQpTable[0], 76 * sizeof(int8_t));
        MOS_SecureMemcpy(data + 76, 76 * sizeof(int8_t), m_vvcPicParams->m_chromaQpTable[1], 76 * sizeof(int8_t));
        MOS_SecureMemcpy(data + 76 * 2, 76 * sizeof(int8_t), m_vvcPicParams->m_chromaQpTable[2], 76 * sizeof(int8_t));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodePicPkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
    {
        DECODE_FUNC_CALL();

        commandBufferSize       = m_pictureStatesSize;
        requestedPatchListSize  = m_picturePatchListSize;

        return MOS_STATUS_SUCCESS;
    }

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS VvcDecodePicPkt::DumpResources() const
    {
        DECODE_FUNC_CALL();

        CodechalDebugInterface *debugInterface = m_vvcPipeline->GetDebugInterface();
        debugInterface->m_frameType            = m_vvcBasicFeature->m_pictureCodingType;
        debugInterface->m_currPic              = m_vvcPicParams->m_currPic;
        debugInterface->m_bufferDumpFrameNum   = m_vvcBasicFeature->m_frameNum;

        auto &par = m_vvcpItf->MHW_GETPAR_F(VVCP_PIPE_BUF_ADDR_STATE)();

        // For multi-slices per frame case, only need dump these resources once.
        if (m_vvcBasicFeature->m_curSlice == 0)
        {
            if (!m_vvcPicParams->m_picMiscFlags.m_fields.m_intraPicFlag)
            {
                for (auto n = 0; n < vvcMaxNumRefFrame; n++)
                {
                    if (par.references[n] != nullptr)
                    {
                        MOS_SURFACE refSurface;
                        MOS_ZeroMemory(&refSurface, sizeof(MOS_SURFACE));
                        refSurface.OsResource = *(par.references[n]);
                        DECODE_CHK_STATUS(m_allocator->GetSurfaceInfo(&refSurface));
                        std::string refSurfName = "RefSurf[" + std::to_string(static_cast<uint32_t>(n)) + "]";
                        DECODE_CHK_STATUS(debugInterface->DumpYUVSurface(
                            &refSurface,
                            CodechalDbgAttr::attrDecodeReferenceSurfaces,
                            refSurfName.c_str()));
                    }
                }
            }

            if (par.apsAlfBuffer != nullptr &&
                !m_allocator->ResourceIsNull(par.apsAlfBuffer))
            {
                DECODE_CHK_STATUS(debugInterface->DumpBuffer(
                    par.apsAlfBuffer,
                    CodechalDbgAttr::attrAlfData,
                    "AlfPipeBuf",
                    MOS_ALIGN_CEIL(m_alfBufferSize, CODECHAL_PAGE_SIZE),
                    CODECHAL_NUM_MEDIA_STATES));
            }
        }

        return MOS_STATUS_SUCCESS;
    }
#endif

    MOS_STATUS VvcDecodePicPkt::CalculatePictureStateCommandSize()
    {
        MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;

        stateCmdSizeParams.bShortFormat    = true;
        stateCmdSizeParams.bHucDummyStream = false;
        stateCmdSizeParams.bSfcInUse       = false;
        // Picture Level Commands
        DECODE_CHK_STATUS(m_hwInterface->GetVvcpStateCommandSize(
                m_vvcBasicFeature->m_mode,
                &m_pictureStatesSize,
                &m_picturePatchListSize,
                &stateCmdSizeParams));

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VVCP_VD_CONTROL_STATE, VvcDecodePicPkt)
    {
        params = {};

        params.pipelineInitialization = true;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodePicPkt::AddAllCmds_VVCP_PIPE_MODE_SELECT(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        // for Gen11+, we need to add MFX wait for both KIN and VRT before and after VVCP Pipemode select...
        auto &mfxWaitParams               = m_miItf->MHW_GETPAR_F(MFX_WAIT)();
        mfxWaitParams                     = {};
        mfxWaitParams.iStallVdboxPipeline = true;
        DECODE_CHK_STATUS((m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer)));

        DECODE_CHK_NULL(m_vvcpItf);
        SETPAR_AND_ADDCMD(VVCP_PIPE_MODE_SELECT, m_vvcpItf, &cmdBuffer);

        mfxWaitParams                     = {};
        mfxWaitParams.iStallVdboxPipeline = true;
        DECODE_CHK_STATUS((m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer)));

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VVCP_PIPE_MODE_SELECT, VvcDecodePicPkt)
    {
        params = {};

        params.codecSelect                = 0; // default value for decode
        params.picStatusErrorReportEnable = false;
        params.codecStandardSelect        = CODEC_STANDARD_SELECT_VVC;
        params.picStatusErrorReportId     = false;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodePicPkt::AddAllCmds_VVCP_SURFACE_STATE(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        // Dest surface
        m_curVvcpSurfStateId = reconPic;
        SETPAR_AND_ADDCMD(VVCP_SURFACE_STATE, m_vvcpItf, &cmdBuffer);

        // Ref surface
        if (!m_vvcPicParams->m_picMiscFlags.m_fields.m_intraPicFlag)
        {
            VvcReferenceFrames &refFrames = m_vvcBasicFeature->m_refFrames;
            uint8_t frameIdx = CODEC_MAX_DPB_NUM_VVC;
            for (uint8_t i = 0; i < vvcMaxNumRefFrame; i++)
            {
                m_curVvcpSurfStateId = i + vvcRefPic0;
                bool valRes = false;
                if (m_vvcPicParams->m_refFrameList[i].PicFlags != PICTURE_INVALID)
                {
                    frameIdx = m_vvcPicParams->m_refFrameList[i].FrameIdx;
                    PMOS_RESOURCE refSurf = refFrames.GetReferenceByFrameIndex(frameIdx);
                    if (refSurf != nullptr)
                    {
                        m_refSurface[i].OsResource = *refSurf;
                        valRes                     = true;
                    }
                }

                if (valRes == false)
                {
                    frameIdx              = refFrames.GetValidReferenceFrameIdx();
                    PMOS_RESOURCE refSurf = refFrames.GetReferenceByFrameIndex(frameIdx);
                    if (refSurf == nullptr)
                    {
                        return MOS_STATUS_INVALID_PARAMETER;
                    }
                    m_refSurface[i].OsResource = *refSurf;
                }

                SETPAR_AND_ADDCMD(VVCP_SURFACE_STATE, m_vvcpItf, &cmdBuffer);
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VVCP_SURFACE_STATE, VvcDecodePicPkt)
    {
        params = {};

        uint32_t     uvPlaneAlignment = 8;
        PMOS_SURFACE psSurface        = nullptr;

        params.surfaceId = m_curVvcpSurfStateId;
        if (params.surfaceId == vvcReconPic)
        {
            // Decode dest surface
            psSurface = &m_vvcBasicFeature->m_destSurface;

        }
        else if (params.surfaceId >= vvcRefPic0 && params.surfaceId <= vvcRefPic14)
        {
            // Decode ref surface
            psSurface = const_cast<PMOS_SURFACE>(&m_refSurface[params.surfaceId - vvcRefPic0]);
            DECODE_CHK_STATUS(m_allocator->GetSurfaceInfo(psSurface));
        }
        else
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }

        params.surfacePitchMinus1   = psSurface->dwPitch - 1;
        params.yOffsetForUCbInPixel = MOS_ALIGN_CEIL((psSurface->UPlaneOffset.iSurfaceOffset - psSurface->dwOffset) /
            psSurface->dwPitch + psSurface->RenderOffset.YUV.U.YOffset,
            uvPlaneAlignment);

#ifdef _MMC_SUPPORTED
        if (params.surfaceId == vvcReconPic)
        {
            DECODE_CHK_STATUS(m_mmcState->SetSurfaceMmcState(&(m_vvcBasicFeature->m_destSurface)));
        }
        DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcFormat(psSurface, &params.compressionFormat));
#endif

        if (m_vvcPicParams->m_spsBitdepthMinus8 == 0)
        {
            if (m_vvcPicParams->m_spsChromaFormatIdc == HCP_CHROMA_FORMAT_YUV420 && psSurface->Format == Format_NV12)  // 4:2:0 8bit surface
            {
                params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_PLANAR4208;
            }
            else if (m_vvcPicParams->m_spsChromaFormatIdc == HCP_CHROMA_FORMAT_YUV420 && psSurface->Format == Format_P010)  // 4:2:0 10bit surface
            {
                params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_P010;
            }
            else
            {
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }
        else if (m_vvcPicParams->m_spsBitdepthMinus8 == 2)
        {
            if (m_vvcPicParams->m_spsChromaFormatIdc == HCP_CHROMA_FORMAT_YUV420 && psSurface->Format == Format_P010)  // 4:2:0 10bit surface
            {
                params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_P010;
            }
            else
            {
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }
        else
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VVCP_PIPE_BUF_ADDR_STATE, VvcDecodePicPkt)
    {
        params = {};

        // Decoded Picture
        params.decodedPic = &(m_vvcBasicFeature->m_destSurface);

        // Current MV buffer
        auto mvBuffers = &(m_vvcBasicFeature->m_mvBuffers);
        MOS_BUFFER *curMvBuffer = mvBuffers->GetCurBuffer();
        DECODE_CHK_NULL(curMvBuffer);
        params.curMvTemporalBuffer = &(curMvBuffer->OsResource);

        // Collocated MV buffer & reference frames
        VvcReferenceFrames &refFrames = m_vvcBasicFeature->m_refFrames;
        if (!refFrames.m_curIsIntra)
        {
            for (uint8_t i = 0; i < vvcMaxNumRefFrame; i++)
            {
                if (m_vvcPicParams->m_refFrameList[i].PicFlags != PICTURE_INVALID)
                {
                    uint8_t frameIdx = m_vvcPicParams->m_refFrameList[i].FrameIdx;
                    if (m_vvcPicParams->m_refFrameList[i].PicFlags == PICTURE_UNAVAILABLE_FRAME)
                    {
                        params.colMvTemporalBuffer[i] = &curMvBuffer->OsResource;
                    }
                    else
                    {
                        MOS_BUFFER *mvBuf = mvBuffers->GetBufferByFrameIndex(frameIdx);
                        DECODE_CHK_NULL(mvBuf);
                        params.colMvTemporalBuffer[i] = &mvBuf->OsResource;
                    }
                    params.references[i] = refFrames.GetReferenceByFrameIndex(frameIdx);
                }
                else
                {
                    params.colMvTemporalBuffer[i] = nullptr;
                    params.references[i]          = nullptr;
                }
            }
        }

        DECODE_CHK_STATUS(FixVvcpPipeBufAddrParams());

        params.apsScalingListDataBuffer = &m_apsScalingListBuffer->OsResource;
        params.apsAlfBuffer = &m_apsAlfBuffer->OsResource;
        params.spsChromaQpTableBuffer = &m_chromaQpBuffer->OsResource;

        params.vcedLineBuffer = &m_vcedLineBuffer->OsResource;
        params.vcmvLineBuffer = &m_vcmvLineBuffer->OsResource;
        params.vcprLineBuffer = &m_vcprLineBuffer->OsResource;

        params.vclfYLineBuffer       = &m_vclfYLineBuffer->OsResource;
        params.vclfYTileRowBuffer    = &m_vclfYTileRowBuffer->OsResource;
        params.vclfYTileColumnBuffer = &m_vclfYTileColumnBuffer->OsResource;
        params.vclfULineBuffer       = &m_vclfULineBuffer->OsResource;
        params.vclfUTileRowBuffer    = &m_vclfUTileRowBuffer->OsResource;
        params.vclfUTileColumnBuffer = &m_vclfUTileColumnBuffer->OsResource;
        params.vclfVLineBuffer       = &m_vclfVLineBuffer->OsResource;
        params.vclfVTileRowBuffer    = &m_vclfVTileRowBuffer->OsResource;
        params.vclfVTileColumnBuffer = &m_vclfVTileColumnBuffer->OsResource;

        params.vcSaoYLineBuffer       = &m_vcSaoYLineBuffer->OsResource;
        params.vcSaoYTileRowBuffer    = &m_vcSaoYTileRowBuffer->OsResource;
        params.vcSaoYTileColumnBuffer = &m_vcSaoYTileColumnBuffer->OsResource;
        params.vcSaoULineBuffer       = &m_vcSaoULineBuffer->OsResource;
        params.vcSaoUTileRowBuffer    = &m_vcSaoUTileRowBuffer->OsResource;
        params.vcSaoUTileColumnBuffer = &m_vcSaoUTileColumnBuffer->OsResource;
        params.vcSaoVLineBuffer       = &m_vcSaoVLineBuffer->OsResource;
        params.vcSaoVTileRowBuffer    = &m_vcSaoVTileRowBuffer->OsResource;
        params.vcSaoVTileColumnBuffer = &m_vcSaoVTileColumnBuffer->OsResource;

        params.vcAlfLineBuffer        = &m_vcAlfLineBuffer->OsResource;
        params.vcAlfTileRowBuffer     = &m_vcAlfTileRowBuffer->OsResource;
        params.vcAlfYTileColumnBuffer = &m_vcAlfYTileColumnBuffer->OsResource;
        params.vcAlfUTileColumnBuffer = &m_vcAlfUTileColumnBuffer->OsResource;
        params.vcAlfVTileColumnBuffer = &m_vcAlfVTileColumnBuffer->OsResource;

        DECODE_CHK_STATUS(SetDataBuffers());

#if USE_CODECHAL_DEBUG_TOOL
        DECODE_CHK_STATUS(DumpResources());
#endif

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VVCP_IND_OBJ_BASE_ADDR_STATE, VvcDecodePicPkt)
    {
        params = {};

        params.dwDataSize     = m_vvcBasicFeature->m_dataSize;
        params.dwDataOffset   = m_vvcBasicFeature->m_dataOffset;
        params.presDataBuffer = &(m_vvcBasicFeature->m_resDataBuffer.OsResource);

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VVCP_PIC_STATE, VvcDecodePicPkt)
    {
        params = {};

        CodecVvcPicParams *picParams = m_vvcBasicFeature->m_vvcPicParams;

        params.spsSubpicInfoPresentFlag        = picParams->m_spsFlags0.m_fields.m_spsSubpicInfoPresentFlag;
        params.spsIndependentSubpicsFlag       = picParams->m_spsFlags0.m_fields.m_spsIndependentSubpicsFlag;
        params.spsSubpicSameSizeFlag           = picParams->m_spsFlags0.m_fields.m_spsSubpicSameSizeFlag;
        params.spsEntropyCodingSyncEnabledFlag = picParams->m_spsFlags0.m_fields.m_spsEntropyCodingSyncEnabledFlag;
        params.spsQtbttDualTreeIntraFlag       = picParams->m_spsFlags0.m_fields.m_spsQtbttDualTreeIntraFlag;
        params.spsMaxLumaTransformSize64Flag   = picParams->m_spsFlags0.m_fields.m_spsMaxLumaTransformSize64Flag;
        params.spsTransformSkipEnabledFlag     = picParams->m_spsFlags0.m_fields.m_spsTransformSkipEnabledFlag;
        params.spsBdpcmEnabledFlag             = picParams->m_spsFlags0.m_fields.m_spsBdpcmEnabledFlag;
        params.spsMtsEnabledFlag               = picParams->m_spsFlags0.m_fields.m_spsMtsEnabledFlag;
        params.spsExplicitMtsIntraEnabledFlag  = picParams->m_spsFlags0.m_fields.m_spsExplicitMtsIntraEnabledFlag;
        params.spsExplicitMtsInterEnabledFlag  = picParams->m_spsFlags0.m_fields.m_spsExplicitMtsInterEnabledFlag;
        params.spsLfnstEnabledFlag             = picParams->m_spsFlags0.m_fields.m_spsLfnstEnabledFlag;
        params.spsJointCbcrEnabledFlag         = picParams->m_spsFlags0.m_fields.m_spsJointCbcrEnabledFlag;
        params.spsSameQpTableForChromaFlag     = picParams->m_spsFlags0.m_fields.m_spsSameQpTableForChromaFlag;

        // derived flags
        params.dLmcsDisabledFlag      = ((picParams->m_spsFlags0.m_fields.m_spsLmcsEnabledFlag == 0) ||
                                        (picParams->m_spsFlags0.m_fields.m_spsLmcsEnabledFlag == 1 &&
                                            picParams->m_phFlags.m_fields.m_phLmcsEnabledFlag == 0))
                                             ? 1
                                             : 0;
        params.dDblkDisabledFlag      = ((picParams->m_ppsFlags.m_fields.m_ppsDeblockingFilterDisabledFlag == 1 &&
                                         picParams->m_ppsFlags.m_fields.m_ppsDeblockingFilterOverrideEnabledFlag == 0) ||
                                        (picParams->m_ppsFlags.m_fields.m_ppsDeblockingFilterOverrideEnabledFlag == 1 &&
                                            picParams->m_phFlags.m_fields.m_phDeblockingFilterDisabledFlag == 1 &&
                                            picParams->m_ppsFlags.m_fields.m_ppsDbfInfoInPhFlag == 1))
                                             ? 1
                                             : 0;
        params.dSaoLumaDisabledFlag   = ((picParams->m_spsFlags0.m_fields.m_spsSaoEnabledFlag == 0) ||
                                           (picParams->m_ppsFlags.m_fields.m_ppsSaoInfoInPhFlag == 1 &&
                                               picParams->m_phFlags.m_fields.m_phSaoLumaEnabledFlag == 0))
                                             ? 1
                                             : 0;
        params.dSaoChromaDisabledFlag = ((picParams->m_spsFlags0.m_fields.m_spsSaoEnabledFlag == 0) ||
                                             (picParams->m_ppsFlags.m_fields.m_ppsSaoInfoInPhFlag == 1 &&
                                                 picParams->m_phFlags.m_fields.m_phSaoChromaEnabledFlag == 0))
                                             ? 1
                                             : 0;
        params.dAlfDisabledFlag       = ((picParams->m_spsFlags0.m_fields.m_spsAlfEnabledFlag == 0) ||
                                       (picParams->m_ppsFlags.m_fields.m_ppsAlfInfoInPhFlag == 1 &&
                                           picParams->m_phFlags.m_fields.m_phAlfEnabledFlag == 0))
                                             ? 1
                                             : 0;
        params.dAlfCbDisabledFlag     = ((picParams->m_spsFlags0.m_fields.m_spsAlfEnabledFlag == 0) ||
                                         (picParams->m_ppsFlags.m_fields.m_ppsAlfInfoInPhFlag == 1 &&
                                             picParams->m_phFlags.m_fields.m_phAlfCbEnabledFlag == 0))
                                             ? 1
                                             : 0;
        params.dAlfCrDisabledFlag     = ((picParams->m_spsFlags0.m_fields.m_spsAlfEnabledFlag == 0) ||
                                         (picParams->m_ppsFlags.m_fields.m_ppsAlfInfoInPhFlag == 1 &&
                                             picParams->m_phFlags.m_fields.m_phAlfCrEnabledFlag == 0))
                                             ? 1
                                             : 0;
        params.dAlfCcCbDisabledFlag   = ((picParams->m_spsFlags0.m_fields.m_spsCcalfEnabledFlag == 0) ||
                                           (picParams->m_ppsFlags.m_fields.m_ppsAlfInfoInPhFlag == 1 &&
                                               picParams->m_phFlags.m_fields.m_phAlfCcCbEnabledFlag == 0))
                                             ? 1
                                             : 0;
        params.dAlfCcCrDisabledFlag   = ((picParams->m_spsFlags0.m_fields.m_spsCcalfEnabledFlag == 0) ||
                                           (picParams->m_ppsFlags.m_fields.m_ppsAlfInfoInPhFlag == 1 &&
                                               picParams->m_phFlags.m_fields.m_phAlfCcCrEnabledFlag == 0))
                                             ? 1
                                             : 0;
        params.dSingleSliceFrameFlag  = (m_vvcBasicFeature->m_numSlices == 1) ? 1 : 0;

        params.spsSbtmvpEnabledFlag                                 = picParams->m_spsFlags1.m_fields.m_spsSbtmvpEnabledFlag;
        params.spsAmvrEnabledFlag                                   = picParams->m_spsFlags1.m_fields.m_spsAmvrEnabledFlag;
        params.spsSmvdEnabledFlag                                   = picParams->m_spsFlags1.m_fields.m_spsSmvdEnabledFlag;
        params.spsMmvdEnabledFlag                                   = picParams->m_spsFlags1.m_fields.m_spsMmvdEnabledFlag;
        params.spsSbtEnabledFlag                                    = picParams->m_spsFlags1.m_fields.m_spsSbtEnabledFlag;
        params.spsAffineEnabledFlag                                 = picParams->m_spsFlags1.m_fields.m_spsAffineEnabledFlag;
        params.sps6ParamAffineEnabledFlag                           = picParams->m_spsFlags1.m_fields.m_sps6paramAffineEnabledFlag;
        params.spsAffineAmvrEnabledFlag                             = picParams->m_spsFlags1.m_fields.m_spsAffineAmvrEnabledFlag;
        params.spsBcwEnabledFlag                                    = picParams->m_spsFlags1.m_fields.m_spsBcwEnabledFlag;
        params.spsCiipEnabledFlag                                   = picParams->m_spsFlags1.m_fields.m_spsCiipEnabledFlag;
        params.spsGpmEnabledFlag                                    = picParams->m_spsFlags1.m_fields.m_spsGpmEnabledFlag;
        params.spsIspEnabledFlag                                    = picParams->m_spsFlags1.m_fields.m_spsIspEnabledFlag;
        params.spsMrlEnabledFlag                                    = picParams->m_spsFlags1.m_fields.m_spsMrlEnabledFlag;
        params.spsMipEnabledFlag                                    = picParams->m_spsFlags1.m_fields.m_spsMipEnabledFlag;
        params.spsCclmEnabledFlag                                   = picParams->m_spsFlags1.m_fields.m_spsCclmEnabledFlag;
        params.spsChromaHorizontalCollocatedFlag                    = picParams->m_spsFlags1.m_fields.m_spsChromaHorizontalCollocatedFlag;
        params.spsChromaVerticalCollocatedFlag                      = picParams->m_spsFlags1.m_fields.m_spsChromaVerticalCollocatedFlag;
        params.spsTemporalMvpEnabledFlag                            = picParams->m_spsFlags1.m_fields.m_spsTemporalMvpEnabledFlag;
        params.spsPaletteEnabledFlag                                = picParams->m_spsFlags2.m_fields.m_spsPaletteEnabledFlag;
        params.spsActEnabledFlag                                    = picParams->m_spsFlags2.m_fields.m_spsActEnabledFlag;
        params.spsIbcEnabledFlag                                    = picParams->m_spsFlags2.m_fields.m_spsIbcEnabledFlag;
        params.spsLadfEnabledFlag                                   = picParams->m_spsFlags2.m_fields.m_spsLadfEnabledFlag;
        params.spsScalingMatrixForLfnstDisabledFlag                 = picParams->m_spsFlags2.m_fields.m_spsScalingMatrixForLfnstDisabledFlag;
        params.spsScalingMatrixForAlternativeColorSpaceDisabledFlag = picParams->m_spsFlags2.m_fields.m_spsScalingMatrixForAlternativeColourSpaceDisabledFlag;
        params.spsScalingMatrixDesignatedColorSpaceFlag             = picParams->m_spsFlags2.m_fields.m_spsScalingMatrixDesignatedColourSpaceFlag;

        params.ppsLoopFilterAcrossTilesEnabledFlag  = picParams->m_ppsFlags.m_fields.m_ppsLoopFilterAcrossTilesEnabledFlag;
        params.ppsRectSliceFlag                     = picParams->m_ppsFlags.m_fields.m_ppsRectSliceFlag;
        params.ppsSingleSlicePerSubpicFlag          = picParams->m_ppsFlags.m_fields.m_ppsSingleSlicePerSubpicFlag;
        params.ppsLoopFilterAcrossSlicesEnabledFlag = picParams->m_ppsFlags.m_fields.m_ppsLoopFilterAcrossSlicesEnabledFlag;
        params.ppsWeightedPredFlag                  = picParams->m_ppsFlags.m_fields.m_ppsWeightedPredFlag;
        params.ppsWeightedBipredFlag                = picParams->m_ppsFlags.m_fields.m_ppsWeightedBipredFlag;
        params.ppsRefWraparoundEnabledFlag          = picParams->m_ppsFlags.m_fields.m_ppsRefWraparoundEnabledFlag;
        params.ppsCuQpDeltaEnabledFlag              = picParams->m_ppsFlags.m_fields.m_ppsCuQpDeltaEnabledFlag;
        params.virtualboundariespresentflag         = (picParams->m_spsFlags2.m_fields.m_spsVirtualBoundariesEnabledFlag &&
                                                   (picParams->m_spsFlags2.m_fields.m_spsVirtualBoundariesPresentFlag || picParams->m_phFlags.m_fields.m_phVirtualBoundariesPresentFlag))
                                                           ? 1
                                                           : 0;
        params.phNonRefPicFlag                      = picParams->m_phFlags.m_fields.m_phNonRefPicFlag;
        params.phChromaResidualScaleFlag            = picParams->m_phFlags.m_fields.m_phChromaResidualScaleFlag;
        params.phTemporalMvpEnabledFlag             = picParams->m_phFlags.m_fields.m_phTemporalMvpEnabledFlag;
        params.phMmvdFullpelOnlyFlag                = picParams->m_phFlags.m_fields.m_phMmvdFullpelOnlyFlag;
        params.phMvdL1ZeroFlag                      = picParams->m_phFlags.m_fields.m_phMvdL1ZeroFlag;
        params.phBdofDisabledFlag                   = picParams->m_phFlags.m_fields.m_phBdofDisabledFlag;
        params.phDmvrDisabledFlag                   = picParams->m_phFlags.m_fields.m_phDmvrDisabledFlag;
        params.phProfDisabledFlag                   = picParams->m_phFlags.m_fields.m_phProfDisabledFlag;
        params.phJointCbcrSignFlag                  = picParams->m_phFlags.m_fields.m_phJointCbcrSignFlag;

        params.spsChromaFormatIdc                  = picParams->m_spsChromaFormatIdc;
        params.spsLog2CtuSizeMinus5                = picParams->m_spsLog2CtuSizeMinus5;
        params.spsBitdepthMinus8                   = picParams->m_spsBitdepthMinus8;
        params.spsLog2MinLumaCodingBlockSizeMinus2 = picParams->m_spsLog2MinLumaCodingBlockSizeMinus2;
        params.spsNumSubpicsMinus1                 = picParams->m_spsNumSubpicsMinus1;

        params.spsLog2TransformSkipMaxSizeMinus2   = picParams->m_spsLog2TransformSkipMaxSizeMinus2;
        params.spsSixMinusMaxNumMergeCand          = picParams->m_spsSixMinusMaxNumMergeCand;
        params.spsFiveMinusMaxNumSubblockMergeCand = picParams->m_spsFiveMinusMaxNumSubblockMergeCand;

        uint8_t maxNumMergeCand = 6 - picParams->m_spsSixMinusMaxNumMergeCand;
        if (picParams->m_spsFlags1.m_fields.m_spsGpmEnabledFlag && (maxNumMergeCand >= 3))
        {
            params.dMaxNumGpmMergeCand = maxNumMergeCand - picParams->m_spsMaxNumMergeCandMinusMaxNumGpmCand;
        }
        else if (picParams->m_spsFlags1.m_fields.m_spsGpmEnabledFlag && (maxNumMergeCand == 2))
        {
            params.dMaxNumGpmMergeCand = 2;
        }
        else
        {
            params.dMaxNumGpmMergeCand = 0;
        }

        params.spsLog2ParallelMergeLevelMinus2 = picParams->m_spsLog2ParallelMergeLevelMinus2;
        params.spsMinQpPrimeTs                 = picParams->m_spsMinQpPrimeTs;
        params.spsSixMinusMaxNumIbcMergeCand   = picParams->m_spsSixMinusMaxNumIbcMergeCand;

        params.spsLadfQpOffset0 = picParams->m_spsLadfQpOffset[0];
        params.spsLadfQpOffset1 = picParams->m_spsLadfQpOffset[1];
        params.spsLadfQpOffset2 = picParams->m_spsLadfQpOffset[2];
        params.spsLadfQpOffset3 = picParams->m_spsLadfQpOffset[3];

        params.spsLadfDeltaThresholdMinus10  = picParams->m_spsLadfDeltaThresholdMinus1[0];
        params.spsLadfDeltaThresholdMinus11  = picParams->m_spsLadfDeltaThresholdMinus1[1];
        params.spsLadfLowestIntervalQpOffset = picParams->m_spsLadfLowestIntervalQpOffset;

        params.spsLadfDeltaThresholdMinus12 = picParams->m_spsLadfDeltaThresholdMinus1[2];
        params.spsLadfDeltaThresholdMinus13 = picParams->m_spsLadfDeltaThresholdMinus1[3];
        params.spsNumLadfIntervalsMinus2    = picParams->m_spsNumLadfIntervalsMinus2;

        params.ppsPicWidthInLumaSamples  = picParams->m_ppsPicWidthInLumaSamples;
        params.ppsPicHeightInLumaSamples = picParams->m_ppsPicHeightInLumaSamples;

        params.ppsScalingWinLeftOffset     = picParams->m_ppsScalingWinLeftOffset;
        params.ppsScalingWinRightOffset    = picParams->m_ppsScalingWinRightOffset;
        params.ppsScalingWinTopOffset      = picParams->m_ppsScalingWinTopOffset;
        params.ppsScalingWinBottomOffset   = picParams->m_ppsScalingWinBottomOffset;

        params.dNumtilerowsminus1    = m_vvcBasicFeature->m_tileRows - 1;
        params.dNumtilecolumnsminus1 = m_vvcBasicFeature->m_tileCols - 1;

        params.ppsCbQpOffset                  = picParams->m_ppsCbQpOffset;
        params.ppsCrQpOffset                  = picParams->m_ppsCrQpOffset;
        params.ppsJointCbcrQpOffsetValue      = picParams->m_ppsJointCbcrQpOffsetValue;
        params.ppsChromaQpOffsetListLenMinus1 = picParams->m_ppsFlags.m_fields.m_ppsCuChromaQpOffsetListEnabledFlag ? picParams->m_ppsChromaQpOffsetListLenMinus1 : 0;

        params.ppsCbQpOffsetList0 = picParams->m_ppsCbQpOffsetList[0];
        params.ppsCbQpOffsetList1 = picParams->m_ppsCbQpOffsetList[1];
        params.ppsCbQpOffsetList2 = picParams->m_ppsCbQpOffsetList[2];
        params.ppsCbQpOffsetList3 = picParams->m_ppsCbQpOffsetList[3];

        params.ppsCbQpOffsetList4               = picParams->m_ppsCbQpOffsetList[4];
        params.ppsCbQpOffsetList5               = picParams->m_ppsCbQpOffsetList[5];
        params.ppsPicWidthMinusWraparoundOffset = picParams->m_ppsPicWidthMinusWraparoundOffset;

        params.ppsCrQpOffsetList0 = picParams->m_ppsCrQpOffsetList[0];
        params.ppsCrQpOffsetList1 = picParams->m_ppsCrQpOffsetList[1];
        params.ppsCrQpOffsetList2 = picParams->m_ppsCrQpOffsetList[2];
        params.ppsCrQpOffsetList3 = picParams->m_ppsCrQpOffsetList[3];

        params.ppsCrQpOffsetList4 = picParams->m_ppsCrQpOffsetList[4];
        params.ppsCrQpOffsetList5 = picParams->m_ppsCrQpOffsetList[5];

        params.ppsJointCbcrQpOffsetList0 = picParams->m_ppsJointCbcrQpOffsetList[0];
        params.ppsJointCbcrQpOffsetList1 = picParams->m_ppsJointCbcrQpOffsetList[1];
        params.ppsJointCbcrQpOffsetList2 = picParams->m_ppsJointCbcrQpOffsetList[2];
        params.ppsJointCbcrQpOffsetList3 = picParams->m_ppsJointCbcrQpOffsetList[3];

        params.ppsJointCbcrQpOffsetList4 = picParams->m_ppsJointCbcrQpOffsetList[4];
        params.ppsJointCbcrQpOffsetList5 = picParams->m_ppsJointCbcrQpOffsetList[5];

        params.numvervirtualboundaries                = picParams->m_numVerVirtualBoundaries;
        params.numhorvirtualboundaries                = picParams->m_numHorVirtualBoundaries;
        params.phLog2DiffMinQtMinCbIntraSliceLuma     = picParams->m_phLog2DiffMinQtMinCbIntraSliceLuma;
        params.phMaxMttHierarchyDepthIntraSliceLuma   = picParams->m_phMaxMtt_hierarchyDepthIntraSliceLuma;
        params.phLog2DiffMaxBtMinQtIntraSliceLuma     = picParams->m_phLog2DiffMaxBtMinQtIntraSliceLuma;
        params.phLog2DiffMaxTtMinQtIntraSliceLuma     = picParams->m_phLog2DiffMax_ttMinQtIntraSliceLuma;
        params.phLog2DiffMinQtMinCbIntraSliceChroma   = picParams->m_phLog2DiffMinQtMinCbIntraSliceChroma;
        params.phMaxMttHierarchyDepthIntraSliceChroma = picParams->m_phMaxMtt_hierarchyDepthIntraSliceChroma;

        params.dVirtualboundaryposxminus10 = (picParams->m_virtualBoundaryPosX[0] >> 3) - 1;
        params.dVirtualboundaryposyminus10 = (picParams->m_virtualBoundaryPosY[0] >> 3) - 1;
        params.dVirtualboundaryposxminus11 = (picParams->m_virtualBoundaryPosX[1] >> 3) - 1;
        params.dVirtualboundaryposyminus11 = (picParams->m_virtualBoundaryPosY[1] >> 3) - 1;
        params.dVirtualboundaryposxminus12 = (picParams->m_virtualBoundaryPosX[2] >> 3) - 1;
        params.dVirtualboundaryposyminus12 = (picParams->m_virtualBoundaryPosY[2] >> 3) - 1;

        params.phLog2DiffMaxBtMinQtIntraSliceChroma = picParams->m_phLog2DiffMaxBtMinQtIntraSliceChroma;
        params.phLog2DiffMaxTtMinQtIntraSliceChroma = picParams->m_phLog2DiffMax_ttMinQtIntraSliceChroma;
        params.phCuQpDeltaSubdivIntraSlice          = picParams->m_phCuQpDeltaSubdivIntraSlice;
        params.phCuChromaQpOffsetSubdivIntraSlice   = picParams->m_phCuChromaQpOffsetSubdivIntraSlice;
        params.phLog2DiffMinQtMinCbInterSlice       = picParams->m_phLog2DiffMinQtMinCbInterSlice;
        params.phMaxMttHierarchyDepthInterSlice     = picParams->m_phMaxMtt_hierarchyDepthInterSlice;

        params.phLog2DiffMaxBtMinQtInterSlice     = picParams->m_phLog2DiffMaxBtMinQtInterSlice;
        params.phLog2DiffMaxTtMinQtInterSlice     = picParams->m_phLog2DiffMax_ttMinQtInterSlice;
        params.phCuQpDeltaSubdivInterSlice        = picParams->m_phCuQpDeltaSubdivInterSlice;
        params.phCuChromaQpOffsetSubdivInterSlice = picParams->m_phCuChromaQpOffsetSubdivInterSlice;

        // ALF
        params.dActiveapsid = m_vvcBasicFeature->m_activeAlfMask;
        MOS_ZeroMemory(params.alfApsArray, 8 * sizeof(CodecVvcAlfData));
        MOS_SecureMemcpy(params.alfApsArray, 8 * sizeof(CodecVvcAlfData), m_vvcBasicFeature->m_alfApsArray, 8 * sizeof(CodecVvcAlfData));

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VVCP_DPB_STATE, VvcDecodePicPkt)
    {
        params = {};

        VvcRefFrameAttributes curFrameAttr;
        DECODE_CHK_STATUS(m_vvcBasicFeature->m_refFrames.GetRefAttrByFrameIndex(
            m_vvcPicParams->m_currPic.FrameIdx,
            &curFrameAttr));

        uint8_t frameIdx = 0;
        for (uint8_t i = 0; i < vvcMaxNumRefFrame; i++)
        {
            if (!m_vvcBasicFeature->m_refFrames.m_curIsIntra)
            {
                if (m_vvcPicParams->m_refFrameList[i].PicFlags != PICTURE_INVALID)
                {
                    frameIdx = m_vvcPicParams->m_refFrameList[i].FrameIdx;
                }
                else
                {
                    frameIdx = m_vvcBasicFeature->m_refFrames.GetValidReferenceFrameIdx();
                }

                if (frameIdx >= CODEC_MAX_DPB_NUM_VVC)
                {
                    return MOS_STATUS_INVALID_PARAMETER;
                }
            }
            else
            {
                frameIdx = m_vvcPicParams->m_currPic.FrameIdx;
            }
            DECODE_CHK_STATUS(m_vvcBasicFeature->m_refFrames.GetRefAttrByFrameIndex(
                frameIdx,
                &params.refFrameAttr[i]));

            params.refPicScaleWidth[i]  = ((params.refFrameAttr[i].m_currPicScalWinWidthL << 14) + (curFrameAttr.m_currPicScalWinWidthL >> 1)) / curFrameAttr.m_currPicScalWinWidthL;
            params.refPicScaleHeight[i] = ((params.refFrameAttr[i].m_currPicScalWinHeightL << 14) + (curFrameAttr.m_currPicScalWinHeightL >> 1)) / curFrameAttr.m_currPicScalWinHeightL;
        }

        return MOS_STATUS_SUCCESS;
    }
}
