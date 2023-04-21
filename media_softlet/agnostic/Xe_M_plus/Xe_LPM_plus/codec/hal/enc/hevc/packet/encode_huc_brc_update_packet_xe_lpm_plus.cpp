/*
* Copyright (c) 2023, Intel Corporation
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
//! \file     encode_huc_brc_update_packet_xe_lpm_plus.cpp
//! \brief    Defines the implementation of xe_lpm_plus huc update packet 
//!

#include "encode_huc_brc_update_packet_xe_lpm_plus.h"
#include "encode_hevc_vdenc_weighted_prediction.h"

namespace encode
{
MOS_STATUS HucBrcUpdatePktXe_Lpm_Plus::Init()
{
    ENCODE_FUNC_CALL();
    m_hwInterface->m_vdencBatchBuffer1stGroupSize = MOS_ALIGN_CEIL(m_hwInterface->m_vdencBatchBuffer1stGroupSize, CODECHAL_CACHELINE_SIZE);
    m_hwInterface->m_vdencBatchBuffer2ndGroupSize = MOS_ALIGN_CEIL(m_hwInterface->m_vdencBatchBuffer2ndGroupSize, CODECHAL_CACHELINE_SIZE);
    m_hwInterface->m_vdencReadBatchBufferSize =
    m_hwInterface->m_vdenc2ndLevelBatchBufferSize = m_hwInterface->m_vdencBatchBuffer1stGroupSize
                                     + m_hwInterface->m_vdencBatchBuffer2ndGroupSize
                                     + ENCODE_HEVC_VDENC_NUM_MAX_SLICES * MOS_ALIGN_CEIL((2 * m_hcpItf->MHW_GETSIZE_F(HCP_WEIGHTOFFSET_STATE)()
                                     + m_hcpItf->MHW_GETSIZE_F(HCP_SLICE_STATE)()
                                     + 2 * m_hcpItf->MHW_GETSIZE_F(HCP_PAK_INSERT_OBJECT)()
                                     + m_vdencItf->MHW_GETSIZE_F(VDENC_WEIGHTSOFFSETS_STATE)()
                                     + 2 * m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)()), CODECHAL_CACHELINE_SIZE);
    m_hwInterface->m_vdencBatchBufferPerSliceConstSize = m_hcpItf->MHW_GETSIZE_F(HCP_SLICE_STATE)()
        + m_vdencItf->MHW_GETSIZE_F(VDENC_WEIGHTSOFFSETS_STATE)()
        + m_hcpItf->MHW_GETSIZE_F(HCP_PAK_INSERT_OBJECT)()
        + m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)() * 2;

    HUC_CHK_STATUS_RETURN(HucBrcUpdatePkt::Init());

    return MOS_STATUS_SUCCESS;
}

    MOS_STATUS HucBrcUpdatePktXe_Lpm_Plus::SetConstDataHuCBrcUpdate() const
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        auto hucConstData = (VdencHevcHucBrcConstantData *)m_allocator->LockResourceForWrite(const_cast<MOS_RESOURCE*>(&m_vdencBrcConstDataBuffer[m_pipeline->m_currRecycledBufIdx]));
        ENCODE_CHK_NULL_RETURN(hucConstData);

        ENCODE_CHK_STATUS_RETURN(SetConstLambdaHucBrcUpdate(hucConstData));
        RUN_FEATURE_INTERFACE_RETURN(HEVCEncodeBRC, HevcFeatureIDs::hevcBrcFeature,
            SetConstForUpdate, hucConstData);

        // starting location in batch buffer for each slice
        uint32_t baseLocation = m_hwInterface->m_vdencBatchBuffer1stGroupSize + m_hwInterface->m_vdencBatchBuffer2ndGroupSize;
        uint32_t currentLocation = baseLocation;

        auto slcData = m_basicFeature->m_slcData;
        // HCP_WEIGHTSOFFSETS_STATE + HCP_SLICE_STATE + HCP_PAK_INSERT_OBJECT + VDENC_WEIGHT_OFFSETS_STATE
        for (uint32_t slcCount = 0; slcCount < m_basicFeature->m_numSlices; slcCount++)
        {
            m_basicFeature->m_curNumSlices = slcCount;
            auto hevcSlcParams = &m_basicFeature->m_hevcSliceParams[slcCount];
            // HuC FW require unit in Bytes
            hucConstData->Slice[slcCount].SizeOfCMDs
                = (uint16_t)(m_hwInterface->m_vdencBatchBufferPerSliceConstSize + m_basicFeature->m_vdencBatchBufferPerSliceVarSize[slcCount]);

            // HCP_WEIGHTOFFSET_STATE cmd
            RUN_FEATURE_INTERFACE_RETURN(
                HevcVdencWeightedPred,
                HevcFeatureIDs::hevcVdencWpFeature,
                SetHucBrcUpdateConstData,
                *hevcSlcParams,
                slcCount,
                m_hcpWeightOffsetStateCmdSize,
                currentLocation,
                *hucConstData);

            // HCP_SLICE_STATE cmd
            hucConstData->Slice[slcCount].SliceState_StartInBytes = (uint16_t)currentLocation;  // HCP_WEIGHTOFFSET is not needed
            currentLocation += m_hcpSliceStateCmdSize;

            // VDENC_WEIGHT_OFFSETS_STATE cmd
            hucConstData->Slice[slcCount].VdencWeightOffset_StartInBytes                      // VdencWeightOffset cmd is the last one expect BatchBufferEnd cmd
                = (uint16_t)(baseLocation + hucConstData->Slice[slcCount].SizeOfCMDs - m_vdencWeightOffsetStateCmdSize - m_miBatchBufferEndCmdSize - m_alignSize[slcCount]);

            currentLocation += m_miBatchBufferEndCmdSize;

            // logic from PakInsertObject cmd
            uint32_t bitSize         = (m_basicFeature->m_hevcSeqParams->SliceSizeControl) ? (hevcSlcParams->BitLengthSliceHeaderStartingPortion) : slcData[slcCount].BitSize;  // 40 for HEVC VDEnc Dynamic Slice
            uint32_t byteSize = (bitSize + 7) >> 3;
            uint32_t sliceHeaderSizeInBytes = (bitSize + 7) >> 3;
            // 1st PakInsertObject cmd with AU, SPS, PPS headers only exists for the first slice
            if (slcCount == 0)
            {
                // assumes that there is no 3rd PakInsertObject cmd for SSC
                currentLocation += m_1stPakInsertObjectCmdSize;
            }

            hucConstData->Slice[slcCount].SliceHeaderPIO_StartInBytes = (uint16_t)currentLocation;

            // HuC FW requires true slice header size in bits without byte alignment
            hucConstData->Slice[slcCount].SliceHeader_SizeInBits = (uint16_t)(sliceHeaderSizeInBytes * 8);
            if (!m_pipeline->IsFirstPass())
            {
                PBSBuffer bsBuffer = &(m_basicFeature->m_bsBuffer);
                ENCODE_CHK_NULL_RETURN(bsBuffer);
                ENCODE_CHK_NULL_RETURN(bsBuffer->pBase);
                uint8_t *sliceHeaderLastByte = (uint8_t*)(bsBuffer->pBase + slcData[slcCount].SliceOffset + sliceHeaderSizeInBytes - 1);
                for (auto i = 0; i < 8; i++)
                {
                    uint8_t mask = 1 << i;
                    if (*sliceHeaderLastByte & mask)
                    {
                        hucConstData->Slice[slcCount].SliceHeader_SizeInBits -= (i + 1);
                        break;
                    }
                }
            }

            baseLocation += hucConstData->Slice[slcCount].SizeOfCMDs;
            currentLocation = baseLocation;
        }

        m_allocator->UnLock(const_cast<MOS_RESOURCE*>(&m_vdencBrcConstDataBuffer[m_pipeline->m_currRecycledBufIdx]));

        return eStatus;
    }

    MOS_STATUS HucBrcUpdatePktXe_Lpm_Plus::ConstructGroup1Cmds()
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MOS_COMMAND_BUFFER constructedCmdBuf;
        MOS_ZeroMemory(&constructedCmdBuf, sizeof(constructedCmdBuf));
        constructedCmdBuf.pCmdBase = constructedCmdBuf.pCmdPtr = (uint32_t *)m_batchbufferAddr;
        constructedCmdBuf.iRemaining = MOS_ALIGN_CEIL(m_hwInterface->m_vdencReadBatchBufferSize, CODECHAL_PAGE_SIZE);

        // 1st Group : PIPE_MODE_SELECT
        // set PIPE_MODE_SELECT command
        // This is not needed for GEN11/GEN12 as single pass SAO is supported
        // for Gen11+, we need to add MFX wait for both KIN and VRT before and after HCP Pipemode select...
        auto &mfxWaitParams                 = m_miItf->MHW_GETPAR_F(MFX_WAIT)();
        mfxWaitParams                       = {};
        mfxWaitParams.iStallVdboxPipeline   = true;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&constructedCmdBuf));

        SETPAR_AND_ADDCMD(HCP_PIPE_MODE_SELECT, m_hcpItf, &constructedCmdBuf);

        mfxWaitParams                       = {};
        mfxWaitParams.iStallVdboxPipeline   = true;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&constructedCmdBuf));

        MHW_BATCH_BUFFER TempBatchBuffer = {};
        TempBatchBuffer.iSize       = MOS_ALIGN_CEIL(m_hwInterface->m_vdencReadBatchBufferSize, CODECHAL_PAGE_SIZE);
        TempBatchBuffer.pData       = m_batchbufferAddr;

        // set MI_BATCH_BUFFER_END command
        int32_t cmdBufOffset = constructedCmdBuf.iOffset;

        TempBatchBuffer.iCurrent    = constructedCmdBuf.iOffset;
        TempBatchBuffer.iRemaining  = constructedCmdBuf.iRemaining;
        ENCODE_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(nullptr, &TempBatchBuffer));
        constructedCmdBuf.pCmdPtr     += (TempBatchBuffer.iCurrent - constructedCmdBuf.iOffset) / 4;
        constructedCmdBuf.iOffset      = TempBatchBuffer.iCurrent;
        constructedCmdBuf.iRemaining   = TempBatchBuffer.iRemaining;

        m_miBatchBufferEndCmdSize = constructedCmdBuf.iOffset - cmdBufOffset;

        uint32_t alignSize = m_hwInterface->m_vdencBatchBuffer1stGroupSize - constructedCmdBuf.iOffset;
        if (alignSize)
        {
            for (uint32_t i = 0; i < (alignSize / 4); i++)
            {
                ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_NOOP)(&constructedCmdBuf));
            }
        }
        ENCODE_CHK_COND_RETURN(
            (m_hwInterface->m_vdencBatchBuffer1stGroupSize != constructedCmdBuf.iOffset), 
            "ERROR - constructed cmd size is mismatch with calculated");
 
        return eStatus;
    }

    MOS_STATUS HucBrcUpdatePktXe_Lpm_Plus::ConstructGroup2Cmds()
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MOS_COMMAND_BUFFER constructedCmdBuf;
        MOS_ZeroMemory(&constructedCmdBuf, sizeof(constructedCmdBuf));
        constructedCmdBuf.pCmdBase = (uint32_t *)m_batchbufferAddr;
        constructedCmdBuf.iOffset = m_hwInterface->m_vdencBatchBuffer1stGroupSize;
        constructedCmdBuf.pCmdPtr = constructedCmdBuf.pCmdBase + constructedCmdBuf.iOffset / 4;
        constructedCmdBuf.iRemaining = MOS_ALIGN_CEIL(m_hwInterface->m_vdencReadBatchBufferSize, CODECHAL_PAGE_SIZE) - constructedCmdBuf.iOffset;

        SETPAR_AND_ADDCMD(VDENC_CMD1, m_vdencItf, &constructedCmdBuf);
        m_basicFeature->m_picStateCmdStartInBytes = constructedCmdBuf.iOffset;

        // set HCP_PIC_STATE command
        SETPAR_AND_ADDCMD(HCP_PIC_STATE, m_hcpItf, &constructedCmdBuf);
        m_cmd2StartInBytes = constructedCmdBuf.iOffset;

        SETPAR_AND_ADDCMD(VDENC_CMD2, m_vdencItf, &constructedCmdBuf);

        // set MI_BATCH_BUFFER_END command
        MHW_BATCH_BUFFER TempBatchBuffer = {};
        TempBatchBuffer.iSize       = MOS_ALIGN_CEIL(m_hwInterface->m_vdencReadBatchBufferSize, CODECHAL_PAGE_SIZE);
        TempBatchBuffer.pData       = m_batchbufferAddr;

        TempBatchBuffer.iCurrent    = constructedCmdBuf.iOffset;
        TempBatchBuffer.iRemaining  = constructedCmdBuf.iRemaining;
        ENCODE_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(nullptr, &TempBatchBuffer));
        constructedCmdBuf.pCmdPtr     += (TempBatchBuffer.iCurrent - constructedCmdBuf.iOffset) / 4;
        constructedCmdBuf.iOffset      = TempBatchBuffer.iCurrent;
        constructedCmdBuf.iRemaining   = TempBatchBuffer.iRemaining;

        uint32_t alignSize = m_hwInterface->m_vdencBatchBuffer2ndGroupSize + m_hwInterface->m_vdencBatchBuffer1stGroupSize - constructedCmdBuf.iOffset;
        if (alignSize)
        {
            for (uint32_t i = 0; i < (alignSize / 4); i++)
            {
                ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_NOOP)(&constructedCmdBuf));
            }
        }

        ENCODE_CHK_COND_RETURN(
            (m_hwInterface->m_vdencBatchBuffer2ndGroupSize + m_hwInterface->m_vdencBatchBuffer1stGroupSize != constructedCmdBuf.iOffset), 
            "ERROR - constructed cmd size is mismatch with calculated");

        return eStatus;
    }

    MOS_STATUS HucBrcUpdatePktXe_Lpm_Plus::ConstructGroup3Cmds()
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
        uint32_t cmdBufOffset = 0;

        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_slcData);

        MOS_COMMAND_BUFFER constructedCmdBuf;
        MOS_ZeroMemory(&constructedCmdBuf, sizeof(constructedCmdBuf));
        constructedCmdBuf.pCmdBase = (uint32_t *)m_batchbufferAddr;
        constructedCmdBuf.iOffset = m_hwInterface->m_vdencBatchBuffer1stGroupSize + m_hwInterface->m_vdencBatchBuffer2ndGroupSize;
        constructedCmdBuf.pCmdPtr = constructedCmdBuf.pCmdBase + constructedCmdBuf.iOffset / 4;
        constructedCmdBuf.iRemaining = MOS_ALIGN_CEIL(m_hwInterface->m_vdencReadBatchBufferSize, CODECHAL_PAGE_SIZE) - constructedCmdBuf.iOffset;

        // slice level cmds for each slice
        PCODEC_ENCODER_SLCDATA slcData = m_basicFeature->m_slcData;
        for (uint32_t startLCU = 0, slcCount = 0; slcCount < m_basicFeature->m_numSlices; slcCount++)
        {
            m_basicFeature->m_curNumSlices    = slcCount;
            m_basicFeature->m_lastSliceInTile = true;

            if (m_pipeline->IsFirstPass())
            {
                slcData[slcCount].CmdOffset = startLCU * (m_hcpItf->GetHcpPakObjSize()) * sizeof(uint32_t);
            }

            RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, SetCurrentTileFromSliceIndex, slcCount, m_pipeline);

            m_basicFeature->m_vdencBatchBufferPerSliceVarSize[slcCount] = 0;

            // set HCP_WEIGHTOFFSET_STATE command
            // This slice level command is issued, if the weighted_pred_flag or weighted_bipred_flag equals one.
            //        If zero, then this command is not issued.
            AddAllCmds_HCP_WEIGHTOFFSET_STATE(&constructedCmdBuf);

            // set HCP_SLICE_STATE command
            cmdBufOffset = constructedCmdBuf.iOffset;
            SETPAR_AND_ADDCMD(HCP_SLICE_STATE, m_hcpItf, &constructedCmdBuf);
            m_hcpSliceStateCmdSize = constructedCmdBuf.iOffset - cmdBufOffset;

            // set MI_BATCH_BUFFER_END command
            MHW_BATCH_BUFFER TempBatchBuffer = {};
            TempBatchBuffer.iSize            = MOS_ALIGN_CEIL(m_hwInterface->m_vdencReadBatchBufferSize, CODECHAL_PAGE_SIZE);
            TempBatchBuffer.pData            = m_batchbufferAddr;

            TempBatchBuffer.iCurrent   = constructedCmdBuf.iOffset;
            TempBatchBuffer.iRemaining = constructedCmdBuf.iRemaining;
            ENCODE_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(nullptr, &TempBatchBuffer));
            constructedCmdBuf.pCmdPtr += (TempBatchBuffer.iCurrent - constructedCmdBuf.iOffset) / 4;
            constructedCmdBuf.iOffset    = TempBatchBuffer.iCurrent;
            constructedCmdBuf.iRemaining = TempBatchBuffer.iRemaining;

            m_basicFeature->m_vdencBatchBufferPerSlicePart2Start[slcCount] = constructedCmdBuf.iOffset;

            AddAllCmds_HCP_PAK_INSERT_OBJECT_SLICE(&constructedCmdBuf);

            cmdBufOffset = constructedCmdBuf.iOffset;
            SETPAR_AND_ADDCMD(VDENC_WEIGHTSOFFSETS_STATE, m_vdencItf, &constructedCmdBuf);
            m_vdencWeightOffsetStateCmdSize = constructedCmdBuf.iOffset - cmdBufOffset;

            // set MI_BATCH_BUFFER_END command
            TempBatchBuffer = {};
            TempBatchBuffer.iSize       = MOS_ALIGN_CEIL(m_hwInterface->m_vdencReadBatchBufferSize, CODECHAL_PAGE_SIZE);
            TempBatchBuffer.pData       = m_batchbufferAddr;

            TempBatchBuffer.iCurrent    = constructedCmdBuf.iOffset;
            TempBatchBuffer.iRemaining  = constructedCmdBuf.iRemaining;
            ENCODE_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(nullptr, &TempBatchBuffer));
            constructedCmdBuf.pCmdPtr     += (TempBatchBuffer.iCurrent - constructedCmdBuf.iOffset) / 4;
            constructedCmdBuf.iOffset      = TempBatchBuffer.iCurrent;
            constructedCmdBuf.iRemaining   = TempBatchBuffer.iRemaining;

            m_alignSize[slcCount] = MOS_ALIGN_CEIL(constructedCmdBuf.iOffset, 64) - constructedCmdBuf.iOffset;
            if (m_alignSize[slcCount] > 0)
            {
                for (uint32_t i = 0; i < (m_alignSize[slcCount] / 4); i++)
                {
                    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_NOOP)(&constructedCmdBuf));
                }
            }
            m_basicFeature->m_vdencBatchBufferPerSliceVarSize[slcCount] += m_alignSize[slcCount];

            startLCU += m_basicFeature->m_hevcSliceParams[slcCount].NumLCUsInSlice;
        }

        m_slbDataSizeInBytes = constructedCmdBuf.iOffset;

        return eStatus;
    }

    MOS_STATUS HucBrcUpdatePktXe_Lpm_Plus::AddAllCmds_HCP_PAK_INSERT_OBJECT_SLICE(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        auto &params = m_hcpItf->MHW_GETPAR_F(HCP_PAK_INSERT_OBJECT)();

        uint32_t                   cmdBufOffset    = 0;
        uint32_t                   bitSize         = 0;
        uint32_t                   offSet          = 0;
        PCODECHAL_NAL_UNIT_PARAMS *ppNalUnitParams = (CODECHAL_NAL_UNIT_PARAMS **)m_basicFeature->m_nalUnitParams;
        PBSBuffer                  pBsBuffer       = &(m_basicFeature->m_bsBuffer);

        params = {};
        // Insert slice header
        params.bLastHeader              = true;
        params.bEmulationByteBitsInsert = true;

        // App does the slice header packing, set the skip count passed by the app
        PCODEC_ENCODER_SLCDATA slcData    = m_basicFeature->m_slcData;
        uint32_t               currSlcIdx = m_basicFeature->m_curNumSlices;

        params.uiSkipEmulationCheckCount = slcData[currSlcIdx].SkipEmulationByteCount;
        bitSize                          = slcData[currSlcIdx].BitSize;
        offSet                           = slcData[currSlcIdx].SliceOffset;

        if (m_basicFeature->m_hevcSeqParams->SliceSizeControl)
        {
            params.bLastHeader                = false;
            params.bEmulationByteBitsInsert   = false;
            bitSize                           = m_basicFeature->m_hevcSliceParams->BitLengthSliceHeaderStartingPortion;
            params.bResetBitstreamStartingPos = true;
            params.dwPadding                  = (MOS_ALIGN_CEIL((bitSize + 7) >> 3, sizeof(uint32_t))) / sizeof(uint32_t);
            params.dataBitsInLastDw           = bitSize % 32;
            if (params.dataBitsInLastDw == 0)
            {
                params.dataBitsInLastDw = 32;
            }

            m_hcpItf->MHW_ADDCMD_F(HCP_PAK_INSERT_OBJECT)(cmdBuffer);
            uint32_t byteSize = (bitSize + 7) >> 3;
            m_basicFeature->m_vdencBatchBufferPerSliceVarSize[m_basicFeature->m_curNumSlices] += (MOS_ALIGN_CEIL(byteSize, sizeof(uint32_t))) / sizeof(uint32_t) * 4;
            if (byteSize)
            {
                MHW_MI_CHK_NULL(pBsBuffer);
                MHW_MI_CHK_NULL(pBsBuffer->pBase);
                uint8_t *data = (uint8_t *)(pBsBuffer->pBase + offSet);
                MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, nullptr, data, byteSize));
            }

            // Send HCP_PAK_INSERT_OBJ command. For dynamic slice, we are skipping the beginning part of slice header.
            params.bLastHeader = true;
            bitSize            = slcData[currSlcIdx].BitSize - m_basicFeature->m_hevcSliceParams->BitLengthSliceHeaderStartingPortion;
            offSet += ((m_basicFeature->m_hevcSliceParams->BitLengthSliceHeaderStartingPortion + 7) / 8);  // Skips the first 5 bytes which is Start Code + Nal Unit Header
            params.dwPadding        = (MOS_ALIGN_CEIL((bitSize + 7) >> 3, sizeof(uint32_t))) / sizeof(uint32_t);
            params.dataBitsInLastDw = bitSize % 32;
            if (params.dataBitsInLastDw == 0)
            {
                params.dataBitsInLastDw = 32;
            }
            params.bResetBitstreamStartingPos = true;
            cmdBufOffset                      = cmdBuffer->iOffset;
            m_hcpItf->MHW_ADDCMD_F(HCP_PAK_INSERT_OBJECT)(cmdBuffer);
            byteSize = (bitSize + 7) >> 3;
            if (byteSize)
            {
                MHW_MI_CHK_NULL(pBsBuffer);
                MHW_MI_CHK_NULL(pBsBuffer->pBase);
                uint8_t *data = (uint8_t *)(pBsBuffer->pBase + offSet);
                MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, nullptr, data, byteSize));
            }
            m_basicFeature->m_vdencBatchBufferPerSliceVarSize[m_basicFeature->m_curNumSlices] += (cmdBuffer->iOffset - cmdBufOffset);
        }
        else
        {
            params.dwPadding        = (MOS_ALIGN_CEIL((bitSize + 7) >> 3, sizeof(uint32_t))) / sizeof(uint32_t);
            params.dataBitsInLastDw = bitSize % 32;
            if (params.dataBitsInLastDw == 0)
            {
                params.dataBitsInLastDw = 32;
            }
            m_hcpItf->MHW_ADDCMD_F(HCP_PAK_INSERT_OBJECT)(cmdBuffer);
            uint32_t byteSize = (bitSize + 7) >> 3;
            m_basicFeature->m_vdencBatchBufferPerSliceVarSize[m_basicFeature->m_curNumSlices] += (MOS_ALIGN_CEIL(byteSize, sizeof(uint32_t))) / sizeof(uint32_t) * 4;
            if (byteSize)
            {
                MHW_MI_CHK_NULL(pBsBuffer);
                MHW_MI_CHK_NULL(pBsBuffer->pBase);
                uint8_t *data = (uint8_t *)(pBsBuffer->pBase + offSet);
                MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, nullptr, data, byteSize));
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    }
