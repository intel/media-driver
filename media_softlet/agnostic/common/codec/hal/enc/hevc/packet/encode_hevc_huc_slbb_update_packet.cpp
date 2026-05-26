/*
* Copyright (c) 2026, Intel Corporation
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
//! \file     encode_hevc_huc_slbb_update_packet.cpp
//! \brief    Defines the implementation of HEVC HuC SLBB update packet
//!

#include "encode_hevc_huc_slbb_update_packet.h"
#include "encode_utils.h"
#include "codechal_debug.h"
#include "encode_hevc_tile.h"
#include "encode_hevc_vdenc_weighted_prediction.h"
#include "encode_hevc_vdenc_scc.h"
#include "encode_hevc_cqp.h"

namespace encode
{
    HEVCHucSLBBUpdatePkt::HEVCHucSLBBUpdatePkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface) :
        HucSLBBUpdatePkt(pipeline, task, hwInterface)
    {
        ENCODE_FUNC_CALL();
        m_vdboxHucKernelDescriptor = m_vdboxHucHevcSlbbUpdateKernelDescriptor;
        m_slbbUpdateDmemBufferSize = sizeof(HucHevcSlbbUpdateDmem);
        m_featureManager = m_pipeline->GetPacketLevelFeatureManager(HevcVdencPipeline::HucSLBBUpdate);
        m_hcpItf         = hwInterface->GetHcpInterfaceNext();
        m_vdencItf       = hwInterface->GetVdencInterfaceNext();
        m_miItf          = hwInterface->GetMiInterfaceNext();
    }

    MOS_STATUS HEVCHucSLBBUpdatePkt::Init()
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_STATUS_RETURN(HucSLBBUpdatePkt::Init());

        ENCODE_CHK_NULL_RETURN(m_pipeline);
        m_allocator = m_pipeline->GetEncodeAllocator();
        ENCODE_CHK_NULL_RETURN(m_allocator);

        ENCODE_CHK_NULL_RETURN(m_featureManager);
        m_hevcBasicFeature = dynamic_cast<HevcBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        ENCODE_CHK_NULL_RETURN(m_hevcBasicFeature);

        // Initialize HEVC-specific parameters
        m_numTiles = m_hevcBasicFeature->m_numTiles;
        m_ctbSize = m_hevcBasicFeature->m_maxLCUSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HEVCHucSLBBUpdatePkt::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(commandBuffer);
        ENCODE_CHK_NULL_RETURN(m_hevcBasicFeature);
        ENCODE_CHK_NULL_RETURN(m_hevcBasicFeature->m_hevcPicParams);
        ENCODE_CHK_NULL_RETURN(m_hevcBasicFeature->m_hevcSeqParams);

        // HEVCHucSLBBUpdatePkt is first-task-in-phase: its prolog (SendPrologCmds) embeds
        // AddWatchdogTimerStartCmd into the CmdBuffer with the current watchdogCountThreshold.
        // The later main VDEnc pkt's SetWatchdogTimerThreshold updates the field but under
        // single-task-phase the main pkt skips its own prolog, so the already-submitted
        // watchdog register value is never refreshed. Set the encoder-appropriate threshold
        // HERE (based on frame size) so the prolog picks it up. Without this, large frames
        // (e.g. 8K) time out under the default 60ms threshold.
        ENCODE_CHK_STATUS_RETURN(m_miItf->SetWatchdogTimerThreshold(
            m_hevcBasicFeature->m_frameWidth, m_hevcBasicFeature->m_frameHeight, true));

        // Construct the SLBB with all command templates before running HuC
        ENCODE_CHK_STATUS_RETURN(ConstructBatchBuffer());

        bool firstTaskInPhase = packetPhase & firstPacket;
        bool requestProlog = false;

        // Set performance tag for HEVC SLBB Update
        uint16_t perfTag = CODECHAL_ENCODE_PERFTAG_CALL_SLBB_UPDATE;
        uint16_t pictureType = m_hevcBasicFeature->m_pictureCodingType;
        if (m_hevcBasicFeature->m_pictureCodingType == B_TYPE && m_hevcBasicFeature->m_ref.IsLowDelay())
        {
            pictureType = 0;
        }
        SetPerfTag(perfTag, (uint16_t)m_hevcBasicFeature->m_mode, pictureType);

        if (!m_pipeline->IsSingleTaskPhaseSupported() || firstTaskInPhase)
        {
            // Send command buffer header at the beginning (OS dependent)
            requestProlog = true;
        }

        // Update HEVC-specific tile parameters
        m_numTiles = m_hevcBasicFeature->m_numTiles;
        
        // Handle HEVC tile coding specifics
        if (m_numTiles > 1)
        {
            // Multi-tile HEVC encoding requires special handling
            m_ctbSize = m_hevcBasicFeature->m_maxLCUSize;
        }

        // Execute HuC with HEVC SLBB Update function
        ENCODE_CHK_STATUS_RETURN(Execute(commandBuffer, true, requestProlog, SLBB_UPDATE));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HEVCHucSLBBUpdatePkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
    {
        ENCODE_FUNC_CALL();

        auto osInterface = m_hwInterface->GetOsInterface();
        ENCODE_CHK_NULL_RETURN(osInterface);

        uint32_t hucCommandsSize = 0;
        uint32_t hucPatchListSize = 0;
        MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;

        ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucStateCommandSize(
            m_hevcBasicFeature->m_mode, (uint32_t*)&hucCommandsSize, (uint32_t*)&hucPatchListSize, &stateCmdSizeParams));

        // Account for HEVC-specific command sizes including tile and CTB handling
        commandBufferSize = hucCommandsSize;
        
        // Add extra space for HEVC tile-specific operations
        if (m_numTiles > 1)
        {
            commandBufferSize += (m_numTiles * CODECHAL_CACHELINE_SIZE);
        }

        requestedPatchListSize = osInterface->bUsesPatchList ? hucPatchListSize : 0;

        if (m_pipeline->IsSingleTaskPhaseSupported())
        {
            commandBufferSize *= m_pipeline->GetPassNum();
        }

        // 4K align since allocation is in chunks of 4K bytes.
        commandBufferSize = MOS_ALIGN_CEIL(commandBufferSize, CODECHAL_PAGE_SIZE);

        // Add space for HUC_IMEM_ADDR command and patch list entry in PPGTT mode
        if (m_isPPGTT)
        {
            ENCODE_CHK_NULL_RETURN(m_itfPPGTT);
            commandBufferSize += m_itfPPGTT->MHW_GETSIZE_F(HUC_IMEM_ADDR)();
            requestedPatchListSize += PATCH_LIST_COMMAND(mhw::vdbox::huc::ItfPPGTT::HUC_IMEM_ADDR_CMD);
            commandBufferSize = MOS_ALIGN_CEIL(commandBufferSize, CODECHAL_PAGE_SIZE);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HEVCHucSLBBUpdatePkt::AllocateResources()
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_STATUS_RETURN(HucSLBBUpdatePkt::AllocateResources());

        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;

        // Allocate HEVC-specific tile SLBB buffer
        if (m_numTiles > 1)
        {
            allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(CODECHAL_PAGE_SIZE * m_numTiles, CODECHAL_PAGE_SIZE);
            allocParamsForBufferLinear.pBufName = "HEVC Tile SLBB Buffer";
            allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE;
            MOS_RESOURCE *allocatedBuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
            ENCODE_CHK_NULL_RETURN(allocatedBuffer);
            m_hevcTileSlbbBuffer = allocatedBuffer;
        }

        // Allocate HEVC CTB parameters buffer
        allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(sizeof(uint32_t) * m_ctbSize * m_ctbSize, CODECHAL_CACHELINE_SIZE);
        allocParamsForBufferLinear.pBufName = "HEVC CTB Parameters Buffer";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE;
        MOS_RESOURCE *ctbBuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
        ENCODE_CHK_NULL_RETURN(ctbBuffer);
        m_hevcCtbParamsBuffer = ctbBuffer;

        // Load HEVC SLBB update kernel binary for PPGTT mode
        if (m_isPPGTT && m_kernelBinBuffer == nullptr)
        {
            ENCODE_CHK_NULL_RETURN(m_hucKernelSource);
            HucKernelSource::HucBinary hucBinary{};
            ENCODE_CHK_STATUS_RETURN(m_hucKernelSource->GetKernelBin(HucKernelSource::hevcSlbbUpdateKernelId, hucBinary));

            ENCODE_CHK_NULL_RETURN(hucBinary.m_data);

            // Initialize allocation parameters for kernel binary buffer
            MOS_ALLOC_GFXRES_PARAMS allocParamsForKernelBuffer;
            MOS_ZeroMemory(&allocParamsForKernelBuffer, sizeof(MOS_ALLOC_GFXRES_PARAMS));
            allocParamsForKernelBuffer.Type               = MOS_GFXRES_BUFFER;
            allocParamsForKernelBuffer.TileType           = MOS_TILE_LINEAR;
            allocParamsForKernelBuffer.Format             = Format_Buffer;
            allocParamsForKernelBuffer.dwBytes            = MOS_ALIGN_CEIL(hucBinary.m_size, CODECHAL_CACHELINE_SIZE);
            allocParamsForKernelBuffer.pBufName           = "HevcSlbbUpdateKernelBinBuffer";
            allocParamsForKernelBuffer.ResUsageType       = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
            allocParamsForKernelBuffer.Flags.bNotLockable = false;  // Resource can be CPU accessed

            m_kernelBinBuffer = m_allocator->AllocateResource(allocParamsForKernelBuffer, false);
            ENCODE_CHK_NULL_RETURN(m_kernelBinBuffer);

            auto data = (uint16_t *)m_allocator->LockResourceForWrite(m_kernelBinBuffer);
            ENCODE_CHK_NULL_RETURN(data);
            MOS_SecureMemcpy(data, hucBinary.m_size, hucBinary.m_data, hucBinary.m_size);
            ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(m_kernelBinBuffer));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HEVCHucSLBBUpdatePkt::ConstructBatchBuffer()
    {
        ENCODE_FUNC_CALL();

        uint32_t recycledBufIdx = m_pipeline->m_currRecycledBufIdx;
        uint32_t brcPass        = m_pipeline->GetCurrentPass();

        PMOS_RESOURCE batchBuffer = m_hevcBasicFeature->GetVdencReadBatchBufferOrigin(recycledBufIdx, brcPass);
        ENCODE_CHK_NULL_RETURN(batchBuffer);

        m_batchbufferAddr = (uint8_t *)m_allocator->LockResourceForWrite(batchBuffer);
        ENCODE_CHK_NULL_RETURN(m_batchbufferAddr);

        // Ensure m_slotForRecNotFiltered is current before the SETPAR chain
        // constructs HCP_PIC_STATE (IbcMotionCompensationBufferReferenceIdc)
        // and VDENC_CMD2 (FwdRefXRefPic for IBC). Without this, the first
        // B-frame after an I-frame uses the stale slot from the previous frame
        // because HCP_SURFACE_STATE SETPAR hasn't run yet.
        auto sccFeature = dynamic_cast<HevcVdencScc *>(
            m_pipeline->GetFeatureManager()->GetFeature(HevcFeatureIDs::hevcVdencSccFeature));
        if (sccFeature)
        {
            ENCODE_CHK_STATUS_RETURN(sccFeature->UpdateSlotForRecNotFiltered());
        }

        ENCODE_CHK_STATUS_RETURN(ConstructGroup1Cmds());
        ENCODE_CHK_STATUS_RETURN(ConstructGroup2Cmds());
        ENCODE_CHK_STATUS_RETURN(ConstructGroup3Cmds());

        // Share SLBB layout offsets with BRC Update via basic feature so that
        // BRC can populate its DMEM without re-constructing the SLBB (which
        // would overwrite Origin with a different layout).
        m_hevcBasicFeature->m_slbbCmd2StartInBytes              = m_cmd2StartInBytes;
        m_hevcBasicFeature->m_slbbSlbDataSizeInBytes            = m_slbDataSizeInBytes;
        m_hevcBasicFeature->m_slbbHcpSliceStateCmdSize          = m_hcpSliceStateCmdSize;
        m_hevcBasicFeature->m_slbbHcpWeightOffsetStateCmdSize  = m_hcpWeightOffsetStateCmdSize;
        m_hevcBasicFeature->m_slbbVdencWeightOffsetStateCmdSize = m_vdencWeightOffsetStateCmdSize;
        m_hevcBasicFeature->m_slbbMiBatchBufferEndCmdSize       = m_miBatchBufferEndCmdSize;
        MOS_SecureMemcpy(m_hevcBasicFeature->m_slbbAlignSize,
                         sizeof(m_hevcBasicFeature->m_slbbAlignSize),
                         m_alignSize, sizeof(m_alignSize));

        m_allocator->UnLock(batchBuffer);

        // TU7 batch buffer construction for AdaptiveTU
        if (m_hevcBasicFeature->m_hevcPicParams->AdaptiveTUEnabled != 0)
        {
            auto original_TU = m_hevcBasicFeature->m_targetUsage;
            m_hevcBasicFeature->m_targetUsage = m_hevcBasicFeature->m_hevcSeqParams->TargetUsage = 7;

            auto cqpFeature = dynamic_cast<HevcEncodeCqp *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcCqpFeature));
            ENCODE_CHK_NULL_RETURN(cqpFeature);
            bool original_RDOQ = cqpFeature->IsRDOQEnabled();
            cqpFeature->SetRDOQ(false);

            // Save offset members — ConstructGroup*Cmds() overwrites them
            auto saved_group1EndOffset                    = m_group1EndOffset;
            auto saved_group2EndOffset                    = m_group2EndOffset;
            auto saved_cmd2StartInBytes                   = m_cmd2StartInBytes;
            auto saved_weightsOffsetsStateOffsetInBytes   = m_weightsOffsetsStateOffsetInBytes;
            auto saved_tileSliceStateOffsetInBytes        = m_tileSliceStateOffsetInBytes;
            auto saved_slbDataSizeInBytes                 = m_slbDataSizeInBytes;

            // Construct TU7 into SLBB HuC input buffer (Region 2 for HuC SLBB processing)
            PMOS_RESOURCE tu7Buffer = m_hevcBasicFeature->GetVdencReadBatchBufferTU7(recycledBufIdx, brcPass);
            ENCODE_CHK_NULL_RETURN(tu7Buffer);

            m_batchbufferAddr = (uint8_t *)m_allocator->LockResourceForWrite(tu7Buffer);
            ENCODE_CHK_NULL_RETURN(m_batchbufferAddr);

            ENCODE_CHK_STATUS_RETURN(ConstructGroup1Cmds());
            ENCODE_CHK_STATUS_RETURN(ConstructGroup2Cmds());
            ENCODE_CHK_STATUS_RETURN(ConstructGroup3Cmds());

            m_allocator->UnLock(tu7Buffer);

            // Also construct TU7 into the 2ndLevelBatchBuffer (BRC Region 12 input).
            // This ensures BRC reads a valid TU7 template with TU=7 and RDOQ disabled,
            // matching the base class ConstructBatchBufferHuCBRC behavior.
            // If HuC SLBB processing runs on TU7, it will overwrite this with
            // processed values; otherwise BRC uses this raw template directly.
            auto tu7OutputBatchBuffer = m_hevcBasicFeature->GetVdenc2ndLevelBatchBufferTU7(recycledBufIdx);
            ENCODE_CHK_NULL_RETURN(tu7OutputBatchBuffer);

            m_batchbufferAddr = (uint8_t *)m_allocator->LockResourceForWrite(&tu7OutputBatchBuffer->OsResource);
            ENCODE_CHK_NULL_RETURN(m_batchbufferAddr);

            ENCODE_CHK_STATUS_RETURN(ConstructGroup1Cmds());
            ENCODE_CHK_STATUS_RETURN(ConstructGroup2Cmds());
            ENCODE_CHK_STATUS_RETURN(ConstructGroup3Cmds());

            m_allocator->UnLock(&tu7OutputBatchBuffer->OsResource);

            // Restore offset members so SetDmem() uses the standard SLBB offsets
            m_group1EndOffset                    = saved_group1EndOffset;
            m_group2EndOffset                    = saved_group2EndOffset;
            m_cmd2StartInBytes                   = saved_cmd2StartInBytes;
            m_weightsOffsetsStateOffsetInBytes   = saved_weightsOffsetsStateOffsetInBytes;
            m_tileSliceStateOffsetInBytes        = saved_tileSliceStateOffsetInBytes;
            m_slbDataSizeInBytes                 = saved_slbDataSizeInBytes;

            m_hevcBasicFeature->m_targetUsage = m_hevcBasicFeature->m_hevcSeqParams->TargetUsage = original_TU;
            cqpFeature->SetRDOQ(original_RDOQ);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HEVCHucSLBBUpdatePkt::ConstructGroup1Cmds()
    {
        ENCODE_FUNC_CALL();

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

        m_group1EndOffset = constructedCmdBuf.iOffset;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HEVCHucSLBBUpdatePkt::ConstructGroup2Cmds()
    {
        ENCODE_FUNC_CALL();

        MOS_COMMAND_BUFFER constructedCmdBuf;
        MOS_ZeroMemory(&constructedCmdBuf, sizeof(constructedCmdBuf));
        constructedCmdBuf.pCmdBase = (uint32_t *)m_batchbufferAddr;
        constructedCmdBuf.iOffset = m_hwInterface->m_vdencBatchBuffer1stGroupSize;
        constructedCmdBuf.pCmdPtr = constructedCmdBuf.pCmdBase + constructedCmdBuf.iOffset / 4;
        constructedCmdBuf.iRemaining = MOS_ALIGN_CEIL(m_hwInterface->m_vdencReadBatchBufferSize, CODECHAL_PAGE_SIZE) - constructedCmdBuf.iOffset;

        SETPAR_AND_ADDCMD(VDENC_CMD1, m_vdencItf, &constructedCmdBuf);
        m_hevcBasicFeature->m_picStateCmdStartInBytes = constructedCmdBuf.iOffset;

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

        m_group2EndOffset = constructedCmdBuf.iOffset;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HEVCHucSLBBUpdatePkt::ConstructGroup3Cmds()
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(m_hevcBasicFeature->m_slcData);

        MOS_COMMAND_BUFFER constructedCmdBuf;
        MOS_ZeroMemory(&constructedCmdBuf, sizeof(constructedCmdBuf));
        constructedCmdBuf.pCmdBase = (uint32_t *)m_batchbufferAddr;
        constructedCmdBuf.iOffset = m_hwInterface->m_vdencBatchBuffer1stGroupSize + m_hwInterface->m_vdencBatchBuffer2ndGroupSize;
        constructedCmdBuf.pCmdPtr = constructedCmdBuf.pCmdBase + constructedCmdBuf.iOffset / 4;
        constructedCmdBuf.iRemaining = MOS_ALIGN_CEIL(m_hwInterface->m_vdencReadBatchBufferSize, CODECHAL_PAGE_SIZE) - constructedCmdBuf.iOffset;

        // slice level cmds for each slice
        PCODEC_ENCODER_SLCDATA slcData = m_hevcBasicFeature->m_slcData;
        for (uint32_t startLCU = 0, slcCount = 0; slcCount < m_hevcBasicFeature->m_numSlices; slcCount++)
        {
            m_hevcBasicFeature->m_curNumSlices = slcCount;

            if (m_pipeline->IsFirstPass())
            {
                slcData[slcCount].CmdOffset = startLCU * (m_hcpItf->GetHcpPakObjSize()) * sizeof(uint32_t);
            }

            RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, SetCurrentTileFromSliceIndex, slcCount, m_pipeline);

            bool sliceInTile    = false;
            bool lastSliceInTile = false;
            EncodeTileData curTileData = {};
            RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetCurrentTile, curTileData);
            RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, IsSliceInTile, slcCount, &curTileData, &sliceInTile, &lastSliceInTile);
            m_hevcBasicFeature->m_lastSliceInTile = lastSliceInTile;

            m_hevcBasicFeature->m_vdencBatchBufferPerSliceVarSize[slcCount] = 0;
            m_hevcBasicFeature->m_vdencBatchBufferPerSlicePart2Size[slcCount] = 0;

            // Add HCP_WEIGHTOFFSET_STATE commands (inline implementation)
            auto &weightOffsetParams = m_hcpItf->MHW_GETPAR_F(HCP_WEIGHTOFFSET_STATE)();
            weightOffsetParams = {};
            auto wpFeature = dynamic_cast<HevcVdencWeightedPred *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcVdencWpFeature));
            ENCODE_CHK_NULL_RETURN(wpFeature);

            if (wpFeature->IsEnabled() && m_hevcBasicFeature->m_hevcPicParams->bEnableGPUWeightedPrediction)
            {
                ENCODE_CHK_STATUS_RETURN(wpFeature->MHW_SETPAR_F(HCP_WEIGHTOFFSET_STATE)(weightOffsetParams));

                uint32_t cmdBufOffset = 0;
                // 1st HCP_WEIGHTOFFSET_STATE cmd - P & B
                if (m_hevcBasicFeature->m_hevcSliceParams->slice_type == encodeHevcPSlice || 
                    m_hevcBasicFeature->m_hevcSliceParams->slice_type == encodeHevcBSlice)
                {
                    weightOffsetParams.ucList = LIST_0;
                    cmdBufOffset = constructedCmdBuf.iOffset;
                    m_hcpItf->MHW_ADDCMD_F(HCP_WEIGHTOFFSET_STATE)(&constructedCmdBuf);
                    m_hcpWeightOffsetStateCmdSize = constructedCmdBuf.iOffset - cmdBufOffset;
                    m_hevcBasicFeature->m_vdencBatchBufferPerSliceVarSize[m_hevcBasicFeature->m_curNumSlices] += m_hcpWeightOffsetStateCmdSize;
                }

                // 2nd HCP_WEIGHTOFFSET_STATE cmd - B only
                if (m_hevcBasicFeature->m_hevcSliceParams->slice_type == encodeHevcBSlice)
                {
                    weightOffsetParams.ucList = LIST_1;
                    cmdBufOffset = constructedCmdBuf.iOffset;
                    m_hcpItf->MHW_ADDCMD_F(HCP_WEIGHTOFFSET_STATE)(&constructedCmdBuf);
                    m_hcpWeightOffsetStateCmdSize = constructedCmdBuf.iOffset - cmdBufOffset;
                    m_hevcBasicFeature->m_vdencBatchBufferPerSliceVarSize[m_hevcBasicFeature->m_curNumSlices] += m_hcpWeightOffsetStateCmdSize;
                }
            }

            // set HCP_SLICE_STATE command
            uint32_t cmdBufOffset = constructedCmdBuf.iOffset;
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

            m_hevcBasicFeature->m_vdencBatchBufferPerSlicePart2Start[slcCount] = constructedCmdBuf.iOffset;

            // Add HCP_PAK_INSERT_OBJECT commands for slice header (inline implementation)
            auto &pakInsertParams = m_hcpItf->MHW_GETPAR_F(HCP_PAK_INSERT_OBJECT)();
            pakInsertParams = {};
            pakInsertParams.bLastHeader              = true;
            pakInsertParams.bEmulationByteBitsInsert = true;

            uint32_t currSlcIdx = m_hevcBasicFeature->m_curNumSlices;
            pakInsertParams.uiSkipEmulationCheckCount = slcData[currSlcIdx].SkipEmulationByteCount;
            uint32_t bitSize = slcData[currSlcIdx].BitSize;
            uint32_t offSet  = slcData[currSlcIdx].SliceOffset;
            PBSBuffer pBsBuffer = &(m_hevcBasicFeature->m_bsBuffer);

            if (m_hevcBasicFeature->m_hevcSeqParams->SliceSizeControl)
            {
                pakInsertParams.bLastHeader                = false;
                pakInsertParams.bEmulationByteBitsInsert   = false;
                bitSize                                    = m_hevcBasicFeature->m_hevcSliceParams->BitLengthSliceHeaderStartingPortion;
                pakInsertParams.bResetBitstreamStartingPos = true;
                pakInsertParams.dwPadding                  = (MOS_ALIGN_CEIL((bitSize + 7) >> 3, sizeof(uint32_t))) / sizeof(uint32_t);
                pakInsertParams.dataBitsInLastDw           = bitSize % 32;
                if (pakInsertParams.dataBitsInLastDw == 0)
                {
                    pakInsertParams.dataBitsInLastDw = 32;
                }

                m_hcpItf->MHW_ADDCMD_F(HCP_PAK_INSERT_OBJECT)(&constructedCmdBuf);
                uint32_t byteSize = (bitSize + 7) >> 3;
                m_hevcBasicFeature->m_vdencBatchBufferPerSliceVarSize[m_hevcBasicFeature->m_curNumSlices] += (MOS_ALIGN_CEIL(byteSize, sizeof(uint32_t))) / sizeof(uint32_t) * 4;
                if (byteSize)
                {
                    ENCODE_CHK_NULL_RETURN(pBsBuffer);
                    ENCODE_CHK_NULL_RETURN(pBsBuffer->pBase);
                    uint8_t *data = (uint8_t *)(pBsBuffer->pBase + offSet);
                    ENCODE_CHK_STATUS_RETURN(Mhw_AddCommandCmdOrBB(m_osInterface, &constructedCmdBuf, nullptr, data, byteSize));
                }

                // Send HCP_PAK_INSERT_OBJ command. For dynamic slice, we are skipping the beginning part of slice header.
                pakInsertParams.bLastHeader = true;
                bitSize = slcData[currSlcIdx].BitSize - m_hevcBasicFeature->m_hevcSliceParams->BitLengthSliceHeaderStartingPortion;
                offSet += ((m_hevcBasicFeature->m_hevcSliceParams->BitLengthSliceHeaderStartingPortion + 7) / 8);
                pakInsertParams.dwPadding        = (MOS_ALIGN_CEIL((bitSize + 7) >> 3, sizeof(uint32_t))) / sizeof(uint32_t);
                pakInsertParams.dataBitsInLastDw = bitSize % 32;
                if (pakInsertParams.dataBitsInLastDw == 0)
                {
                    pakInsertParams.dataBitsInLastDw = 32;
                }
                pakInsertParams.bResetBitstreamStartingPos = true;
                cmdBufOffset                               = constructedCmdBuf.iOffset;
                m_hcpItf->MHW_ADDCMD_F(HCP_PAK_INSERT_OBJECT)(&constructedCmdBuf);
                byteSize = (bitSize + 7) >> 3;
                if (byteSize)
                {
                    ENCODE_CHK_NULL_RETURN(pBsBuffer);
                    ENCODE_CHK_NULL_RETURN(pBsBuffer->pBase);
                    uint8_t *data = (uint8_t *)(pBsBuffer->pBase + offSet);
                    ENCODE_CHK_STATUS_RETURN(Mhw_AddCommandCmdOrBB(m_osInterface, &constructedCmdBuf, nullptr, data, byteSize));
                }
                m_hevcBasicFeature->m_vdencBatchBufferPerSliceVarSize[m_hevcBasicFeature->m_curNumSlices] += (constructedCmdBuf.iOffset - cmdBufOffset);
            }
            else
            {
                pakInsertParams.dwPadding        = (MOS_ALIGN_CEIL((bitSize + 7) >> 3, sizeof(uint32_t))) / sizeof(uint32_t);
                pakInsertParams.dataBitsInLastDw = bitSize % 32;
                if (pakInsertParams.dataBitsInLastDw == 0)
                {
                    pakInsertParams.dataBitsInLastDw = 32;
                }
                m_hcpItf->MHW_ADDCMD_F(HCP_PAK_INSERT_OBJECT)(&constructedCmdBuf);
                uint32_t byteSize = (bitSize + 7) >> 3;
                m_hevcBasicFeature->m_vdencBatchBufferPerSliceVarSize[m_hevcBasicFeature->m_curNumSlices] += (MOS_ALIGN_CEIL(byteSize, sizeof(uint32_t))) / sizeof(uint32_t) * 4;
                if (byteSize)
                {
                    ENCODE_CHK_NULL_RETURN(pBsBuffer);
                    ENCODE_CHK_NULL_RETURN(pBsBuffer->pBase);
                    uint8_t *data = (uint8_t *)(pBsBuffer->pBase + offSet);
                    ENCODE_CHK_STATUS_RETURN(Mhw_AddCommandCmdOrBB(m_osInterface, &constructedCmdBuf, nullptr, data, byteSize));
                }
            }

            cmdBufOffset = constructedCmdBuf.iOffset;
            if (slcCount == 0)
            {
                m_weightsOffsetsStateOffsetInBytes = constructedCmdBuf.iOffset;
            }
            SETPAR_AND_ADDCMD(VDENC_WEIGHTSOFFSETS_STATE, m_vdencItf, &constructedCmdBuf);

            {
                MHW_BATCH_BUFFER TempBBEnd    = {};
                TempBBEnd.iSize               = MOS_ALIGN_CEIL(m_hwInterface->m_vdencReadBatchBufferSize, CODECHAL_PAGE_SIZE);
                TempBBEnd.pData               = m_batchbufferAddr;
                TempBBEnd.iCurrent            = constructedCmdBuf.iOffset;
                TempBBEnd.iRemaining          = constructedCmdBuf.iRemaining;
                ENCODE_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(nullptr, &TempBBEnd));
                constructedCmdBuf.pCmdPtr    += (TempBBEnd.iCurrent - constructedCmdBuf.iOffset) / 4;
                constructedCmdBuf.iOffset     = TempBBEnd.iCurrent;
                constructedCmdBuf.iRemaining  = TempBBEnd.iRemaining;
            }
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_NOOP)(&constructedCmdBuf));

            if (slcCount == 0)
            {
                m_tileSliceStateOffsetInBytes = constructedCmdBuf.iOffset;
            }
            m_hevcBasicFeature->m_vdencBatchBufferTileSliceStart[slcCount] = constructedCmdBuf.iOffset;
            SETPAR_AND_ADDCMD(VDENC_HEVC_VP9_TILE_SLICE_STATE, m_vdencItf, &constructedCmdBuf);
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
            m_hevcBasicFeature->m_vdencBatchBufferPerSliceVarSize[slcCount] += m_alignSize[slcCount];
            m_hevcBasicFeature->m_vdencBatchBufferPerSlicePart2Size[slcCount] = constructedCmdBuf.iOffset - m_hevcBasicFeature->m_vdencBatchBufferPerSlicePart2Start[slcCount];
            startLCU += m_hevcBasicFeature->m_hevcSliceParams[slcCount].NumLCUsInSlice;
        }

        m_slbDataSizeInBytes = constructedCmdBuf.iOffset;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HEVCHucSLBBUpdatePkt::SetDmem() const
    {
        ENCODE_FUNC_CALL();
        
        // Null pointer checks
        ENCODE_CHK_NULL_RETURN(m_hevcBasicFeature);
        ENCODE_CHK_NULL_RETURN(m_hevcBasicFeature->m_hevcPicParams);
        ENCODE_CHK_NULL_RETURN(m_hevcBasicFeature->m_hevcSeqParams);
        ENCODE_CHK_NULL_RETURN(m_allocator);
        ENCODE_CHK_NULL_RETURN(m_pipeline);
        ENCODE_CHK_NULL_RETURN(m_hwInterface);
        ENCODE_CHK_NULL_RETURN(m_hcpItf);
        ENCODE_CHK_NULL_RETURN(m_vdencItf);
        ENCODE_CHK_NULL_RETURN(m_miItf);

        // Get buffer index
        const uint32_t bufIdx = m_pipeline->m_currRecycledBufIdx;

        // Lock DMEM buffer for writing
        auto dmem = (HucHevcSlbbUpdateDmem *)m_allocator->LockResourceForWrite(const_cast<MOS_RESOURCE*>(&m_slbbUpdateDmemBuffer[bufIdx]));
        ENCODE_CHK_NULL_RETURN(dmem);

        // Zero-initialize DMEM buffer
        MOS_ZeroMemory(dmem, sizeof(HucHevcSlbbUpdateDmem));

        // Validate structure size matches HuC DMEM layout
        ENCODE_ASSERT(sizeof(HucHevcSlbbUpdateDmem) == 66);

        // Calculate SLBB Size
        dmem->slbbSize = (uint16_t)m_slbDataSizeInBytes;
        ENCODE_CHK_COND_RETURN((m_slbDataSizeInBytes > UINT16_MAX), "SLBB size exceeds uint16_t range");

        // Group1: offsets recorded at SLBB construction time
        dmem->mfxWaitOffset           = 0;   // MFX_WAIT always starts at byte 0
        dmem->hcpPipeModeSelectOffset = (uint16_t)m_miItf->MHW_GETSIZE_F(MFX_WAIT)(); // follows first MFX_WAIT (fixed 4-byte MI cmd)
        dmem->group1EndOffset         = (uint16_t)m_group1EndOffset;

        // Group2: offsets recorded at SLBB construction time
        dmem->vdencCmd1Offset   = (uint16_t)m_group1EndOffset;
        dmem->hcpPicStateOffset = (uint16_t)m_hevcBasicFeature->m_picStateCmdStartInBytes;
        dmem->vdencCmd2Offset   = (uint16_t)m_cmd2StartInBytes;
        dmem->group2EndOffset   = (uint16_t)m_group2EndOffset;

        // currentOffset starts at Group3 boundary for subsequent Group3 offset calculations
        uint32_t currentOffset = m_group2EndOffset;

        // Calculate Group3 Command Offsets (first slice only)
        // Note: Group3 contains variable-size commands, so we calculate offsets for the first slice
        // and mark positions where HuC kernel will need to handle variable data
        
        // Check if weighted prediction is enabled (must match ConstructGroup3Cmds logic)
        auto wpFeature = dynamic_cast<HevcVdencWeightedPred *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcVdencWpFeature));
        bool wpEnabled = (wpFeature != nullptr) && wpFeature->IsEnabled() && 
                        m_hevcBasicFeature->m_hevcPicParams->bEnableGPUWeightedPrediction;
        
        if (wpEnabled)
        {
            // Check overflow before assignment
            ENCODE_CHK_COND_RETURN((currentOffset > UINT16_MAX), "Group3 hcpWeightOffsetStateOffset exceeds uint16_t range");
            dmem->hcpWeightOffsetStateOffset = (uint16_t)currentOffset;
            
            // Add HCP_WEIGHTOFFSET_STATE for P/B frames (LIST_0)
            if (m_hevcBasicFeature->m_hevcPicParams->CodingType == P_TYPE ||
                m_hevcBasicFeature->m_hevcPicParams->CodingType == B_TYPE)
            {
                currentOffset += m_hcpItf->MHW_GETSIZE_F(HCP_WEIGHTOFFSET_STATE)();
            }
            
            // Add second HCP_WEIGHTOFFSET_STATE for B frames (LIST_1)
            if (m_hevcBasicFeature->m_hevcPicParams->CodingType == B_TYPE)
            {
                currentOffset += m_hcpItf->MHW_GETSIZE_F(HCP_WEIGHTOFFSET_STATE)();
            }
        }
        else
        {
            dmem->hcpWeightOffsetStateOffset = 0; // Not present
        }
        
        // HCP_SLICE_STATE offset
        ENCODE_CHK_COND_RETURN((currentOffset > UINT16_MAX), "Group3 hcpSliceStateOffset exceeds uint16_t range");
        dmem->hcpSliceStateOffset = (uint16_t)currentOffset;
        currentOffset += m_hcpItf->MHW_GETSIZE_F(HCP_SLICE_STATE)();
        
        // MI_BATCH_BUFFER_END after HCP_SLICE_STATE
        currentOffset += m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)();
        
        // HCP_PAK_INSERT_OBJECT offset (marks start of variable-size slice header insertion)
        // Note: This offset points to where HCP_PAK_INSERT_OBJECT command starts
        // The actual command size varies based on slice header bitstream data
        // HuC kernel will handle the variable portion
        ENCODE_CHK_COND_RETURN((currentOffset > UINT16_MAX), "Group3 hcpPakInsertObjectOffset exceeds uint16_t range");
        dmem->hcpPakInsertObjectOffset = (uint16_t)currentOffset;
        
        // Use actual byte offsets recorded during ConstructGroup3Cmds instead of
        // calculated sizes — HCP_PAK_INSERT_OBJECT has variable-length inline data
        // that MHW_GETSIZE_F does not account for.
        ENCODE_CHK_COND_RETURN((m_weightsOffsetsStateOffsetInBytes > UINT16_MAX), "Group3 vdencWeightsOffsetsStateOffset exceeds uint16_t range");
        dmem->vdencWeightsOffsetsStateOffset = (uint16_t)m_weightsOffsetsStateOffsetInBytes;

        ENCODE_CHK_COND_RETURN((m_tileSliceStateOffsetInBytes > UINT16_MAX), "Group3 vdencHevcVp9TileSliceStateOffset exceeds uint16_t range");
        dmem->vdencHevcVp9TileSliceStateOffset = (uint16_t)m_tileSliceStateOffsetInBytes;

        currentOffset = m_tileSliceStateOffsetInBytes + m_vdencItf->MHW_GETSIZE_F(VDENC_HEVC_VP9_TILE_SLICE_STATE)();
        
        // MI_BATCH_BUFFER_END after variable commands
        currentOffset += m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)();
        
        // Group3 end offset (check overflow before assignment)
        ENCODE_CHK_COND_RETURN((currentOffset > UINT16_MAX), "Group3 end offset exceeds uint16_t range");
        dmem->group3EndOffset = (uint16_t)currentOffset;
        
        // Note: Group3 actual size will be larger due to variable-size HCP_PAK_INSERT_OBJECT data
        // The offsets calculated here represent the command structure for the first slice
        // HuC kernel uses these offsets as reference points and adjusts for variable data

        // Populate Command Metadata
        ENCODE_CHK_COND_RETURN((m_numTiles > UINT16_MAX), "Tile count exceeds uint16_t range");
        dmem->tileNum = (uint16_t)m_numTiles;

        // Populate Encoding Parameters
        dmem->codingType = (uint8_t)m_hevcBasicFeature->m_hevcPicParams->CodingType;
        
        // Clamp QP to valid range (0-51)
        int32_t qpY   = m_hevcBasicFeature->m_hevcPicParams->QpY + m_hevcBasicFeature->m_hevcSliceParams->slice_qp_delta;
        dmem->qpValue = (uint8_t)MOS_CLAMP_MIN_MAX(qpY, 0, 51);
        
        // Set low delay mode — use per-frame IsLowDelay() (based on reference POCs)
        // not m_hevcSeqParams->LowDelayMode (static sequence parameter).
        // The CMD2 lambda receives m_ref.IsLowDelay() which returns true for I_TYPE
        // (no references) even when LowDelayMode=0 (Random Access B).
        dmem->lowDelayMode = (uint8_t)(m_hevcBasicFeature->m_ref.IsLowDelay() ? 1 : 0);

        // Set sequence-level LowDelayMode (used for CMD2 predictor priority swap)
        // This differs from per-frame IsLowDelay() above: SeqParams->LowDelayMode is
        // 0 for Random Access B (BRefType > 0) regardless of individual frame refs.
        dmem->seqLowDelayMode = (uint8_t)(m_hevcBasicFeature->m_hevcSeqParams->LowDelayMode ? 1 : 0);

        // Set target usage (1-7 range)
        dmem->targetUsage = (uint8_t)m_hevcBasicFeature->m_hevcSeqParams->TargetUsage;
        
        // Set GOP size (distance between anchor frames)
        uint32_t gopRefDist = m_hevcBasicFeature->m_hevcSeqParams->GopRefDist;
        ENCODE_CHK_COND_RETURN((gopRefDist > UINT16_MAX), "GOP reference distance exceeds uint16_t range");
        dmem->bGopSize = (uint16_t)gopRefDist;
        
        // Calculate encoding depth from HierarchLevelPlus1
        dmem->depth = m_hevcBasicFeature->m_hevcPicParams->HierarchLevelPlus1 ? 
                     (uint8_t)(m_hevcBasicFeature->m_hevcPicParams->HierarchLevelPlus1 - 1) : 0;
        
        // Populate new CMD1-related fields
        dmem->numROI = (uint8_t)(m_hevcBasicFeature->m_hevcPicParams->NumROI > 0 ? 1 : 0);
        dmem->rateControlMethod = (uint8_t)m_hevcBasicFeature->m_hevcSeqParams->RateControlMethod;
        dmem->scenarioInfo = (uint8_t)m_hevcBasicFeature->m_hevcSeqParams->ScenarioInfo;

        // Populate rounding raw parameters (computation ported to HuC)
        dmem->roundingFlags = (uint8_t)(
            (m_hevcBasicFeature->m_hevcPicParams->CustomRoundingOffsetsParams.fields.EnableCustomRoudingIntra ? 1 : 0) |
            (m_hevcBasicFeature->m_hevcPicParams->CustomRoundingOffsetsParams.fields.EnableCustomRoudingInter ? 2 : 0) |
            (m_hevcBasicFeature->m_hevcSeqParams->HierarchicalFlag ? 4 : 0));
        dmem->customRoundingOffsetIntra = (uint8_t)m_hevcBasicFeature->m_hevcPicParams->CustomRoundingOffsetsParams.fields.RoundingOffsetIntra;
        dmem->customRoundingOffsetInter = (uint8_t)m_hevcBasicFeature->m_hevcPicParams->CustomRoundingOffsetsParams.fields.RoundingOffsetInter;

        // RDOQ for CMD2 (DW55-DW60): m_hevcRdoqEnabled in const_settings is always false
        // in the softlet pipeline (never assigned). The CQP feature's m_rdoqEnable is a
        // separate flag used for CMD1 parameters only.
        dmem->rdoqEnabled              = (uint8_t)0;
        dmem->ppsCurrPicRefEnabled     = (uint8_t)(m_hevcBasicFeature->m_hevcPicParams->pps_curr_pic_ref_enabled_flag ? 1 : 0);
        dmem->paletteModeEnabled       = (uint8_t)(m_hevcBasicFeature->m_hevcSeqParams->palette_mode_enabled_flag ? 1 : 0);
        dmem->chromaFormatIdc          = (uint8_t)m_hevcBasicFeature->m_hevcSeqParams->chroma_format_idc;
        dmem->roundingPrecisionEnabled = (uint8_t)1;  // Default enabled (matching EncodeHevcVdencConstSettings default)
        dmem->numRefL0                 = (uint8_t)(m_hevcBasicFeature->m_hevcSliceParams->num_ref_idx_l0_active_minus1 + 1);
        dmem->numRefL1                 = (uint8_t)(m_hevcBasicFeature->m_hevcSliceParams->num_ref_idx_l1_active_minus1 + 1);
        dmem->bitDepthLumaMinus8       = (uint8_t)m_hevcBasicFeature->m_hevcSeqParams->bit_depth_luma_minus8;
        // SCC tile slice state params for HuC SLBB processing
        // Note: bitDepthLumaMinus8 and paletteModeEnabled already set above (shared with CMD2)
        dmem->sliceQpDelta      = (uint8_t)(int8_t)m_hevcBasicFeature->m_hevcSliceParams->slice_qp_delta;

        dmem->AdaptiveTUEnabled = m_hevcBasicFeature->m_hevcPicParams->AdaptiveTUEnabled;

        // LA pass marker: tells HuC to keep Region 0 cost/cmd2 written by driver.
        dmem->lookAheadPhase = (m_hevcBasicFeature->m_hevcSeqParams->LookaheadDepth > 0 &&
                                m_hevcBasicFeature->m_hevcSeqParams->bLookAheadPhase) ? 1 : 0;

        // Extended platform flag: 0 = HuC writes the vendor-specific CMD2 bit from TU table (base platform).
        // 1 = HuC preserves driver pre-fill (extended platform).
        // Subclasses override IsExtendedPlatform() to return true on extended-platform packets.
        dmem->ExtendedPlatform = IsExtendedPlatform() ? 1 : 0;

        // Wa_16025947269: gates the protected VDENC_CMD2 DW52 bit at TU1/TU2.
        // 1 = WA active, HuC keeps the TU-table value; 0 = HW fix in place, HuC forces 0.
        auto osInterface = m_hwInterface->GetOsInterface();
        auto waTable     = osInterface ? osInterface->pfnGetWaTable(osInterface) : nullptr;
        dmem->Wa_16025947269 = (waTable != nullptr) && MEDIA_IS_WA(waTable, Wa_16025947269) ? 1 : 0;

        // Unlock DMEM buffer
        ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(const_cast<MOS_RESOURCE*>(&m_slbbUpdateDmemBuffer[bufIdx])));

        return MOS_STATUS_SUCCESS;
    }

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS HEVCHucSLBBUpdatePkt::DumpInput()
{
    ENCODE_FUNC_CALL();

    CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
    ENCODE_CHK_NULL_RETURN(debugInterface);

    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpHucDmem(
        &m_slbbUpdateDmemBuffer[m_pipeline->m_currRecycledBufIdx],
        sizeof(HucHevcSlbbUpdateDmem),
        0,
        hucRegionDumpSlbbUpdate));

    ENCODE_CHK_STATUS_RETURN(DumpRegion(0, "_InputSLBB", true, hucRegionDumpSlbbUpdate,
        MOS_ALIGN_CEIL(m_hwInterface->m_vdencReadBatchBufferSize, CODECHAL_PAGE_SIZE)));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HEVCHucSLBBUpdatePkt::DumpOutput()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(DumpRegion(1, "_OutputSLBB", false, hucRegionDumpSlbbUpdate,
        MOS_ALIGN_CEIL(m_hwInterface->m_vdencReadBatchBufferSize, CODECHAL_PAGE_SIZE)));

    return MOS_STATUS_SUCCESS;
}
#endif

    MHW_SETPAR_DECL_SRC(HCP_PIPE_MODE_SELECT, HEVCHucSLBBUpdatePkt)
    {
        params.codecStandardSelect = CODEC_STANDARD_SELECT_HEVC;
        params.bStreamOutEnabled   = true;
        params.bVdencEnabled       = true;
        params.codecSelect         = 1;
        params.multiEngineMode     = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY;
        params.pipeWorkMode        = MHW_VDBOX_HCP_PIPE_WORK_MODE_LEGACY;

        MhwCpInterface *cpInterface     = m_hwInterface->GetCpInterface();
        bool            twoPassScalable = false;

        ENCODE_CHK_NULL_RETURN(cpInterface);
        params.setProtectionSettings = [=](uint32_t *data) { return cpInterface->SetProtectionSettingsForHcpPipeModeSelect(data, twoPassScalable); };

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HUC_IMEM_STATE, HEVCHucSLBBUpdatePkt)
    {
        params.kernelDescriptor = m_vdboxHucHevcSlbbUpdateKernelDescriptor;
        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HUC_VIRTUAL_ADDR_STATE, HEVCHucSLBBUpdatePkt)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(m_hevcBasicFeature);

        params.function = SLBB_UPDATE;

        const uint32_t bufIdx = m_pipeline->m_currRecycledBufIdx;

        // Region 0 - Input SLB Buffer (Input Origin)
        int32_t currentPass = m_pipeline->GetCurrentPass();
        if (currentPass < 0)
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }
        MOS_RESOURCE *inputBuffer = m_hevcBasicFeature->GetVdencReadBatchBufferOrigin(bufIdx, currentPass);
        ENCODE_CHK_NULL_RETURN(inputBuffer);
        params.regionParams[0].presRegion = inputBuffer;

        // Region 1 - Output SLBB Buffer (HuC firmware writes to HUC_REGION1)
        auto vdenc2ndLevelBatchBuffer = m_hevcBasicFeature->GetVdenc2ndLevelBatchBuffer(bufIdx);
        ENCODE_CHK_NULL_RETURN(vdenc2ndLevelBatchBuffer);
        params.regionParams[1].presRegion = &vdenc2ndLevelBatchBuffer->OsResource;
        params.regionParams[1].isWritable = true;

        // Region 2/3 - TU7 SLBB Buffer (AdaptiveTU: separate input and output buffers)
        if (m_hevcBasicFeature->m_hevcPicParams->AdaptiveTUEnabled != 0)
        {
            MOS_RESOURCE *tu7InputBuffer = m_hevcBasicFeature->GetVdencReadBatchBufferTU7(bufIdx, currentPass);
            ENCODE_CHK_NULL_RETURN(tu7InputBuffer);
            params.regionParams[2].presRegion = tu7InputBuffer;

            auto tu7OutputBuffer = m_hevcBasicFeature->GetVdenc2ndLevelBatchBufferTU7(bufIdx);
            ENCODE_CHK_NULL_RETURN(tu7OutputBuffer);
            params.regionParams[3].presRegion = &tu7OutputBuffer->OsResource;
            params.regionParams[3].isWritable = true;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_HEVC_VP9_TILE_SLICE_STATE, HEVCHucSLBBUpdatePkt)
    {
        switch (m_pipeline->GetPipeNum())
        {
        case 0:
        case 1:
            params.numPipe = VDENC_PIPE_SINGLE_PIPE;
            break;
        case 2:
            params.numPipe = VDENC_PIPE_TWO_PIPE;
            break;
        case 4:
            params.numPipe = VDENC_PIPE_FOUR_PIPE;
            break;
        default:
            params.numPipe = VDENC_PIPE_INVALID;
            ENCODE_ASSERT(false);
            break;
        }
        return MOS_STATUS_SUCCESS;
    }
}