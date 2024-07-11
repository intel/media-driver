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
//! \file     decode_vvc_pipeline.cpp
//! \brief    Defines the interface for vvc decode pipeline
//!
#include "decode_vvc_pipeline.h"
#include "decode_utils.h"
#include "codechal_debug.h"
#include "decode_vvc_feature_manager.h"
#include "decode_vvc_packet.h"
#include "decode_mem_compression.h"
#include "media_debug_fast_dump.h"

namespace decode
{
    VvcPipeline::VvcPipeline(
        CodechalHwInterfaceNext *   hwInterface,
        CodechalDebugInterface *debugInterface)
        : DecodePipeline(hwInterface, debugInterface)
    {
        m_hwInterface    = hwInterface;
    }

    MOS_STATUS VvcPipeline::InitContext()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_basicFeature);

        DecodeScalabilityPars scalPars;
        MOS_ZeroMemory(&scalPars, sizeof(scalPars));
        scalPars.disableScalability = true;
        scalPars.enableVE           = MOS_VE_SUPPORTED(m_osInterface);
        scalPars.usingSlimVdbox     = false;
        scalPars.numVdbox           = m_numVdbox;

        m_mediaContext->SwitchContext(VdboxDecodeFunc, &scalPars, &m_scalability);
        DECODE_CHK_NULL(m_scalability);

        m_decodeContext = m_osInterface->pfnGetGpuContext(m_osInterface);
        m_decodeContextHandle = m_osInterface->CurrentGpuContextHandle;

        m_passNum = (uint16_t)m_basicFeature->m_numSlices;
        m_basicFeature->m_curSlice = 0;
        m_scalability->SetPassNumber(m_passNum);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcPipeline::ActivateDecodePackets()
    {
        DECODE_FUNC_CALL();
        if (m_basicFeature->m_shortFormatInUse)
        {
            DECODE_CHK_STATUS(ActivatePacket(DecodePacketId(this, vvcDecodeS2LPktId), false, 0, 0));
        }

        for (uint16_t curPass = 0; curPass < GetPassNum(); curPass++)
        {
            DECODE_CHK_STATUS(ActivatePacket(DecodePacketId(this, vvcDecodePacketId), false, curPass, 0));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcPipeline::Prepare(void *params)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(params);
        DECODE_CHK_NULL(m_basicFeature);

        DecodePipelineParams *pipelineParams = (DecodePipelineParams *)params;
        m_pipeMode = pipelineParams->m_pipeMode;

        PERF_UTILITY_AUTO((__FUNCTION__ + std::to_string((int)m_pipeMode)).c_str(), PERF_DECODE, PERF_LEVEL_HAL);

        if (IsFirstProcessPipe(*pipelineParams))
        {
            DECODE_CHK_STATUS(DecodePipeline::Prepare(params));
        }

        DECODE_CHK_STATUS(m_preSubPipeline->Prepare(*pipelineParams));
        DECODE_CHK_STATUS(m_postSubPipeline->Prepare(*pipelineParams));

        if (m_pipeMode == decodePipeModeProcess)
        {
            if (IsCompleteBitstream())
            {
                CODECHAL_DEBUG_TOOL(DECODE_CHK_STATUS(DumpParams(*m_basicFeature)));

                DecodeStatusParameters inputParameters = {};
                MOS_ZeroMemory(&inputParameters, sizeof(DecodeStatusParameters));
                inputParameters.statusReportFeedbackNumber = m_basicFeature->m_vvcPicParams->m_statusReportFeedbackNumber;
                inputParameters.codecFunction              = m_basicFeature->m_codecFunction;
                inputParameters.picWidthInMb               = m_basicFeature->m_picWidthInMb;
                inputParameters.pictureCodingType          = m_basicFeature->m_pictureCodingType;
                inputParameters.currOriginalPic            = m_basicFeature->m_curRenderPic;
                inputParameters.currDecodedPicRes          = m_basicFeature->m_destSurface.OsResource;
                inputParameters.numUsedVdbox               = m_numVdbox;
                m_allocator->UpdateResoreceUsageType(&inputParameters.currDecodedPicRes, resourceOutputPicture);
                m_statusReport->Init(&inputParameters);
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcPipeline::Execute()
    {
        DECODE_FUNC_CALL();

        PERF_UTILITY_AUTO((__FUNCTION__ + std::to_string((int)m_pipeMode)).c_str(), PERF_DECODE, PERF_LEVEL_HAL);

        if (m_pipeMode == decodePipeModeBegin)
        {
            return MOS_STATUS_SUCCESS;
        }

        DECODE_CHK_NULL(m_basicFeature);
        DECODE_CHK_NULL(m_basicFeature->m_vvcPicParams);

        if (m_pipeMode == decodePipeModeProcess)
        {
            DECODE_CHK_STATUS(m_preSubPipeline->Execute());

            if (IsCompleteBitstream())
            {
                AllocateResources(*m_basicFeature);
                DECODE_CHK_STATUS(InitContext());
                DECODE_CHK_STATUS(ActivateDecodePackets());
                DECODE_CHK_STATUS(ExecuteActivePackets());
            }

            DECODE_CHK_STATUS(m_postSubPipeline->Execute());
        }
        else if (m_pipeMode == decodePipeModeEnd)
        {
            DECODE_CHK_NULL(m_basicFeature);
            CODECHAL_DEBUG_TOOL(
                m_debugInterface->m_bufferDumpFrameNum = m_basicFeature->m_frameNum;
                DECODE_CHK_STATUS(m_debugInterface->DumpYUVSurface(
                    &m_basicFeature->m_destSurface,
                    CodechalDbgAttr::attrDecodeOutputSurface,
                    "DstSurf"));)

            CODECHAL_DEBUG_TOOL(
                PMHW_BATCH_BUFFER batchBuffer = m_vvcDecodePkt->GetPictureLvlBB();
                DECODE_CHK_NULL(batchBuffer);
                batchBuffer->iLastCurrent = batchBuffer->iSize;
                batchBuffer->dwOffset = 0;
                DECODE_CHK_STATUS(m_debugInterface->Dump2ndLvlBatch(
                    batchBuffer,
                    CODECHAL_NUM_MEDIA_STATES,
                    "VVC_DEC_Pic"));)

            if (m_basicFeature->m_shortFormatInUse) // Is Short Format In Use
            {
                CODECHAL_DEBUG_TOOL(
                    PMHW_BATCH_BUFFER batchBuffer = GetSliceLvlCmdBuffer();
                    DECODE_CHK_NULL(batchBuffer);
                    batchBuffer->iLastCurrent = batchBuffer->iSize;
                    batchBuffer->dwOffset     = 0;
                    DECODE_CHK_STATUS(m_debugInterface->Dump2ndLvlBatch(
                        batchBuffer,
                        CODECHAL_NUM_MEDIA_STATES,
                        "VVC_DEC_Slice"));)
                CODECHAL_DEBUG_TOOL(
                    PMHW_BATCH_BUFFER pbatchBuffer = GetTileLvlCmdBuffer();
                    DECODE_CHK_NULL(pbatchBuffer);
                    pbatchBuffer->iLastCurrent = pbatchBuffer->iSize;
                    pbatchBuffer->dwOffset     = 0;
                    DECODE_CHK_STATUS(m_debugInterface->Dump2ndLvlBatch(
                        pbatchBuffer,
                        CODECHAL_NUM_MEDIA_STATES,
                        "VVC_DEC_Tile"));)
            }

#if (_DEBUG || _RELEASE_INTERNAL)
            DECODE_CHK_STATUS(StatusCheck());
#endif

            // Only update user features for the first frame.
            if (m_basicFeature->m_frameNum == 0)
            {
                DECODE_CHK_STATUS(UserFeatureReport());
            }
            
            DecodeFrameIndex++;
            m_basicFeature->m_frameNum = DecodeFrameIndex;

            DECODE_CHK_STATUS(m_statusReport->Reset());
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcPipeline::GetStatusReport(void *status, uint16_t numStatus)
    {
        DECODE_FUNC_CALL();

        m_statusReport->GetReport(numStatus, status);

        return MOS_STATUS_SUCCESS;
    }

    uint32_t VvcPipeline::GetCompletedReport()
    {
        DECODE_FUNC_CALL();

        uint32_t completedCount = m_statusReport->GetCompletedCount();
        uint32_t reportedCount = m_statusReport->GetReportedCount();

        if (reportedCount > completedCount)
        {
            DECODE_ASSERTMESSAGE("No report available at all");
            return 0;
        }
        else
        {
            uint32_t availableCount = completedCount - reportedCount;
            return availableCount;
        }
    }

    MOS_STATUS VvcPipeline::Destroy()
    {
        DECODE_FUNC_CALL();

        Uninitialize();

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcPipeline::Initialize(void *settings)
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_STATUS(DecodePipeline::Initialize(settings));
        m_basicFeature = dynamic_cast<VvcBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        DECODE_CHK_NULL(m_basicFeature);

        DECODE_CHK_STATUS(InitMmcState());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcPipeline::Uninitialize()
    {
        DECODE_FUNC_CALL();

        for (auto pair : m_packetList)
        {
            pair.second->Destroy();
        }

        if (m_mmcState != nullptr)
        {
            MOS_Delete(m_mmcState);
        }

        if (m_allocator && m_basicFeature->m_shortFormatInUse && m_sliceLevelBBArray)
        {
            DECODE_CHK_STATUS(m_allocator->Destroy(m_sliceLevelBBArray));
        }
        if (m_allocator && m_basicFeature->m_shortFormatInUse && m_tileLevelBBArray)
        {
            DECODE_CHK_STATUS(m_allocator->Destroy(m_tileLevelBBArray));
        }


        return DecodePipeline::Uninitialize();
    }

    MOS_STATUS VvcPipeline::UserFeatureReport()
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_STATUS(DecodePipeline::UserFeatureReport());

    #ifdef _MMC_SUPPORTED
        CODECHAL_DEBUG_TOOL(
            if (m_mmcState != nullptr){
                m_mmcState->UpdateUserFeatureKey(&(m_basicFeature->m_destSurface));
            })
    #endif
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcPipeline::CreateSubPackets(DecodeSubPacketManager &subPacketManager, CodechalSetting &codecSettings)
    {
        DECODE_CHK_STATUS(DecodePipeline::CreateSubPackets(subPacketManager, codecSettings));

        VvcDecodePicPkt *pictureDecodePkt = MOS_New(VvcDecodePicPkt, this, m_hwInterface);
        DECODE_CHK_NULL(pictureDecodePkt);
        DECODE_CHK_STATUS(subPacketManager.Register(
                            DecodePacketId(this, vvcPictureSubPacketId), *pictureDecodePkt));

        VvcDecodeSlicePkt *sliceDecodePkt = MOS_New(VvcDecodeSlicePkt, this, m_hwInterface);
        DECODE_CHK_NULL(sliceDecodePkt);
        DECODE_CHK_STATUS(subPacketManager.Register(
                            DecodePacketId(this, vvcSliceSubPacketId), *sliceDecodePkt));

        if(m_decodecp!=nullptr)
        {
            auto feature = dynamic_cast<VvcBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
            DECODE_CHK_NULL(feature);
            DecodeSubPacket *cpSubPkt = (DecodeSubPacket *)m_decodecp->CreateDecodeCpIndSubPkt((DecodePipeline *)this, feature->m_mode, m_hwInterface);
            DECODE_CHK_NULL(cpSubPkt);
            DECODE_CHK_STATUS(subPacketManager.Register(
                            DecodePacketId(this, vvcCpSubPacketId), *cpSubPkt));
        }
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcPipeline::InitMmcState()
    {
#ifdef _MMC_SUPPORTED
        DECODE_CHK_NULL(m_hwInterface);
        DECODE_CHK_NULL(m_basicFeature);

        m_mmcState = MOS_New(DecodeMemComp, m_hwInterface);
        DECODE_CHK_NULL(m_mmcState);
        DECODE_CHK_STATUS(m_basicFeature->SetMmcState(m_mmcState->IsMmcEnabled()));
#endif

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcPipeline::AllocateResources(VvcBasicFeature &basicFeature)
    {
        DECODE_FUNC_CALL();
        uint32_t vvcpSliceCommandsSize  = 0;
        uint32_t vvcpSlicePatchListSize = 0;
        uint32_t vvcpTileCommandsSize   = 0;
        uint32_t vvcpTilePatchListSize  = 0;

        DECODE_CHK_NULL(m_hwInterface);
        CodechalHwInterfaceNext *hwInterface = dynamic_cast<CodechalHwInterfaceNext *>(m_hwInterface);
        DECODE_CHK_NULL(hwInterface);
        DECODE_CHK_STATUS(hwInterface->GetVvcpPrimitiveCommandSize(
            basicFeature.m_mode, &vvcpSliceCommandsSize, &vvcpSlicePatchListSize, &vvcpTileCommandsSize, &vvcpTilePatchListSize));
        DECODE_CHK_STATUS(hwInterface->GetVvcpSliceLvlCmdSize(&vvcpSliceCommandsSize));
        uint32_t size     = MOS_ALIGN_CEIL(vvcpSliceCommandsSize, 64) * basicFeature.m_numSlices;
        m_sliceLvlBufSize = MOS_ALIGN_CEIL(vvcpSliceCommandsSize, 64);

        // In VVC short format decode, second level command buffer is programmed by Huc, so not need lock it.
        if (m_basicFeature->m_shortFormatInUse)
        {
            //Slice Level BB Array Allocation
            if (m_sliceLevelBBArray == nullptr)
            {
                m_sliceLevelBBArray = m_allocator->AllocateBatchBufferArray(
                    size, 1, CODEC_VVC_BUFFER_ARRAY_SIZE, true, notLockableVideoMem);
                DECODE_CHK_NULL(m_sliceLevelBBArray);
                PMHW_BATCH_BUFFER &batchBuf = m_sliceLevelBBArray->Fetch();
                DECODE_CHK_NULL(batchBuf);
            }
            else
            {
                PMHW_BATCH_BUFFER &batchBuf = m_sliceLevelBBArray->Fetch();
                DECODE_CHK_NULL(batchBuf);
                DECODE_CHK_STATUS(m_allocator->Resize(
                    batchBuf, size, basicFeature.m_numSlices, notLockableVideoMem));
            }

            //Tile Level BB Array Allocation
            uint32_t tileLvlCmdSize = 0;
            if (!m_basicFeature->m_vvcPicParams->m_ppsFlags.m_fields.m_ppsRectSliceFlag)  // Raster Scan Mode
            {
                tileLvlCmdSize = vvcpTileCommandsSize * vvcMaxTileRowNum;
            }
            else //Rect Scan Mode
            {
                tileLvlCmdSize = (m_basicFeature->m_numSlices + m_basicFeature->m_tileCols * m_basicFeature->m_tileRows) * vvcpTileCommandsSize;
            }
            tileLvlCmdSize   = MOS_ALIGN_CEIL(tileLvlCmdSize, 64);
            m_tileLvlBufSize = tileLvlCmdSize;
            if (m_tileLevelBBArray == nullptr)
            {
                m_tileLevelBBArray          = m_allocator->AllocateBatchBufferArray(tileLvlCmdSize, 1, CODEC_VVC_BUFFER_ARRAY_SIZE, true, notLockableVideoMem);
                PMHW_BATCH_BUFFER&BatchBuf  = m_tileLevelBBArray->Fetch();
                DECODE_CHK_NULL(BatchBuf);
            }
            else
            {
                PMHW_BATCH_BUFFER &batchBuf = m_tileLevelBBArray->Fetch();
                DECODE_CHK_NULL(batchBuf);
                DECODE_CHK_STATUS(m_allocator->Resize(
                    batchBuf, tileLvlCmdSize, 1, notLockableVideoMem));
            }

        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_BATCH_BUFFER *VvcPipeline::GetSliceLvlCmdBuffer()
    {
        if (m_sliceLevelBBArray == nullptr)
        {
            return nullptr;
        }
        return m_sliceLevelBBArray->Peek();
    }

    MHW_BATCH_BUFFER *VvcPipeline::GetTileLvlCmdBuffer()
    {
        if (m_tileLevelBBArray == nullptr)
        {
            return nullptr;
        }
        return m_tileLevelBBArray->Peek();
    }

 #if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS VvcPipeline::DumpParams(VvcBasicFeature &basicFeature)
    {
        m_debugInterface->m_bufferDumpFrameNum = basicFeature.m_frameNum;

        DECODE_CHK_STATUS(DumpPicParams(basicFeature.m_vvcPicParams, basicFeature.m_shortFormatInUse));
        DECODE_CHK_STATUS(DumpApsAlfData(basicFeature.m_alfApsArray, vvcMaxAlfNum));
        DECODE_CHK_STATUS(DumpApsLmcsData(basicFeature.m_lmcsApsArray, vvcMaxLmcsNum));
        DECODE_CHK_STATUS(DumpApsQuantMatrix(basicFeature.m_scalingListArray, vvcMaxScalingMatrixNum));
        DECODE_CHK_STATUS(DumpTileParams(basicFeature.m_tileParams, basicFeature.m_vvcPicParams->m_ppsNumExpTileColumnsMinus1 + basicFeature.m_vvcPicParams->m_ppsNumExpTileRowsMinus1 + 2));
        DECODE_CHK_STATUS(DumpSubpicParams(basicFeature.m_subPicParams, (basicFeature.m_vvcPicParams->m_spsFlags0.m_fields.m_spsSubpicInfoPresentFlag && basicFeature.m_vvcPicParams->m_spsNumSubpicsMinus1 > 0)? (basicFeature.m_vvcPicParams->m_spsNumSubpicsMinus1+1) : 0));
        DECODE_CHK_STATUS(DumpSliceStructParams(basicFeature.m_sliceStructParams, ((basicFeature.m_vvcPicParams->m_ppsFlags.m_fields.m_ppsRectSliceFlag && basicFeature.m_vvcPicParams->m_numSliceStructsMinus1 > 0))? (basicFeature.m_vvcPicParams->m_numSliceStructsMinus1 + 1) : 0));
        DECODE_CHK_STATUS(DumpRplStructParams(basicFeature.m_rplParams, basicFeature.m_vvcPicParams->m_numRefPicListStructs, basicFeature.m_shortFormatInUse));
        DECODE_CHK_STATUS(DumpSliceParams(basicFeature.m_vvcSliceParams, basicFeature.m_numSlices, basicFeature.m_shortFormatInUse));
        DECODE_CHK_STATUS(DumpBitstream(&basicFeature.m_resDataBuffer.OsResource, basicFeature.m_dataSize, 0));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcPipeline::DumpPicParams(
        CodecVvcPicParams *picParams, 
        bool               shortFormatInUse)
    {
        CODECHAL_DEBUG_FUNCTION_ENTER;

        if (picParams == nullptr)
        {
            return MOS_STATUS_SUCCESS;
        }

        if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrPicParams))
        {
            const char *fileName = m_debugInterface->CreateFileName(
                "_DEC",
                CodechalDbgBufferType::bufPicParams,
                CodechalDbgExtType::txt);

            if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrEnableFastDump))
            {
                MediaDebugFastDump::Dump(
                    (uint8_t *)picParams,
                    fileName,
                    sizeof(CodecVvcPicParams),
                    0);
            }
            else
            {
                DumpDecodeVvcPicParams(picParams, fileName, shortFormatInUse);
            }
        }
        
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcPipeline::DumpApsAlfData(
            CodecVvcAlfData *alfDataBuf,
            uint32_t        numAlf)
    {
        CODECHAL_DEBUG_FUNCTION_ENTER;

        if (alfDataBuf == nullptr)
        {
            return MOS_STATUS_SUCCESS;
        }

        if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrAlfData))
        {
            const char *fileName = m_debugInterface->CreateFileName(
                "_DEC",
                CodechalDbgBufferType::bufAlfData,
                CodechalDbgExtType::txt);

            if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrEnableFastDump))
            {
                MediaDebugFastDump::Dump(
                    (uint8_t *)alfDataBuf,
                    fileName,
                    sizeof(CodecVvcAlfData) * numAlf,
                    0);
            }
            else
            {
                DumpDecodeVvcAlfParams(alfDataBuf, numAlf, fileName);
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcPipeline::DumpApsLmcsData(
            CodecVvcLmcsData *lmcsDataBuf,
            uint32_t         numLmcs)
    {
        CODECHAL_DEBUG_FUNCTION_ENTER;

        if (lmcsDataBuf == nullptr)
        {
            return MOS_STATUS_SUCCESS;
        }

        if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrLmcsData))
        {
            const char *fileName = m_debugInterface->CreateFileName(
                "_DEC",
                CodechalDbgBufferType::bufLmcsData,
                CodechalDbgExtType::txt);
            
            if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrEnableFastDump))
            {
                MediaDebugFastDump::Dump(
                    (uint8_t *)lmcsDataBuf,
                    fileName,
                    sizeof(CodecVvcLmcsData) * numLmcs,
                    0);
            }
            else
            {
                DumpDecodeVvcLmcsParams(lmcsDataBuf, numLmcs, fileName);
            }
        }
        
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcPipeline::DumpApsQuantMatrix(
            CodecVvcQmData *quantMatrixBuf,
            uint32_t        numScalingList)
    {
        CODECHAL_DEBUG_FUNCTION_ENTER;

        if (quantMatrixBuf == nullptr)
        {
            return MOS_STATUS_SUCCESS;
        }

        if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrIqParams))
        {
            const char *fileName = m_debugInterface->CreateFileName(
                "_DEC",
                CodechalDbgBufferType::bufIqParams,
                CodechalDbgExtType::txt);

            if(m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrEnableFastDump))
            {
                MediaDebugFastDump::Dump(
                    (uint8_t *)quantMatrixBuf,
                    fileName,
                    sizeof(CodecVvcQmData) * numScalingList,
                    0);
            }
            else
            {
                DumpDecodeVvcIqParams(quantMatrixBuf, numScalingList, fileName);
            }
        }
        
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcPipeline::DumpTileParams(
            CodecVvcTileParam *tileParams,
            uint32_t          numElements)
    {
        CODECHAL_DEBUG_FUNCTION_ENTER;

        if (tileParams == nullptr)
        {
            return MOS_STATUS_SUCCESS;
        }

        if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrTileParams))
        {
            const char *fileName = m_debugInterface->CreateFileName(
                "_DEC",
                CodechalDbgBufferType::bufTileParams,
                CodechalDbgExtType::txt);

            if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrEnableFastDump))
            {
                MediaDebugFastDump::Dump(
                    (uint8_t *)tileParams,
                    fileName,
                    sizeof(CodecVvcTileParam) * numElements,
                    0);
            }
            else
            {
                DumpDecodeVvcTileParams(tileParams, numElements, fileName);
            }
        }
        
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcPipeline::DumpSubpicParams(
            CodecVvcSubpicParam *subpicParamsBuf,
            uint32_t            numSubpics)
    {
        CODECHAL_DEBUG_FUNCTION_ENTER;

        if (subpicParamsBuf == nullptr)
        {
            return MOS_STATUS_SUCCESS;
        }

        if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSubpicParams))
        {
            const char *fileName = m_debugInterface->CreateFileName(
                "_DEC",
                CodechalDbgBufferType::bufSubpicParams,
                CodechalDbgExtType::txt);

            if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrEnableFastDump))
            {
                MediaDebugFastDump::Dump(
                    (uint8_t *)subpicParamsBuf,
                    fileName,
                    sizeof(CodecVvcSubpicParam) * numSubpics,
                    0);
            }
            else
            {
                DumpDecodeVvcSubpicParams(subpicParamsBuf, numSubpics, fileName);
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcPipeline::DumpSliceStructParams(
            CodecVvcSliceStructure *sliceStructParamsBuf,
            uint32_t                numSlcStruct)
    {
        CODECHAL_DEBUG_FUNCTION_ENTER;

        if (sliceStructParamsBuf == nullptr)
        {
            return MOS_STATUS_SUCCESS;
        }

        if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSliceStruct))
        {
            const char *fileName = m_debugInterface->CreateFileName(
                "_DEC",
                CodechalDbgBufferType::bufSliceStruct,
                CodechalDbgExtType::txt);

            if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrEnableFastDump))
            {
                MediaDebugFastDump::Dump(
                    (uint8_t *)sliceStructParamsBuf,
                    fileName,
                    sizeof(CodecVvcSliceStructure) * numSlcStruct,
                    0);
            }
            else
            {
                DumpDecodeVvcSliceStructureParams(sliceStructParamsBuf, numSlcStruct, fileName);
            }
        }
        
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcPipeline::DumpRplStructParams(
            CodecVvcRplStructure *rplParams,
            uint32_t             numRpls,
            bool                 shortFormatInUse)
    {
        CODECHAL_DEBUG_FUNCTION_ENTER;

        if (rplParams == nullptr)
        {
            return MOS_STATUS_SUCCESS;
        }

        if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrRplStruct))
        {
            const char *fileName = m_debugInterface->CreateFileName(
                "_DEC",
                CodechalDbgBufferType::bufRplStruct,
                CodechalDbgExtType::txt);

            if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrEnableFastDump))
            {
                MediaDebugFastDump::Dump(
                    (uint8_t *)rplParams,
                    fileName,
                    sizeof(CodecVvcRplStructure) * numRpls,
                    0);
            }
            else
            {
                DumpDecodeVvcRplStructureParams(rplParams, numRpls, fileName);
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcPipeline::DumpSliceParams(
        CodecVvcSliceParams* sliceParams,
        uint32_t             numSlices,
        bool                 shortFormatInUse)
    {
        CODECHAL_DEBUG_FUNCTION_ENTER;

        if (sliceParams == nullptr)
        {
            return MOS_STATUS_SUCCESS;
        }

        if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSlcParams))
        {
            const char *fileName = m_debugInterface->CreateFileName(
                "_DEC",
                CodechalDbgBufferType::bufSlcParams,
                CodechalDbgExtType::txt);

            if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrEnableFastDump))
            {
                MediaDebugFastDump::Dump(
                    (uint8_t *)sliceParams,
                    fileName,
                    sizeof(CodecVvcSliceParams) * numSlices,
                    0);
            }
            else
            {
                DumpDecodeVvcSliceParams(sliceParams, numSlices, fileName, shortFormatInUse);
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    void DumpDecodeVvcPicParams(CodecVvcPicParams *picParams, std::string fileName, bool shortFormatInUse)
    {
        std::ostringstream oss;
        oss.setf(std::ios::showbase | std::ios::uppercase);

        // SPS info
        oss << "spsPicWidthMaxInLumaSamples: " << +picParams->m_spsPicWidthMaxInLumaSamples << std::endl;
        oss << "spsPicHeightMaxInLumaSamples: " << +picParams->m_spsPicHeightMaxInLumaSamples << std::endl;

        oss << "spsNumSubpicsMinus1: " << +picParams->m_spsNumSubpicsMinus1 << std::endl;
        oss << "spsSubpicIdLenMinus1: " << +picParams->m_spsSubpicIdLenMinus1 << std::endl;
        oss << "spsChromaFormatIdc: " << +picParams->m_spsChromaFormatIdc << std::endl;
        oss << "spsBitdepthMinus8: " << +picParams->m_spsBitdepthMinus8 << std::endl;
        oss << "spsLog2CtuSizeMinus5: " << +picParams->m_spsLog2CtuSizeMinus5 << std::endl;
        oss << "spsLog2MaxPicOrderCntLsbMinus4: " << +picParams->m_spsLog2MaxPicOrderCntLsbMinus4 << std::endl;
        oss << "spsLog2MinLumaCodingBlockSizeMinus2: " << +picParams->m_spsLog2MinLumaCodingBlockSizeMinus2 << std::endl;
        oss << "spsPocMsbCycleLenMinus1: " << +picParams->m_spsPocMsbCycleLenMinus1 << std::endl;
        oss << "numExtraPhBits: " << +picParams->m_numExtraPhBits << std::endl;
        oss << "numExtraShBits: " << +picParams->m_numExtraShBits << std::endl;
        oss << "spsLog2TransformSkipMaxSizeMinus2: " << +picParams->m_spsLog2TransformSkipMaxSizeMinus2 << std::endl;

        for (auto i = 0; i < 3; i++)
        {
            for (auto j = 0; j < 76; j++)
            {
                oss << "chromaQpTable[" << +i << "][" << +j << "]: " << +picParams->m_chromaQpTable[i][j] << std::endl;
            }
        }

        for (auto i = 0; i < 2; i++)
        {
            oss << "spsNumRefPicLists[" << +i << "]: " << +picParams->m_spsNumRefPicLists[i] << std::endl;
        }
        oss << "spsSixMinusMaxNumMergeCand: " << +picParams->m_spsSixMinusMaxNumMergeCand << std::endl;
        oss << "spsFiveMinusMaxNumSubblockMergeCand: " << +picParams->m_spsFiveMinusMaxNumSubblockMergeCand << std::endl;
        oss << "spsMaxNumMergeCandMinusMaxNumGpmCand: " << +picParams->m_spsMaxNumMergeCandMinusMaxNumGpmCand << std::endl;
        oss << "spsLog2ParallelMergeLevelMinus2: " << +picParams->m_spsLog2ParallelMergeLevelMinus2 << std::endl;
        oss << "spsMinQpPrimeTs: " << +picParams->m_spsMinQpPrimeTs << std::endl;
        oss << "spsSixMinusMaxNumIbcMergeCand: " << +picParams->m_spsSixMinusMaxNumIbcMergeCand << std::endl;
        oss << "spsNumLadfIntervalsMinus2: " << +picParams->m_spsNumLadfIntervalsMinus2 << std::endl;
        oss << "spsLadfLowestIntervalQpOffset: " << +picParams->m_spsLadfLowestIntervalQpOffset << std::endl;
        for (auto i = 0; i < 4; i++)
        {
            oss << "spsLadfQpOffset[" << +i << "]: " << +picParams->m_spsLadfQpOffset[i] << std::endl;
        }
        for (auto i = 0; i < 4; i++)
        {
            oss << "spsLadfDeltaThresholdMinus1[" << +i << "]: " << +picParams->m_spsLadfDeltaThresholdMinus1[i] << std::endl;
        }

        oss << "spsFlags0 value: " << +picParams->m_spsFlags0.m_value << std::endl;
        oss << "spsSubpicInfoPresentFlag: " << +picParams->m_spsFlags0.m_fields.m_spsSubpicInfoPresentFlag << std::endl;
        oss << "spsIndependentSubpicsFlag: " << +picParams->m_spsFlags0.m_fields.m_spsIndependentSubpicsFlag << std::endl;
        oss << "spsSubpicSameSizeFlag: " << +picParams->m_spsFlags0.m_fields.m_spsSubpicSameSizeFlag << std::endl;
        oss << "spsEntropyCodingSyncEnabledFlag: " << +picParams->m_spsFlags0.m_fields.m_spsEntropyCodingSyncEnabledFlag << std::endl;
        oss << "spsEntryPointOffsetsPresentFlag: " << +picParams->m_spsFlags0.m_fields.m_spsEntryPointOffsetsPresentFlag << std::endl;
        oss << "spsPocMsbCycleFlag: " << +picParams->m_spsFlags0.m_fields.m_spsPocMsbCycleFlag << std::endl;
        oss << "spsPartitionConstraintsOverrideEnabledFlag: " << +picParams->m_spsFlags0.m_fields.m_spsPartitionConstraintsOverrideEnabledFlag << std::endl;
        oss << "spsQtbttDualTreeIntraFlag: " << +picParams->m_spsFlags0.m_fields.m_spsQtbttDualTreeIntraFlag << std::endl;
        oss << "spsMaxLumaTransformSize64Flag: " << +picParams->m_spsFlags0.m_fields.m_spsMaxLumaTransformSize64Flag << std::endl;
        oss << "spsTransformSkipEnabledFlag: " << +picParams->m_spsFlags0.m_fields.m_spsTransformSkipEnabledFlag << std::endl;
        oss << "spsBdpcmEnabledFlag: " << +picParams->m_spsFlags0.m_fields.m_spsBdpcmEnabledFlag << std::endl;
        oss << "spsMtsEnabledFlag: " << +picParams->m_spsFlags0.m_fields.m_spsMtsEnabledFlag << std::endl;
        oss << "spsExplicitMtsIntraEnabledFlag: " << +picParams->m_spsFlags0.m_fields.m_spsExplicitMtsIntraEnabledFlag << std::endl;
        oss << "spsExplicitMtsInterEnabledFlag: " << +picParams->m_spsFlags0.m_fields.m_spsExplicitMtsInterEnabledFlag << std::endl;
        oss << "spsLfnstEnabledFlag: " << +picParams->m_spsFlags0.m_fields.m_spsLfnstEnabledFlag << std::endl;
        oss << "spsJointCbcrEnabledFlag: " << +picParams->m_spsFlags0.m_fields.m_spsJointCbcrEnabledFlag << std::endl;
        oss << "spsSameQpTableForChromaFlag: " << +picParams->m_spsFlags0.m_fields.m_spsSameQpTableForChromaFlag << std::endl;
        oss << "spsSaoEnabledFlag: " << +picParams->m_spsFlags0.m_fields.m_spsSaoEnabledFlag << std::endl;
        oss << "spsAlfEnabledFlag: " << +picParams->m_spsFlags0.m_fields.m_spsAlfEnabledFlag << std::endl;
        oss << "spsCcalfEnabledFlag: " << +picParams->m_spsFlags0.m_fields.m_spsCcalfEnabledFlag << std::endl;
        oss << "spsLmcsEnabledFlag: " << +picParams->m_spsFlags0.m_fields.m_spsLmcsEnabledFlag << std::endl;
        oss << "spsLongTermRefPicsFlag: " << +picParams->m_spsFlags0.m_fields.m_spsLongTermRefPicsFlag << std::endl;
        oss << "spsInterLayerPredictionEnabledFlag: " << +picParams->m_spsFlags0.m_fields.m_spsInterLayerPredictionEnabledFlag << std::endl;
        oss << "spsIdrRplPresentFlag: " << +picParams->m_spsFlags0.m_fields.m_spsIdrRplPresentFlag << std::endl;

        oss << "spsFlags1 value: " << +picParams->m_spsFlags1.m_value << std::endl;
        oss << "spsTemporalMvpEnabledFlag: " << +picParams->m_spsFlags1.m_fields.m_spsTemporalMvpEnabledFlag << std::endl;
        oss << "spsSbtmvpEnabledFlag: " << +picParams->m_spsFlags1.m_fields.m_spsSbtmvpEnabledFlag << std::endl;
        oss << "spsAmvrEnabledFlag: " << +picParams->m_spsFlags1.m_fields.m_spsAmvrEnabledFlag << std::endl;
        oss << "spsBdofEnabledFlag: " << +picParams->m_spsFlags1.m_fields.m_spsBdofEnabledFlag << std::endl;
        oss << "spsBdofControlPresentInPhFlag: " << +picParams->m_spsFlags1.m_fields.m_spsBdofControlPresentInPhFlag << std::endl;
        oss << "spsSmvdEnabledFlag: " << +picParams->m_spsFlags1.m_fields.m_spsSmvdEnabledFlag << std::endl;
        oss << "spsDmvrEnabledFlag: " << +picParams->m_spsFlags1.m_fields.m_spsDmvrEnabledFlag << std::endl;
        oss << "spsDmvrControlPresentInPhFlag: " << +picParams->m_spsFlags1.m_fields.m_spsDmvrControlPresentInPhFlag << std::endl;
        oss << "spsMmvdEnabledFlag: " << +picParams->m_spsFlags1.m_fields.m_spsMmvdEnabledFlag << std::endl;
        oss << "spsMmvdFullpelOnlyEnabledFlag: " << +picParams->m_spsFlags1.m_fields.m_spsMmvdFullpelOnlyEnabledFlag << std::endl;
        oss << "spsSbtEnabledFlag: " << +picParams->m_spsFlags1.m_fields.m_spsSbtEnabledFlag << std::endl;
        oss << "spsAffineEnabledFlag: " << +picParams->m_spsFlags1.m_fields.m_spsAffineEnabledFlag << std::endl;
        oss << "sps6paramAffineEnabledFlag: " << +picParams->m_spsFlags1.m_fields.m_sps6paramAffineEnabledFlag << std::endl;
        oss << "spsAffineAmvrEnabledFlag: " << +picParams->m_spsFlags1.m_fields.m_spsAffineAmvrEnabledFlag << std::endl;
        oss << "spsAffineProfEnabledFlag: " << +picParams->m_spsFlags1.m_fields.m_spsAffineProfEnabledFlag << std::endl;
        oss << "spsProfControlPresentInPhFlag: " << +picParams->m_spsFlags1.m_fields.m_spsProfControlPresentInPhFlag << std::endl;
        oss << "spsBcwEnabledFlag: " << +picParams->m_spsFlags1.m_fields.m_spsBcwEnabledFlag << std::endl;
        oss << "spsCiipEnabledFlag: " << +picParams->m_spsFlags1.m_fields.m_spsCiipEnabledFlag << std::endl;
        oss << "spsGpmEnabledFlag: " << +picParams->m_spsFlags1.m_fields.m_spsGpmEnabledFlag << std::endl;
        oss << "spsIspEnabledFlag: " << +picParams->m_spsFlags1.m_fields.m_spsIspEnabledFlag << std::endl;
        oss << "spsMrlEnabledFlag: " << +picParams->m_spsFlags1.m_fields.m_spsMrlEnabledFlag << std::endl;
        oss << "spsMipEnabledFlag: " << +picParams->m_spsFlags1.m_fields.m_spsMipEnabledFlag << std::endl;
        oss << "spsCclmEnabledFlag: " << +picParams->m_spsFlags1.m_fields.m_spsCclmEnabledFlag << std::endl;
        oss << "spsChromaHorizontalCollocatedFlag: " << +picParams->m_spsFlags1.m_fields.m_spsChromaHorizontalCollocatedFlag << std::endl;
        oss << "spsChromaVerticalCollocatedFlag: " << +picParams->m_spsFlags1.m_fields.m_spsChromaVerticalCollocatedFlag << std::endl;

        oss << "spsFlags2 value: " << +picParams->m_spsFlags2.m_value << std::endl;
        oss << "spsPaletteEnabledFlag: " << +picParams->m_spsFlags2.m_fields.m_spsPaletteEnabledFlag << std::endl;
        oss << "spsActEnabledFlag: " << +picParams->m_spsFlags2.m_fields.m_spsActEnabledFlag << std::endl;
        oss << "spsIbcEnabledFlag: " << +picParams->m_spsFlags2.m_fields.m_spsIbcEnabledFlag << std::endl;
        oss << "spsLadfEnabledFlag: " << +picParams->m_spsFlags2.m_fields.m_spsLadfEnabledFlag << std::endl;
        oss << "spsExplicitScalingListEnabledFlag: " << +picParams->m_spsFlags2.m_fields.m_spsExplicitScalingListEnabledFlag << std::endl;
        oss << "spsScalingMatrixForLfnstDisabledFlag: " << +picParams->m_spsFlags2.m_fields.m_spsScalingMatrixForLfnstDisabledFlag << std::endl;
        oss << "spsScalingMatrixForAlternativeColourSpaceDisabledFlag: " << +picParams->m_spsFlags2.m_fields.m_spsScalingMatrixForAlternativeColourSpaceDisabledFlag << std::endl;
        oss << "spsScalingMatrixDesignatedColourSpaceFlag: " << +picParams->m_spsFlags2.m_fields.m_spsScalingMatrixDesignatedColourSpaceFlag << std::endl;
        oss << "spsDepQuantEnabledFlag: " << +picParams->m_spsFlags2.m_fields.m_spsDepQuantEnabledFlag << std::endl;
        oss << "spsSignDataHidingEnabledFlag: " << +picParams->m_spsFlags2.m_fields.m_spsSignDataHidingEnabledFlag << std::endl;
        oss << "spsVirtualBoundariesEnabledFlag: " << +picParams->m_spsFlags2.m_fields.m_spsVirtualBoundariesEnabledFlag << std::endl;
        oss << "spsVirtualBoundariesPresentFlag: " << +picParams->m_spsFlags2.m_fields.m_spsVirtualBoundariesPresentFlag << std::endl;
        oss << "spsWeightedPredFlag: " << +picParams->m_spsFlags2.m_fields.m_spsWeightedPredFlag << std::endl;
        oss << "spsWeightedBipredFlag: " << +picParams->m_spsFlags2.m_fields.m_spsWeightedBipredFlag << std::endl;

        // PPS info
        oss << "ppsPicWidthInLumaSamples: " << +picParams->m_ppsPicWidthInLumaSamples << std::endl;
        oss << "ppsPicHeightInLumaSamples: " << +picParams->m_ppsPicHeightInLumaSamples << std::endl;
        oss << "numVerVirtualBoundaries: " << +picParams->m_numVerVirtualBoundaries << std::endl;
        oss << "numHorVirtualBoundaries: " << +picParams->m_numHorVirtualBoundaries << std::endl;
        for (auto i = 0; i < 3; i++)
        {
            oss << "virtualBoundaryPosX[" << +i << "]: " << +picParams->m_virtualBoundaryPosX[i] << std::endl;
        }
        for (auto i = 0; i < 3; i++)
        {
            oss << "virtualBoundaryPosY[" << +i << "]: " << +picParams->m_virtualBoundaryPosY[i] << std::endl;
        }

        oss << "ppsScalingWinLeftOffset: " << +picParams->m_ppsScalingWinLeftOffset << std::endl;
        oss << "ppsScalingWinRightOffset: " << +picParams->m_ppsScalingWinRightOffset << std::endl;
        oss << "ppsScalingWinTopOffset: " << +picParams->m_ppsScalingWinTopOffset << std::endl;
        oss << "ppsScalingWinBottomOffset: " << +picParams->m_ppsScalingWinBottomOffset << std::endl;

        oss << "ppsNumExpTileColumnsMinus1: " << +picParams->m_ppsNumExpTileColumnsMinus1 << std::endl;
        oss << "ppsNumExpTileRowsMinus1: " << +picParams->m_ppsNumExpTileRowsMinus1 << std::endl;
        oss << "ppsNumSlicesInPicMinus1: " << +picParams->m_ppsNumSlicesInPicMinus1 << std::endl;
        for (auto i = 0; i < 2; i++)
        {
            oss << "ppsNumRefIdxDefaultActiveMinus1[" << +i << "]: " << +picParams->m_ppsNumRefIdxDefaultActiveMinus1[i] << std::endl;
        }
        oss << "ppsPicWidthMinusWraparoundOffset: " << +picParams->m_ppsPicWidthMinusWraparoundOffset << std::endl;
        oss << "ppsInitQpMinus26: " << +picParams->m_ppsInitQpMinus26 << std::endl;
        oss << "ppsCbQpOffset: " << +picParams->m_ppsCbQpOffset << std::endl;
        oss << "ppsCrQpOffset: " << +picParams->m_ppsCrQpOffset << std::endl;
        oss << "ppsJointCbcrQpOffsetValue: " << +picParams->m_ppsJointCbcrQpOffsetValue << std::endl;
        oss << "ppsChromaQpOffsetListLenMinus1: " << +picParams->m_ppsChromaQpOffsetListLenMinus1 << std::endl;
        for (auto i = 0; i < 6; i++)
        {
            oss << "ppsCbQpOffsetList[" << +i << "]: " << +picParams->m_ppsCbQpOffsetList[i] << std::endl;
        }
        for (auto i = 0; i < 6; i++)
        {
            oss << "ppsCrQpOffsetList[" << +i << "]: " << +picParams->m_ppsCrQpOffsetList[i] << std::endl;
        }
        for (auto i = 0; i < 6; i++)
        {
            oss << "ppsJointCbcrQpOffsetList[" << +i << "]: " << +picParams->m_ppsJointCbcrQpOffsetList[i] << std::endl;
        }

        oss << "numScalingMatrixBuffers: " << +picParams->m_numScalingMatrixBuffers << std::endl;
        oss << "numAlfBuffers: " << +picParams->m_numAlfBuffers << std::endl;
        oss << "numLmcsBuffers: " << +picParams->m_numLmcsBuffers << std::endl;
        oss << "numRefPicListStructs: " << +picParams->m_numRefPicListStructs << std::endl;
        oss << "numSliceStructsMinus1: " << +picParams->m_numSliceStructsMinus1 << std::endl;

        oss << "ppsFlags value: " << +picParams->m_ppsFlags.m_value << std::endl;
        oss << "ppsOutputFlagPresentFlag: " << +picParams->m_ppsFlags.m_fields.m_ppsOutputFlagPresentFlag << std::endl;
        oss << "ppsLoopFilterAcrossTilesEnabledFlag: " << +picParams->m_ppsFlags.m_fields.m_ppsLoopFilterAcrossTilesEnabledFlag << std::endl;
        oss << "ppsRectSliceFlag: " << +picParams->m_ppsFlags.m_fields.m_ppsRectSliceFlag << std::endl;
        oss << "ppsSingleSlicePerSubpicFlag: " << +picParams->m_ppsFlags.m_fields.m_ppsSingleSlicePerSubpicFlag << std::endl;
        oss << "ppsLoopFilterAcrossSlicesEnabledFlag: " << +picParams->m_ppsFlags.m_fields.m_ppsLoopFilterAcrossSlicesEnabledFlag << std::endl;
        oss << "ppsCabacInitPresentFlag: " << +picParams->m_ppsFlags.m_fields.m_ppsCabacInitPresentFlag << std::endl;
        oss << "ppsRpl1IdxPresentFlag: " << +picParams->m_ppsFlags.m_fields.m_ppsRpl1IdxPresentFlag << std::endl;
        oss << "ppsWeightedPredFlag: " << +picParams->m_ppsFlags.m_fields.m_ppsWeightedPredFlag << std::endl;
        oss << "ppsWeightedBipredFlag: " << +picParams->m_ppsFlags.m_fields.m_ppsWeightedBipredFlag << std::endl;
        oss << "ppsRefWraparoundEnabledFlag: " << +picParams->m_ppsFlags.m_fields.m_ppsRefWraparoundEnabledFlag << std::endl;
        oss << "ppsCuQpDeltaEnabledFlag: " << +picParams->m_ppsFlags.m_fields.m_ppsCuQpDeltaEnabledFlag << std::endl;
        oss << "ppsChroma_toolOffsetsPresentFlag: " << +picParams->m_ppsFlags.m_fields.m_ppsChroma_toolOffsetsPresentFlag << std::endl;
        oss << "ppsSliceChromaQpOffsetsPresentFlag: " << +picParams->m_ppsFlags.m_fields.m_ppsSliceChromaQpOffsetsPresentFlag << std::endl;
        oss << "ppsCuChromaQpOffsetListEnabledFlag: " << +picParams->m_ppsFlags.m_fields.m_ppsCuChromaQpOffsetListEnabledFlag << std::endl;
        oss << "ppsDeblockingFilterOverrideEnabledFlag: " << +picParams->m_ppsFlags.m_fields.m_ppsDeblockingFilterOverrideEnabledFlag << std::endl;
        oss << "ppsDeblockingFilterDisabledFlag: " << +picParams->m_ppsFlags.m_fields.m_ppsDeblockingFilterDisabledFlag << std::endl;
        oss << "ppsDbfInfoInPhFlag: " << +picParams->m_ppsFlags.m_fields.m_ppsDbfInfoInPhFlag << std::endl;
        oss << "ppsRplInfoInPhFlag: " << +picParams->m_ppsFlags.m_fields.m_ppsRplInfoInPhFlag << std::endl;
        oss << "ppsSaoInfoInPhFlag: " << +picParams->m_ppsFlags.m_fields.m_ppsSaoInfoInPhFlag << std::endl;
        oss << "ppsAlfInfoInPhFlag: " << +picParams->m_ppsFlags.m_fields.m_ppsAlfInfoInPhFlag << std::endl;
        oss << "ppsWpInfoInPhFlag: " << +picParams->m_ppsFlags.m_fields.m_ppsWpInfoInPhFlag << std::endl;
        oss << "ppsQpDeltaInfoInPhFlag: " << +picParams->m_ppsFlags.m_fields.m_ppsQpDeltaInfoInPhFlag << std::endl;
        oss << "ppsPictureHeaderExtensionPresentFlag: " << +picParams->m_ppsFlags.m_fields.m_ppsPictureHeaderExtensionPresentFlag << std::endl;
        oss << "ppsSliceHeaderExtensionPresentFlag: " << +picParams->m_ppsFlags.m_fields.m_ppsSliceHeaderExtensionPresentFlag << std::endl;

        // PH info
        oss << "phNumAlfApsIdsLuma: " << +picParams->m_phNumAlfApsIdsLuma << std::endl;
        for (auto i = 0; i < 7; i++)
        {
            oss << "phAlfApsIdLuma[" << +i << "]: " << +picParams->m_phAlfApsIdLuma[i] << std::endl;
        }
        oss << "phAlfApsIdChroma: " << +picParams->m_phAlfApsIdChroma << std::endl;
        oss << "phAlfCcCbApsId: " << +picParams->m_phAlfCcCbApsId << std::endl;
        oss << "phAlfCcCrApsId: " << +picParams->m_phAlfCcCrApsId << std::endl;
        oss << "phLmcsApsId: " << +picParams->m_phLmcsApsId << std::endl;
        oss << "phScalingListApsId: " << +picParams->m_phScalingListApsId << std::endl;
        oss << "phLog2DiffMinQtMinCbIntraSliceLuma: " << +picParams->m_phLog2DiffMinQtMinCbIntraSliceLuma << std::endl;
        oss << "phMaxMtt_hierarchyDepthIntraSliceLuma: " << +picParams->m_phMaxMtt_hierarchyDepthIntraSliceLuma << std::endl;
        oss << "phLog2DiffMaxBtMinQtIntraSliceLuma: " << +picParams->m_phLog2DiffMaxBtMinQtIntraSliceLuma << std::endl;
        oss << "phLog2DiffMax_ttMinQtIntraSliceLuma: " << +picParams->m_phLog2DiffMax_ttMinQtIntraSliceLuma << std::endl;
        oss << "phLog2DiffMinQtMinCbIntraSliceChroma: " << +picParams->m_phLog2DiffMinQtMinCbIntraSliceChroma << std::endl;
        oss << "phMaxMtt_hierarchyDepthIntraSliceChroma: " << +picParams->m_phMaxMtt_hierarchyDepthIntraSliceChroma << std::endl;
        oss << "phLog2DiffMaxBtMinQtIntraSliceChroma: " << +picParams->m_phLog2DiffMaxBtMinQtIntraSliceChroma << std::endl;
        oss << "phLog2DiffMax_ttMinQtIntraSliceChroma: " << +picParams->m_phLog2DiffMax_ttMinQtIntraSliceChroma << std::endl;
        oss << "phCuQpDeltaSubdivIntraSlice: " << +picParams->m_phCuQpDeltaSubdivIntraSlice << std::endl;
        oss << "phCuChromaQpOffsetSubdivIntraSlice: " << +picParams->m_phCuChromaQpOffsetSubdivIntraSlice << std::endl;
        oss << "phLog2DiffMinQtMinCbInterSlice: " << +picParams->m_phLog2DiffMinQtMinCbInterSlice << std::endl;
        oss << "phMaxMtt_hierarchyDepthInterSlice: " << +picParams->m_phMaxMtt_hierarchyDepthInterSlice << std::endl;
        oss << "phLog2DiffMaxBtMinQtInterSlice: " << +picParams->m_phLog2DiffMaxBtMinQtInterSlice << std::endl;
        oss << "phLog2DiffMax_ttMinQtInterSlice: " << +picParams->m_phLog2DiffMax_ttMinQtInterSlice << std::endl;
        oss << "phCuQpDeltaSubdivInterSlice: " << +picParams->m_phCuQpDeltaSubdivInterSlice << std::endl;
        oss << "phCuChromaQpOffsetSubdivInterSlice: " << +picParams->m_phCuChromaQpOffsetSubdivInterSlice << std::endl;
        oss << "phCollocatedRefIdx: " << +picParams->m_phCollocatedRefIdx << std::endl;
        oss << "phQpDelta: " << +picParams->m_phQpDelta << std::endl;
        oss << "phLumaBetaOffsetDiv2: " << +picParams->m_phLumaBetaOffsetDiv2 << std::endl;
        oss << "phLumaTcOffsetDiv2: " << +picParams->m_phLumaTcOffsetDiv2 << std::endl;
        oss << "phCbBetaOffsetDiv2: " << +picParams->m_phCbBetaOffsetDiv2 << std::endl;
        oss << "phCbTcOffsetDiv2: " << +picParams->m_phCbTcOffsetDiv2 << std::endl;
        oss << "phCrBetaOffsetDiv2: " << +picParams->m_phCrBetaOffsetDiv2 << std::endl;
        oss << "phCrTcOffsetDiv2: " << +picParams->m_phCrTcOffsetDiv2 << std::endl;

        // weighted prediction info
        oss << "lumaLog2WeightDenom: " << +picParams->m_wpInfo.m_lumaLog2WeightDenom << std::endl;
        oss << "deltaChromaLog2WeightDenom: " << +picParams->m_wpInfo.m_deltaChromaLog2WeightDenom << std::endl;
        oss << "numL0Weights: " << +picParams->m_wpInfo.m_numL0Weights << std::endl;
        for (auto i = 0; i < 15; i++)
        {
            oss << "lumaWeightL0Flag[" << +i << "]: " << +picParams->m_wpInfo.m_lumaWeightL0Flag[i] << std::endl;
        }
        for (auto i = 0; i < 15; i++)
        {
            oss << "chromaWeightL0Flag[" << +i << "]: " << +picParams->m_wpInfo.m_chromaWeightL0Flag[i] << std::endl;
        }
        for (auto i = 0; i < 15; i++)
        {
            oss << "deltaLumaWeightL0[" << +i << "]: " << +picParams->m_wpInfo.m_deltaLumaWeightL0[i] << std::endl;
        }
        for (auto i = 0; i < 15; i++)
        {
            oss << "lumaOffsetL0[" << +i << "]: " << +picParams->m_wpInfo.m_lumaOffsetL0[i] << std::endl;
        }
        for (auto i = 0; i < 15; i++)
        {
            for (auto j = 0; j < 2; j++)
            {
                oss << "deltaChromaWeightL0[" << +i << "][" << +j << "]: " << +picParams->m_wpInfo.m_deltaChromaWeightL0[i][j] << std::endl;
            }
        }
        for (auto i = 0; i < 15; i++)
        {
            for (auto j = 0; j < 2; j++)
            {
                oss << "deltaChromaOffsetL0[" << +i << "][" << +j << "]: " << +picParams->m_wpInfo.m_deltaChromaOffsetL0[i][j] << std::endl;
            }
        }
        oss << "numL1Weights: " << +picParams->m_wpInfo.m_numL1Weights << std::endl;
        for (auto i = 0; i < 15; i++)
        {
            oss << "lumaWeightL1Flag[" << +i << "]: " << +picParams->m_wpInfo.m_lumaWeightL1Flag[i] << std::endl;
        }
        for (auto i = 0; i < 15; i++)
        {
            oss << "chromaWeightL1Flag[" << +i << "]: " << +picParams->m_wpInfo.m_chromaWeightL1Flag[i] << std::endl;
        }
        for (auto i = 0; i < 15; i++)
        {
            oss << "deltaLumaWeightL1[" << +i << "]: " << +picParams->m_wpInfo.m_deltaLumaWeightL1[i] << std::endl;
        }
        for (auto i = 0; i < 15; i++)
        {
            oss << "lumaOffsetL1[" << +i << "]: " << +picParams->m_wpInfo.m_lumaOffsetL1[i] << std::endl;
        }
        for (auto i = 0; i < 15; i++)
        {
            for (auto j = 0; j < 2; j++)
            {
                oss << "deltaChromaWeightL1[" << +i << "][" << +j << "]: " << +picParams->m_wpInfo.m_deltaChromaWeightL1[i][j] << std::endl;
            }
        }
        for (auto i = 0; i < 15; i++)
        {
            for (auto j = 0; j < 2; j++)
            {
                oss << "deltaChromaOffsetL1[" << +i << "][" << +j << "]: " << +picParams->m_wpInfo.m_deltaChromaOffsetL1[i][j] << std::endl;
            }
        }

        oss << "phFlags value: " << +picParams->m_phFlags.m_value << std::endl;
        oss << "phNonRefPicFlag: " << +picParams->m_phFlags.m_fields.m_phNonRefPicFlag << std::endl;
        oss << "phInterSliceAllowedFlag: " << +picParams->m_phFlags.m_fields.m_phInterSliceAllowedFlag << std::endl;
        oss << "phAlfEnabledFlag: " << +picParams->m_phFlags.m_fields.m_phAlfEnabledFlag << std::endl;
        oss << "phAlfCbEnabledFlag: " << +picParams->m_phFlags.m_fields.m_phAlfCbEnabledFlag << std::endl;
        oss << "phAlfCrEnabledFlag: " << +picParams->m_phFlags.m_fields.m_phAlfCrEnabledFlag << std::endl;
        oss << "phAlfCcCbEnabledFlag: " << +picParams->m_phFlags.m_fields.m_phAlfCcCbEnabledFlag << std::endl;
        oss << "phAlfCcCrEnabledFlag: " << +picParams->m_phFlags.m_fields.m_phAlfCcCrEnabledFlag << std::endl;
        oss << "phLmcsEnabledFlag: " << +picParams->m_phFlags.m_fields.m_phLmcsEnabledFlag << std::endl;
        oss << "phChromaResidualScaleFlag: " << +picParams->m_phFlags.m_fields.m_phChromaResidualScaleFlag << std::endl;
        oss << "phExplicitScalingListEnabledFlag: " << +picParams->m_phFlags.m_fields.m_phExplicitScalingListEnabledFlag << std::endl;
        oss << "phVirtualBoundariesPresentFlag: " << +picParams->m_phFlags.m_fields.m_phVirtualBoundariesPresentFlag << std::endl;
        oss << "phTemporalMvpEnabledFlag: " << +picParams->m_phFlags.m_fields.m_phTemporalMvpEnabledFlag << std::endl;
        oss << "numRefEntries0RplIdx0LargerThan0: " << +picParams->m_phFlags.m_fields.m_numRefEntries0RplIdx0LargerThan0 << std::endl;
        oss << "numRefEntries1RplIdx1LargerThan0: " << +picParams->m_phFlags.m_fields.m_numRefEntries1RplIdx1LargerThan0 << std::endl;
        oss << "phCollocatedFromL0Flag: " << +picParams->m_phFlags.m_fields.m_phCollocatedFromL0Flag << std::endl;
        oss << "phMmvdFullpelOnlyFlag: " << +picParams->m_phFlags.m_fields.m_phMmvdFullpelOnlyFlag << std::endl;
        oss << "phMvdL1ZeroFlag: " << +picParams->m_phFlags.m_fields.m_phMvdL1ZeroFlag << std::endl;
        oss << "phBdofDisabledFlag: " << +picParams->m_phFlags.m_fields.m_phBdofDisabledFlag << std::endl;
        oss << "phDmvrDisabledFlag: " << +picParams->m_phFlags.m_fields.m_phDmvrDisabledFlag << std::endl;
        oss << "phProfDisabledFlag: " << +picParams->m_phFlags.m_fields.m_phProfDisabledFlag << std::endl;
        oss << "phJointCbcrSignFlag: " << +picParams->m_phFlags.m_fields.m_phJointCbcrSignFlag << std::endl;
        oss << "phSaoLumaEnabledFlag: " << +picParams->m_phFlags.m_fields.m_phSaoLumaEnabledFlag << std::endl;
        oss << "phSaoChromaEnabledFlag: " << +picParams->m_phFlags.m_fields.m_phSaoChromaEnabledFlag << std::endl;
        oss << "phDeblockingFilterDisabledFlag: " << +picParams->m_phFlags.m_fields.m_phDeblockingFilterDisabledFlag << std::endl;

        // reference lists
        oss << "CurrPic FrameIdx: " << +picParams->m_currPic.FrameIdx << std::endl;
        oss << "picOrderCntVal: " << +picParams->m_picOrderCntVal << std::endl;
        for (auto i = 0; i < 15; i++)
        {
            oss << "refFramePocList[" << +i << "]: " << +picParams->m_refFramePocList[i] << std::endl;
        }
        for (auto i = 0; i < 15; i++)
        {
            oss << "refFrameList[" << +i << "] FrameIdx: " << +picParams->m_refFrameList[i].FrameIdx << std::endl;
        }
        for (auto i = 0; i < 2; i++)
        {
            for (auto j = 0; j < 15; j++)
            {
                oss << "refPicList[" << +i << "][" << +j << "] FrameIdx: " << +picParams->m_refPicList[i][j].FrameIdx << std::endl;
            }
        }

        oss << "picMiscFlags value: " << +picParams->m_picMiscFlags.m_value << std::endl;
        oss << "intraPicFlag: " << +picParams->m_picMiscFlags.m_fields.m_intraPicFlag << std::endl;

        oss << "statusReportFeedbackNumber: " << +picParams->m_statusReportFeedbackNumber << std::endl;

        // multilayer
        if (shortFormatInUse)
        {
            for (auto i = 0; i < 15; i++)
            {
                oss << "refFrameListNuhLayerId[" << +i << "]: " << +picParams->m_refFrameListNuhLayerId[i] << std::endl;
            }
            oss << "nuhLayerId: " << +picParams->m_nuhLayerId << std::endl;
            oss << "vpsMaxLayersMinus1: " << +picParams->m_vpsMaxLayersMinus1 << std::endl;
            for (auto i = 0; i < 56; i++)
            {
                oss << "vpsLayerId[" << +i << "]: " << +picParams->m_vpsLayerId[i] << std::endl;
            }
            for (auto i = 0; i < 56; i++)
            {
                for (auto j = 0; j < 7; j++)
                {
                    oss << "vpsDirectRefLayerFlag[" << +i << "][" << +j << "]: " << +picParams->m_vpsDirectRefLayerFlag[i][j] << std::endl;
                }
            }
        }

        std::ofstream ofs(fileName, std::ios::out);
        ofs << oss.str();
        ofs.close();
    }

    void DumpDecodeVvcAlfParams(CodecVvcAlfData *alfDataBuf, uint32_t numAlf, std::string fileName)
    {
        CodecVvcAlfData *alfData = nullptr;

        std::ostringstream oss;
        oss.setf(std::ios::showbase | std::ios::uppercase);

        for (uint16_t j = 0; j < numAlf; j++)
        {
            alfData = &alfDataBuf[j];
            oss << "====================================================================================================" << std::endl;
            oss << "Data for APS ALF number = " << +j << std::endl;
            oss << "apsAdaptationParameterSetId: " << +alfData->m_apsAdaptationParameterSetId << std::endl;
            oss << "alfLumaNumFiltersSignalledMinus1: " << +alfData->m_alfLumaNumFiltersSignalledMinus1 << std::endl;
            for (auto i = 0; i < 25; i++)
            {
                oss << "alfLumaCoeffDeltaIdx[" << +i << "]: " << +alfData->m_alfLumaCoeffDeltaIdx[i] << std::endl;
            }
            for (auto i = 0; i < 25; i++)
            {
                for (auto j = 0; j < 12; j++)
                {
                    oss << "alfCoeffL[" << +i << "][" << +j << "]: " << +alfData->m_alfCoeffL[i][j] << std::endl;
                }
            }
            for (auto i = 0; i < 25; i++)
            {
                for (auto j = 0; j < 12; j++)
                {
                    oss << "alfLumaClipIdx[" << +i << "][" << +j << "]: " << +alfData->m_alfLumaClipIdx[i][j] << std::endl;
                }
            }
            oss << "alfChromaNumAltFiltersMinus1: " << +alfData->m_alfChromaNumAltFiltersMinus1 << std::endl;
            for (auto i = 0; i < 8; i++)
            {
                for (auto j = 0; j < 6; j++)
                {
                    oss << "alfCoeffC[" << +i << "][" << +j << "]: " << +alfData->m_alfCoeffC[i][j] << std::endl;
                }
            }
            for (auto i = 0; i < 8; i++)
            {
                for (auto j = 0; j < 6; j++)
                {
                    oss << "alfChromaClipIdx[" << +i << "][" << +j << "]: " << +alfData->m_alfChromaClipIdx[i][j] << std::endl;
                }
            }
            oss << "alfCcCbFiltersSignalledMinus1: " << +alfData->m_alfCcCbFiltersSignalledMinus1 << std::endl;
            for (auto i = 0; i < 4; i++)
            {
                for (auto j = 0; j < 7; j++)
                {
                    oss << "CcAlfApsCoeffCb[" << +i << "][" << +j << "]: " << +alfData->m_ccAlfApsCoeffCb[i][j] << std::endl;
                }
            }
            oss << "alfCcCrFiltersSignalledMinus1: " << +alfData->m_alfCcCrFiltersSignalledMinus1 << std::endl;
            for (auto i = 0; i < 4; i++)
            {
                for (auto j = 0; j < 7; j++)
                {
                    oss << "CcAlfApsCoeffCr[" << +i << "][" << +j << "]: " << +alfData->m_ccAlfApsCoeffCr[i][j] << std::endl;
                }
            }

            oss << "alfFlags value: " << +alfData->m_alfFlags.m_value << std::endl;
            oss << "alfLumaFilterSignalFlag: " << +alfData->m_alfFlags.m_fields.m_alfLumaFilterSignalFlag << std::endl;
            oss << "alfChromaFilterSignalFlag: " << +alfData->m_alfFlags.m_fields.m_alfChromaFilterSignalFlag << std::endl;
            oss << "alfCcCbFilterSignalFlag: " << +alfData->m_alfFlags.m_fields.m_alfCcCbFilterSignalFlag << std::endl;
            oss << "alfCcCrFilterSignalFlag: " << +alfData->m_alfFlags.m_fields.m_alfCcCrFilterSignalFlag << std::endl;
            oss << "alfLumaClipFlag: " << +alfData->m_alfFlags.m_fields.m_alfLumaClipFlag << std::endl;
            oss << "alfChromaClipFlag: " << +alfData->m_alfFlags.m_fields.m_alfChromaClipFlag << std::endl;
        }

        std::ofstream ofs(fileName, std::ios::out);
        ofs << oss.str();
        ofs.close();
    }

    void DumpDecodeVvcLmcsParams(CodecVvcLmcsData *lmcsDataBuf, uint32_t numLmcs, std::string fileName)
    {
        CodecVvcLmcsData *lmcsData = nullptr;

        std::ostringstream oss;
        oss.setf(std::ios::showbase | std::ios::uppercase);

        for (uint16_t j = 0; j < numLmcs; j++)
        {
            lmcsData = &lmcsDataBuf[j];
            oss << "====================================================================================================" << std::endl;
            oss << "Data for APS LMCS number = " << +j << std::endl;
            oss << "apsAdaptationParameterSetId: " << +lmcsData->m_apsAdaptationParameterSetId << std::endl;
            oss << "lmcsMinBinIdx: " << +lmcsData->m_lmcsMinBinIdx << std::endl;
            oss << "lmcsDeltaMaxBinIdx: " << +lmcsData->m_lmcsDeltaMaxBinIdx << std::endl;
            for (auto i = 0; i < 16; i++)
            {
                oss << "lmcsDeltaCW[" << +i << "]: " << +lmcsData->m_lmcsDeltaCW[i] << std::endl;
            }
            oss << "lmcsDeltaCrs: " << +lmcsData->m_lmcsDeltaCrs << std::endl;
        }

        std::ofstream ofs(fileName, std::ios::out);
        ofs << oss.str();
        ofs.close();
    }

    void DumpDecodeVvcIqParams(CodecVvcQmData *quantMatrixBuf, uint32_t numScalingList, std::string fileName)
    {
        CodecVvcQmData *quantMatrix = nullptr;

        std::ostringstream oss;
        oss.setf(std::ios::showbase | std::ios::uppercase);

        for (uint16_t j = 0; j < numScalingList; j++)
        {
            quantMatrix = &quantMatrixBuf[j];
            oss << "====================================================================================================" << std::endl;
            oss << "Data for APS Scaling List number = " << +j << std::endl;
            oss << "apsAdaptationParameterSetId: " << +quantMatrix->m_apsAdaptationParameterSetId << std::endl;
            for (auto i = 0; i < 14; i++)
            {
                oss << "scalingMatrixDCRec[" << +i << "]: " << +quantMatrix->m_scalingMatrixDCRec[i] << std::endl;
            }
            for (auto i = 0; i < 2; i++)
            {
                for (auto j = 0; j < 2; j++)
                {
                    for (auto k = 0; k < 2; k++)
                    {
                        oss << "scalingMatrixRec2x2[" << +i << "][" << +j << "][" << +k << "]: " << +quantMatrix->m_scalingMatrixRec2x2[i][j][k] << std::endl;
                    }
                }
            }
            for (auto i = 0; i < 6; i++)
            {
                for (auto j = 0; j < 4; j++)
                {
                    for (auto k = 0; k < 4; k++)
                    {
                        oss << "scalingMatrixRec4x4[" << +i << "][" << +j << "][" << +k << "]: " << +quantMatrix->m_scalingMatrixRec4x4[i][j][k] << std::endl;
                    }
                }
            }
            for (auto i = 0; i < 20; i++)
            {
                for (auto j = 0; j < 8; j++)
                {
                    for (auto k = 0; k < 8; k++)
                    {
                        oss << "scalingMatrixRec8x8[" << +i << "][" << +j << "][" << +k << "]: " << +quantMatrix->m_scalingMatrixRec8x8[i][j][k] << std::endl;
                    }
                }
            }
        }

        std::ofstream ofs(fileName, std::ios::out);
        ofs << oss.str();
        ofs.close();
    }

    void DumpDecodeVvcTileParams(CodecVvcTileParam *tileParams, uint32_t numElements, std::string fileName)
    {
        CodecVvcTileParam *element = nullptr;

        std::ostringstream oss;
        oss.setf(std::ios::showbase | std::ios::uppercase);

        for (uint16_t j = 0; j < numElements; j++)
        {
            element = &tileParams[j];
            oss << "====================================================================================================" << std::endl;
            oss << "Data for Tile Params element = " << +j << std::endl;
            oss << "tileDimension: " << +element->m_tileDimension << std::endl;
        }

        std::ofstream ofs(fileName, std::ios::out);
        ofs << oss.str();
        ofs.close();
    }

    void DumpDecodeVvcSubpicParams(CodecVvcSubpicParam *subpicParamsBuf, uint32_t numSubpics, std::string fileName)
    {
        CodecVvcSubpicParam *subpicParams = nullptr;

        std::ostringstream oss;
        oss.setf(std::ios::showbase | std::ios::uppercase);

        for (uint16_t j = 0; j < numSubpics; j++)
        {
            subpicParams = &subpicParamsBuf[j];
            oss << "====================================================================================================" << std::endl;
            oss << "Data for SubPic Params number = " << +j << std::endl;
            oss << "spsSubpicCtuTopLeftX: " << +subpicParams->m_spsSubpicCtuTopLeftX << std::endl;
            oss << "spsSubpicCtuTopLeftY: " << +subpicParams->m_spsSubpicCtuTopLeftY << std::endl;
            oss << "spsSubpicWidthMinus1: " << +subpicParams->m_spsSubpicWidthMinus1 << std::endl;
            oss << "spsSubpicHeightMinus1: " << +subpicParams->m_spsSubpicHeightMinus1 << std::endl;
            oss << "subpicIdVal: " << +subpicParams->m_subpicIdVal << std::endl;

            oss << "subPicFlags value: " << +subpicParams->m_subPicFlags.m_value << std::endl;
            oss << "spsSubpicTreatedAsPicFlag: " << +subpicParams->m_subPicFlags.m_fields.m_spsSubpicTreatedAsPicFlag << std::endl;
            oss << "spsLoopFilterAcrossSubpicEnabledFlag: " << +subpicParams->m_subPicFlags.m_fields.m_spsLoopFilterAcrossSubpicEnabledFlag << std::endl;

            //Additional params, miss m_sliceIdx pointer
            oss << "endCtbX: " << +subpicParams->m_endCtbX << std::endl;
            oss << "endCtbY: " << +subpicParams->m_endCtbY << std::endl;
            oss << "numSlices: " << +subpicParams->m_numSlices << std::endl;
        }

        std::ofstream ofs(fileName, std::ios::out);
        ofs << oss.str();
        ofs.close();
    }

    void DumpDecodeVvcSliceStructureParams(CodecVvcSliceStructure *sliceStructParamsBuf, uint32_t numSlcStruct, std::string fileName)
    {
        CodecVvcSliceStructure *sliceStructParams = nullptr;

        std::ostringstream oss;
        oss.setf(std::ios::showbase | std::ios::uppercase);

        for (uint16_t j = 0; j < numSlcStruct; j++)
        {
            sliceStructParams = &sliceStructParamsBuf[j];
            oss << "====================================================================================================" << std::endl;
            oss << "Data for Slice Struct Params number = " << +j << std::endl;
            oss << "sliceTopLeftTileIdx: " << +sliceStructParams->m_sliceTopLeftTileIdx << std::endl;
            oss << "ppsSliceWidthInTilesMinus1: " << +sliceStructParams->m_ppsSliceWidthInTilesMinus1 << std::endl;
            oss << "ppsSliceHeightInTilesMinus1: " << +sliceStructParams->m_ppsSliceHeightInTilesMinus1 << std::endl;
            oss << "ppsExpSliceHeightInCtusMinus1: " << +sliceStructParams->m_ppsExpSliceHeightInCtusMinus1 << std::endl;
        }

        std::ofstream ofs(fileName, std::ios::out);
        ofs << oss.str();
        ofs.close();
    }

    void DumpDecodeVvcRplStructureParams(CodecVvcRplStructure *rplParams, uint32_t numRpls, std::string fileName)
    {
        CodecVvcRplStructure *rplStructParams = nullptr;

        std::ostringstream oss;
        oss.setf(std::ios::showbase | std::ios::uppercase);

        for (uint16_t j = 0; j < numRpls; j++)
        {
            rplStructParams = &rplParams[j];
            oss << "====================================================================================================" << std::endl;
            oss << "Data for RPL Params number = " << +j << std::endl;
            oss << "listIdx: " << +rplStructParams->m_listIdx << std::endl;
            oss << "rplsIdx: " << +rplStructParams->m_rplsIdx << std::endl;
            oss << "numRefEntries: " << +rplStructParams->m_numRefEntries << std::endl;
            oss << "ltrpInHeaderFlag: " << +rplStructParams->m_ltrpInHeaderFlag << std::endl;
            for (auto i = 0; i < 29; i++)
            {
                oss << "stRefPicFlag[" << +i << "]: " << +rplStructParams->m_stRefPicFlag[i] << std::endl;
            }
            for (auto i = 0; i < 29; i++)
            {
                oss << "DeltaPocSt[" << +i << "]: " << +rplStructParams->m_deltaPocSt[i] << std::endl;
            }
            for (auto i = 0; i < 29; i++)
            {
                oss << "rplsPocLsbLt[" << +i << "]: " << +rplStructParams->m_rplsPocLsbLt[i] << std::endl;
            }
            for (auto i = 0; i < 29; i++)
            {
                oss << "interLayerRefPicFlag[" << +i << "]: " << +rplStructParams->m_interLayerRefPicFlag[i] << std::endl;
            }
            for (auto i = 0; i < 29; i++)
            {
                oss << "ilrpIdx[" << +i << "]: " << +rplStructParams->m_ilrpIdx[i] << std::endl;
            }
        }

        std::ofstream ofs(fileName, std::ios::out);
        ofs << oss.str();
        ofs.close();
    }

    void DumpDecodeVvcSliceParams(CodecVvcSliceParams *sliceParams, uint32_t numSlices, std::string fileName, bool shortFormatInUse)
    {
        CodecVvcSliceParams *vvcSliceControl = nullptr;

        std::ostringstream oss;
        oss.setf(std::ios::showbase | std::ios::uppercase);

        for (uint16_t j = 0; j < numSlices; j++)
        {
            vvcSliceControl = &sliceParams[j];
            oss << "====================================================================================================" << std::endl;
            oss << "Data for Slice number = " << +j << std::endl;
            oss << "bSNALunitDataLocation: " << +vvcSliceControl->m_bSNALunitDataLocation << std::endl;
            oss << "sliceBytesInBuffer: " << +vvcSliceControl->m_sliceBytesInBuffer << std::endl;
            oss << "wBadSliceChopping: " << +vvcSliceControl->m_wBadSliceChopping << std::endl;

            if (!shortFormatInUse)
            {
                oss << "shSubpicId: " << +vvcSliceControl->m_shSubpicId << std::endl;
                oss << "shSliceAddress: " << +vvcSliceControl->m_shSliceAddress << std::endl;
                oss << "shNumTilesInSliceMinus1: " << +vvcSliceControl->m_shNumTilesInSliceMinus1 << std::endl;
                oss << "shSliceType: " << +vvcSliceControl->m_shSliceType << std::endl;
                oss << "shNumAlfApsIdsLuma: " << +vvcSliceControl->m_shNumAlfApsIdsLuma << std::endl;
                for (auto i = 0; i < 7; i++)
                {
                    oss << "shAlfApsIdLuma[" << +i << "]: " << +vvcSliceControl->m_shAlfApsIdLuma[i] << std::endl;
                }
                oss << "shAlfApsIdChroma: " << +vvcSliceControl->m_shAlfApsIdChroma << std::endl;
                oss << "shAlfCcCbApsId: " << +vvcSliceControl->m_shAlfCcCbApsId << std::endl;
                oss << "shAlfCcCrApsId: " << +vvcSliceControl->m_shAlfCcCrApsId << std::endl;
                for (auto i = 0; i < 2; i++)
                {
                    oss << "numRefIdxActive[" << +i << "]: " << +vvcSliceControl->m_numRefIdxActive[i] << std::endl;
                }
                oss << "shCollocatedRefIdx: " << +vvcSliceControl->m_shCollocatedRefIdx << std::endl;
                oss << "sliceQpY: " << +vvcSliceControl->m_sliceQpY << std::endl;
                oss << "shCbQpOffset: " << +vvcSliceControl->m_shCbQpOffset << std::endl;
                oss << "shCrQpOffset: " << +vvcSliceControl->m_shCrQpOffset << std::endl;
                oss << "shJointCbcrQpOffset: " << +vvcSliceControl->m_shJointCbcrQpOffset << std::endl;
                oss << "shLumaBetaOffsetDiv2: " << +vvcSliceControl->m_shLumaBetaOffsetDiv2 << std::endl;
                oss << "shLumaTcOffsetDiv2: " << +vvcSliceControl->m_shLumaTcOffsetDiv2 << std::endl;
                oss << "shCbBetaOffsetDiv2: " << +vvcSliceControl->m_shCbBetaOffsetDiv2 << std::endl;
                oss << "shCbTcOffsetDiv2: " << +vvcSliceControl->m_shCbTcOffsetDiv2 << std::endl;
                oss << "shCrBetaOffsetDiv2: " << +vvcSliceControl->m_shCrBetaOffsetDiv2 << std::endl;
                oss << "shCrTcOffsetDiv2: " << +vvcSliceControl->m_shCrTcOffsetDiv2 << std::endl;

                oss << "byteOffsetToSliceData: " << +vvcSliceControl->m_byteOffsetToSliceData << std::endl;
                oss << "numEntryPoints: " << +vvcSliceControl->m_numEntryPoints << std::endl;

                for (auto i = 0; i < 2; i++)
                {
                    for (auto j = 0; j < 15; j++)
                    {
                        oss << "refPicList[" << +i << "][" << +j << "] FrameIdx: " << +vvcSliceControl->m_refPicList[i][j].FrameIdx << std::endl;
                    }
                }
                oss << "lumaLog2WeightDenom: " << +vvcSliceControl->m_wpInfo.m_lumaLog2WeightDenom << std::endl;
                oss << "deltaChromaLog2WeightDenom: " << +vvcSliceControl->m_wpInfo.m_deltaChromaLog2WeightDenom << std::endl;
                oss << "numL0Weights: " << +vvcSliceControl->m_wpInfo.m_numL0Weights << std::endl;
                for (auto i = 0; i < 15; i++)
                {
                    oss << "lumaWeightL0Flag[" << +i << "]: " << +vvcSliceControl->m_wpInfo.m_lumaWeightL0Flag[i] << std::endl;
                }
                for (auto i = 0; i < 15; i++)
                {
                    oss << "chromaWeightL0Flag[" << +i << "]: " << +vvcSliceControl->m_wpInfo.m_chromaWeightL0Flag[i] << std::endl;
                }
                for (auto i = 0; i < 15; i++)
                {
                    oss << "deltaLumaWeightL0[" << +i << "]: " << +vvcSliceControl->m_wpInfo.m_deltaLumaWeightL0[i] << std::endl;
                }
                for (auto i = 0; i < 15; i++)
                {
                    oss << "lumaOffsetL0[" << +i << "]: " << +vvcSliceControl->m_wpInfo.m_lumaOffsetL0[i] << std::endl;
                }
                for (auto i = 0; i < 15; i++)
                {
                    for (auto j = 0; j < 2; j++)
                    {
                        oss << "deltaChromaWeightL0[" << +i << "][" << +j << "]: " << +vvcSliceControl->m_wpInfo.m_deltaChromaWeightL0[i][j] << std::endl;
                    }
                }
                for (auto i = 0; i < 15; i++)
                {
                    for (auto j = 0; j < 2; j++)
                    {
                        oss << "deltaChromaOffsetL0[" << +i << "][" << +j << "]: " << +vvcSliceControl->m_wpInfo.m_deltaChromaOffsetL0[i][j] << std::endl;
                    }
                }
                oss << "numL1Weights: " << +vvcSliceControl->m_wpInfo.m_numL1Weights << std::endl;
                for (auto i = 0; i < 15; i++)
                {
                    oss << "lumaWeightL1Flag[" << +i << "]: " << +vvcSliceControl->m_wpInfo.m_lumaWeightL1Flag[i] << std::endl;
                }
                for (auto i = 0; i < 15; i++)
                {
                    oss << "chromaWeightL1Flag[" << +i << "]: " << +vvcSliceControl->m_wpInfo.m_chromaWeightL1Flag[i] << std::endl;
                }
                for (auto i = 0; i < 15; i++)
                {
                    oss << "deltaLumaWeightL1[" << +i << "]: " << +vvcSliceControl->m_wpInfo.m_deltaLumaWeightL1[i] << std::endl;
                }
                for (auto i = 0; i < 15; i++)
                {
                    oss << "lumaOffsetL1[" << +i << "]: " << +vvcSliceControl->m_wpInfo.m_lumaOffsetL1[i] << std::endl;
                }
                for (auto i = 0; i < 15; i++)
                {
                    for (auto j = 0; j < 2; j++)
                    {
                        oss << "deltaChromaWeightL1[" << +i << "][" << +j << "]: " << +vvcSliceControl->m_wpInfo.m_deltaChromaWeightL1[i][j] << std::endl;
                    }
                }
                for (auto i = 0; i < 15; i++)
                {
                    for (auto j = 0; j < 2; j++)
                    {
                        oss << "deltaChromaOffsetL1[" << +i << "][" << +j << "]: " << +vvcSliceControl->m_wpInfo.m_deltaChromaOffsetL1[i][j] << std::endl;
                    }
                }

                oss << "longSliceFlags value: " << +vvcSliceControl->m_longSliceFlags.m_value << std::endl;
                oss << "shAlfEnabledFlag: " << +vvcSliceControl->m_longSliceFlags.m_fields.m_shAlfEnabledFlag << std::endl;
                oss << "shAlfCbEnabledFlag: " << +vvcSliceControl->m_longSliceFlags.m_fields.m_shAlfCbEnabledFlag << std::endl;
                oss << "shAlfCrEnabledFlag: " << +vvcSliceControl->m_longSliceFlags.m_fields.m_shAlfCrEnabledFlag << std::endl;
                oss << "shAlfCcCbEnabledFlag: " << +vvcSliceControl->m_longSliceFlags.m_fields.m_shAlfCcCbEnabledFlag << std::endl;
                oss << "shAlfCcCrEnabledFlag: " << +vvcSliceControl->m_longSliceFlags.m_fields.m_shAlfCcCrEnabledFlag << std::endl;
                oss << "shLmcsUsedFlag: " << +vvcSliceControl->m_longSliceFlags.m_fields.m_shLmcsUsedFlag << std::endl;
                oss << "shExplicitScalingListUsedFlag: " << +vvcSliceControl->m_longSliceFlags.m_fields.m_shExplicitScalingListUsedFlag << std::endl;
                oss << "shCabacInitFlag: " << +vvcSliceControl->m_longSliceFlags.m_fields.m_shCabacInitFlag << std::endl;
                oss << "shCollocatedFromL0Flag: " << +vvcSliceControl->m_longSliceFlags.m_fields.m_shCollocatedFromL0Flag << std::endl;
                oss << "shCuChromaQpOffsetEnabledFlag: " << +vvcSliceControl->m_longSliceFlags.m_fields.m_shCuChromaQpOffsetEnabledFlag << std::endl;
                oss << "shSaoLumaUsedFlag: " << +vvcSliceControl->m_longSliceFlags.m_fields.m_shSaoLumaUsedFlag << std::endl;
                oss << "shSaoChromaUsedFlag: " << +vvcSliceControl->m_longSliceFlags.m_fields.m_shSaoChromaUsedFlag << std::endl;
                oss << "shDeblockingFilterDisabledFlag: " << +vvcSliceControl->m_longSliceFlags.m_fields.m_shDeblockingFilterDisabledFlag << std::endl;
                oss << "shDepQuantUsedFlag: " << +vvcSliceControl->m_longSliceFlags.m_fields.m_shDepQuantUsedFlag << std::endl;
                oss << "shSignDataHidingUsedFlag: " << +vvcSliceControl->m_longSliceFlags.m_fields.m_shSignDataHidingUsedFlag << std::endl;
                oss << "shTsResidualCodingDisabledFlag: " << +vvcSliceControl->m_longSliceFlags.m_fields.m_shTsResidualCodingDisabledFlag << std::endl;
                oss << "lastSliceOfPic: " << +vvcSliceControl->m_longSliceFlags.m_fields.m_lastSliceOfPic << std::endl;
                oss << "noBackwardPredFlag: " << +vvcSliceControl->m_longSliceFlags.m_fields.m_noBackwardPredFlag << std::endl;
            }
        }

        std::ofstream ofs(fileName, std::ios::out);
        ofs << oss.str();
        ofs.close();
    }
#endif

    MOS_STATUS VvcPipeline::CreateFeatureManager()
    {
        DECODE_FUNC_CALL();

        m_featureManager = MOS_New(DecodeVvcFeatureManager, m_allocator, dynamic_cast<CodechalHwInterfaceNext *>(m_hwInterface), m_osInterface);
        DECODE_CHK_NULL(m_featureManager);

        return MOS_STATUS_SUCCESS;
    }

}
