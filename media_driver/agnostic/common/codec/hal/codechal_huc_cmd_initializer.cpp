/*
* Copyright (c) 2017-2018, Intel Corporation
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
//! \file     codechal_huc_cmd_initializer.cpp
//! \brief    Defines base class for command initializer encoder.
//!

#include "codec_def_encode_hevc.h"
#if defined (_HEVC_ENCODE_VME_SUPPORTED) || defined (_HEVC_ENCODE_VDENC_SUPPORTED)
#include "codechal_encode_hevc_base.h"
#endif
#include "codechal_huc_cmd_initializer.h"
#include "codechal_encoder_base.h"
#ifdef _VP9_ENCODE_VDENC_SUPPORTED
#include "codechal_vdenc_vp9_base.h"
#endif

#define VDBOX_HUC_CMD_INITIALIZER_HEVC_CMD1_STARTOFFSERT 28
#define VDBOX_HUC_CMD_INITIALIZER_HEVC_CMD2_STARTOFFSERT 276
#define VDBOX_HUC_CMD_INITIALIZER_HEVC_CQP_CMD2_STARTOFFSET 248

CodechalCmdInitializer::CodechalCmdInitializer(
    CodechalEncoderState *encoder)
{
    m_encoder = encoder;
    m_cmdCount = 0;
    m_currentPass = 0;
}

MOS_STATUS CodechalCmdInitializer::CmdInitializerAllocateResources(
    CodechalHwInterface *hwInterface)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_hwInterface = hwInterface;
    m_osInterface = m_hwInterface->GetOsInterface();
    m_miInterface = m_hwInterface->GetMiInterface();
    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format = Format_Buffer;

    //Allocate buffers for every pass.
    for (int i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
    {
        for(int j = 0; j < 3; j++)
        {
            // Cmd Initializer DMEM
            allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(sizeof(HucComDmem), CODECHAL_CACHELINE_SIZE);
            allocParamsForBufferLinear.pBufName = "VDEnc CmdInitializer Dmem Buffer";
            CODECHAL_ENCODE_CHK_STATUS_RETURN(
                m_osInterface->pfnAllocateResource(
                    m_osInterface,
                    &allocParamsForBufferLinear,
                    &m_cmdInitializerDmemBuffer[i][j]));

            // Cmd Initializer Data buffer
            allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(sizeof(HucComData), CODECHAL_PAGE_SIZE);
            allocParamsForBufferLinear.pBufName = "VDEnc CmdInitializer Data Buffer";
            CODECHAL_ENCODE_CHK_STATUS_RETURN(
                m_osInterface->pfnAllocateResource(
                    m_osInterface,
                    &allocParamsForBufferLinear,
                    &m_cmdInitializerDataBuffer[i][j]));

            MOS_LOCK_PARAMS lockFlagsWriteOnly;
            MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
            lockFlagsWriteOnly.WriteOnly = 1;

            uint8_t *pData = (uint8_t *)m_osInterface->pfnLockResource(
                m_osInterface,
                &m_cmdInitializerDataBuffer[i][j],
                &lockFlagsWriteOnly);
            CODECHAL_ENCODE_CHK_NULL_RETURN(pData);
            MOS_ZeroMemory(pData, allocParamsForBufferLinear.dwBytes);
            m_osInterface->pfnUnlockResource(m_osInterface, &m_cmdInitializerDataBuffer[i][j]);
        }
    }

    //Allocate extra buffers for dynamic scaling
    // Cmd Initializer DMEM
    allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(sizeof(HucComDmem), CODECHAL_CACHELINE_SIZE);
    allocParamsForBufferLinear.pBufName = "VDEnc Dynamic Sclaing CmdInitializer Dmem Buffer";
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_cmdInitializerDysScalingDmemBuffer));

    // Cmd Initializer Data buffer
    allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(sizeof(HucComData), CODECHAL_PAGE_SIZE);
    allocParamsForBufferLinear.pBufName = "VDEnc Dynamic Sclaing CmdInitializer Data Buffer";
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_cmdInitializerDysScalingDataBuffer));

    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;

    uint8_t* pData = (uint8_t*)m_osInterface->pfnLockResource(
        m_osInterface,
        &m_cmdInitializerDysScalingDataBuffer,
        &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pData);
    MOS_ZeroMemory(pData, allocParamsForBufferLinear.dwBytes);
    m_osInterface->pfnUnlockResource(m_osInterface, &m_cmdInitializerDysScalingDataBuffer);

    return eStatus;
}

void CodechalCmdInitializer::CmdInitializerFreeResources()
{
    for (int i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
    {
        for(int j = 0; j < 3; j++)
        {
            m_osInterface->pfnFreeResource(m_osInterface, &m_cmdInitializerDmemBuffer[i][j]);
            m_osInterface->pfnFreeResource(m_osInterface, &m_cmdInitializerDataBuffer[i][j]);
        }
    }
    m_osInterface->pfnFreeResource(m_osInterface, &m_cmdInitializerDysScalingDmemBuffer);
    m_osInterface->pfnFreeResource(m_osInterface, &m_cmdInitializerDysScalingDataBuffer);

}

#if defined (_HEVC_ENCODE_VME_SUPPORTED) || defined (_HEVC_ENCODE_VDENC_SUPPORTED)
MOS_STATUS CodechalCmdInitializer::CmdInitializerSetDmem(bool brcEnabled)
{
    HucComDmem*             hucCmdInitializerDmem;
    MOS_LOCK_PARAMS         lockFlagsWriteOnly;
    uint16_t                offset = 0;
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_osInterface = m_encoder->GetOsInterface();

    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;

    // Setup CmdInitializer DMEM
    hucCmdInitializerDmem = (HucComDmem *)m_osInterface->pfnLockResource(
        m_osInterface, &m_cmdInitializerDmemBuffer[m_encoder->m_currRecycledBufIdx][m_currentPass], &lockFlagsWriteOnly);

    MOS_ZeroMemory(hucCmdInitializerDmem, sizeof(HucComDmem));
    
    CODECHAL_ENCODE_ASSERT(m_cmdCount == 2);
    hucCmdInitializerDmem->TotalOutputCommands = 2;

    hucCmdInitializerDmem->TargetUsage         = 4;
    switch (m_encoder->m_standard)
    {
        case CODECHAL_HEVC:
            hucCmdInitializerDmem->Codec       = 0;
            hucCmdInitializerDmem->TargetUsage = (uint8_t)m_encoder->m_targetUsage;
            break;
        case CODECHAL_VP9:
            hucCmdInitializerDmem->Codec       = 1;
            break;
        default:
            hucCmdInitializerDmem->Codec       = 0;
            break;
    }
    hucCmdInitializerDmem->FrameType = m_encoder->m_pictureCodingType - 1;
    hucCmdInitializerDmem->OutputCOM[0].ID           = 2;
    hucCmdInitializerDmem->OutputCOM[0].Type         = 1;
    hucCmdInitializerDmem->OutputCOM[0].StartInBytes = GetCmd1StartOffset(brcEnabled);

    offset += CODECHAL_CMDINITIALIZER_MAX_CMD_SIZE;

    // Command ID 1
    hucCmdInitializerDmem->OutputCOM[1].ID           = 1;
    hucCmdInitializerDmem->OutputCOM[1].Type         = 1;
    hucCmdInitializerDmem->OutputCOM[1].StartInBytes = GetCmd2StartOffset(brcEnabled);

    offset += CODECHAL_CMDINITIALIZER_MAX_CMD_SIZE;

    hucCmdInitializerDmem->OutputSize = offset;

    m_osInterface->pfnUnlockResource(m_osInterface, &m_cmdInitializerDmemBuffer[m_encoder->m_currRecycledBufIdx][m_currentPass]);

    return eStatus;
}

MOS_STATUS CodechalCmdInitializer::ConstructHevcHucCmd1ConstData(
    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS seqParams,
    PCODEC_HEVC_ENCODE_PICTURE_PARAMS  picParams,
    PCODEC_HEVC_ENCODE_SLICE_PARAMS    sliceParams,
    struct HucComData *                hucConstData)
{
    hucConstData->InputCOM[1].ID         = 1;
    hucConstData->InputCOM[1].SizeOfData = sizeof(HucInputCmd1) / sizeof(uint8_t);

    auto   qpPrimeYAC = 10;  //This is constant from Arch C Model
    double qpScale    = (picParams->CodingType == I_TYPE) ? 0.60 : 0.65;

    HucInputCmd1 cmd1;
    MOS_ZeroMemory(&cmd1, sizeof(HucInputCmd1));

    // Shared HEVC/VP9
    cmd1.FrameWidthInMinCbMinus1  = seqParams->wFrameWidthInMinCbMinus1;
    cmd1.FrameHeightInMinCbMinus1 = seqParams->wFrameHeightInMinCbMinus1;

    cmd1.log2_min_coding_block_size_minus3 = seqParams->log2_min_coding_block_size_minus3;

    cmd1.VdencStreamInEnabled         = (uint8_t)m_streamInEnabled;
    cmd1.PakOnlyMultipassEnable       = m_pakOnlyPass;
    cmd1.num_ref_idx_l0_active_minus1 = sliceParams->num_ref_idx_l0_active_minus1;

    auto   qpPrimeYac = CodecHal_Clip3(0, 51, picParams->QpY + sliceParams->slice_qp_delta);
    double lambda     = sqrt(qpScale * pow(2.0, MOS_MAX(0, qpPrimeYac - 12) / 3.0));
    cmd1.SADQPLambda  = (uint16_t)(lambda * 4 + 0.5);
    cmd1.RDQPLambda   = (uint16_t)(qpScale * pow(2.0, MOS_MAX(0, picParams->QpY - 12) / 3.0) * 4 + 0.5);  //U14.2

    cmd1.num_ref_idx_l1_active_minus1 = sliceParams->num_ref_idx_l1_active_minus1;
    cmd1.ROIStreamInEnabled           = (uint8_t)m_roiStreamInEnabled;
    cmd1.UseDefaultQpDeltas           = (m_acqpEnabled && seqParams->QpAdjustment) || (m_brcEnabled && seqParams->MBBRC != mbBrcDisabled);
    cmd1.TemporalMvpEnableFlag        = seqParams->sps_temporal_mvp_enable_flag;
    cmd1.PanicEnabled                 = m_panicEnabled;

    if (m_roiStreamInEnabled)
    {
        for (int8_t i = 0; i < ENCODE_VDENC_HEVC_MAX_STREAMINROI_G10; i++)
        {
            cmd1.ROIDeltaQp[i] = picParams->ROIDistinctDeltaQp[i];
        }
    }

    // default
    cmd1.FwdPocNumForRefId0inL0 = 0x01;
    cmd1.FwdPocNumForRefId0inL1 = 0xff;
    cmd1.FwdPocNumForRefId1inL0 = 0x02;
    cmd1.FwdPocNumForRefId1inL1 = 0xfe;
    cmd1.FwdPocNumForRefId2inL0 = 0x03;
    cmd1.FwdPocNumForRefId2inL1 = 0xfd;
    cmd1.FwdPocNumForRefId3inL0 = 0x04;
    cmd1.FwdPocNumForRefId3inL1 = 0xfc;

    if (picParams->CodingType != I_TYPE)
    {
        uint8_t refFrameID;
        char    diff_poc;

        refFrameID                  = sliceParams->RefPicList[0][0].FrameIdx;
        diff_poc                    = picParams->RefFramePOCList[refFrameID] - picParams->CurrPicOrderCnt;
        cmd1.FwdPocNumForRefId0inL0 = -diff_poc;
        cmd1.FwdPocNumForRefId0inL1 = -diff_poc;

        refFrameID                  = sliceParams->RefPicList[0][1].FrameIdx;
        diff_poc                    = picParams->RefFramePOCList[refFrameID] - picParams->CurrPicOrderCnt;
        cmd1.FwdPocNumForRefId1inL0 = -diff_poc;
        cmd1.FwdPocNumForRefId1inL1 = -diff_poc;

        refFrameID                  = sliceParams->RefPicList[0][2].FrameIdx;
        diff_poc                    = picParams->RefFramePOCList[refFrameID] - picParams->CurrPicOrderCnt;
        cmd1.FwdPocNumForRefId2inL0 = -diff_poc;
        cmd1.FwdPocNumForRefId2inL1 = -diff_poc;
    }

    cmd1.EnableRollingIntraRefresh          = picParams->bEnableRollingIntraRefresh;
    cmd1.QpDeltaForInsertedIntra            = picParams->QpDeltaForInsertedIntra;
    cmd1.IntraInsertionSize                 = picParams->IntraInsertionSize;
    cmd1.IntraInsertionLocation             = picParams->IntraInsertionLocation;
    cmd1.IntraInsertionReferenceLocation[0] = picParams->RollingIntraReferenceLocation[0];
    cmd1.IntraInsertionReferenceLocation[1] = picParams->RollingIntraReferenceLocation[1];
    cmd1.IntraInsertionReferenceLocation[2] = picParams->RollingIntraReferenceLocation[2];

    cmd1.QpY             = picParams->QpY + sliceParams->slice_qp_delta;
    cmd1.RoundingEnabled = (uint8_t)m_roundingEnabled;

    MOS_SecureMemcpy(hucConstData->InputCOM[1].data, sizeof(HucInputCmd1), &cmd1, sizeof(HucInputCmd1));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalCmdInitializer::ConstructHevcHucCmd2ConstData(
    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS seqParams,
    PCODEC_HEVC_ENCODE_PICTURE_PARAMS  picParams,
    PCODEC_HEVC_ENCODE_SLICE_PARAMS    sliceParams,
    struct HucComData *                hucConstData)
{
    hucConstData->InputCOM[0].ID         = 2;
    hucConstData->InputCOM[0].SizeOfData = 2;

    auto qpPrimeYAC = 10;  //This is constant from Arch C Model

    double qpScale        = (picParams->CodingType == I_TYPE) ? 0.60 : 0.65;
    double lambdaInputCom = sqrt(qpScale * pow(2.0, MOS_MAX(0, qpPrimeYAC - 12) / 3.0));

    // SADQPLambda
    hucConstData->InputCOM[0].data[0] = (uint32_t)(lambdaInputCom * 4 + 0.5);
    hucConstData->InputCOM[0].data[1] = m_roiStreamInEnabled;

    return MOS_STATUS_SUCCESS;
}

uint16_t CodechalCmdInitializer::GetCmd1StartOffset(bool brcEnabled)
{
    return brcEnabled ? VDBOX_HUC_CMD_INITIALIZER_HEVC_CMD1_STARTOFFSERT : 0;
}

uint16_t CodechalCmdInitializer::GetCmd2StartOffset(bool brcEnabled)
{
    return brcEnabled ? VDBOX_HUC_CMD_INITIALIZER_HEVC_CMD2_STARTOFFSERT : VDBOX_HUC_CMD_INITIALIZER_HEVC_CQP_CMD2_STARTOFFSET;
}

MOS_STATUS CodechalCmdInitializer::CmdInitializerSetConstData(
    PMOS_INTERFACE                              osInterface,
    MhwMiInterface                              *miInterface,
    MhwVdboxVdencInterface                      *vdencInterface,
    void*                                       sequenceParams,
    void*                                       pictureParams,
    void*                                       slcParams,
    bool                                        pakOnlyPass,
    bool                                        acqpEnabled,
    bool                                        brcEnabled,
    bool                                        streaminEnabled,
    bool                                        roiStreamInEnabled,
    bool                                        roundingEnabled,
    bool                                        panicEnabled,
    int32_t                                     currentPass
    )
{
    HucComData*                                     hucConstData;
    MOS_LOCK_PARAMS                                 lockFlagsWriteOnly;
    MOS_STATUS                                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(osInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(miInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(vdencInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(sequenceParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pictureParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(slcParams);

    m_osInterface    = osInterface;
    m_miInterface    = miInterface;
    m_vdencInterface = vdencInterface;
    m_seqParams      = sequenceParams;
    m_picParams      = pictureParams;
    m_sliceParams    = slcParams;

    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS seqParams = (PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS)sequenceParams;
    PCODEC_HEVC_ENCODE_PICTURE_PARAMS picParams = (PCODEC_HEVC_ENCODE_PICTURE_PARAMS)pictureParams;
    PCODEC_HEVC_ENCODE_SLICE_PARAMS sliceParams = (PCODEC_HEVC_ENCODE_SLICE_PARAMS)slcParams;

    m_pakOnlyPass        = pakOnlyPass;
    m_acqpEnabled        = acqpEnabled;
    m_brcEnabled         = brcEnabled;
    m_streamInEnabled    = streaminEnabled;
    m_roundingEnabled    = roundingEnabled;
    m_panicEnabled       = panicEnabled;
    m_roiStreamInEnabled = roiStreamInEnabled;
    m_currentPass        = currentPass;

    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;

    hucConstData = (HucComData *)m_osInterface->pfnLockResource(m_osInterface, &m_cmdInitializerDataBuffer[m_encoder->m_currRecycledBufIdx][currentPass], &lockFlagsWriteOnly);

    MOS_ZeroMemory(hucConstData, sizeof(HucComData));
    m_cmdCount = 0;

    // Command ID 2
    ConstructHevcHucCmd2ConstData(seqParams, picParams, sliceParams, hucConstData);
    m_cmdCount++;

    // Command ID 1
    ConstructHevcHucCmd1ConstData(seqParams, picParams, sliceParams, hucConstData);
    m_cmdCount++;

    hucConstData->TotalCommands = m_cmdCount;

    m_osInterface->pfnUnlockResource(m_osInterface, &m_cmdInitializerDataBuffer[m_encoder->m_currRecycledBufIdx][currentPass]);

    return eStatus;
}

MOS_STATUS CodechalCmdInitializer::CmdInitializerExecute(
    bool                brcEnabled,
    PMOS_RESOURCE       secondlevelBB,
    MOS_COMMAND_BUFFER* cmdBuffer)
{
    MHW_MI_FLUSH_DW_PARAMS                  flushDwParams;
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS       pipeModeSelectParams;
    MHW_VDBOX_HUC_IMEM_STATE_PARAMS         imemParams;
    MHW_VDBOX_HUC_DMEM_STATE_PARAMS         dmemParams;
    MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS       virtualAddrParams;
    MOS_LOCK_PARAMS                         lockFlagsWriteOnly;
    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS          vdPipeFlushParams;
    bool                                    requestFrameTracking;
    uint8_t                                 codec;
    MOS_STATUS                              eStatus = MOS_STATUS_SUCCESS;
    bool                                    renderingFlags;
    CodechalHwInterface                     *hwInterface;
    bool                                    externCmdBuffer = (cmdBuffer != nullptr);

    CODECHAL_ENCODE_FUNCTION_ENTER;

    hwInterface    = m_encoder->GetHwInterface();
    m_osInterface  = m_encoder->GetOsInterface();
    m_miInterface  = hwInterface->GetMiInterface();

    // for scalability, the cmdbuffer is passed outside
    // otherwise the cmdbuffer is fetched here
    if (cmdBuffer == nullptr)
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, cmdBuffer, 0));

    if (!m_encoder->m_singleTaskPhaseSupported || m_encoder->m_firstTaskInPhase)
    {
        // Send command buffer header at the beginning (OS dependent)
        requestFrameTracking = m_encoder->m_singleTaskPhaseSupported ? m_encoder->m_firstTaskInPhase : 0;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->SendPrologWithFrameTracking(cmdBuffer, requestFrameTracking));
    }

    // load kernel from WOPCM into L2 storage RAM
    MOS_ZeroMemory(&imemParams, sizeof(imemParams));
    imemParams.dwKernelDescriptor = m_hucCmdInitializerKernelDescriptor;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->GetHwInterface()->GetHucInterface()->AddHucImemStateCmd(cmdBuffer, &imemParams));

    // HUC_PIPE_MODE_SELECT    
    pipeModeSelectParams.Mode = m_encoder->m_mode;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->GetHwInterface()->GetHucInterface()->AddHucPipeModeSelectCmd(cmdBuffer, &pipeModeSelectParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CmdInitializerSetDmem(brcEnabled));

    // set HuC DMEM param
    MOS_ZeroMemory(&dmemParams, sizeof(dmemParams));
    dmemParams.presHucDataSource = &m_cmdInitializerDmemBuffer[m_encoder->m_currRecycledBufIdx][m_currentPass];
    dmemParams.dwDataLength      = MOS_ALIGN_CEIL(sizeof(HucComDmem), CODECHAL_CACHELINE_SIZE);
    dmemParams.dwDmemOffset      = HUC_DMEM_OFFSET_RTOS_GEMS;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->GetHwInterface()->GetHucInterface()->AddHucDmemStateCmd(cmdBuffer, &dmemParams));

    MOS_ZeroMemory(&virtualAddrParams, sizeof(virtualAddrParams));
    virtualAddrParams.regionParams[0].presRegion = &m_cmdInitializerDataBuffer[m_encoder->m_currRecycledBufIdx][m_currentPass];

    virtualAddrParams.regionParams[1].presRegion = secondlevelBB;

    virtualAddrParams.regionParams[1].isWritable = true;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->GetHwInterface()->GetHucInterface()->AddHucVirtualAddrStateCmd(cmdBuffer, &virtualAddrParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->GetHwInterface()->GetHucInterface()->AddHucStartCmd(cmdBuffer, true));

    // wait Huc completion (use HEVC bit for now)
    MOS_ZeroMemory(&vdPipeFlushParams, sizeof(vdPipeFlushParams));
    vdPipeFlushParams.Flags.bFlushHEVC = 1;
    vdPipeFlushParams.Flags.bWaitDoneHEVC = 1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->GetHwInterface()->GetVdencInterface()->AddVdPipelineFlushCmd(cmdBuffer, &vdPipeFlushParams));

    // Flush the engine to ensure memory written out
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));

    if (!m_encoder->m_singleTaskPhaseSupported && (m_osInterface->bNoParsingAssistanceInKmd))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));
    }

    if ((!m_encoder->m_singleTaskPhaseSupported))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(cmdBuffer, nullptr));
    }

    // if the cmdbuffer is passed outside, then we don't need to submit, just return
    if (externCmdBuffer)
        return eStatus;

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, cmdBuffer, 0);

    if (!m_encoder->m_singleTaskPhaseSupported)
    {
        renderingFlags = m_encoder->m_videoContextUsesNullHw;

        CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->GetDebugInterface()->DumpCmdBuffer(
            cmdBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            "HucCmd")));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, cmdBuffer, renderingFlags));
        CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpHucCmdInit(secondlevelBB)));
    }

    return eStatus;
}
#endif

#ifdef _VP9_ENCODE_VDENC_SUPPORTED
//VP9 Specific functions

MOS_STATUS CodechalCmdInitializer::CommandInitializerSetVp9Params(CodechalVdencVp9State *state)
{
    HucComData *    hucConstData;
    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(state);
    MOS_ZeroMemory(&m_vp9Params, sizeof(m_vp9Params));
    m_vp9Params.prevFrameSegEnabled = state->m_prevFrameSegEnabled;
    m_vp9Params.seqParams                    = state->m_vp9SeqParams;
    m_vp9Params.picParams                    = state->m_vp9PicParams;
    m_vp9Params.segmentationEnabled          = state->m_vp9PicParams->PicFlags.fields.segmentation_enabled;
    m_vp9Params.segmentMapProvided           = state->m_segmentMapProvided;
    m_vp9Params.prevFrameSegEnabled          = state->m_prevFrameSegEnabled;
    m_vp9Params.numRefFrames                 = state->m_numRefFrames;
    m_vp9Params.me16Enabled                  = state->m_16xMeEnabled;
    m_vp9Params.dysVdencMultiPassEnabled     = state->m_dysVdencMultiPassEnabled;
    m_vp9Params.vdencPakOnlyMultipassEnabled = state->m_vdencPakonlyMultipassEnabled;
    m_vp9Params.pictureCodingType            = state->m_pictureCodingType;
    m_vp9Params.currentPass                  = state->GetCurrentPass();
    m_currentPass                            = m_vp9Params.currentPass;
    m_vp9Params.singleTaskPhaseSupported     = state->m_singleTaskPhaseSupported;
    m_vp9Params.lastTaskInPhase              = state->m_lastTaskInPhase;
    m_vp9Params.firstTaskInPhase             = state->m_firstTaskInPhase;
    m_vp9Params.mode                         = state->m_mode;
    m_vdencInterface                         = state->m_vdencInterface;
    m_vp9Params.videoContextUsesNullHw       = state->m_videoContextUsesNullHw;
    m_vp9Params.debugInterface               = state->GetDebugInterface();
    m_vp9Params.dynamicScalingEnabled        = (state->m_dysRefFrameFlags != DYS_REF_NONE) ? true : false;
    m_vp9Params.segmentParams                = state->m_vp9SegmentParams;
    m_vp9Params.bPrevFrameKey                = state->m_prevFrameInfo.KeyFrame;
    return eStatus;
}

MOS_STATUS CodechalCmdInitializer::CmdInitializerVp9Execute(PMOS_COMMAND_BUFFER cmdBuffer, PMOS_RESOURCE picStateBuffer)
{
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams;
    MHW_VDBOX_HUC_IMEM_STATE_PARAMS   imemParams;
    MHW_VDBOX_HUC_DMEM_STATE_PARAMS   dmemParams;
    MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS virtualAddrParams;
    MOS_LOCK_PARAMS                   lockFlagsWriteOnly;
    bool                              requestFrameTracking;
    uint8_t                           codec;
    MOS_STATUS                        eStatus = MOS_STATUS_SUCCESS;
    bool                              renderingFlags;

    CODECHAL_ENCODE_FUNCTION_ENTER;
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_encoder);
    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    // load kernel from WOPCM into L2 storage RAM
    MOS_ZeroMemory(&imemParams, sizeof(imemParams));
    imemParams.dwKernelDescriptor = m_hucCmdInitializerKernelDescriptor;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucInterface()->AddHucImemStateCmd(cmdBuffer, &imemParams));

    // HUC_PIPE_MODE_SELECT
    pipeModeSelectParams.Mode = m_vp9Params.mode;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucInterface()->AddHucPipeModeSelectCmd(cmdBuffer, &pipeModeSelectParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CmdInitializerVp9SetDmem());

    // set HuC DMEM param
    MOS_ZeroMemory(&dmemParams, sizeof(dmemParams));
    if (m_vp9Params.dynamicScalingEnabled)
    {
        dmemParams.presHucDataSource = &m_cmdInitializerDysScalingDmemBuffer;
    }
    else
    {
        dmemParams.presHucDataSource = &m_cmdInitializerDmemBuffer[m_encoder->m_currRecycledBufIdx][m_vp9Params.currentPass];
    }
    dmemParams.dwDataLength = MOS_ALIGN_CEIL(sizeof(HucComDmem), CODECHAL_CACHELINE_SIZE);
    dmemParams.dwDmemOffset = HUC_DMEM_OFFSET_RTOS_GEMS;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucInterface()->AddHucDmemStateCmd(cmdBuffer, &dmemParams));

    MOS_ZeroMemory(&virtualAddrParams, sizeof(virtualAddrParams));
    if (m_vp9Params.dynamicScalingEnabled)
    {
        virtualAddrParams.regionParams[0].presRegion = &m_cmdInitializerDysScalingDataBuffer;
        virtualAddrParams.regionParams[1].presRegion = picStateBuffer; // Region 1  Output SLB Buffer Pass 1 (Output)
    }
    else
    {
        virtualAddrParams.regionParams[0].presRegion = &m_cmdInitializerDataBuffer[m_encoder->m_currRecycledBufIdx][m_vp9Params.currentPass];
        virtualAddrParams.regionParams[1].presRegion = picStateBuffer; // Region 1  Output SLB Buffer Pass 1 (Output)
    }

    virtualAddrParams.regionParams[1].isWritable = true;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucInterface()->AddHucVirtualAddrStateCmd(cmdBuffer, &virtualAddrParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucInterface()->AddHucStartCmd(cmdBuffer, true));

    // wait Huc completion (use HEVC bit for now)
    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdPipeFlushParams;
    MOS_ZeroMemory(&vdPipeFlushParams, sizeof(vdPipeFlushParams));
    vdPipeFlushParams.Flags.bFlushHEVC = 1;
    vdPipeFlushParams.Flags.bWaitDoneHEVC = 1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetVdencInterface()->AddVdPipelineFlushCmd(cmdBuffer, &vdPipeFlushParams));

    // Flush the engine to ensure memory written out
    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));

    if (!m_vp9Params.singleTaskPhaseSupported)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(cmdBuffer, nullptr));
    }

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, cmdBuffer, 0);

    if (!m_vp9Params.singleTaskPhaseSupported)
    {
        bool renderFlags = m_vp9Params.videoContextUsesNullHw;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, cmdBuffer, renderFlags));
    }

    return eStatus;
}

MOS_STATUS CodechalCmdInitializer::CmdInitializerVp9SetDmem()
{
    HucComDmem *    hucCmdInitializerDmem;
    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    uint16_t        offset  = 0;
    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_encoder);

    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;

    HucComData *hucConstData;
    if (m_vp9Params.dynamicScalingEnabled)
    {
        hucConstData = (HucComData *)m_osInterface->pfnLockResource(m_osInterface, &m_cmdInitializerDysScalingDataBuffer, &lockFlagsWriteOnly);
    }
    else
    {
        hucConstData = (HucComData *)m_osInterface->pfnLockResource(m_osInterface, &m_cmdInitializerDataBuffer[m_encoder->m_currRecycledBufIdx][m_vp9Params.currentPass], &lockFlagsWriteOnly);
    }

    MOS_ZeroMemory(hucConstData, sizeof(HucComData));

    hucConstData->TotalCommands = 2;

    // Command ID 2
    hucConstData->InputCOM[0].ID = 2;
    hucConstData->InputCOM[0].SizeOfData = 2;

    double qpScale = (m_vp9Params.pictureCodingType == I_TYPE) ? 0.31 : 0.33;
    uint8_t qp = m_vp9Params.picParams->LumaACQIndex;
    double lambda = qpScale * CODECHAL_VP9_QUANT_AC[qp] / 8;

    // SADQPLambda
    hucConstData->InputCOM[0].data[0] = (uint32_t)(lambda * 4 + 0.5);

    // Command ID 1
    hucConstData->InputCOM[1].ID = 1;
    hucConstData->InputCOM[1].SizeOfData = 0x17;

    HucInputCmd1  hucInputCmd1;
    MOS_ZeroMemory(&hucInputCmd1, sizeof(hucInputCmd1));
    hucInputCmd1.VdencStreamInEnabled = m_vp9Params.segmentMapProvided || m_vp9Params.me16Enabled;
    hucInputCmd1.SegMapStreamInEnabled = m_vp9Params.segmentMapProvided || m_vp9Params.me16Enabled;
    hucInputCmd1.PakOnlyMultipassEnable = m_vp9Params.vdencPakOnlyMultipassEnabled;
    hucInputCmd1.num_ref_idx_l0_active_minus1 = (m_vp9Params.picParams->PicFlags.fields.frame_type) ? m_vp9Params.numRefFrames - 1 : 0;

    hucInputCmd1.SADQPLambda = (uint16_t)(lambda * 4 + 0.5);
    hucInputCmd1.RDQPLambda = (uint16_t)(lambda * lambda * 4 + 0.5); //U14.2

    hucInputCmd1.SrcFrameHeightMinus1 = m_vp9Params.picParams->SrcFrameHeightMinus1;
    hucInputCmd1.SrcFrameWidthMinus1 = m_vp9Params.picParams->SrcFrameWidthMinus1;
    hucInputCmd1.SegmentationEnabled = m_vp9Params.segmentationEnabled;
    hucInputCmd1.PrevFrameSegEnabled = m_vp9Params.prevFrameSegEnabled;
    hucInputCmd1.LumaACQIndex = m_vp9Params.picParams->LumaACQIndex;
    hucInputCmd1.LumaDCQIndexDelta = m_vp9Params.picParams->LumaDCQIndexDelta;

    if (m_vp9Params.segmentationEnabled)
    {
        for (int i = 0; i < 8; i++)
        {
            hucInputCmd1.SegmentQIndexDelta[i] = m_vp9Params.segmentParams->SegData[i].SegmentQIndexDelta;
        }
    }

    MOS_SecureMemcpy(hucConstData->InputCOM[1].data, sizeof(HucInputCmd1), &hucInputCmd1, sizeof(HucInputCmd1));
    if (m_vp9Params.dynamicScalingEnabled)
    {
        m_osInterface->pfnUnlockResource(m_osInterface, &m_cmdInitializerDysScalingDataBuffer);
    }
    else
    {
        m_osInterface->pfnUnlockResource(m_osInterface, &m_cmdInitializerDataBuffer[m_encoder->m_currRecycledBufIdx][m_vp9Params.currentPass]);
    }
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;

    // Setup CmdInitializer DMEM
    if (m_vp9Params.dynamicScalingEnabled)
    {
        hucCmdInitializerDmem = (HucComDmem *)m_osInterface->pfnLockResource(
            m_osInterface, &m_cmdInitializerDysScalingDmemBuffer, &lockFlagsWriteOnly);
    }
    else
    {
        hucCmdInitializerDmem = (HucComDmem *)m_osInterface->pfnLockResource(
            m_osInterface, &m_cmdInitializerDmemBuffer[m_encoder->m_currRecycledBufIdx][m_vp9Params.currentPass], &lockFlagsWriteOnly);
    }
    MOS_ZeroMemory(hucCmdInitializerDmem, sizeof(HucComDmem));

    hucCmdInitializerDmem->TotalOutputCommands = 2;

    hucCmdInitializerDmem->TargetUsage = 4;

    hucCmdInitializerDmem->Codec = 1;
    hucCmdInitializerDmem->TargetUsage = (uint8_t)m_vp9Params.seqParams->TargetUsage;
    hucCmdInitializerDmem->FrameType = (uint8_t)m_vp9Params.picParams->PicFlags.fields.frame_type;
    hucCmdInitializerDmem->OutputCOM[0].ID = 2;
    hucCmdInitializerDmem->OutputCOM[0].Type = 1;
    hucCmdInitializerDmem->OutputCOM[0].StartInBytes = 0;

                                                          // Command ID 1
    hucCmdInitializerDmem->OutputCOM[1].ID = 1;
    hucCmdInitializerDmem->OutputCOM[1].Type = 1;
    hucCmdInitializerDmem->OutputCOM[1].StartInBytes = 544;

    hucCmdInitializerDmem->OutputSize = 544 + CODECHAL_CMD2_SIZE;
    if (m_vp9Params.dynamicScalingEnabled)
    {
        m_osInterface->pfnUnlockResource(m_osInterface, &m_cmdInitializerDysScalingDmemBuffer);
    }
    else
    {
        m_osInterface->pfnUnlockResource(m_osInterface, &m_cmdInitializerDmemBuffer[m_encoder->m_currRecycledBufIdx][m_vp9Params.currentPass]);
    }

    return eStatus;
}
#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS CodechalCmdInitializer::DumpHucCmdInit(PMOS_RESOURCE secondlevelBB)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    int idx = (m_encoder == nullptr) ? 0 : m_encoder->m_currRecycledBufIdx;
    // Dump DMEM
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->GetDebugInterface()->DumpHucDmem(
        &m_cmdInitializerDmemBuffer[idx][m_currentPass],
        sizeof(HucComDmem),
        m_currentPass,
        hucRegionDumpCmdInitializer));

    // Region 0 - input data buffer
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->GetDebugInterface()->DumpHucRegion(
        &m_cmdInitializerDataBuffer[idx][m_currentPass],
        0,
        sizeof(HucComData),
        0,
        "",
        true,
        m_currentPass,
        hucRegionDumpCmdInitializer));

    // Region 1 - output cmd
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->GetDebugInterface()->DumpHucRegion(
        secondlevelBB,
        0,
        m_hwInterface->m_vdencReadBatchBufferSize,
        1,
        "",
        false,
        m_currentPass,
        hucRegionDumpCmdInitializer));

    return MOS_STATUS_SUCCESS;
}
#endif
#endif
