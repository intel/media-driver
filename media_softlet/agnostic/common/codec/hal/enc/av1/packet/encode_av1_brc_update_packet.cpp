/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     encode_av1_brc_update_packet.cpp
//! \brief    Defines the implementation of av1 brc update packet
//!

#include "encode_av1_brc_update_packet.h"
#include "codechal_debug.h"
#include "encode_av1_brc.h"
#include "encode_av1_vdenc_packet.h"
#include "encode_av1_vdenc_lpla_enc.h"
#if _MEDIA_RESERVED
#include "encode_av1_scc.h"
#endif

namespace encode
{
    MOS_STATUS Av1BrcUpdatePkt::Init()
    {
        ENCODE_FUNC_CALL();
        HUC_CHK_STATUS_RETURN(EncodeHucPkt::Init());

        ENCODE_CHK_NULL_RETURN(m_pipeline);
        m_allocator = m_pipeline->GetEncodeAllocator();
        ENCODE_CHK_NULL_RETURN(m_allocator);

        ENCODE_CHK_NULL_RETURN(m_featureManager);
        m_basicFeature = dynamic_cast<Av1BasicFeature *>(m_featureManager->GetFeature(Av1FeatureIDs::basicFeature));
        ENCODE_CHK_NULL_RETURN(m_basicFeature);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BrcUpdatePkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
    {
        ENCODE_FUNC_CALL();

        auto osInterface = m_hwInterface->GetOsInterface();
        ENCODE_CHK_NULL_RETURN(osInterface);

        uint32_t hucCommandsSize = 0;
        uint32_t hucPatchListSize = 0;
        MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;

        ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucStateCommandSize(
            m_basicFeature->m_mode, (uint32_t*)&hucCommandsSize, (uint32_t*)&hucPatchListSize, &stateCmdSizeParams));

        commandBufferSize = hucCommandsSize;
        requestedPatchListSize = osInterface->bUsesPatchList ? hucPatchListSize : 0;

        if (m_pipeline->IsSingleTaskPhaseSupported())
        {
            commandBufferSize *= m_pipeline->GetPassNum();
        }

        // 4K align since allocation is in chunks of 4K bytes.
        commandBufferSize = MOS_ALIGN_CEIL(commandBufferSize, CODECHAL_PAGE_SIZE);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BrcUpdatePkt::AllocateResources()
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_STATUS_RETURN(EncodeHucPkt::AllocateResources());

        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;

        MOS_RESOURCE *allocatedbuffer       = nullptr;

        for (auto k = 0; k < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; k++)
        {
            // Const Data buffer
            allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(m_vdencBrcConstDataBufferSize, CODECHAL_PAGE_SIZE);
            allocParamsForBufferLinear.pBufName = "VDENC BRC Const Data Buffer";
            allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE;
            allocatedbuffer       = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
            ENCODE_CHK_NULL_RETURN(allocatedbuffer);
            m_vdencBrcConstDataBuffer[k] = *allocatedbuffer;

            // Pak insert buffer (input for HuC FW)
            allocParamsForBufferLinear.dwBytes  = CODECHAL_PAGE_SIZE;
            allocParamsForBufferLinear.pBufName = "VDENC Read Batch Buffer";
            allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE;
            allocatedbuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
            ENCODE_CHK_NULL_RETURN(allocatedbuffer);
            m_vdencPakInsertBatchBuffer[k] = *allocatedbuffer;

            for (auto i = 0; i < VDENC_BRC_NUM_OF_PASSES; i++)
            {
                // VDEnc read batch buffer (input for HuC FW)
                allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(m_hwInterface->m_vdencReadBatchBufferSize, CODECHAL_PAGE_SIZE);
                allocParamsForBufferLinear.pBufName = "VDENC Read Origin Batch Buffer";
                allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE;
                allocatedbuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
                ENCODE_CHK_NULL_RETURN(allocatedbuffer);
                m_vdencReadBatchBufferOrigin[k][i] = *allocatedbuffer;

                // VDEnc read batch buffer (input for HuC FW)
                allocParamsForBufferLinear.dwBytes      = MOS_ALIGN_CEIL(m_hwInterface->m_vdencReadBatchBufferSize, CODECHAL_PAGE_SIZE);
                allocParamsForBufferLinear.pBufName     = "VDENC Read TU7 Batch Buffer";
                allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE;
                allocatedbuffer                         = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
                ENCODE_CHK_NULL_RETURN(allocatedbuffer);
                m_vdencReadBatchBufferTU7[k][i] = *allocatedbuffer;

                // BRC update DMEM
                allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(m_vdencBrcUpdateDmemBufferSize, CODECHAL_CACHELINE_SIZE);
                allocParamsForBufferLinear.pBufName = "VDENC BrcUpdate DmemBuffer";
                allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE;
                allocatedbuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
                ENCODE_CHK_NULL_RETURN(allocatedbuffer);
                m_vdencBrcUpdateDmemBuffer[k][i] = *allocatedbuffer;
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BrcUpdatePkt::SetConstDataHuCBrcUpdate()
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        auto hucConstData = (VdencAv1HucBrcConstantData *)m_allocator->LockResourceForWrite(&m_vdencBrcConstDataBuffer[m_pipeline->m_currRecycledBufIdx]);
        ENCODE_CHK_NULL_RETURN(hucConstData);

        RUN_FEATURE_INTERFACE_RETURN(Av1Brc, Av1FeatureIDs::av1BrcFeature, SetConstForUpdate, hucConstData);

        m_allocator->UnLock(&m_vdencBrcConstDataBuffer[m_pipeline->m_currRecycledBufIdx]);

        return eStatus;
    }

    MOS_STATUS Av1BrcUpdatePkt::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_recycleBuf);
        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_av1PicParams);
        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_av1SeqParams);

        ENCODE_CHK_STATUS_RETURN(ConstructBatchBufferHuCBRC(&m_vdencReadBatchBufferOrigin[m_pipeline->m_currRecycledBufIdx][m_pipeline->GetCurrentPass()]));
    
        if (m_basicFeature->m_av1PicParams->AdaptiveTUEnabled != 0)
        {
            auto original_TU              = m_basicFeature->m_targetUsage;
            m_basicFeature->m_targetUsage = m_basicFeature->m_av1SeqParams->TargetUsage = 7;
            ENCODE_CHK_STATUS_RETURN(ConstructBatchBufferHuCBRC(&m_vdencReadBatchBufferTU7[m_pipeline->m_currRecycledBufIdx][m_pipeline->GetCurrentPass()]));
            m_basicFeature->m_targetUsage = m_basicFeature->m_av1SeqParams->TargetUsage = original_TU;
        }        

        ENCODE_CHK_STATUS_RETURN(ConstructPakInsertHucBRC(&m_vdencPakInsertBatchBuffer[m_pipeline->m_currRecycledBufIdx]));

        bool firstTaskInPhase = packetPhase & firstPacket;
        bool requestProlog = false;

        auto brcFeature = dynamic_cast<Av1Brc *>(m_featureManager->GetFeature(Av1FeatureIDs::av1BrcFeature));
        ENCODE_CHK_NULL_RETURN(brcFeature);

        uint16_t perfTag = m_pipeline->IsFirstPass() ? CODECHAL_ENCODE_PERFTAG_CALL_BRC_UPDATE : CODECHAL_ENCODE_PERFTAG_CALL_BRC_UPDATE_SECOND_PASS;
        uint16_t pictureType = (m_basicFeature->m_pictureCodingType == I_TYPE) ? 0 : (m_basicFeature->m_ref.IsLowDelay() ? (m_basicFeature->m_ref.IsPFrame() ? 1 : 3) : 2);
        SetPerfTag(perfTag, (uint16_t)m_basicFeature->m_mode, pictureType);

        if (!m_pipeline->IsSingleTaskPhaseSupported() || firstTaskInPhase)
        {
            // Send command buffer header at the beginning (OS dependent)
            requestProlog = true;
        }

        ENCODE_CHK_STATUS_RETURN(Execute(commandBuffer, true, requestProlog, BRC_UPDATE));

        if (!IsHuCStsUpdNeeded())
        {
            // Write HUC_STATUS mask: DW1 (mask value)
            auto &storeDataParams            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
            storeDataParams                  = {};
            storeDataParams.pOsResource      = m_basicFeature->m_recycleBuf->GetBuffer(VdencBrcPakMmioBuffer, 0);
            storeDataParams.dwResourceOffset = sizeof(uint32_t);
            storeDataParams.dwValue          = AV1_BRC_HUC_STATUS_REENCODE_MASK;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(commandBuffer));

            // store HUC_STATUS register: DW0 (actual value)
            ENCODE_CHK_COND_RETURN((m_vdboxIndex > MHW_VDBOX_NODE_1), "ERROR - vdbox index exceed the maximum");
            auto mmioRegisters              = m_hucItf->GetMmioRegisters(m_vdboxIndex);
            auto &storeRegParams            = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
            storeDataParams                 = {};
            storeRegParams.presStoreBuffer  = m_basicFeature->m_recycleBuf->GetBuffer(VdencBrcPakMmioBuffer, 0);
            storeRegParams.dwOffset         = 0;
            storeRegParams.dwRegister       = mmioRegisters->hucStatusRegOffset;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(commandBuffer));
        }

        return MOS_STATUS_SUCCESS;
    }

    inline MOS_STATUS AddBBEnd(std::shared_ptr<mhw::mi::Itf> m_miItf, MOS_COMMAND_BUFFER& cmdBuffer)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(m_miItf);
        
        MHW_BATCH_BUFFER TempBatchBuffer = {};
        TempBatchBuffer.iSize = CODECHAL_PAGE_SIZE;
        TempBatchBuffer.pData = (uint8_t *)cmdBuffer.pCmdBase;
        TempBatchBuffer.iCurrent = cmdBuffer.iOffset;
        TempBatchBuffer.iRemaining = cmdBuffer.iRemaining;

        ENCODE_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(nullptr, &TempBatchBuffer));
            
        cmdBuffer.pCmdPtr += (TempBatchBuffer.iCurrent - cmdBuffer.iOffset) / 4;
        cmdBuffer.iOffset = TempBatchBuffer.iCurrent;
        cmdBuffer.iRemaining = TempBatchBuffer.iRemaining;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BrcUpdatePkt::ConstructBatchBufferHuCBRC(PMOS_RESOURCE batchBuffer)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(batchBuffer);

        auto batchbufferAddr = (uint8_t *)m_allocator->LockResourceForWrite(batchBuffer);
        ENCODE_CHK_NULL_RETURN(batchbufferAddr);
        MOS_COMMAND_BUFFER constructedCmdBuf;
        MOS_ZeroMemory(&constructedCmdBuf, sizeof(constructedCmdBuf));
        constructedCmdBuf.pCmdBase = constructedCmdBuf.pCmdPtr = (uint32_t *)batchbufferAddr;
        constructedCmdBuf.iRemaining = MOS_ALIGN_CEIL(m_hwInterface->m_vdencReadBatchBufferSize, CODECHAL_PAGE_SIZE);

        /*--------------SLBB layout--------------------*/
        /*----Group1----*/
        //AVP_SEGMENT_STATE
        //AVP_INLOOP_FILTER_STATE
        //BB_END
        //
        /*----Group2----*/
        //VDENC_CMD1
        //BB_END
        //
        /*----Group3----*/
        //VDENC_CMD2
        //BB_END
        //
        /*----Group4----*/
        //AVP_PIC_STATE
        //BB_END
        //AVP_PIC_STATE (if multi tile)
        //BB_END (if multi tile)
        //
        /*--------------End of SLBB layout--------------------*/

        SlbData slbData;
        /*----Group1----*/
        slbData.avpSegmentStateOffset = (uint16_t)constructedCmdBuf.iOffset;
        ENCODE_CHK_STATUS_RETURN(AddAllCmds_AVP_SEGMENT_STATE(&constructedCmdBuf));
        slbData.avpInloopFilterStateOffset = (uint16_t)constructedCmdBuf.iOffset;
        SETPAR_AND_ADDCMD(AVP_INLOOP_FILTER_STATE, m_avpItf, &constructedCmdBuf);
        ENCODE_CHK_STATUS_RETURN(AddBBEnd(m_miItf, constructedCmdBuf));
        /*----Group2----*/
        slbData.vdencCmd1Offset = (uint16_t)constructedCmdBuf.iOffset;
        SETPAR_AND_ADDCMD(VDENC_CMD1, m_vdencItf, &constructedCmdBuf);
        ENCODE_CHK_STATUS_RETURN(AddBBEnd(m_miItf, constructedCmdBuf));
        /*----Group3----*/
        slbData.vdencCmd2Offset = (uint16_t)constructedCmdBuf.iOffset;
        SETPAR_AND_ADDCMD(VDENC_CMD2, m_vdencItf, &constructedCmdBuf);
        ENCODE_CHK_STATUS_RETURN(AddBBEnd(m_miItf, constructedCmdBuf));
        /*----Group4----*/
        slbData.avpPicStateOffset = (uint16_t)constructedCmdBuf.iOffset;
        ENCODE_CHK_STATUS_RETURN(AddAvpPicStateBaseOnTile(constructedCmdBuf, slbData));

        /*----Group5----*/
        if (m_basicFeature->m_av1PicParams->PicFlags.fields.PaletteModeEnable)
        {
            slbData.vdencTileSliceStateOffset = (uint16_t)constructedCmdBuf.iOffset;
            ENCODE_CHK_STATUS_RETURN(AddVdencTileSliceBaseOnTile(constructedCmdBuf, slbData));
        }

        slbData.slbSize = (uint16_t)constructedCmdBuf.iOffset - slbData.avpSegmentStateOffset;

        RUN_FEATURE_INTERFACE_NO_RETURN(Av1Brc, Av1FeatureIDs::av1BrcFeature, SetSLBData, slbData);

        m_allocator->UnLock(batchBuffer);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BrcUpdatePkt::ConstructPakInsertHucBRC(PMOS_RESOURCE batchBuffer)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(batchBuffer);

        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, SetCurrentTile, 0, 0, m_pipeline);

        auto batchbufferAddr = (uint8_t *)m_allocator->LockResourceForWrite(batchBuffer);
        ENCODE_CHK_NULL_RETURN(batchbufferAddr);

        MOS_COMMAND_BUFFER            constructedCmdBuf;
        MOS_ZeroMemory(&constructedCmdBuf, sizeof(constructedCmdBuf));
        constructedCmdBuf.pCmdBase = constructedCmdBuf.pCmdPtr = (uint32_t *)batchbufferAddr;
        constructedCmdBuf.iRemaining                           = MOS_ALIGN_CEIL(m_hwInterface->m_vdencReadBatchBufferSize, CODECHAL_PAGE_SIZE);

        auto brcFeature = dynamic_cast<Av1Brc *>(m_featureManager->GetFeature(Av1FeatureIDs::av1BrcFeature));
        ENCODE_CHK_NULL_RETURN(brcFeature);
        auto slbData = brcFeature->GetSLBData();

        ENCODE_CHK_STATUS_RETURN(AddAllCmds_AVP_PAK_INSERT_OBJECT(&constructedCmdBuf));
        ENCODE_CHK_STATUS_RETURN(AddBBEnd(m_miItf, constructedCmdBuf));

        slbData.pakInsertSlbSize = (uint16_t)constructedCmdBuf.iOffset;
        RUN_FEATURE_INTERFACE_NO_RETURN(Av1Brc, Av1FeatureIDs::av1BrcFeature, SetSLBData, slbData);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BrcUpdatePkt::AddAvpPicStateBaseOnTile(MOS_COMMAND_BUFFER& cmdBuffer, SlbData &slbData)
    {
        ENCODE_FUNC_CALL();

        uint16_t numTileColumns = 1;
        uint16_t numTileRows = 1;
        bool firstTileInGroup = false;
        uint32_t tileGroupIdx = 0;
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileRowColumns, numTileRows, numTileColumns);
        slbData.tileNum = numTileRows * numTileColumns;

        for (uint32_t tileRow = 0; tileRow < numTileRows; tileRow++)
        {
            for (uint32_t tileCol = 0; tileCol < numTileColumns; tileCol++)
            {
                RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, SetCurrentTile, tileRow, tileCol, m_pipeline);
                RUN_FEATURE_INTERFACE_NO_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, IsFirstTileInGroup, firstTileInGroup, tileGroupIdx);
#if _MEDIA_RESERVED
                RUN_FEATURE_INTERFACE_RETURN(Av1Scc, Av1FeatureIDs::av1Scc, UpdateIBCStatusForCurrentTile);
#endif
                if (firstTileInGroup)
                {
                    SETPAR_AND_ADDCMD(AVP_PIC_STATE, m_avpItf, &cmdBuffer);
                    ENCODE_CHK_STATUS_RETURN(AddBBEnd(m_miItf, cmdBuffer));
                }
                else
                {
                    slbData.avpPicStateCmdNum = 2;
                    slbData.secondAvpPicStateOffset = (uint16_t)cmdBuffer.iOffset;
                    SETPAR_AND_ADDCMD(AVP_PIC_STATE, m_avpItf, &cmdBuffer);
                    ENCODE_CHK_STATUS_RETURN(AddBBEnd(m_miItf, cmdBuffer));
                    break;
                }
            }

            if (slbData.avpPicStateCmdNum == 2)
            {
                break;
            }
        }

        // Add MI_NOOPs to align with CODECHAL_CACHELINE_SIZE
        uint32_t size = (MOS_ALIGN_CEIL(cmdBuffer.iOffset, CODECHAL_CACHELINE_SIZE) - cmdBuffer.iOffset) / sizeof(uint32_t);
        for (uint32_t i = 0; i < size; i++)
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_NOOP)(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BrcUpdatePkt::AddVdencTileSliceBaseOnTile(MOS_COMMAND_BUFFER& cmdBuffer, SlbData& slbData)
    {
        ENCODE_FUNC_CALL();

        uint16_t numTileColumns = 1;
        uint16_t numTileRows = 1;
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileRowColumns, numTileRows, numTileColumns);

        auto& par = m_vdencItf->MHW_GETPAR_F(VDENC_HEVC_VP9_TILE_SLICE_STATE)();
        par = {};

        for (uint32_t tileRow = 0; tileRow < numTileRows; tileRow++)
        {
            for (uint32_t tileCol = 0; tileCol < numTileColumns; tileCol++)
            {
                RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, SetCurrentTile, tileRow, tileCol, m_pipeline);
                auto tileIdx = tileRow * numTileColumns + tileCol;

                m_basicFeature->m_vdencTileSliceStart[tileIdx] = cmdBuffer.iOffset;
                SETPAR_AND_ADDCMD(VDENC_HEVC_VP9_TILE_SLICE_STATE, m_vdencItf, &cmdBuffer);
                ENCODE_CHK_STATUS_RETURN(AddBBEnd(m_miItf, cmdBuffer));

                // Add MI_NOOPs to align with CODECHAL_CACHELINE_SIZE
                uint32_t size = (MOS_ALIGN_CEIL(cmdBuffer.iOffset, CODECHAL_CACHELINE_SIZE) - cmdBuffer.iOffset) / sizeof(uint32_t);
                for (uint32_t i = 0; i < size; i++)
                    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_NOOP)(&cmdBuffer));
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BrcUpdatePkt::AddAllCmds_AVP_SEGMENT_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(m_featureManager);

        auto& par = m_avpItf->MHW_GETPAR_F(AVP_SEGMENT_STATE)();
        par      = {};

        auto segmentFeature = dynamic_cast<Av1Segmentation *>(m_featureManager->GetFeature(Av1FeatureIDs::av1Segmentation));
        ENCODE_CHK_NULL_RETURN(segmentFeature);

        MHW_CHK_STATUS_RETURN(segmentFeature->MHW_SETPAR_F(AVP_SEGMENT_STATE)(par));

        const bool segmentEnabled = par.av1SegmentParams.m_enabled;

        for (uint8_t i = 0; i < av1MaxSegments; i++)
        {
            par.currentSegmentId = i;
            m_avpItf->MHW_ADDCMD_F(AVP_SEGMENT_STATE)(cmdBuffer);

            // If segmentation is not enabled, then AV1_SEGMENT_STATE must still be sent once for SegmentID = 0
            // If i == numSegments -1, means all segments are issued, break the loop
            if (!segmentEnabled || (i == par.numSegments - 1))
            {
                break;
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BrcUpdatePkt::AddAllCmds_AVP_PAK_INSERT_OBJECT(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(m_osInterface);
        auto& params = m_avpItf->MHW_GETPAR_F(AVP_PAK_INSERT_OBJECT)();
        params      = {};

        auto GetExtraData = [&]() { return params.bsBuffer->pBase + params.offset; };
        auto GetExtraSize = [&]() { return (params.bitSize + 7) >> 3; };

        // First, Send all other OBU bit streams other than tile group OBU when it's first tile in frame
        uint32_t tileIdx    = 0;
        bool     tgOBUValid = m_basicFeature->m_slcData[0].BitSize > 0 ? true : false;

        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileIdx, tileIdx);

        if (tileIdx == 0)
        {
            uint32_t nalNum = 0;
            for (uint32_t i = 0; i < MAX_NUM_OBU_TYPES && m_basicFeature->m_nalUnitParams[i]->uiSize > 0; i++)
            {
                nalNum++;
            }

            params.bsBuffer             = &m_basicFeature->m_bsBuffer;
            params.endOfHeaderInsertion = false;

            // Support multiple packed header buffer
            for (uint32_t i = 0; i < nalNum; i++)
            {
                uint32_t nalUnitSize   = m_basicFeature->m_nalUnitParams[i]->uiSize;
                uint32_t nalUnitOffset = m_basicFeature->m_nalUnitParams[i]->uiOffset;

                ENCODE_ASSERT(nalUnitSize < CODECHAL_ENCODE_AV1_PAK_INSERT_UNCOMPRESSED_HEADER);

                if (IsFrameHeader(*(m_basicFeature->m_bsBuffer.pBase + nalUnitOffset)))
                {
                    params.bitSize    = nalUnitSize * 8;
                    params.offset     = nalUnitOffset;
                    params.lastHeader = !tgOBUValid && (i+1 == nalNum);

                    m_avpItf->MHW_ADDCMD_F(AVP_PAK_INSERT_OBJECT)(cmdBuffer);
                    m_osInterface->pfnAddCommand(cmdBuffer, GetExtraData(), GetExtraSize());
                }
            }
        }

        return MOS_STATUS_SUCCESS;
    }

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS Av1BrcUpdatePkt::DumpInput()
    {
        ENCODE_FUNC_CALL();
        int32_t currentPass = m_pipeline->GetCurrentPass();

        CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
        ENCODE_CHK_NULL_RETURN(debugInterface);

        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpHucDmem(
            &m_vdencBrcUpdateDmemBuffer[m_pipeline->m_currRecycledBufIdx][m_pipeline->GetCurrentPass()],
            m_vdencBrcUpdateDmemBufferSize,
            currentPass,
            hucRegionDumpUpdate));

        ENCODE_CHK_STATUS_RETURN(DumpRegion(0, "_BrcHistory", true, hucRegionDumpUpdate, 6080));
        ENCODE_CHK_STATUS_RETURN(DumpRegion(1, "_VdencStats", true, hucRegionDumpUpdate, 48*4));
        ENCODE_CHK_STATUS_RETURN(DumpRegion(3, "_InputSLBB_Origin", true, hucRegionDumpUpdate, 600*4));
        ENCODE_CHK_STATUS_RETURN(DumpRegion(5, "_ConstData", true, hucRegionDumpUpdate, MOS_ALIGN_CEIL(m_vdencBrcConstDataBufferSize, CODECHAL_PAGE_SIZE)));
        ENCODE_CHK_STATUS_RETURN(DumpRegion(7, "_PakMmio", true, hucRegionDumpUpdate, 16*4));
        ENCODE_CHK_STATUS_RETURN(DumpRegion(8, "_InputPakInsert", true, hucRegionDumpUpdate, 100));
        ENCODE_CHK_STATUS_RETURN(DumpRegion(10, "_InputCdfTable", true, hucRegionDumpUpdate, 4 * MOS_ALIGN_CEIL(m_basicFeature->m_cdfMaxNumBytes, CODECHAL_CACHELINE_SIZE)));
        if (m_basicFeature->m_av1PicParams->AdaptiveTUEnabled != 0)
        {
            ENCODE_CHK_STATUS_RETURN(DumpRegion(12, "_InputSLBB_TU7", true, hucRegionDumpUpdate, 600 * 4));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BrcUpdatePkt::DumpOutput()
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_STATUS_RETURN(DumpRegion(0, "_BrcHistory", false, hucRegionDumpUpdate, 6080));
        ENCODE_CHK_STATUS_RETURN(DumpRegion(4, "_BrcData", false, hucRegionDumpUpdate, 32));
        ENCODE_CHK_STATUS_RETURN(DumpRegion(6, "_OutputSLBB", false, hucRegionDumpUpdate, 600 * 4));
        ENCODE_CHK_STATUS_RETURN(DumpRegion(9, "_OutputPakInsert", false, hucRegionDumpUpdate, 100));
        ENCODE_CHK_STATUS_RETURN(DumpRegion(11, "_OutputCdfTable", false, hucRegionDumpUpdate, MOS_ALIGN_CEIL(m_basicFeature->m_cdfMaxNumBytes, CODECHAL_CACHELINE_SIZE)));

        return MOS_STATUS_SUCCESS;
    }
#endif

    MHW_SETPAR_DECL_SRC(HUC_IMEM_STATE, Av1BrcUpdatePkt)
    {
        params.kernelDescriptor = m_vdboxHucAv1BrcUpdateKernelDescriptor;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HUC_DMEM_STATE, Av1BrcUpdatePkt)
    {
        params.function      = BRC_UPDATE;
        params.passNum       = static_cast<uint8_t>(m_pipeline->GetPassNum());
        params.currentPass   = static_cast<uint8_t> (m_pipeline->GetCurrentPass());
        params.hucDataSource = const_cast<PMOS_RESOURCE> (& m_vdencBrcUpdateDmemBuffer[m_pipeline->m_currRecycledBufIdx][m_pipeline->GetCurrentPass()]);
        params.dataLength    = MOS_ALIGN_CEIL(m_vdencBrcUpdateDmemBufferSize, CODECHAL_CACHELINE_SIZE);
        params.dmemOffset    = HUC_DMEM_OFFSET_RTOS_GEMS;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VD_PIPELINE_FLUSH, Av1BrcUpdatePkt)
    {
        params.waitDoneVDCmdMsgParser = true;
        params.waitDoneAV1 = true;
        params.flushAV1 = true;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HUC_VIRTUAL_ADDR_STATE, Av1BrcUpdatePkt)
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        int32_t currentPass = m_pipeline->GetCurrentPass();
        if (currentPass < 0)
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }

        ENCODE_CHK_NULL_RETURN(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_recycleBuf);

        params.function = BRC_UPDATE;

        uint32_t prevBufIdx = 0;
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetPrevStatisticsBufferIndex, prevBufIdx);
        uint32_t statBufIdx = 0;
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetStatisticsBufferIndex, statBufIdx);
        MOS_RESOURCE *resTileBasedStatisticsBuffer = nullptr;
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileBasedStatisticsBuffer, currentPass == 0? prevBufIdx : statBufIdx, resTileBasedStatisticsBuffer);
        uint32_t offset = 0;
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileStatsOffset, offset);

        MOS_RESOURCE *resBrcDataBuffer = nullptr;
        RUN_FEATURE_INTERFACE_RETURN(Av1Brc, Av1FeatureIDs::av1BrcFeature, GetBrcDataBuffer, resBrcDataBuffer);

        const uint32_t bufIdx = m_pipeline->m_currRecycledBufIdx;
        auto brcFeature = dynamic_cast<Av1Brc *>(m_featureManager->GetFeature(Av1FeatureIDs::av1BrcFeature));
        ENCODE_CHK_NULL_RETURN(brcFeature);
        auto vdenc2ndLevelBatchBuffer = brcFeature->GetVdenc2ndLevelBatchBuffer(bufIdx);
        ENCODE_CHK_NULL_RETURN(vdenc2ndLevelBatchBuffer);

        auto pakInsertOutputBatchBuffer = brcFeature->GetPakInsertOutputBatchBuffer(bufIdx);
        ENCODE_CHK_NULL_RETURN(pakInsertOutputBatchBuffer);

        // Region 0 - History Buffer (Input/Output)
        params.regionParams[0].presRegion = m_basicFeature->m_recycleBuf->GetBuffer(VdencBRCHistoryBuffer, 0);
        params.regionParams[0].isWritable = true;
        // Region 1 - VDenc Stats Buffer (Input)
        params.regionParams[1].presRegion = resTileBasedStatisticsBuffer;
        params.regionParams[1].dwOffset   = offset;
        // Region 3 - Input SLB Buffer (Input Origin)
        params.regionParams[3].presRegion = const_cast<PMOS_RESOURCE>(&m_vdencReadBatchBufferOrigin[bufIdx][currentPass]);
        // Region 4 - BRC Data for next frame's width/height - (Output)
        params.regionParams[4].presRegion = resBrcDataBuffer;
        params.regionParams[4].isWritable = true;
        // Region 5 - Const Data (Input)
        params.regionParams[5].presRegion = const_cast<PMOS_RESOURCE>(&m_vdencBrcConstDataBuffer[bufIdx]);
        // Region 6 - Output SLBB - (Output)
        params.regionParams[6].presRegion = &vdenc2ndLevelBatchBuffer->OsResource;
        params.regionParams[6].isWritable = true;
        // Region 7 - PAK MMIO for frame size (Input)
        params.regionParams[7].presRegion = m_basicFeature->m_recycleBuf->GetBuffer(PakInfo, 0);
        // Region 8 - PAK INSERT cmd with FrameHeader before back annotation (Input)
        params.regionParams[8].presRegion = const_cast<PMOS_RESOURCE>(&m_vdencPakInsertBatchBuffer[bufIdx]);
        // Region 9 - PAK INSERT cmd with FrameHeader after back annotation (Output)
        params.regionParams[9].presRegion = &pakInsertOutputBatchBuffer->OsResource;
        params.regionParams[9].isWritable = true;
        // Region 10 - CDF table (Input)
        params.regionParams[10].presRegion = m_basicFeature->m_defaultCdfBuffers;
        // Region 11 - CDF (output)
        params.regionParams[11].presRegion = m_basicFeature->m_defaultCdfBufferInUse;
        params.regionParams[11].isWritable = true;
        // Region 12 - Input SLB Buffer (Input TU7)
        if (m_basicFeature->m_av1PicParams->AdaptiveTUEnabled != 0)
        {
            params.regionParams[12].presRegion = const_cast<PMOS_RESOURCE>(&m_vdencReadBatchBufferTU7[bufIdx][currentPass]);
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_PIC_STATE, Av1BrcUpdatePkt)
    {
        params.notFirstPass = !m_pipeline->IsFirstPass();

        return MOS_STATUS_SUCCESS;
    }

}
