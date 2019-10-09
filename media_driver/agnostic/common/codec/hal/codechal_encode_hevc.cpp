/*
* Copyright (c) 2016-2017, Intel Corporation
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
//! \file     codechal_encode_hevc.cpp
//! \brief    Defines base class for HEVC dual-pipe encoder.
//!

#include "codechal_encode_hevc.h"
#include "codechal_mmc_encode_hevc.h"

uint32_t CodechalEncHevcState::GetStartCodeOffset(uint8_t* addr, uint32_t size)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint32_t count = 0;

    if (addr)
    {
        // count # 0 bytes before end of start code to determine header offset
        // (ie. 00 00 00 00 01 = 4)
        while (count < size && *addr != 0x01)
        {
            // Damage control if start code is non-existent or malformed
            // so header will still be a reasonable size
            if (*addr != 0)
                break;

            count++;
            addr++;
        }
    }

    return count + 1; // +1 to account for 01 byte
}

uint32_t CodechalEncHevcState::GetPicHdrSize()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint32_t firstHdrSz = 0;
    for(auto i = 0 ; i < HEVC_MAX_NAL_UNIT_TYPE ; i++)
    {
        if (m_nalUnitParams[i]->uiSize != 0)
        {
            firstHdrSz = m_nalUnitParams[i]->uiSize;
            break;
        }
    }

    uint8_t* hdrPtr = m_bsBuffer.pBase;
    // hdr offset = # 0's + 01 byte (start code) + offset over temporal ID and NAL type byte
    uint32_t hdrBegin = GetStartCodeOffset(hdrPtr, firstHdrSz) + HEVC_START_CODE_NAL_OFFSET;

    uint32_t accum = 0, numEmuBytes = 0;
    for(auto i = 0 ; i < HEVC_MAX_NAL_UNIT_TYPE ; i++)
    {
        if (m_nalUnitParams[i]->uiSize == 0)
            continue;
        uint32_t origSize = m_nalUnitParams[i]->uiSize;

        if (m_hevcPicParams->bEmulationByteInsertion)
        {
            hdrPtr = m_bsBuffer.pBase + accum;
            uint32_t hdrOffset = GetStartCodeOffset(hdrPtr, origSize);
            hdrPtr += hdrOffset;

            uint32_t zeroCount = 0;
            for (uint32_t j = 0 ; j < origSize - hdrOffset ; j++)
            {
                // Check if Emulation Prevention Byte needed for hex 00 00 00/00 00 01/00 00 02/00 00 03
                if (zeroCount == 2 && !(*hdrPtr & 0xFC))
                {
                    zeroCount = 0;
                    numEmuBytes++;   //increment by prevention byte
                }

                if (*hdrPtr == 0x00)
                {
                    zeroCount++;
                }
                else
                {
                    zeroCount = 0;
                }

                *hdrPtr++;
            }
        }

        accum += origSize;
    }

    // Add emulation bytes found
    accum += numEmuBytes;

    // Make sure that header size is valid or cap to provided hdr size
    hdrBegin = MOS_MIN(hdrBegin, accum);

    // Hdr size is in # bits
    return (accum - hdrBegin) * 8;
}

MOS_STATUS CodechalEncHevcState::ValidateRefFrameData(PCODEC_HEVC_ENCODE_SLICE_PARAMS slcParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(slcParams);

    uint8_t maxNumRef0 = 0;
    uint8_t maxNumRef1 = 0;

    GetMaxRefFrames(maxNumRef0, maxNumRef1);

    if (slcParams->num_ref_idx_l0_active_minus1 > maxNumRef0 - 1)
    {
        CODECHAL_ENCODE_ASSERT(false);
        slcParams->num_ref_idx_l0_active_minus1 = maxNumRef0 - 1;
    }

    if (slcParams->num_ref_idx_l1_active_minus1 > maxNumRef1 - 1)
    {
        CODECHAL_ENCODE_ASSERT(false);
        slcParams->num_ref_idx_l1_active_minus1 = maxNumRef1 - 1;
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcState::SetSequenceStructs()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeHevcBase::SetSequenceStructs());

    m_cqpEnabled = (m_hevcSeqParams->RateControlMethod == RATECONTROL_CQP);

    if (m_hevcSeqParams->ParallelBRC == false)
    {
        m_brcBuffers.uiCurrBrcPakStasIdxForRead = m_brcBuffers.uiCurrBrcPakStasIdxForWrite = 0;
    }

    // check LCU size
    if (m_2xMeSupported &&
        m_hevcSeqParams->log2_max_coding_block_size_minus3 == 3)
    {
        // LCU64 support
        m_isMaxLcu64       = true;
        m_2xScalingEnabled = true;
    }
    else
    {
        if (m_hevcSeqParams->log2_max_coding_block_size_minus3 != 2)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Invalid LCU.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }

        m_isMaxLcu64       = false;
        m_2xScalingEnabled = false;
    }

    // allocate resources only needed in LCU64 kernel
    if (m_firstFrame && m_isMaxLcu64)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateEncResourcesLCU64());
    }

    if (m_hevcSeqParams->FrameRate.Denominator == 0)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CODECHAL_ENCODE_ASSERTMESSAGE("FrameRate Denominator can not be zero.");
        return eStatus;
    }
    uint8_t framerate = m_hevcSeqParams->FrameRate.Numerator / m_hevcSeqParams->FrameRate.Denominator;

    m_slidingWindowSize = MOS_MIN(framerate, 60);

    return eStatus;
}

MOS_STATUS CodechalEncHevcState::SetPictureStructs()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeHevcBase::SetPictureStructs());

    // do not support interlaced coding now
    if (CodecHal_PictureIsField(m_currOriginalPic))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

        // Set min/max QP values based on frame type if atleast one of them is non-zero
    if (m_hevcPicParams->BRCMinQp || m_hevcPicParams->BRCMaxQp)
    {
        m_minMaxQpControlEnabled = true;
        if (m_hevcPicParams->CodingType == I_TYPE)
        {
            m_maxQpForI = MOS_MIN(MOS_MAX(m_hevcPicParams->BRCMaxQp, 1), 51);           // Clamp to the max QP to [1, 51] . Zero is not used by our Kernel.
            m_minQpForI = MOS_MIN(MOS_MAX(m_hevcPicParams->BRCMinQp, 1), m_maxQpForI);  // Clamp the min QP to [1, maxQP] to make sure minQP <= maxQP
            if (!m_minMaxQpControlForP)
            {
                m_minQpForP = m_minQpForI;
                m_maxQpForP = m_maxQpForI;
            }
            if (!m_minMaxQpControlForB)
            {
                m_minQpForB = m_minQpForI;
                m_maxQpForB = m_maxQpForI;
            }
        }
        else if (m_hevcPicParams->CodingType == P_TYPE)
        {
            m_minMaxQpControlForP = true;
            m_maxQpForP           = MOS_MIN(MOS_MAX(m_hevcPicParams->BRCMaxQp, 1), 51);           // Clamp to the max QP to [1, 51]. Zero is not used by our Kernel.
            m_minQpForP           = MOS_MIN(MOS_MAX(m_hevcPicParams->BRCMinQp, 1), m_maxQpForP);  // Clamp the min QP to [1, maxQP] to make sure minQP <= maxQP
            if (!m_minMaxQpControlForB)
            {
                m_minQpForB = m_minQpForP;
                m_maxQpForB = m_maxQpForP;
            }
        }
        else if (m_hevcPicParams->CodingType == B_TYPE)
        {
            m_minMaxQpControlForB = true;
            m_maxQpForB           = MOS_MIN(MOS_MAX(m_hevcPicParams->BRCMaxQp, 1), 51);           // Clamp to the max QP to [1, 51]. Zero is not used by our Kernel.
            m_minQpForB           = MOS_MIN(MOS_MAX(m_hevcPicParams->BRCMinQp, 1), m_maxQpForB);  // Clamp the min QP to [1, maxQP] to make sure minQP <= maxQP
        }
    }

    // CQP with Fast Surveillance [Distortion Surface needs to be allocated]
    if (m_brcEnabled || m_hevcSeqParams->bVideoSurveillance || m_cqpEnabled)
    {
        m_brcDistortion = (m_pictureCodingType == I_TYPE) ? &m_brcBuffers.sBrcIntraDistortionBuffer : &m_brcBuffers.sMeBrcDistortionBuffer;
    }

    if (m_brcEnabled)
    {
        // For ICQ mode or when min/max QP used, ignore BRCPrecision sent by the app and set the number of passes internally
        if ((m_hevcSeqParams->RateControlMethod == RATECONTROL_ICQ) || (m_minMaxQpControlEnabled))
        {
            m_numPasses = 0;  // no IPCM for HEVC
        }
        else
        {
            m_numPasses = (uint8_t)(m_mfxInterface->GetBrcNumPakPasses() - 1);  // 1 original plus extra to handle BRC
        }
    }
    else
    {
        m_numPasses = 0;  // no IPCM for HEVC
    }

    //add for FEI multiple Pass Pak
    if (CodecHalIsFeiEncode(m_codecFunction))
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_hevcFeiPicParams);
        if (m_hevcFeiPicParams->dwMaxFrameSize != 0)
        {
            m_numPasses = (uint8_t)m_hevcFeiPicParams->dwNumPasses;
        }
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcState::SetSliceStructs()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeHevcBase::SetSliceStructs());

    return eStatus;
}

MOS_STATUS CodechalEncHevcState::ReadBrcPakStats(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint32_t offset = (m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize) +
        m_encodeStatusBuf.dwNumPassesOffset +   // Num passes offset
        sizeof(uint32_t)* 2;                               // pEncodeStatus is offset by 2 DWs in the resource

    EncodeReadBrcPakStatsParams   readBrcPakStatsParams;
    readBrcPakStatsParams.pHwInterface               = m_hwInterface;
    readBrcPakStatsParams.presBrcPakStatisticBuffer  = &m_brcBuffers.resBrcPakStatisticBuffer[m_brcBuffers.uiCurrBrcPakStasIdxForWrite];
    readBrcPakStatsParams.presStatusBuffer           = &m_encodeStatusBuf.resStatusBuffer;
    readBrcPakStatsParams.dwStatusBufNumPassesOffset = offset;
    readBrcPakStatsParams.ucPass                     = (uint8_t)GetCurrentPass();
    readBrcPakStatsParams.VideoContext               = m_videoContext;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadBrcPakStatistics(
        cmdBuffer,
        &readBrcPakStatsParams));

    return eStatus;
}

MOS_STATUS CodechalEncHevcState::AddHcpPipeModeSelectCmd(MOS_COMMAND_BUFFER* cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams;
    SetHcpPipeModeSelectParams(pipeModeSelectParams);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPipeModeSelectCmd(cmdBuffer, &pipeModeSelectParams));

    return eStatus;
}

MOS_STATUS CodechalEncHevcState::AddHcpSurfaceStateCmds(MOS_COMMAND_BUFFER* cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_VDBOX_SURFACE_PARAMS srcSurfaceParams;
    SetHcpSrcSurfaceParams(srcSurfaceParams);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpSurfaceCmd(cmdBuffer, &srcSurfaceParams));

    MHW_VDBOX_SURFACE_PARAMS reconSurfaceParams;
    SetHcpReconSurfaceParams(reconSurfaceParams);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpSurfaceCmd(cmdBuffer, &reconSurfaceParams));

    return eStatus;
}

MOS_STATUS CodechalEncHevcState::AddHcpPictureStateCmd(MOS_COMMAND_BUFFER* cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_VDBOX_HEVC_PIC_STATE picStateParams;
    SetHcpPicStateParams(picStateParams);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPicStateCmd(cmdBuffer, &picStateParams));

    return eStatus;
}

MOS_STATUS CodechalEncHevcState::ExecutePictureLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifyCommandBufferSize());

    if (!m_singleTaskPhaseSupportedInPak)
    {
        // Command buffer or patch list size are too small and so we cannot submit multiple pass of PAKs together
        m_firstTaskInPhase = true;
        m_lastTaskInPhase  = true;
    }

    if (m_vdboxIndex > m_mfxInterface->GetMaxVdboxIndex())
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("ERROR - vdbox index exceed the maximum");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommandBuffer(&cmdBuffer));

    if ((!m_singleTaskPhaseSupported) || m_firstTaskInPhase)
    {
        // Send command buffer header at the beginning (OS dependent)
        // frame tracking tag is only added in the last command buffer header
        bool requestFrameTracking = m_singleTaskPhaseSupported ?
            m_firstTaskInPhase :
            m_lastTaskInPhase;

        // When brc + 2 pass sao is enabled need to disable frame tracking for first cmd buffer
        if (m_brcEnabled && m_hevcSeqParams->SAO_enabled_flag)
        {
            requestFrameTracking = false;
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(&cmdBuffer, requestFrameTracking));
    }

    // Enable frame tracking for the last cmd buffer when brc + 2 pass sao is on
    if (m_brcEnabled && m_hevcSeqParams->SAO_enabled_flag && m_currPass == m_uc2NdSaoPass)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(&cmdBuffer, true));
    }

    if (m_brcEnabled &&
        !IsFirstPass() &&
        m_currPass != m_uc2NdSaoPass)  // Only the regular BRC passes have the conditional batch buffer end
    {
        // Insert conditional batch buffer end
        MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS miConditionalBatchBufferEndParams;
        MOS_ZeroMemory(
            &miConditionalBatchBufferEndParams,
            sizeof(MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS));
        uint32_t baseOffset = (m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize) +
                sizeof(uint32_t) * 2;  // pEncodeStatus is offset by 2 DWs in the resource       ;

        CODECHAL_ENCODE_ASSERT((m_encodeStatusBuf.dwImageStatusMaskOffset & 7) == 0); // Make sure uint64_t aligned
        CODECHAL_ENCODE_ASSERT((m_encodeStatusBuf.dwImageStatusMaskOffset + sizeof(uint32_t)) == m_encodeStatusBuf.dwImageStatusCtrlOffset);

        miConditionalBatchBufferEndParams.presSemaphoreBuffer = &m_encodeStatusBuf.resStatusBuffer;
        miConditionalBatchBufferEndParams.dwOffset = baseOffset + m_encodeStatusBuf.dwImageStatusMaskOffset;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiConditionalBatchBufferEndCmd(
            &cmdBuffer,
            &miConditionalBatchBufferEndParams));

        auto mmioRegisters = m_hcpInterface->GetMmioRegisters(m_vdboxIndex);
        MHW_MI_STORE_REGISTER_MEM_PARAMS miStoreRegMemParams;
        MHW_MI_COPY_MEM_MEM_PARAMS miCpyMemMemParams;
        // Write back the HCP image control register for RC6 may clean it out
        MHW_MI_LOAD_REGISTER_MEM_PARAMS miLoadRegMemParams;
        MOS_ZeroMemory(&miLoadRegMemParams, sizeof(miLoadRegMemParams));
        miLoadRegMemParams.presStoreBuffer = &m_encodeStatusBuf.resStatusBuffer;
        miLoadRegMemParams.dwOffset = baseOffset + m_encodeStatusBuf.dwImageStatusCtrlOffset;
        miLoadRegMemParams.dwRegister = mmioRegisters->hcpEncImageStatusCtrlRegOffset;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterMemCmd(&cmdBuffer, &miLoadRegMemParams));

        MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
        miStoreRegMemParams.presStoreBuffer = &m_brcBuffers.resBrcPakStatisticBuffer[m_brcBuffers.uiCurrBrcPakStasIdxForWrite];
        miStoreRegMemParams.dwOffset = CODECHAL_OFFSETOF(CODECHAL_ENCODE_HEVC_PAK_STATS_BUFFER, HCP_IMAGE_STATUS_CONTROL_FOR_LAST_PASS);
        miStoreRegMemParams.dwRegister = mmioRegisters->hcpEncImageStatusCtrlRegOffset;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(&cmdBuffer, &miStoreRegMemParams));

        MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
        miStoreRegMemParams.presStoreBuffer =  &m_encodeStatusBuf.resStatusBuffer;
        miStoreRegMemParams.dwOffset = baseOffset + m_encodeStatusBuf.dwImageStatusCtrlOfLastBRCPassOffset;
        miStoreRegMemParams.dwRegister = mmioRegisters->hcpEncImageStatusCtrlRegOffset;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(&cmdBuffer, &miStoreRegMemParams));
    }

    if (IsFirstPass() && m_osInterface->bTagResourceSync)
    {
        // This is a short term solution to solve the sync tag issue: the sync tag write for PAK is inserted at the end of 2nd pass PAK BB
        // which may be skipped in multi-pass PAK enabled case. The idea here is to insert the previous frame's tag at the beginning
        // of the BB and keep the current frame's tag at the end of the BB. There will be a delay for tag update but it should be fine
        // as long as Dec/VP/Enc won't depend on this PAK so soon.

        MOS_RESOURCE globalGpuContextSyncTagBuffer;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetGpuStatusBufferResource(
            m_osInterface,
            &globalGpuContextSyncTagBuffer));

        MHW_MI_STORE_DATA_PARAMS params;
        params.pOsResource = &globalGpuContextSyncTagBuffer;
        params.dwResourceOffset = m_osInterface->pfnGetGpuStatusTagOffset(m_osInterface, m_osInterface->CurrentGpuContextOrdinal);
        uint32_t value = m_osInterface->pfnGetGpuStatusTag(m_osInterface, m_osInterface->CurrentGpuContextOrdinal);
        params.dwValue = (value > 0) ? (value - 1) : 0;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(&cmdBuffer, &params));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(StartStatusReport(&cmdBuffer, CODECHAL_NUM_MEDIA_STATES));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(AddHcpPipeModeSelectCmd(&cmdBuffer));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(AddHcpSurfaceStateCmds(&cmdBuffer));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(AddHcpPipeBufAddrCmd(&cmdBuffer));

    MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS indObjBaseAddrParams;
    SetHcpIndObjBaseAddrParams(indObjBaseAddrParams);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpIndObjBaseAddrCmd(&cmdBuffer, &indObjBaseAddrParams));

    MHW_VDBOX_QM_PARAMS fqmParams, qmParams;
    SetHcpQmStateParams(fqmParams, qmParams);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpFqmStateCmd(&cmdBuffer, &fqmParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpQmStateCmd(&cmdBuffer, &qmParams));

    if (m_brcEnabled)
    {
        if (m_hevcSeqParams->SAO_enabled_flag && m_currPass == m_uc2NdSaoPass)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(AddHcpPictureStateCmd(&cmdBuffer));
        }
        else
        {
            uint32_t picStateCmdOffset;
            picStateCmdOffset = GetCurrentPass();

            MHW_BATCH_BUFFER batchBuffer;
            MOS_ZeroMemory(&batchBuffer, sizeof(batchBuffer));
            batchBuffer.OsResource = m_brcBuffers.resBrcImageStatesWriteBuffer[m_currRecycledBufIdx];
            batchBuffer.dwOffset = picStateCmdOffset * (m_brcBuffers.dwBrcHcpPicStateSize / CODECHAL_ENCODE_BRC_MAXIMUM_NUM_PASSES);
            batchBuffer.bSecondLevel = true;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(
                &cmdBuffer,
                &batchBuffer));
        }
    }
    else
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AddHcpPictureStateCmd(&cmdBuffer));
    }

    // Send HEVC_VP9_RDOQ_STATE command
    if (m_hevcRdoqEnabled)
    {
        MHW_VDBOX_HEVC_PIC_STATE picStateParams;
        SetHcpPicStateParams(picStateParams);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpHevcVp9RdoqStateCmd(&cmdBuffer, &picStateParams));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReturnCommandBuffer(&cmdBuffer));

    return eStatus;
}

MOS_STATUS CodechalEncHevcState::AddHcpWeightOffsetStateCmd(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_BATCH_BUFFER               batchBuffer,
    PCODEC_HEVC_ENCODE_SLICE_PARAMS hevcSlcParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(hevcSlcParams);

    if (cmdBuffer == nullptr && batchBuffer == nullptr)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("There was no valid buffer to add the HW command to.");
        return MOS_STATUS_NULL_POINTER;
    }

    MHW_VDBOX_HEVC_WEIGHTOFFSET_PARAMS hcpWeightOffsetParams;
    MOS_ZeroMemory(&hcpWeightOffsetParams, sizeof(hcpWeightOffsetParams));

    for (auto k = 0; k < 2; k++) // k=0: LIST_0, k=1: LIST_1
    {
        // Luma, Chroma offset
        for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
        {
            hcpWeightOffsetParams.LumaOffsets[k][i] = (int16_t)hevcSlcParams->luma_offset[k][i];
            // Cb, Cr
            for (auto j = 0; j < 2; j++)
            {
                hcpWeightOffsetParams.ChromaOffsets[k][i][j] = (int16_t)hevcSlcParams->chroma_offset[k][i][j];
            }
        }

        // Luma Weight
        CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &hcpWeightOffsetParams.LumaWeights[k],
            sizeof(hcpWeightOffsetParams.LumaWeights[k]),
            &hevcSlcParams->delta_luma_weight[k],
            sizeof(hevcSlcParams->delta_luma_weight[k])));

        // Chroma Weight
        CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &hcpWeightOffsetParams.ChromaWeights[k],
            sizeof(hcpWeightOffsetParams.ChromaWeights[k]),
            &hevcSlcParams->delta_chroma_weight[k],
            sizeof(hevcSlcParams->delta_chroma_weight[k])));
    }

    if (hevcSlcParams->slice_type == CODECHAL_ENCODE_HEVC_P_SLICE || hevcSlcParams->slice_type == CODECHAL_ENCODE_HEVC_B_SLICE)
    {
        hcpWeightOffsetParams.ucList = LIST_0;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpWeightOffsetStateCmd(cmdBuffer, batchBuffer, &hcpWeightOffsetParams));
    }

    if (hevcSlcParams->slice_type == CODECHAL_ENCODE_HEVC_B_SLICE)
    {
        hcpWeightOffsetParams.ucList = LIST_1;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpWeightOffsetStateCmd(cmdBuffer, batchBuffer, &hcpWeightOffsetParams));
    }

    return eStatus;
}

//------------------------------------------------------------------------------------
// Build slices with header insertion
//------------------------------------------------------------------------------------
MOS_STATUS CodechalEncHevcState::SendHwSliceEncodeCommand(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_VDBOX_HEVC_SLICE_STATE     params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pHevcPicIdx);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->presDataBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pEncodeHevcSeqParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pEncodeHevcPicParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pEncodeHevcSliceParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pBsBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->ppNalUnitParams);

    PMHW_BATCH_BUFFER batchBufferInUse = nullptr;
    PMOS_COMMAND_BUFFER cmdBufferInUse = nullptr;

    if (params->bSingleTaskPhaseSupported)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(params->pBatchBufferForPakSlices);
        batchBufferInUse = params->pBatchBufferForPakSlices;
    }
    else
    {
        cmdBufferInUse = cmdBuffer;
    }

    // add HCP_REF_IDX command
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AddHcpRefIdxCmd(cmdBufferInUse, batchBufferInUse, params));

    if (params->bWeightedPredInUse)
    {
        //add weghtoffset command
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AddHcpWeightOffsetStateCmd(cmdBufferInUse, batchBufferInUse, m_hevcSliceParams));
    }

    // add HEVC Slice state commands
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpSliceStateCmd(cmdBufferInUse, params));

    // add HCP_PAK_INSERT_OBJECTS command
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AddHcpPakInsertNALUs(cmdBufferInUse, batchBufferInUse, params));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(AddHcpPakInsertSliceHeader(cmdBufferInUse, batchBufferInUse, params));

    if (params->bSingleTaskPhaseSupported && batchBufferInUse)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(nullptr, batchBufferInUse));

        MHW_BATCH_BUFFER secondLevelBatchBuffer;
        MOS_ZeroMemory(&secondLevelBatchBuffer, sizeof(MHW_BATCH_BUFFER));
        secondLevelBatchBuffer.OsResource = batchBufferInUse->OsResource;
        secondLevelBatchBuffer.dwOffset = params->dwBatchBufferForPakSlicesStartOffset;
        secondLevelBatchBuffer.bSecondLevel = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(cmdBuffer, &secondLevelBatchBuffer));
    }

    // Insert Batch Buffer Start command to send HCP_PAK_OBJ data for LCUs in this slice
    MHW_BATCH_BUFFER secondLevelBatchBuffer;
    MOS_ZeroMemory(&secondLevelBatchBuffer, sizeof(MHW_BATCH_BUFFER));
    secondLevelBatchBuffer.OsResource = *params->presDataBuffer;
    secondLevelBatchBuffer.dwOffset = params->dwDataBufferOffset;
    secondLevelBatchBuffer.bSecondLevel = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(cmdBuffer, &secondLevelBatchBuffer));

    return eStatus;
}

MOS_STATUS CodechalEncHevcState::ExecuteSliceLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_slcData);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetBatchBufferForPakSlices());

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommandBuffer(&cmdBuffer));

    SetHcpSliceStateCommonParams(*m_sliceStateParams);

    PCODEC_ENCODER_SLCDATA slcData = m_slcData;
    for (uint32_t startLCU = 0, SlcCount = 0; SlcCount < m_numSlices; SlcCount++)
    {
        if (m_currPass == 0)
        {
            slcData[SlcCount].CmdOffset = startLCU * (m_hcpInterface->GetHcpPakObjSize()) * sizeof(uint32_t);
        }

        SetHcpSliceStateParams(*m_sliceStateParams, slcData, SlcCount);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendHwSliceEncodeCommand(&cmdBuffer, m_sliceStateParams));

        startLCU += m_hevcSliceParams[SlcCount].NumLCUsInSlice;

        m_batchBufferForPakSlicesStartOffset =
            (uint32_t)m_batchBufferForPakSlices[m_currPakSliceIdx].iCurrent;
    }

    if (m_useBatchBufferForPakSlices)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(Mhw_UnlockBb(
            m_osInterface,
            &m_batchBufferForPakSlices[m_currPakSliceIdx],
            m_lastTaskInPhase));
    }

    // Insert end of sequence/stream if set
    if (m_lastPicInStream || m_lastPicInSeq)
    {
        MHW_VDBOX_PAK_INSERT_PARAMS pakInsertObjectParams;
        MOS_ZeroMemory(&pakInsertObjectParams, sizeof(pakInsertObjectParams));
        pakInsertObjectParams.bLastPicInSeq     = m_lastPicInSeq;
        pakInsertObjectParams.bLastPicInStream  = m_lastPicInStream;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPakInsertObject(&cmdBuffer, &pakInsertObjectParams));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadHcpStatus(&cmdBuffer));

    // BRC PAK statistics different for each pass
    if (m_brcEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadBrcPakStats(&cmdBuffer));
    }

    if (!Mos_ResourceIsNull(&m_resFrameStatStreamOutBuffer))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadSseStatistics(&cmdBuffer));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, CODECHAL_NUM_MEDIA_STATES));

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }

    std::string pakPassName = "PAK_PASS" + std::to_string(static_cast<uint32_t>(m_currPass));
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN( m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            pakPassName.data()));)

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReturnCommandBuffer(&cmdBuffer));

    if ((!m_pakOnlyTest) &&  // In the PAK only test, no need to wait for ENC's completion
        (m_currPass == 0) &&
        !Mos_ResourceIsNull(&m_resSyncObjectRenderContextInUse))
    {
        MOS_SYNC_PARAMS syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContext;
        syncParams.presSyncResource = &m_resSyncObjectRenderContextInUse;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
    }

    bool renderingFlags = m_videoContextUsesNullHw;

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, renderingFlags));

        CODECHAL_DEBUG_TOOL(
            if (m_mmcState)
            {
                m_mmcState->UpdateUserFeatureKey(&m_reconSurface);
            }
        )

        if ((m_currPass == m_numPasses) &&
            m_signalEnc &&
            m_currRefSync &&
            !Mos_ResourceIsNull(&m_currRefSync->resSyncObject))
        {
            // signal semaphore
            MOS_SYNC_PARAMS syncParams = g_cInitSyncParams;
            syncParams.GpuContext = m_videoContext;
            syncParams.presSyncResource = &m_currRefSync->resSyncObject;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));
            m_currRefSync->uiSemaphoreObjCount++;
            m_currRefSync->bInUsed = true;
        }
    }

    // Reset parameters for next PAK execution
    if (m_currPass == m_numPasses)
    {
        if (!m_singleTaskPhaseSupported)
        {
            m_osInterface->pfnResetPerfBufferID(m_osInterface);
        }

        m_currPakSliceIdx = (m_currPakSliceIdx + 1) % CODECHAL_HEVC_NUM_PAK_SLICE_BATCH_BUFFERS;

        if (m_hevcSeqParams->ParallelBRC)
        {
            m_brcBuffers.uiCurrBrcPakStasIdxForWrite =
                (m_brcBuffers.uiCurrBrcPakStasIdxForWrite + 1) % CODECHAL_ENCODE_RECYCLED_BUFFER_NUM;
        }

        m_newPpsHeader = 0;
        m_newSeqHeader = 0;
        m_frameNum++;
    }

    return eStatus;
}

//------------------------------------------------------------------------------
//| Purpose:    Retrieves the HCP registers and stores them in the status report
//| Return:     N/A
//------------------------------------------------------------------------------
MOS_STATUS CodechalEncHevcState::ReadHcpStatus(PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeHevcBase::ReadHcpStatus(cmdBuffer));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadImageStatus(cmdBuffer))

    return eStatus;
}

uint8_t CodechalEncHevcState::CalculateROIRatio()
{
    uint32_t roiSize = 0;
    for (uint32_t i = 0; i < m_hevcPicParams->NumROI; ++i)
    {
        roiSize += (ENCODE_DP_HEVC_ROI_BLOCK_Width * (MOS_ABS(m_hevcPicParams->ROI[i].Top - m_hevcPicParams->ROI[i].Bottom) + 1 )) *
                   (ENCODE_DP_HEVC_ROI_BLOCK_HEIGHT * (MOS_ABS(m_hevcPicParams->ROI[i].Right - m_hevcPicParams->ROI[i].Left) + 1));
    }

    uint32_t roiRatio = 0;
    if (roiSize)
    {
        uint32_t numMBs = m_picWidthInMb * m_picHeightInMb;
        roiRatio = 2 * (numMBs * 256 / roiSize - 1);
        roiRatio = MOS_MIN(51, roiRatio);
    }

    return (uint8_t)roiRatio;
}

int16_t CodechalEncHevcState::ComputeTemporalDifference(const CODEC_PICTURE& refPic)
{
    int16_t diff_poc = 0;

    if (!CodecHal_PictureIsInvalid(refPic))
    {
        diff_poc = m_hevcPicParams->CurrPicOrderCnt - m_hevcPicParams->RefFramePOCList[refPic.FrameIdx];

        if(diff_poc < -128)
        {
            diff_poc = -128;
        }
        else if(diff_poc > 127)
        {
            diff_poc = 127;
        }
    }

    return diff_poc;
}

MOS_STATUS CodechalEncHevcState::WaitForRefFrameReady(uint8_t mbCodeIdx)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (!m_refSync[mbCodeIdx].bInUsed)
    {
        return eStatus;
    }

    MOS_SYNC_PARAMS syncParams = g_cInitSyncParams;
    syncParams.GpuContext = m_renderContext;
    syncParams.presSyncResource = &m_refSync[mbCodeIdx].resSyncObject;
    syncParams.uiSemaphoreCount = m_refSync[mbCodeIdx].uiSemaphoreObjCount;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
    m_refSync[mbCodeIdx].uiSemaphoreObjCount = 0;
    m_refSync[mbCodeIdx].bInUsed             = false;

    return eStatus;
}

MOS_STATUS CodechalEncHevcState::WaitForPak()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_pictureCodingType == I_TYPE && !m_brcEnabled)
    {
        return eStatus;
    }

    if (!m_firstFrame && m_brcEnabled && !m_hevcSeqParams->ParallelBRC)
    {
        // When there is no parallel BRC, we still need to wait for previous PAK
        CODECHAL_ENCODE_CHK_STATUS_RETURN(WaitForRefFrameReady(m_lastMbCodeIndex));
        return eStatus;
    }

    // check all reference frames. If one of them has not be waited, then it needs to be wait and ensure it has been encoded completely.
    auto slcParams = m_hevcSliceParams;
    for (uint32_t s = 0; s < m_numSlices; s++, slcParams++)
    {
        for (auto ll = 0; ll < 2; ll++)
        {
            uint32_t uiNumRef = (ll == 0) ? slcParams->num_ref_idx_l0_active_minus1 :
                slcParams->num_ref_idx_l1_active_minus1;

            for (uint32_t i = 0; i <= uiNumRef; i++)
            {
                CODEC_PICTURE refPic = slcParams->RefPicList[ll][i];
                if (!CodecHal_PictureIsInvalid(refPic) &&
                    !CodecHal_PictureIsInvalid(m_hevcPicParams->RefFrameList[refPic.FrameIdx]))
                {
                    uint32_t idx       = m_hevcPicParams->RefFrameList[refPic.FrameIdx].FrameIdx;
                    uint8_t  mbCodeIdx = m_refList[idx]->ucMbCodeIdx;
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(WaitForRefFrameReady(mbCodeIdx));
                }
            }
        }
    }

    if (!m_firstTwoFrames && m_brcEnabled && m_hevcSeqParams->ParallelBRC)
    {
        // When parallel BRC, we still need to wait for the (N-2) PAK
        CODECHAL_ENCODE_CHK_STATUS_RETURN(WaitForRefFrameReady(m_currMinus2MbCodeIndex));
        return eStatus;
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcState::UserFeatureKeyReport()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeHevcBase::UserFeatureKeyReport());

    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_REGION_NUMBER_ID, m_numRegionsInSlice);

    return eStatus;
}

MOS_STATUS CodechalEncHevcState::InitSurfaceCodecParams2D(
    CODECHAL_SURFACE_CODEC_PARAMS* params,
    PMOS_SURFACE surface,
    uint32_t cacheabilityControl,
    uint32_t bindingTableOffset,
    uint32_t verticalLineStride,
    bool isWritable)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    MOS_ZeroMemory(params, sizeof(*params));
    params->bIs2DSurface = true;
    params->bMediaBlockRW = true; // Use media block RW for DP 2D surface access
    params->psSurface = surface;
    params->dwCacheabilityControl = cacheabilityControl;
    params->dwBindingTableOffset = bindingTableOffset;
    params->dwVerticalLineStride = verticalLineStride;
    params->bIsWritable =
    params->bRenderTarget = isWritable;

    return eStatus;
}

MOS_STATUS CodechalEncHevcState::InitSurfaceCodecParamsVME(
    CODECHAL_SURFACE_CODEC_PARAMS* params,
    PMOS_SURFACE surface,
    uint32_t cacheabilityControl,
    uint32_t bindingTableOffset)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    MOS_ZeroMemory(params, sizeof(*params));
    params->bUseAdvState = true;
    params->psSurface = surface;
    params->dwCacheabilityControl= cacheabilityControl;
    params->dwBindingTableOffset = bindingTableOffset;
    params->ucVDirection = CODECHAL_VDIRECTION_FRAME;

    // surface has valid values and have support for the formats specified below
    if (surface != nullptr && (surface->Format == Format_YUY2V || surface->Format == Format_Y216V))
    {
        params->dwWidthInUse = surface->dwWidth;
        params->dwHeightInUse = surface->dwHeight;
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcState::InitSurfaceCodecParams1D(
    CODECHAL_SURFACE_CODEC_PARAMS* params,
    PMOS_RESOURCE buffer,
    uint32_t size,
    uint32_t offset,
    uint32_t cacheabilityControl,
    uint32_t bindingTableOffset,
    bool isWritable)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    MOS_ZeroMemory(params, sizeof(*params));
    params->presBuffer = buffer;
    params->dwSize = size;
    params->dwOffset = offset;
    params->dwCacheabilityControl = cacheabilityControl;
    params->dwBindingTableOffset = bindingTableOffset;
    params->bIsWritable =
    params->bRenderTarget = isWritable;

    return eStatus;
}

CodechalEncHevcState::CodechalEncHevcState(
    CodechalHwInterface* hwInterface,
    CodechalDebugInterface* debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo)
    :CodechalEncodeHevcBase(hwInterface, debugInterface, standardInfo)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_noMeKernelForPFrame = true;

    // initialze class members
    MOS_ZeroMemory(&m_formatConvertedSurface, sizeof(m_formatConvertedSurface));
    MOS_ZeroMemory(&m_brcBuffers, sizeof(m_brcBuffers));
}

CodechalEncHevcState::~CodechalEncHevcState()
{
    MOS_Delete(m_hmeKernel);
}

MOS_STATUS CodechalEncHevcState::AllocateBrcResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    // initiate allocation paramters and lock flags
    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format = Format_Buffer;

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBuffer2D;
    MOS_ZeroMemory(&allocParamsForBuffer2D, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBuffer2D.Type = MOS_GFXRES_2D;
    allocParamsForBuffer2D.TileType = MOS_TILE_LINEAR;
    allocParamsForBuffer2D.Format = Format_Buffer_2D;

    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = true;

    // BRC history buffer
    uint32_t size = m_brcHistoryBufferSize;
    allocParamsForBufferLinear.dwBytes = size;
    allocParamsForBufferLinear.pBufName = "BRC History Buffer";

    MOS_STATUS eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_brcBuffers.resBrcHistoryBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate BRC History Buffer.");
        return eStatus;
    }

    uint8_t *data = (uint8_t *)m_osInterface->pfnLockResource(
        m_osInterface,
        &(m_brcBuffers.resBrcHistoryBuffer),
        &lockFlagsWriteOnly);

    if (data == nullptr)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to Lock BRC History Buffer.");
        eStatus = MOS_STATUS_UNKNOWN;
        return eStatus;
    }

    MOS_ZeroMemory(data, size);
    m_osInterface->pfnUnlockResource(m_osInterface, &m_brcBuffers.resBrcHistoryBuffer);

    // BRC Intra Distortion Surface
    uint32_t width = MOS_ALIGN_CEIL((m_downscaledWidthInMb4x << 3), 64);
    uint32_t height = MOS_ALIGN_CEIL((m_downscaledHeightInMb4x << 2), 8) << 1;
    allocParamsForBuffer2D.dwWidth = width;
    allocParamsForBuffer2D.dwHeight = height;
    allocParamsForBuffer2D.pBufName = "BRC Distortion Surface Buffer";

    CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
                                                  m_osInterface,
                                                  &allocParamsForBuffer2D,
                                                  &m_brcBuffers.sBrcIntraDistortionBuffer.OsResource),
        "Failed to allocate  ME BRC Distortion Buffer.");

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(m_osInterface, &m_brcBuffers.sBrcIntraDistortionBuffer));
    m_brcBuffers.sBrcIntraDistortionBuffer.bArraySpacing = true;
    size                                                 = m_brcBuffers.sBrcIntraDistortionBuffer.dwHeight * m_brcBuffers.sBrcIntraDistortionBuffer.dwPitch;

    CODECHAL_ENCODE_CHK_NULL_RETURN(data = (uint8_t *)m_osInterface->pfnLockResource(
                                        m_osInterface,
                                        &m_brcBuffers.sBrcIntraDistortionBuffer.OsResource,
                                        &lockFlagsWriteOnly));

    MOS_ZeroMemory(data, size);
    m_osInterface->pfnUnlockResource(
        m_osInterface, &m_brcBuffers.sBrcIntraDistortionBuffer.OsResource);

    // PAK Statistics buffer
    size                                = m_hevcBrcPakStatisticsSize;
    allocParamsForBufferLinear.dwBytes = size;
    allocParamsForBufferLinear.pBufName = "BRC PAK Statistics Buffer";

    for (auto i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
    {
        eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_brcBuffers.resBrcPakStatisticBuffer[i]);

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate BRC PAK Statistics Buffer.");
            return eStatus;
        }

        data = (uint8_t *)m_osInterface->pfnLockResource(
            m_osInterface,
            &(m_brcBuffers.resBrcPakStatisticBuffer[i]),
            &lockFlagsWriteOnly);

        if (data == nullptr)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to Lock BRC PAK Statistics Buffer.");
            eStatus = MOS_STATUS_UNKNOWN;
            return eStatus;
        }

        MOS_ZeroMemory(data, size);
        m_osInterface->pfnUnlockResource(m_osInterface, &m_brcBuffers.resBrcPakStatisticBuffer[i]);
    }

    // PAK HCP_PICTURE_STATEs buffer
    size                                = m_brcBuffers.dwBrcHcpPicStateSize;
    allocParamsForBufferLinear.dwBytes = size;
    allocParamsForBufferLinear.pBufName = "PAK HCP PICTURE State Read Buffer";

    for (auto i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
    {
        eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_brcBuffers.resBrcImageStatesReadBuffer[i]);

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate HCP PICTURE State Read Buffer.");
            return eStatus;
        }

        data = (uint8_t *)m_osInterface->pfnLockResource(
            m_osInterface,
            &(m_brcBuffers.resBrcImageStatesReadBuffer[i]),
            &lockFlagsWriteOnly);

        if (data == nullptr)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to Lock HCP PICTURE State Read Buffer.");
            eStatus = MOS_STATUS_UNKNOWN;
            return eStatus;
        }

        MOS_ZeroMemory(data, size);
        m_osInterface->pfnUnlockResource(m_osInterface, &m_brcBuffers.resBrcImageStatesReadBuffer[i]);
    }

    allocParamsForBufferLinear.pBufName = "PAK HCP PICTURE State Write Buffer";
    for (auto i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
    {
        eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_brcBuffers.resBrcImageStatesWriteBuffer[i]);

        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(eStatus, "Failed to allocate HCP PICTURE State Write Buffer.");

        data = (uint8_t *)m_osInterface->pfnLockResource(
            m_osInterface,
            &m_brcBuffers.resBrcImageStatesWriteBuffer[i],
            &lockFlagsWriteOnly);

        if (data == nullptr)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to Lock HCP PICTURE State Write Buffer.");
            eStatus = MOS_STATUS_NULL_POINTER;
            return eStatus;
        }

        MOS_ZeroMemory(data, size);
        m_osInterface->pfnUnlockResource(m_osInterface, &m_brcBuffers.resBrcImageStatesWriteBuffer[i]);
    }

    // BRC constant data surface
    allocParamsForBuffer2D.dwWidth  = MOS_ALIGN_CEIL(m_brcBuffers.dwBrcConstantSurfaceWidth, 64);
    allocParamsForBuffer2D.dwHeight = m_brcBuffers.dwBrcConstantSurfaceHeight;
    allocParamsForBuffer2D.pBufName = "BRC Constant Data Buffer";

    for (auto i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
    {
        eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBuffer2D,
            &m_brcBuffers.sBrcConstantDataBuffer[i].OsResource);

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate BRC Constant Data Buffer.");
            return eStatus;
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(m_osInterface, &m_brcBuffers.sBrcConstantDataBuffer[i]));
        m_brcBuffers.sBrcConstantDataBuffer[i].bArraySpacing = true;
    }

    // Use the Mb QP buffer in BrcBuffer for LCU-based Qp surface in HEVC
    MOS_ZeroMemory(&m_brcBuffers.sBrcMbQpBuffer, sizeof(m_brcBuffers.sBrcMbQpBuffer));

    // original picture size in MB units aligned to 64 bytes along width and 8 bytes along height
    width = MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * SCALE_FACTOR_4x), 64);
    height = MOS_ALIGN_CEIL((m_downscaledHeightInMb4x * SCALE_FACTOR_4x), 8);
    size = width * height;

    allocParamsForBuffer2D.dwWidth = width;
    allocParamsForBuffer2D.dwHeight = height;
    allocParamsForBuffer2D.pBufName = "BRC MB QP Buffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBuffer2D,
        &m_brcBuffers.sBrcMbQpBuffer.OsResource);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate BRC MB QP Buffer.");
        return eStatus;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(m_osInterface, &m_brcBuffers.sBrcMbQpBuffer));
    m_brcBuffers.sBrcMbQpBuffer.bArraySpacing = true;

    data = (uint8_t *)m_osInterface->pfnLockResource(
        m_osInterface,
        &(m_brcBuffers.sBrcMbQpBuffer.OsResource),
        &lockFlagsWriteOnly);

    if (data == nullptr)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to Lock BRC MB QP Buffer.");
        eStatus = MOS_STATUS_UNKNOWN;
        return eStatus;
    }

    MOS_ZeroMemory(data, size);
    m_osInterface->pfnUnlockResource(
        m_osInterface,
        &m_brcBuffers.sBrcMbQpBuffer.OsResource);

    // ROI surface
    MOS_ZeroMemory(&m_brcBuffers.sBrcRoiSurface, sizeof(m_brcBuffers.sBrcRoiSurface));

    // original picture size in MB units aligned to 64 bytes along width and 8 bytes along height
    // ROI buffer size uses MB units for HEVC, not LCU
    width = MOS_ALIGN_CEIL((m_downscaledWidthInMb4x << 4), 64);
    height = MOS_ALIGN_CEIL((m_downscaledHeightInMb4x << 2), 8);

    MOS_ZeroMemory(&m_brcBuffers.sBrcRoiSurface, sizeof(m_brcBuffers.sBrcRoiSurface));
    m_brcBuffers.sBrcRoiSurface.TileType       = MOS_TILE_LINEAR;
    m_brcBuffers.sBrcRoiSurface.bArraySpacing  = true;
    m_brcBuffers.sBrcRoiSurface.Format         = Format_Buffer_2D;
    m_brcBuffers.sBrcRoiSurface.dwWidth        = width;
    m_brcBuffers.sBrcRoiSurface.dwPitch        = width;
    m_brcBuffers.sBrcRoiSurface.dwHeight       = height;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
        &m_brcBuffers.sBrcRoiSurface,
        width,
        height,
        "ROI Buffer"));
    
    return eStatus;
}

MOS_STATUS CodechalEncHevcState::FreeBrcResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_brcBuffers.resBrcHistoryBuffer);

    for (auto i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_brcBuffers.resBrcPakStatisticBuffer[i]);

        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_brcBuffers.resBrcImageStatesReadBuffer[i]);

        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_brcBuffers.resBrcImageStatesWriteBuffer[i]);

        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_brcBuffers.sBrcConstantDataBuffer[i].OsResource);
    }

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_brcBuffers.sBrcIntraDistortionBuffer.OsResource);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_brcBuffers.sBrcMbQpBuffer.OsResource);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_brcBuffers.sBrcRoiSurface.OsResource);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncHevcState::AllocateEncStatsResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (Mos_ResourceIsNull(&m_encStatsBuffers.m_puStatsSurface.OsResource))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
            &m_encStatsBuffers.m_puStatsSurface,
            m_widthAlignedMaxLcu,
            m_heightAlignedMaxLcu >> 5,
            "32x32 PU statistics Data Dump surface"));
    }

    if (Mos_ResourceIsNull(&m_encStatsBuffers.m_8x8PuHaarDist.OsResource))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
            &m_encStatsBuffers.m_8x8PuHaarDist,
            m_widthAlignedMaxLcu,
            m_heightAlignedMaxLcu >> 4,
            "8x8 PU Haar distortion for 16x16 surface"));
    }

    if (Mos_ResourceIsNull(&m_encStatsBuffers.m_8x8PuFrameStats.sResource))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
                    &m_encStatsBuffers.m_8x8PuFrameStats,
                    m_8x8PuFrameStatsSize,
                    "8x8 PU frame statistics surface"));
    }

    if (Mos_ResourceIsNull(&m_encStatsBuffers.m_mbEncStatsSurface.OsResource))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
            &m_encStatsBuffers.m_mbEncStatsSurface,
            m_widthAlignedMaxLcu,
            m_heightAlignedMaxLcu >> 5,
            "MB Enc Statistics data dump surface"));
    }

    if (Mos_ResourceIsNull(&m_encStatsBuffers.m_mbEncFrameStats.sResource))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
                    &m_encStatsBuffers.m_mbEncFrameStats,
                    m_mbEncFrameStatsSize,
                    "MB Enc frame statistics surface"));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncHevcState::FreeEncStatsResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_encStatsBuffers.m_puStatsSurface.OsResource);

    m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_encStatsBuffers.m_8x8PuHaarDist.OsResource);

    m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_encStatsBuffers.m_8x8PuFrameStats.sResource);

    m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_encStatsBuffers.m_mbEncStatsSurface.OsResource);

    m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_encStatsBuffers.m_mbEncFrameStats.sResource);

    return MOS_STATUS_SUCCESS;
}

bool CodechalEncHevcState::CheckSupportedFormat(PMOS_SURFACE surface)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    bool isColorFormatSupported = false;

    if (nullptr == surface)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid (nullptr) Pointer.");
        return isColorFormatSupported;
    }

    switch (surface->Format)
    {
    case Format_NV12:
        isColorFormatSupported = IS_Y_MAJOR_TILE_FORMAT(surface->TileType);
        break;
    case Format_P010:
        isColorFormatSupported = true;
    case Format_YUY2:
    case Format_YUYV:
    case Format_A8R8G8B8:
        break;
    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Input surface color format = %d not supported!", surface->Format);
        break;
    }

    return isColorFormatSupported;
}

MOS_STATUS CodechalEncHevcState::Initialize(CodechalSetting * settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // common initilization
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeHevcBase::Initialize(settings));

    m_brcBuffers.dwBrcHcpPicStateSize       = BRC_IMG_STATE_SIZE_PER_PASS * CODECHAL_ENCODE_BRC_MAXIMUM_NUM_PASSES;
    m_brcBuffers.uiCurrBrcPakStasIdxForRead = 0;
    //Reading buffer is with 2 frames late for BRC kernel uses the PAK statstic info of the frame before the previous frame
    m_brcBuffers.uiCurrBrcPakStasIdxForWrite =
        (m_brcBuffers.uiCurrBrcPakStasIdxForRead + 2) % CODECHAL_ENCODE_RECYCLED_BUFFER_NUM;

    m_widthAlignedLcu32  = MOS_ALIGN_CEIL(m_frameWidth, 32);
    m_heightAlignedLcu32 = MOS_ALIGN_CEIL(m_frameHeight, 32);

    m_hucCommandsSize = m_hwInterface->m_hucCommandBufferSize * CODECHAL_HEVC_MAX_NUM_BRC_PASSES;

    return eStatus;
}

MOS_STATUS CodechalEncHevcState::GetFrameBrcLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_lowDelay)
    {
        // LDB
        if (m_pictureCodingType == I_TYPE)
        {
            if (m_hevcPicParams->HierarchLevelPlus1 == 0)
            {
                m_currFrameBrcLevel = HEVC_BRC_FRAME_TYPE_I;
            }
            else
            {
                CODECHAL_ENCODE_ASSERTMESSAGE("FrameLevel can only be 0 for I type for LDB\n");
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }
        else if ((m_pictureCodingType == P_TYPE) || (m_pictureCodingType == B_TYPE))
        {
            if (m_hevcPicParams->HierarchLevelPlus1 == 0)
            {
                m_currFrameBrcLevel = HEVC_BRC_FRAME_TYPE_P_OR_LB;
            }
            else if (m_hevcPicParams->HierarchLevelPlus1 == 1)
            {
                m_currFrameBrcLevel = HEVC_BRC_FRAME_TYPE_B;
            }
            else if (m_hevcPicParams->HierarchLevelPlus1 == 2)
            {
                m_currFrameBrcLevel = HEVC_BRC_FRAME_TYPE_B1;
            }
            else if (m_hevcPicParams->HierarchLevelPlus1 == 3)
            {
                CODECHAL_ENCODE_ASSERTMESSAGE("FrameLevel 3 is not supported for LDB\n");
                return MOS_STATUS_INVALID_PARAMETER;
            }
            else
            {
                CODECHAL_ENCODE_ASSERT(false);
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }
        else if ((m_pictureCodingType == B1_TYPE) || (m_pictureCodingType == B2_TYPE))
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("B1 & B2 Type is not supported for LDB\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }
        else
        {
            CODECHAL_ENCODE_ASSERT(false);
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }
    else
    {
        // HB
        if (m_pictureCodingType == I_TYPE)
        {
                m_currFrameBrcLevel = HEVC_BRC_FRAME_TYPE_I;
        }
        else if (m_pictureCodingType == B_TYPE)
        {
                m_currFrameBrcLevel = HEVC_BRC_FRAME_TYPE_B;
        }
        else if (m_pictureCodingType == B1_TYPE)
        {
                m_currFrameBrcLevel = HEVC_BRC_FRAME_TYPE_B1;
        }
        else if (m_pictureCodingType == B2_TYPE)
        {
                m_currFrameBrcLevel = HEVC_BRC_FRAME_TYPE_B2;
        }
        else if (m_pictureCodingType == P_TYPE)
        {
                m_currFrameBrcLevel = HEVC_BRC_FRAME_TYPE_P_OR_LB;
        }
        else
        {
                CODECHAL_ENCODE_ASSERT(false);
                return MOS_STATUS_INVALID_PARAMETER;
        }
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcState::InitializePicture(const EncoderParams& params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeHevcBase::InitializePicture(params));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetFrameBrcLevel());
    
    return eStatus;
}

MOS_STATUS CodechalEncHevcState::SetMeCurbeParams(
    CodechalKernelHme::CurbeParam &curbeParams)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Setup ME Params
    MOS_ZeroMemory(&curbeParams, sizeof(curbeParams));
    curbeParams.subPelMode        = 3;
    curbeParams.currOriginalPic   = m_hevcPicParams->CurrOriginalPic;
    curbeParams.qpPrimeY          = m_hevcPicParams->QpY + m_hevcSliceParams->slice_qp_delta;
    curbeParams.targetUsage       = m_hevcSeqParams->TargetUsage;
    curbeParams.maxMvLen          = CODECHAL_ENCODE_HEVC_MAX_MV_LEN_AVC_LEVEL_51;
    curbeParams.numRefIdxL0Minus1 = m_hevcSliceParams->num_ref_idx_l0_active_minus1;
    curbeParams.numRefIdxL1Minus1 = m_hevcSliceParams->num_ref_idx_l1_active_minus1;
    curbeParams.bmeMethodTable    = m_bmeMethodTable;
    curbeParams.meMethodTable     = m_meMethodTable;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncHevcState::SetMeSurfaceParams(
    CodechalKernelHme::SurfaceParams     &surfaceParams)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Setup ME Params
    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
    surfaceParams.mbaffEnabled                     = false;
    surfaceParams.numRefIdxL0ActiveMinus1          = m_hevcSliceParams->num_ref_idx_l0_active_minus1;
    surfaceParams.numRefIdxL1ActiveMinus1          = m_hevcSliceParams->num_ref_idx_l1_active_minus1;
    surfaceParams.verticalLineStride               = m_verticalLineStride;
    surfaceParams.verticalLineStrideOffset         = m_verticalLineStrideOffset;
    surfaceParams.meBrcDistortionBuffer            = &m_brcBuffers.sMeBrcDistortionBuffer;
    surfaceParams.meBrcDistortionBottomFieldOffset = m_brcBuffers.dwMeBrcDistortionBottomFieldOffset;
    surfaceParams.refList                          = &m_refList[0];
    surfaceParams.picIdx                           = &m_picIdx[0];
    surfaceParams.currOriginalPic                  = &m_currOriginalPic;
    surfaceParams.refL0List                        = &(m_hevcSliceParams->RefPicList[LIST_0][0]);
    surfaceParams.refL1List                        = &(m_hevcSliceParams->RefPicList[LIST_1][0]);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncHevcState::EncodeMeKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Walker must be used for HME call and scaling one
    CODECHAL_ENCODE_ASSERT(m_hwWalker);

    if (m_hmeKernel && m_hmeKernel->Is4xMeEnabled())
    {
        CodechalKernelHme::CurbeParam curbeParam;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetMeCurbeParams(curbeParam));

        CodechalKernelHme::SurfaceParams surfaceParam;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetMeSurfaceParams(surfaceParam));
        if (m_hmeKernel->Is16xMeEnabled())
        {
            if (m_hmeKernel->Is32xMeEnabled())
            {
                surfaceParam.downScaledWidthInMb         = m_downscaledWidthInMb32x;
                surfaceParam.downScaledHeightInMb        = m_downscaledFrameFieldHeightInMb32x;
                surfaceParam.downScaledBottomFieldOffset = m_scaled32xBottomFieldOffset;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hmeKernel->Execute(curbeParam, surfaceParam, CodechalKernelHme::HmeLevel::hmeLevel32x));
            }
            surfaceParam.downScaledWidthInMb         = m_downscaledWidthInMb16x;
            surfaceParam.downScaledHeightInMb        = m_downscaledFrameFieldHeightInMb16x;
            surfaceParam.downScaledBottomFieldOffset = m_scaled16xBottomFieldOffset;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hmeKernel->Execute(curbeParam, surfaceParam, CodechalKernelHme::HmeLevel::hmeLevel16x));
        }
        surfaceParam.downScaledWidthInMb         = m_downscaledWidthInMb4x;
        surfaceParam.downScaledHeightInMb        = m_downscaledFrameFieldHeightInMb4x;
        surfaceParam.downScaledBottomFieldOffset = m_scaledBottomFieldOffset;
        m_lastTaskInPhase = true;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hmeKernel->Execute(curbeParam, surfaceParam, CodechalKernelHme::HmeLevel::hmeLevel4x));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpHMESurfaces());

    return eStatus;
}

MOS_STATUS CodechalEncHevcState::DumpHMESurfaces()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_DEBUG_TOOL(
    if (m_hmeEnabled) {
        PMOS_SURFACE dumpBuffer = m_hmeKernel->GetSurface(CodechalKernelHme::SurfaceId::me4xMvDataBuffer);
        if (dumpBuffer)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &dumpBuffer->OsResource,
                CodechalDbgAttr::attrOutput,
                "MvData",
                dumpBuffer->dwHeight *dumpBuffer->dwPitch,
                CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 32), 64) * (m_downscaledFrameFieldHeightInMb4x * 4) : 0,
                CODECHAL_MEDIA_STATE_4X_ME));
        }

        dumpBuffer = m_hmeKernel->GetSurface(CodechalKernelHme::SurfaceId::me4xDistortionBuffer);
        if (dumpBuffer)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &dumpBuffer->OsResource,
                CodechalDbgAttr::attrOutput,
                "MeDist",
                dumpBuffer->dwHeight * dumpBuffer->dwPitch,
                CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 8), 64) * MOS_ALIGN_CEIL((m_downscaledFrameFieldHeightInMb4x * 4), 8) : 0,
                CODECHAL_MEDIA_STATE_4X_ME));
        }

        if (m_b16XMeEnabled)
        {
            dumpBuffer = m_hmeKernel->GetSurface(CodechalKernelHme::SurfaceId::me16xMvDataBuffer);
            //    &meOutputParams));

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &dumpBuffer->OsResource,
                CodechalDbgAttr::attrOutput,
                "MvData",
                dumpBuffer->dwHeight *dumpBuffer->dwPitch,
                CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb16x * 32), 64) * (m_downscaledFrameFieldHeightInMb16x * 4) : 0,
                CODECHAL_MEDIA_STATE_16X_ME));

            if (m_b32XMeEnabled)
            {
                dumpBuffer = m_hmeKernel->GetSurface(CodechalKernelHme::SurfaceId::me32xMvDataBuffer);
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    &dumpBuffer->OsResource,
                    CodechalDbgAttr::attrOutput,
                    "MvData",
                    dumpBuffer->dwHeight *dumpBuffer->dwPitch,
                    CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb32x * 32), 64) * (m_downscaledFrameFieldHeightInMb32x * 4) : 0,
                    CODECHAL_MEDIA_STATE_32X_ME));
            }
        }
    })

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncHevcState::SetupROISurface()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_LOCK_PARAMS ReadOnly;
    MOS_ZeroMemory(&ReadOnly, sizeof(ReadOnly));
    ReadOnly.ReadOnly = 1;
    uint32_t * dataPtr = (uint32_t *)m_osInterface->pfnLockResource(m_osInterface, &m_brcBuffers.sBrcRoiSurface.OsResource, &ReadOnly);
    if (!dataPtr)
    {
        eStatus = MOS_STATUS_INVALID_HANDLE;
        return eStatus;
    }

    uint32_t bufferWidthInByte  = m_brcBuffers.sBrcRoiSurface.dwPitch;
    uint32_t bufferHeightInByte = MOS_ALIGN_CEIL((m_downscaledHeightInMb4x << 2), 8);
    uint32_t numMBs = m_picWidthInMb * m_picHeightInMb;
    for (uint32_t uMB = 0; uMB <= numMBs; uMB++)
    {
        int32_t curMbY = uMB / m_picWidthInMb;
        int32_t curMbX = uMB - curMbY * m_picWidthInMb;

        uint32_t outdata = 0;
        for (int32_t roiIdx = (m_hevcPicParams->NumROI - 1); roiIdx >= 0; roiIdx--)
        {
            int32_t qpLevel;
            if (m_roiValueInDeltaQp)
            {
                qpLevel = m_hevcPicParams->ROI[roiIdx].PriorityLevelOrDQp;
            }
            else
            {
                // QP Level sent to ROI surface is (priority * 5)
                //qpLevel = m_hevcPicParams->ROI[roiIdx].PriorityLevelOrDQp * 6;
                CODECHAL_ENCODE_ASSERTMESSAGE("error: ROI does not support priority level for now.");
                return MOS_STATUS_INVALID_PARAMETER;
            }

            if (qpLevel == 0)
            {
                continue;
            }

            if ((curMbX >= (int32_t)m_hevcPicParams->ROI[roiIdx].Left) && (curMbX < (int32_t)m_hevcPicParams->ROI[roiIdx].Right) &&
                (curMbY >= (int32_t)m_hevcPicParams->ROI[roiIdx].Top) && (curMbY < (int32_t)m_hevcPicParams->ROI[roiIdx].Bottom))
            {
                outdata = 15 | ((qpLevel & 0xFF) << 16);
            }
            else if (m_roiRegionSmoothEnabled)
            {
                if ((curMbX >= (int32_t)m_hevcPicParams->ROI[roiIdx].Left - 1) && (curMbX < (int32_t)m_hevcPicParams->ROI[roiIdx].Right + 1) &&
                    (curMbY >= (int32_t)m_hevcPicParams->ROI[roiIdx].Top - 1) && (curMbY < (int32_t)m_hevcPicParams->ROI[roiIdx].Bottom + 1))
                {
                    outdata = 14 | ((qpLevel & 0xFF) << 16);
                }
                else if ((curMbX >= (int32_t)m_hevcPicParams->ROI[roiIdx].Left - 2) && (curMbX < (int32_t)m_hevcPicParams->ROI[roiIdx].Right + 2) &&
                         (curMbY >= (int32_t)m_hevcPicParams->ROI[roiIdx].Top - 2) && (curMbY < (int32_t)m_hevcPicParams->ROI[roiIdx].Bottom + 2))
                {
                    outdata = 13 | ((qpLevel & 0xFF) << 16);
                }
                else if ((curMbX >= (int32_t)m_hevcPicParams->ROI[roiIdx].Left - 3) && (curMbX < (int32_t)m_hevcPicParams->ROI[roiIdx].Right + 3) &&
                         (curMbY >= (int32_t)m_hevcPicParams->ROI[roiIdx].Top - 3) && (curMbY < (int32_t)m_hevcPicParams->ROI[roiIdx].Bottom + 3))
                {
                    outdata = 12 | ((qpLevel & 0xFF) << 16);
                }
            }
        }
        dataPtr[(curMbY * (bufferWidthInByte >> 2)) + curMbX] = outdata;
    }

    m_osInterface->pfnUnlockResource(m_osInterface, &m_brcBuffers.sBrcRoiSurface.OsResource);

    uint32_t bufferSize = bufferWidthInByte * bufferHeightInByte;
    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
        &m_brcBuffers.sBrcRoiSurface.OsResource,
        CodechalDbgAttr::attrROISurface,
        "ROIInputSurface",
        bufferSize,
        0,
        CODECHAL_NUM_MEDIA_STATES)));

    return eStatus;
}

MOS_STATUS CodechalEncHevcState::GetRoundingIntraInterToUse()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_hevcPicParams->CustomRoundingOffsetsParams.fields.EnableCustomRoudingIntra)
    {
        m_roundingIntraInUse = m_hevcPicParams->CustomRoundingOffsetsParams.fields.RoundingOffsetIntra;
    }
    else
    {
        if (m_hevcSeqParams->NumOfBInGop[1] != 0 || m_hevcSeqParams->NumOfBInGop[2] != 0)
        {
            //Hierachical B GOP
            if (m_hevcPicParams->CodingType == I_TYPE || 
                m_hevcPicParams->CodingType == P_TYPE)
            {
                m_roundingIntraInUse = 4;
            }
            else if (m_hevcPicParams->CodingType == B_TYPE)
            {
                m_roundingIntraInUse = 3;
            }
            else
            {
                m_roundingIntraInUse = 2;
            }
        }
        else
        {
            m_roundingIntraInUse = 10;
        }
    }

    if (m_hevcPicParams->CustomRoundingOffsetsParams.fields.EnableCustomRoudingInter)
    {
        m_roundingInterInUse = m_hevcPicParams->CustomRoundingOffsetsParams.fields.RoundingOffsetInter;
    }
    else
    {
        if (m_hevcSeqParams->NumOfBInGop[1] != 0 || m_hevcSeqParams->NumOfBInGop[2] != 0)
        {
            //Hierachical B GOP 
            if (m_hevcPicParams->CodingType == I_TYPE || 
                m_hevcPicParams->CodingType == P_TYPE)
            {
                m_roundingInterInUse = 4;
            }
            else if (m_hevcPicParams->CodingType == B_TYPE)
            {
                m_roundingInterInUse = 3;
            }
            else
            {
                m_roundingInterInUse = 2;
            }
        }
        else
        {
            m_roundingInterInUse = 4;
        }
    }

    return eStatus;
}

