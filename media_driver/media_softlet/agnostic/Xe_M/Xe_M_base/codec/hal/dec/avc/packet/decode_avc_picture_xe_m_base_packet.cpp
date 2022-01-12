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
//! \file     decode_avc_picture_xe_m_base_packet.cpp
//! \brief    Defines the interface for avc decode picture packet
//!
#include "codechal_utilities.h"
#include "decode_avc_picture_xe_m_base_packet.h"
#include "codechal_debug.h"
#include "decode_common_feature_defs.h"
#include "decode_resource_auto_lock.h"

namespace decode{
    AvcDecodePicPktXe_M_Base::~AvcDecodePicPktXe_M_Base()
    {
        FreeResources();
    }

    MOS_STATUS AvcDecodePicPktXe_M_Base::FreeResources()
    {
        DECODE_FUNC_CALL();

        if (m_allocator != nullptr)
        {
            m_allocator->Destroy(m_resMfdDeblockingFilterRowStoreScratchBuffer);
            if (!m_mfxInterface->IsBsdMpcRowstoreCacheEnabled())
            {
                m_allocator->Destroy(m_resBsdMpcRowStoreScratchBuffer);
            }
            if (!m_mfxInterface->IsIntraRowstoreCacheEnabled())
            {
                m_allocator->Destroy(m_resMfdIntraRowStoreScratchBuffer);
            }
            if (!m_mfxInterface->IsMprRowstoreCacheEnabled())
            {
                m_allocator->Destroy(m_resMprRowStoreScratchBuffer);
            }
#if MOS_EVENT_TRACE_DUMP_SUPPORTED
            m_allocator->Destroy(m_tempRefSurf);
#endif
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodePicPktXe_M_Base::Init()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_featureManager);
        DECODE_CHK_NULL(m_hwInterface);
        DECODE_CHK_NULL(m_osInterface);
        DECODE_CHK_NULL(m_miInterface);
        DECODE_CHK_NULL(m_avcPipeline);
        DECODE_CHK_NULL(m_mfxInterface);

        m_avcBasicFeature = dynamic_cast<AvcBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        DECODE_CHK_NULL(m_avcBasicFeature);

#ifdef _DECODE_PROCESSING_SUPPORTED
        m_downSamplingFeature = dynamic_cast<DecodeDownSamplingFeature*>(m_featureManager->GetFeature(DecodeFeatureIDs::decodeDownSampling));
        DecodeSubPacket* subPacket = m_avcPipeline->GetSubPacket(DecodePacketId(m_avcPipeline, downSamplingSubPacketId));
        m_downSamplingPkt = dynamic_cast<DecodeDownSamplingPkt *>(subPacket);
#endif
        m_allocator = m_pipeline ->GetDecodeAllocator();
        DECODE_CHK_NULL(m_allocator);

        DECODE_CHK_STATUS(AllocateFixedResources());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodePicPktXe_M_Base::Prepare()
    {
        DECODE_FUNC_CALL();

        m_avcPicParams      = m_avcBasicFeature->m_avcPicParams;

#ifdef _MMC_SUPPORTED
        m_mmcState = m_avcPipeline->GetMmcState();
        DECODE_CHK_NULL(m_mmcState);
#endif

        DECODE_CHK_STATUS(SetRowstoreCachingOffsets());

        DECODE_CHK_STATUS(AllocateVariableResources());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodePicPktXe_M_Base::SetRowstoreCachingOffsets()
    {
        if (m_mfxInterface->IsRowStoreCachingSupported())
        {
            MHW_VDBOX_ROWSTORE_PARAMS rowstoreParams;
            MOS_ZeroMemory(&rowstoreParams, sizeof(rowstoreParams));
            rowstoreParams.dwPicWidth       = m_avcBasicFeature->m_width;
            rowstoreParams.bMbaff           = m_avcPicParams->seq_fields.mb_adaptive_frame_field_flag;
            rowstoreParams.Mode             = CODECHAL_DECODE_MODE_AVCVLD;
            rowstoreParams.bIsFrame         = m_avcPicParams->seq_fields.frame_mbs_only_flag;
            DECODE_CHK_STATUS(static_cast<CodechalHwInterfaceG12*>(m_hwInterface)->SetRowstoreCachingOffsets(&rowstoreParams));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodePicPktXe_M_Base::AllocateFixedResources()
    {
        DECODE_FUNC_CALL();

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodePicPktXe_M_Base::AllocateVariableResources()
    {
        DECODE_FUNC_CALL();

        uint16_t picWidthInMB = MOS_MAX(m_picWidthInMbLastMaxAlloced, (m_avcPicParams->pic_width_in_mbs_minus1 + 1));
        uint16_t picHeightInMB = MOS_MAX(m_picHeightInMbLastMaxAlloced, (m_avcPicParams->pic_height_in_mbs_minus1 + 1));
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

        if (m_mfxInterface->IsBsdMpcRowstoreCacheEnabled() == false)
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

        if (m_mfxInterface->IsIntraRowstoreCacheEnabled() == false)
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

        if (m_mfxInterface->IsMprRowstoreCacheEnabled() == false)
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

    void AvcDecodePicPktXe_M_Base::SetMfxPipeModeSelectParams(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12 &pipeModeSelectParams)
    {
        DECODE_FUNC_CALL();

        pipeModeSelectParams.Mode                      = CODECHAL_DECODE_MODE_AVCVLD;
        pipeModeSelectParams.bDeblockerStreamOutEnable = false;
        pipeModeSelectParams.bStreamOutEnabled         = m_avcBasicFeature->m_streamOutEnabled;
        pipeModeSelectParams.bPostDeblockOutEnable     = m_avcBasicFeature->m_deblockingEnabled;
        pipeModeSelectParams.bPreDeblockOutEnable      = !m_avcBasicFeature->m_deblockingEnabled;
        pipeModeSelectParams.bShortFormatInUse         = m_avcBasicFeature->m_shortFormatInUse;
    }

    MOS_STATUS AvcDecodePicPktXe_M_Base::SetMfxSurfaceParams(MHW_VDBOX_SURFACE_PARAMS &dstSurfaceParams)
    {
        DECODE_FUNC_CALL();

        MOS_ZeroMemory(&dstSurfaceParams, sizeof(dstSurfaceParams));
        dstSurfaceParams.Mode      = CODECHAL_DECODE_MODE_AVCVLD;
        dstSurfaceParams.psSurface = &m_avcBasicFeature->m_destSurface;

#ifdef _MMC_SUPPORTED
        DECODE_CHK_STATUS(m_mmcState->SetSurfaceMmcState(&(m_avcBasicFeature->m_destSurface)));
        DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcState(dstSurfaceParams.psSurface, &dstSurfaceParams.mmcState));
        DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcFormat(dstSurfaceParams.psSurface, &dstSurfaceParams.dwCompressionFormat));
#endif
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodePicPktXe_M_Base::SetMfxPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParams)
    {
        DECODE_FUNC_CALL();

        pipeBufAddrParams.Mode = CODECHAL_DECODE_MODE_AVCVLD;

        if (m_avcBasicFeature->m_deblockingEnabled)
        {
            pipeBufAddrParams.psPostDeblockSurface = &(m_avcBasicFeature->m_destSurface);
        }
        else
        {
            pipeBufAddrParams.psPreDeblockSurface = &(m_avcBasicFeature->m_destSurface);
        }
        pipeBufAddrParams.presMfdIntraRowStoreScratchBuffer  = &m_resMfdIntraRowStoreScratchBuffer->OsResource;
        pipeBufAddrParams.presMfdDeblockingFilterRowStoreScratchBuffer = &m_resMfdDeblockingFilterRowStoreScratchBuffer->OsResource;

        if (m_avcBasicFeature->m_streamOutEnabled)
        {
            pipeBufAddrParams.presStreamOutBuffer = m_avcBasicFeature->m_externalStreamOutBuffer;
        }

        AvcReferenceFrames &refFrames = m_avcBasicFeature->m_refFrames;
        const std::vector<uint8_t> & activeRefList = refFrames.GetActiveReferenceList(*m_avcPicParams);

        for (uint8_t i = 0; i < activeRefList.size(); i++)
        {
            uint8_t frameIdx = activeRefList[i];
            uint8_t frameId = (m_avcBasicFeature->m_picIdRemappingInUse) ? i : refFrames.m_refList[frameIdx]->ucFrameId;
            pipeBufAddrParams.presReferences[frameId] = refFrames.GetReferenceByFrameIndex(frameIdx);
        }

        DECODE_CHK_STATUS(FixMfxPipeBufAddrParams(pipeBufAddrParams));

        CODECHAL_DEBUG_TOOL(DumpResources(pipeBufAddrParams));

#if MOS_EVENT_TRACE_DUMP_SUPPORTED
        if (MOS_GetTraceEventKeyword() & EVENT_DECODE_REFYUV_KEYWORD)
        {
            TraceDataDumpReferences(pipeBufAddrParams);
        }
#endif

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodePicPktXe_M_Base::FixMfxPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParams)
    {
        DECODE_FUNC_CALL();

        PMOS_RESOURCE validRefPic = nullptr;
        PMOS_RESOURCE dummyRef = &(m_avcBasicFeature->m_dummyReference.OsResource);
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
            // error concealment for the unset reference addresses and unset mv buffers
            if (pipeBufAddrParams.presReferences[i] == nullptr)
            {
                pipeBufAddrParams.presReferences[i] = validRefPic;
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    void AvcDecodePicPktXe_M_Base::SetMfxIndObjBaseAddrParams(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS &indObjBaseAddrParams)
    {
        DECODE_FUNC_CALL();

        MOS_ZeroMemory(&indObjBaseAddrParams, sizeof(indObjBaseAddrParams));
        indObjBaseAddrParams.Mode            = CODECHAL_DECODE_MODE_AVCVLD;
        indObjBaseAddrParams.dwDataSize      = m_avcBasicFeature->m_dataSize;
        indObjBaseAddrParams.dwDataOffset    = m_avcBasicFeature->m_dataOffset;
        indObjBaseAddrParams.presDataBuffer  = &(m_avcBasicFeature->m_resDataBuffer.OsResource);
    }

    void AvcDecodePicPktXe_M_Base::SetMfxBspBufBaseAddrParams(MHW_VDBOX_BSP_BUF_BASE_ADDR_PARAMS &bspBufBaseAddrParams)
    {
        DECODE_FUNC_CALL();

        MOS_ZeroMemory(&bspBufBaseAddrParams, sizeof(bspBufBaseAddrParams));
        bspBufBaseAddrParams.presBsdMpcRowStoreScratchBuffer = &m_resBsdMpcRowStoreScratchBuffer->OsResource;
        bspBufBaseAddrParams.presMprRowStoreScratchBuffer    = &m_resMprRowStoreScratchBuffer->OsResource;
    }

    void AvcDecodePicPktXe_M_Base::SetMfdAvcDpbParams(MHW_VDBOX_AVC_DPB_PARAMS &dpbParams)
    {
        DECODE_FUNC_CALL();

        MOS_ZeroMemory(&dpbParams, sizeof(dpbParams));
        dpbParams.pAvcPicParams        = m_avcPicParams;
        dpbParams.pMvcExtPicParams     = m_avcBasicFeature->m_mvcExtPicParams;
        dpbParams.ppAvcRefList         = &(m_avcBasicFeature->m_refFrames.m_refList[0]);
        dpbParams.bPicIdRemappingInUse = m_avcBasicFeature->m_picIdRemappingInUse;
    }

    void AvcDecodePicPktXe_M_Base::SetMfdAvcPicidParams(MHW_VDBOX_PIC_ID_PARAMS &picIdParams)
    {
        DECODE_FUNC_CALL();

        MOS_ZeroMemory(&picIdParams, sizeof(picIdParams));
        AvcReferenceFrames &refFrames = m_avcBasicFeature->m_refFrames;
        picIdParams.bPicIdRemappingInUse = m_avcBasicFeature->m_picIdRemappingInUse;
        picIdParams.pAvcPicIdx = &(refFrames.m_avcPicIdx[0]);
    }

    void AvcDecodePicPktXe_M_Base::SetMfxAvcImgParams(MHW_VDBOX_AVC_IMG_PARAMS &imgParams)
    {
        DECODE_FUNC_CALL();

        MOS_ZeroMemory(&imgParams, sizeof(imgParams));
        imgParams.pAvcPicParams = m_avcPicParams;
        imgParams.pMvcExtPicParams = m_avcBasicFeature->m_mvcExtPicParams;

        AvcReferenceFrames &refFrames = m_avcBasicFeature->m_refFrames;
        const std::vector<uint8_t> & activeRefList = refFrames.GetActiveReferenceList(*m_avcPicParams);
        uint8_t activeFrameCnt = activeRefList.size();
        imgParams.ucActiveFrameCnt = activeFrameCnt;
    }

    MOS_STATUS AvcDecodePicPktXe_M_Base::SetMfxAvcDirectmodeParams(MHW_VDBOX_AVC_DIRECTMODE_PARAMS &avcDirectmodeParams)
    {
        DECODE_FUNC_CALL();

        MOS_ZeroMemory(&avcDirectmodeParams, sizeof(avcDirectmodeParams));
        MOS_ZeroMemory(&m_resAvcDmvBuffers, (sizeof(MOS_RESOURCE) * CODEC_AVC_NUM_DMV_BUFFERS));

        AvcReferenceFrames &refFrames = m_avcBasicFeature->m_refFrames;
        auto mvBuffers = &(m_avcBasicFeature->m_mvBuffers);
        PMOS_BUFFER curMvBuffer = mvBuffers->GetCurBuffer();
        DECODE_CHK_NULL(curMvBuffer);

        m_resAvcDmvBuffers[0] = curMvBuffer->OsResource;
        PMOS_BUFFER curAvailableBuffers = mvBuffers->GetAvailableBuffer();
        DECODE_CHK_NULL(curAvailableBuffers);
        m_resAvcDmvBuffers[CODEC_AVC_NUM_REF_DMV_BUFFERS] = curAvailableBuffers->OsResource;

        const std::vector<uint8_t> & activeRefList = refFrames.GetActiveReferenceList(*m_avcPicParams);

        for (uint8_t i = 0; i < activeRefList.size(); i++)
        {
            uint8_t frameIdx = activeRefList[i];
            if (m_avcBasicFeature->m_secondField && activeRefList.size() > m_avcBasicFeature->m_avcPicParams->frame_num &&
                (frameIdx == m_avcBasicFeature->m_curRenderPic.FrameIdx))
            {
                m_resAvcDmvBuffers[i + 1] = curMvBuffer->OsResource;
            }
            else
            {
                PMOS_BUFFER mvBuf = mvBuffers->GetBufferByFrameIndex(frameIdx);

                // m_resAvcDmvBuffers[0] is used as current mv buffer itself.
                if (mvBuf != nullptr)
                {
                    m_resAvcDmvBuffers[i + 1] = mvBuf->OsResource;
                }
                else
                {
                    PMOS_BUFFER curAvailableBuf = mvBuffers->GetAvailableBuffer();
                    DECODE_CHK_NULL(curAvailableBuf);
                    m_resAvcDmvBuffers[i + 1] = curAvailableBuf->OsResource;
                }
            }
            refFrames.m_refList[frameIdx]->ucDMVIdx[0] = i + 1;
        }

        avcDirectmodeParams.CurrPic                 = m_avcPicParams->CurrPic;
        avcDirectmodeParams.uiUsedForReferenceFlags = m_avcPicParams->UsedForReferenceFlags;
        avcDirectmodeParams.presAvcDmvBuffers       = &m_resAvcDmvBuffers[0];
        avcDirectmodeParams.ucAvcDmvIdx             = 0;
        avcDirectmodeParams.pAvcPicIdx              = &(refFrames.m_avcPicIdx[0]);
        avcDirectmodeParams.avcRefList              = (void**)refFrames.m_refList;
        avcDirectmodeParams.bPicIdRemappingInUse    = m_avcBasicFeature->m_picIdRemappingInUse;

        return MOS_STATUS_SUCCESS;
    }

    void AvcDecodePicPktXe_M_Base::SetMfxQmParams(MHW_VDBOX_QM_PARAMS &qmParams)
    {
        DECODE_FUNC_CALL();

        MOS_ZeroMemory(&qmParams, sizeof(qmParams));
        qmParams.Standard = CODECHAL_AVC;
        qmParams.pAvcIqMatrix = (PMHW_VDBOX_AVC_QM_PARAMS)m_avcBasicFeature->m_avcIqMatrixParams;
    }

    MOS_STATUS AvcDecodePicPktXe_M_Base::AddMfxBspBufBaseAddrCmd(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        MHW_VDBOX_BSP_BUF_BASE_ADDR_PARAMS BspBufBaseAddrParams;
        SetMfxBspBufBaseAddrParams(BspBufBaseAddrParams);
        DECODE_CHK_STATUS(m_mfxInterface->AddMfxBspBufBaseAddrCmd(&cmdBuffer, &BspBufBaseAddrParams));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodePicPktXe_M_Base::AddMfxSurfacesCmd(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        MHW_VDBOX_SURFACE_PARAMS dstSurfaceParams;
        DECODE_CHK_STATUS(SetMfxSurfaceParams(dstSurfaceParams));
        DECODE_CHK_STATUS(m_mfxInterface->AddMfxSurfaceCmd(&cmdBuffer, &dstSurfaceParams));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodePicPktXe_M_Base::AddMfxIndObjBaseAddrCmd(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS indObjBaseAddrParams;
        SetMfxIndObjBaseAddrParams(indObjBaseAddrParams);
        DECODE_CHK_STATUS(m_mfxInterface->AddMfxIndObjBaseAddrCmd(&cmdBuffer, &indObjBaseAddrParams));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodePicPktXe_M_Base::AddMfdAvcDpbCmd(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        MHW_VDBOX_AVC_DPB_PARAMS dpbParams;
        SetMfdAvcDpbParams(dpbParams);
        DECODE_CHK_STATUS(m_mfxInterface->AddMfdAvcDpbCmd(&cmdBuffer, &dpbParams));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodePicPktXe_M_Base::AddMfxQmCmd(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        MHW_VDBOX_QM_PARAMS qmParams;
        SetMfxQmParams(qmParams);
        DECODE_CHK_STATUS(m_mfxInterface->AddMfxQmCmd(&cmdBuffer, &qmParams));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodePicPktXe_M_Base::AddMfxAvcDirectmodeCmd(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        MHW_VDBOX_AVC_DIRECTMODE_PARAMS avcDirectmodeParams;
        SetMfxAvcDirectmodeParams(avcDirectmodeParams);
        DECODE_CHK_STATUS(m_mfxInterface->AddMfxAvcDirectmodeCmd(&cmdBuffer, &avcDirectmodeParams));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodePicPktXe_M_Base::AddMfdAvcPicidCmd(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        MHW_VDBOX_PIC_ID_PARAMS picIdParams;
        SetMfdAvcPicidParams(picIdParams);
        DECODE_CHK_STATUS(m_mfxInterface->AddMfdAvcPicidCmd(&cmdBuffer, &picIdParams));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodePicPktXe_M_Base::AddMfxAvcImgCmd(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        MHW_VDBOX_AVC_IMG_PARAMS imgParams;
        SetMfxAvcImgParams(imgParams);
        DECODE_CHK_STATUS(m_mfxInterface->AddMfxAvcImgCmd(&cmdBuffer, nullptr, &imgParams));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodePicPktXe_M_Base::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
    {
        DECODE_FUNC_CALL();

        commandBufferSize = m_pictureStatesSize;
        requestedPatchListSize = m_picturePatchListSize;

        return MOS_STATUS_SUCCESS;
    }

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS AvcDecodePicPktXe_M_Base::DumpResources(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParams)
    {
        DECODE_FUNC_CALL();

        CodechalDebugInterface *debugInterface = m_avcPipeline->GetDebugInterface();

        for (auto n = 0; n < CODEC_AVC_MAX_NUM_REF_FRAME; n++)
        {
            if (m_avcBasicFeature->m_refFrames.m_avcPicIdx[n].bValid)
            {
                MOS_SURFACE destSurface;
                MOS_ZeroMemory(&destSurface, sizeof(MOS_SURFACE));
                destSurface.OsResource = *(pipeBufAddrParams.presReferences[n]);
                DECODE_CHK_STATUS(m_allocator->GetSurfaceInfo(&destSurface));
                std::string refSurfName = "RefSurf[" + std::to_string(static_cast<uint32_t>(n)) + "]";
                DECODE_CHK_STATUS(debugInterface->DumpYUVSurface(
                    &destSurface,
                    CodechalDbgAttr::attrReferenceSurfaces,
                    refSurfName.c_str()));
            }
        }

        DECODE_CHK_STATUS(debugInterface->DumpBuffer(
            &m_avcBasicFeature->m_resDataBuffer.OsResource,
            CodechalDbgAttr::attrBitstream,
            "DEC",
            m_avcBasicFeature->m_dataSize,
            m_avcBasicFeature->m_dataOffset,
            CODECHAL_NUM_MEDIA_STATES));

        return MOS_STATUS_SUCCESS;
    }

#endif

#if MOS_EVENT_TRACE_DUMP_SUPPORTED
    MOS_STATUS AvcDecodePicPktXe_M_Base::TraceDataDumpReferences(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParams)
    {
        bool bReport = false;

        for (auto n = 0; n < CODEC_AVC_MAX_NUM_REF_FRAME; n++)
        {
            if (m_avcBasicFeature->m_refFrames.m_avcPicIdx[n].bValid)
            {
                bool        bAllocate = false;
                MOS_SURFACE dstSurface;
                MOS_ZeroMemory(&dstSurface, sizeof(MOS_SURFACE));
                dstSurface.OsResource = *(pipeBufAddrParams.presReferences[n]);
                DECODE_CHK_STATUS(m_allocator->GetSurfaceInfo(&dstSurface));

                if (!m_allocator->ResourceIsNull(&dstSurface.OsResource))
                {
                    if (m_tempRefSurf == nullptr || m_allocator->ResourceIsNull(&m_tempRefSurf->OsResource))
                    {
                        bAllocate = true;
                    }
                    else if (m_tempRefSurf->dwWidth < dstSurface.dwWidth ||
                             m_tempRefSurf->dwHeight < dstSurface.dwHeight)
                    {
                        bAllocate = true;
                    }
                    else
                    {
                        bAllocate = false;
                    }

                    if (bAllocate)
                    {
                        if (!m_allocator->ResourceIsNull(&m_tempRefSurf->OsResource))
                        {
                            m_allocator->Destroy(m_tempRefSurf);
                        }

                        m_tempRefSurf = m_allocator->AllocateLinearSurface(
                            dstSurface.dwWidth,
                            dstSurface.dwHeight,
                            "Decode Ref Surf",
                            dstSurface.Format,
                            dstSurface.bIsCompressed,
                            resourceInputReference,
                            lockableSystemMem,
                            MOS_TILE_LINEAR_GMM);
                    }

                    DECODE_CHK_STATUS(m_osInterface->pfnDoubleBufferCopyResource(
                        m_osInterface,
                        &dstSurface.OsResource,
                        &m_tempRefSurf->OsResource,
                        false));

                    if (!bReport)
                    {
                        DECODE_EVENTDATA_YUV_SURFACE_INFO eventData =
                        {
                                PICTURE_FRAME,
                                0,
                                m_tempRefSurf->dwOffset,
                                m_tempRefSurf->YPlaneOffset.iYOffset,
                                m_tempRefSurf->dwPitch,
                                m_tempRefSurf->dwWidth,
                                m_tempRefSurf->dwHeight,
                                (uint32_t)m_tempRefSurf->Format,
                                m_tempRefSurf->UPlaneOffset.iLockSurfaceOffset,
                                m_tempRefSurf->VPlaneOffset.iLockSurfaceOffset,
                                m_tempRefSurf->UPlaneOffset.iSurfaceOffset,
                                m_tempRefSurf->VPlaneOffset.iSurfaceOffset,
                        };
                        MOS_TraceEvent(EVENT_DECODE_REF_DUMPINFO, EVENT_TYPE_INFO, &eventData, sizeof(eventData), NULL, 0);

                        bReport = true;
                    }

                    ResourceAutoLock resLock(m_allocator, &m_tempRefSurf->OsResource);
                    auto             pData = (uint8_t *)resLock.LockResourceForRead();

                    MOS_TraceDataDump(
                        "Decode_AVCRefSurf",
                        n,
                        pData,
                        (uint32_t)m_tempRefSurf->OsResource.pGmmResInfo->GetSizeMainSurface());
                }
            }
        }

        return MOS_STATUS_SUCCESS;
    }
#endif

}
