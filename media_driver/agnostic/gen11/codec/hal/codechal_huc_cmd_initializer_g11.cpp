/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     codechal_huc_cmd_initializer_g11.cpp
//! \brief    Defines class for g11 command initializer encoder.
//!

#include "codec_def_encode_hevc.h"
#if defined (_HEVC_ENCODE_VME_SUPPORTED) || defined (_HEVC_ENCODE_VDENC_SUPPORTED)
#include "codechal_encode_hevc_base.h"
#endif
#include "codechal_huc_cmd_initializer_g11.h"
#include "codechal_encoder_base.h"
#ifdef _VP9_ENCODE_VDENC_SUPPORTED
#include "codechal_vdenc_vp9_base.h"
#endif

#define VDBOX_HUC_CMD_INITIALIZER_HEVC_CMD1_STARTOFFSERT_G11 36
#define VDBOX_HUC_CMD_INITIALIZER_HEVC_CMD2_STARTOFFSERT_G11 308
#define VDBOX_HUC_CMD_INITIALIZER_HEVC_CQP_CMD2_STARTOFFSET_G11 272
#define VDBOX_HUC_CMD_INITIALIZER_HEVC_CMD5_BRC_STARTOFFSET_G11 156
#define VDBOX_HUC_CMD_INITIALIZER_HEVC_CMD5_CQP_STARTOFFSET_G11 120

#define CMD_LIST_MODE 1
#define BATCH_BUFFER_END 0x05000000

CodechalCmdInitializerG11::CodechalCmdInitializerG11(
    CodechalEncoderState *encoder)
    : CodechalCmdInitializer(encoder)
{
}

#if defined (_HEVC_ENCODE_VME_SUPPORTED) || defined (_HEVC_ENCODE_VDENC_SUPPORTED)

MOS_STATUS CodechalCmdInitializerG11::ConstructHevcHucCmd1ConstData(
    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS seqParams,
    PCODEC_HEVC_ENCODE_PICTURE_PARAMS  picParams,
    PCODEC_HEVC_ENCODE_SLICE_PARAMS    sliceParams,
    struct HucComData *                hucConstData)
{
    hucConstData->InputCOM[1].ID         = 1;
    hucConstData->InputCOM[1].SizeOfData = sizeof(HucInputCmd1G11) / sizeof(uint32_t);

    auto   qpPrimeYAC = 10;  //This is constant from Arch C Model
    double qpScale    = (picParams->CodingType == I_TYPE) ? 0.60 : 0.65;

    HucInputCmd1G11 cmd1;
    MOS_ZeroMemory(&cmd1, sizeof(HucInputCmd1G11));

    // Shared HEVC/VP9
    cmd1.FrameWidthInMinCbMinus1  = seqParams->wFrameWidthInMinCbMinus1;
    cmd1.FrameHeightInMinCbMinus1 = seqParams->wFrameHeightInMinCbMinus1;
    cmd1.TransformSkip            = picParams->transform_skip_enabled_flag;

    cmd1.log2_min_coding_block_size_minus3 = seqParams->log2_min_coding_block_size_minus3;

    cmd1.tiles_enabled_flag           = picParams->tiles_enabled_flag;
    cmd1.VdencStreamInEnabled         = (uint8_t)m_streamInEnabled;
    cmd1.PakOnlyMultipassEnable       = m_pakOnlyPass;
    cmd1.num_ref_idx_l0_active_minus1 = sliceParams->num_ref_idx_l0_active_minus1;

    auto   qpPrimeYac = CodecHal_Clip3(10, 51, picParams->QpY + sliceParams->slice_qp_delta);
    double lambda     = sqrt(qpScale * pow(2.0, MOS_MAX(0, qpPrimeYac - 12) / 3.0));
    cmd1.SADQPLambda  = (uint16_t)(lambda * 4 + 0.5);
    cmd1.RDQPLambda   = (uint16_t)(qpScale * pow(2.0, MOS_MAX(0, picParams->QpY - 12) / 3.0) * 4 + 0.5);  //U14.2

    if (m_hevcVisualQualityImprovement)
    {
        if (qpPrimeYac >= 22 && qpPrimeYac <= 51 && sliceParams->slice_type == SLICE_I)
        {
            double ScalingFactor = 1.0 + 0.025 * (qpPrimeYac - 22);
            ScalingFactor = (ScalingFactor >= 1.5 ? 1.5 : ScalingFactor);
            cmd1.SADQPLambda = (uint16_t)(ScalingFactor * lambda * 4 + 0.5); //U8.2
        }

        if (picParams->QpY >= 22 && picParams->QpY <= 51)
        {
            cmd1.Intra32X32ModeMask = 507;
        }
    }

    cmd1.num_ref_idx_l1_active_minus1 = sliceParams->num_ref_idx_l1_active_minus1;
    cmd1.ROIStreamInEnabled           = (uint8_t)m_roiStreamInEnabled;
    cmd1.UseDefaultQpDeltas           = (m_acqpEnabled && seqParams->QpAdjustment) || (m_brcEnabled && seqParams->MBBRC != mbBrcDisabled);
    cmd1.TemporalMvpEnableFlag        = sliceParams->slice_temporal_mvp_enable_flag;
    cmd1.PanicEnabled                 = m_panicEnabled;

    if (m_roiStreamInEnabled)
    {
        for (int8_t i = 0; i < ENCODE_VDENC_HEVC_MAX_STREAMINROI_G10; i++)
        {
            cmd1.ROIDeltaQp[i] = picParams->ROIDistinctDeltaQp[i];
        }
    }

    // For TCBRC adaptive region boost
    if(m_brcAdaptiveRegionBoostEnabled)
    {
        cmd1.ROIStreamInEnabled = 1;
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

    cmd1.BRCMinQp = picParams->BRCMinQp < 0x0a ? 0x0a : picParams->BRCMinQp;                                        // Setting values from arch spec
    cmd1.BRCMaxQp = picParams->BRCMaxQp < 0x0a ? 0x33 : (picParams->BRCMaxQp > 0x33 ? 0x33 : picParams->BRCMaxQp);  // Setting values from arch spec

    MOS_SecureMemcpy(hucConstData->InputCOM[1].data, sizeof(HucInputCmd1G11), &cmd1, sizeof(HucInputCmd1G11));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalCmdInitializerG11::ConstructHevcHucCmd2ConstData(
    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS seqParams,
    PCODEC_HEVC_ENCODE_PICTURE_PARAMS  picParams,
    PCODEC_HEVC_ENCODE_SLICE_PARAMS    sliceParams,
    struct HucComData *                hucConstData)
{
    hucConstData->InputCOM[0].ID = 2;
    hucConstData->InputCOM[0].SizeOfData = 2;

    auto qpPrimeYAC = 10;  //This is constant from Arch C Model

    double qpScale = (picParams->CodingType == I_TYPE) ? 0.60 : 0.65;
    double lambdaInputCom = sqrt(qpScale * pow(2.0, MOS_MAX(0, qpPrimeYAC - 12) / 3.0));

    uint8_t SadPenaltyforIntraNonDC32X32PredMode = (uint8_t)(0.00 * 4 + 0.5);
    if (m_hevcVisualQualityImprovement)
    {
        auto   qpPrimeYac = CodecHal_Clip3(10, 51, picParams->QpY + sliceParams->slice_qp_delta);
        if (qpPrimeYac >= 22 && qpPrimeYac <= 51)
        {
            uint8_t penaltyForIntraNonDC32x32Predmode = (uint8_t)((0.1 * 63) * (qpPrimeYac - 22));
            penaltyForIntraNonDC32x32Predmode = (penaltyForIntraNonDC32x32Predmode >= 63) ? 63 : penaltyForIntraNonDC32x32Predmode;
            SadPenaltyforIntraNonDC32X32PredMode = penaltyForIntraNonDC32x32Predmode;
        }
    }

    hucConstData->InputCOM[0].data[0] = (uint32_t)(lambdaInputCom * 4 + 0.5);
    hucConstData->InputCOM[0].data[1] = (SadPenaltyforIntraNonDC32X32PredMode << 8) | (uint8_t)m_roiStreamInEnabled;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalCmdInitializerG11::AddCmdConstData(
    uint16_t cmdId,
    uint32_t* data,
    uint16_t size,
    uint32_t startOffset
)
{
    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    HucComData* hucConstData;
    int idx = m_cmdCount++;

    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;

    hucConstData = (HucComData *)m_osInterface->pfnLockResource(m_osInterface, &m_cmdInitializerDataBuffer[m_encoder->m_currRecycledBufIdx][m_currentPass], &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(hucConstData);

    hucConstData->TotalCommands = m_cmdCount;

    hucConstData->InputCOM[idx].ID = cmdId;
    hucConstData->InputCOM[idx].SizeOfData = size;
    
    MOS_SecureMemcpy(hucConstData->InputCOM[idx].data, size, data, size);

    m_osInterface->pfnUnlockResource(m_osInterface, &m_cmdInitializerDataBuffer[m_encoder->m_currRecycledBufIdx][m_currentPass]);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalCmdInitializerG11::CmdInitializerSetDmem(bool brcEnabled)
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
    CODECHAL_ENCODE_CHK_NULL_RETURN(hucCmdInitializerDmem);

    MOS_ZeroMemory(hucCmdInitializerDmem, sizeof(HucComDmem));

    CODECHAL_ENCODE_ASSERT(m_cmdCount == 3);
    hucCmdInitializerDmem->TotalOutputCommands = 3;

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
    hucCmdInitializerDmem->OutputCOM[1].BBEnd        = BATCH_BUFFER_END;

    offset += CODECHAL_CMDINITIALIZER_MAX_CMD_SIZE;

    // Command ID 5
    hucCmdInitializerDmem->OutputCOM[2].ID           = 5;
    hucCmdInitializerDmem->OutputCOM[2].Type         = 1;
    hucCmdInitializerDmem->OutputCOM[2].StartInBytes = GetCmd5StartOffset(brcEnabled);

    offset += CODECHAL_CMDINITIALIZER_MAX_CMD_SIZE;

    hucCmdInitializerDmem->OutputSize = offset;

    m_osInterface->pfnUnlockResource(m_osInterface, &m_cmdInitializerDmemBuffer[m_encoder->m_currRecycledBufIdx][m_currentPass]);

    return eStatus;
}

uint16_t CodechalCmdInitializerG11::GetCmd2StartOffset(bool brcEnabled)
{
    return brcEnabled ? VDBOX_HUC_CMD_INITIALIZER_HEVC_CMD2_STARTOFFSERT_G11 : VDBOX_HUC_CMD_INITIALIZER_HEVC_CQP_CMD2_STARTOFFSET_G11;
}

uint16_t CodechalCmdInitializerG11::GetCmd1StartOffset(bool brcEnabled)
{
    return brcEnabled ? VDBOX_HUC_CMD_INITIALIZER_HEVC_CMD1_STARTOFFSERT_G11 : 0;
}

uint16_t CodechalCmdInitializerG11::GetCmd5StartOffset(bool brcEnabled)
{
    return brcEnabled ? VDBOX_HUC_CMD_INITIALIZER_HEVC_CMD5_BRC_STARTOFFSET_G11 : VDBOX_HUC_CMD_INITIALIZER_HEVC_CMD5_CQP_STARTOFFSET_G11;
}


#endif

#ifdef _VP9_ENCODE_VDENC_SUPPORTED
//VP9 Specific functions
MOS_STATUS CodechalCmdInitializerG11::CmdInitializerVp9Execute(PMOS_COMMAND_BUFFER cmdBuffer, PMOS_RESOURCE picStateBuffer)
{
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS       pipeModeSelectParams;
    MHW_VDBOX_HUC_IMEM_STATE_PARAMS         imemParams;
    MHW_VDBOX_HUC_DMEM_STATE_PARAMS         dmemParams;
    MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS       virtualAddrParams;
    MOS_LOCK_PARAMS                         lockFlagsWriteOnly;
    bool                                    requestFrameTracking;
    uint8_t                                 codec;
    MOS_STATUS                              eStatus = MOS_STATUS_SUCCESS;
    bool                                    renderingFlags;

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

    return eStatus;
}

MOS_STATUS CodechalCmdInitializerG11::CmdInitializerVp9SetDmem()
{
    HucComDmem*             hucCmdInitializerDmem;
    MOS_LOCK_PARAMS         lockFlagsWriteOnly;
    uint16_t                offset = 0;
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_encoder);

    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;

    HucComData* hucConstData;
    if (m_vp9Params.dynamicScalingEnabled)
    {
        hucConstData = (HucComData*)m_osInterface->pfnLockResource(m_osInterface, &m_cmdInitializerDysScalingDataBuffer, &lockFlagsWriteOnly);
    }
    else
    {

        hucConstData = (HucComData*)m_osInterface->pfnLockResource(m_osInterface, &m_cmdInitializerDataBuffer[m_encoder->m_currRecycledBufIdx][m_vp9Params.currentPass], &lockFlagsWriteOnly);
    }
    CODECHAL_ENCODE_CHK_NULL_RETURN(hucConstData);

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
    hucConstData->InputCOM[1].SizeOfData = sizeof(HucInputCmd1G11) / sizeof(uint32_t);

    HucInputCmd1G11 hucInputCmd1;
    MOS_ZeroMemory(&hucInputCmd1, sizeof(hucInputCmd1));
    hucInputCmd1.VdencStreamInEnabled = m_vp9Params.segmentMapProvided || m_vp9Params.me16Enabled;
    hucInputCmd1.SegMapStreamInEnabled = m_vp9Params.segmentMapProvided || m_vp9Params.me16Enabled;
    hucInputCmd1.PakOnlyMultipassEnable = m_vp9Params.vdencPakOnlyMultipassEnabled;
    hucInputCmd1.num_ref_idx_l0_active_minus1 = (m_vp9Params.picParams->PicFlags.fields.frame_type) ? m_vp9Params.numRefFrames - 1 : 0;

    hucInputCmd1.SADQPLambda = (uint16_t)(lambda * 4 + 0.5);
    hucInputCmd1.RDQPLambda = (uint16_t)(lambda * lambda * 4 + 0.5); //U14.2

    hucInputCmd1.SrcFrameHeight = m_vp9Params.picParams->SrcFrameHeightMinus1 + 1;
    hucInputCmd1.SrcFrameWidth = m_vp9Params.picParams->SrcFrameWidthMinus1 + 1;
    hucInputCmd1.SegmentationEnabled = m_vp9Params.segmentationEnabled;
    hucInputCmd1.PrevFrameSegEnabled = m_vp9Params.prevFrameSegEnabled;
    hucInputCmd1.LumaACQIndex = m_vp9Params.picParams->LumaACQIndex;
    hucInputCmd1.LumaDCQIndexDelta = m_vp9Params.picParams->LumaDCQIndexDelta;
    hucInputCmd1.log2_tile_columns = m_vp9Params.picParams->log2_tile_columns;
    hucInputCmd1.log2_tile_rows = m_vp9Params.picParams->log2_tile_rows;
    hucInputCmd1.DynamicScalingEnabled = m_vp9Params.dynamicScalingEnabled;
    hucInputCmd1.dysVdencMultiPassEnabled = m_vp9Params.dysVdencMultiPassEnabled;

    if (m_vp9Params.segmentationEnabled)
    {
        for (int i = 0; i < 8; i++)
        {
            hucInputCmd1.SegmentQIndexDelta[i] = m_vp9Params.segmentParams->SegData[i].SegmentQIndexDelta;
        }
    }

    MOS_SecureMemcpy(hucConstData->InputCOM[1].data, sizeof(HucInputCmd1G11), &hucInputCmd1, sizeof(HucInputCmd1G11));
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
        hucCmdInitializerDmem = (HucComDmem*)m_osInterface->pfnLockResource(
            m_osInterface, &m_cmdInitializerDysScalingDmemBuffer, &lockFlagsWriteOnly);
    }
    else
    {
        hucCmdInitializerDmem = (HucComDmem*)m_osInterface->pfnLockResource(
            m_osInterface, &m_cmdInitializerDmemBuffer[m_encoder->m_currRecycledBufIdx][m_vp9Params.currentPass], &lockFlagsWriteOnly);
    }
    CODECHAL_ENCODE_CHK_NULL_RETURN(hucCmdInitializerDmem);

    MOS_ZeroMemory(hucCmdInitializerDmem, sizeof(HucComDmem));

    hucCmdInitializerDmem->TotalOutputCommands = 2;

    hucCmdInitializerDmem->Codec = 1;
    hucCmdInitializerDmem->TargetUsage = (uint8_t)m_vp9Params.seqParams->TargetUsage;
    hucCmdInitializerDmem->FrameType = (uint8_t)m_vp9Params.picParams->PicFlags.fields.frame_type;

    // Command ID 2
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
#endif

MOS_STATUS CodechalCmdInitializerG11::CmdInitializerAllocateResources(CodechalHwInterface* hwInterface)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalCmdInitializer::CmdInitializerAllocateResources(hwInterface));

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format = Format_Buffer;

    for (int i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
    {
        for (int j = 0; j < CODECHAL_VDENC_BRC_NUM_OF_PASSES; j++)
        {
            // Cmd Initializer Copy DMEM
            allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(sizeof(HucComDmem), CODECHAL_CACHELINE_SIZE);
            allocParamsForBufferLinear.pBufName = "VDEnc CmdInitializer Copy Dmem Buffer";
            CODECHAL_ENCODE_CHK_STATUS_RETURN(
                m_osInterface->pfnAllocateResource(
                    m_osInterface,
                    &allocParamsForBufferLinear,
                    &m_cmdInitializerCopyDmemBuffer[i][j]));

            // Cmd Initializer Copy Data buffer
            allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(sizeof(HucComData), CODECHAL_PAGE_SIZE);
            allocParamsForBufferLinear.pBufName = "VDEnc CmdInitializer Copy Data Buffer";
            CODECHAL_ENCODE_CHK_STATUS_RETURN(
                m_osInterface->pfnAllocateResource(
                    m_osInterface,
                    &allocParamsForBufferLinear,
                    &m_cmdInitializerCopyDataBuffer[i][j]));

            MOS_LOCK_PARAMS lockFlagsWriteOnly;
            MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
            lockFlagsWriteOnly.WriteOnly = 1;

            uint8_t* pData = (uint8_t*)m_osInterface->pfnLockResource(
                m_osInterface,
                &m_cmdInitializerCopyDataBuffer[i][j],
                &lockFlagsWriteOnly);
            CODECHAL_ENCODE_CHK_NULL_RETURN(pData);
            MOS_ZeroMemory(pData, allocParamsForBufferLinear.dwBytes);
            m_osInterface->pfnUnlockResource(m_osInterface, &m_cmdInitializerCopyDataBuffer[i][j]);
        }
    }

    MOS_ZeroMemory(&m_vdencCopyBatchBuffer, sizeof(m_vdencCopyBatchBuffer));
    m_vdencCopyBatchBuffer.bSecondLevel = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
        m_osInterface,
        &m_vdencCopyBatchBuffer,
        nullptr,
        m_hwInterface->m_vdencCopyBatchBufferSize));
    
    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_VQI_ENABLE_ID,
        &userFeatureData,
        m_osInterface->pOsContext);
    m_hevcVisualQualityImprovement = userFeatureData.i32Data ? true : false;

    return eStatus;
}

void CodechalCmdInitializerG11::CmdInitializerFreeResources()
{
    CodechalCmdInitializer::CmdInitializerFreeResources();

    for (int i = 0; i < CODECHAL_VDENC_BRC_NUM_OF_PASSES; i++)
    {
        for (int j = 0; j < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; j++)
        {
            m_osInterface->pfnFreeResource(m_osInterface, &m_cmdInitializerCopyDmemBuffer[j][i]);
            m_osInterface->pfnFreeResource(m_osInterface, &m_cmdInitializerCopyDataBuffer[j][i]);
        }
    }

    Mhw_FreeBb(m_osInterface, &m_vdencCopyBatchBuffer, nullptr);
}

MOS_STATUS CodechalCmdInitializerG11::AddCopyCmds(
    PMOS_COMMAND_BUFFER cmdBuffer,
    HucCopyParams* params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    // load kernel from WOPCM into L2 storage RAM
    MHW_VDBOX_HUC_IMEM_STATE_PARAMS imemParams;
    MOS_ZeroMemory(&imemParams, sizeof(imemParams));
    imemParams.dwKernelDescriptor = m_hucCmdInitializerKernelDescriptor;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucInterface()->AddHucImemStateCmd(cmdBuffer, &imemParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCopyDmem());

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCopyData(params));

    // pipe mode select
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams;
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_encoder);
    pipeModeSelectParams.Mode = m_encoder->m_mode;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucInterface()->AddHucPipeModeSelectCmd(cmdBuffer, &pipeModeSelectParams));

    // set HuC DMEM param
    MHW_VDBOX_HUC_DMEM_STATE_PARAMS dmemParams;
    MOS_ZeroMemory(&dmemParams, sizeof(dmemParams));
    dmemParams.presHucDataSource = &m_cmdInitializerCopyDmemBuffer[m_encoder->m_currRecycledBufIdx][m_currentPass];
    dmemParams.dwDataLength = MOS_ALIGN_CEIL(sizeof(HucComDmem), CODECHAL_CACHELINE_SIZE);
    dmemParams.dwDmemOffset = HUC_DMEM_OFFSET_RTOS_GEMS;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucInterface()->AddHucDmemStateCmd(cmdBuffer, &dmemParams));

    MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS virtualAddrParams;
    MOS_ZeroMemory(&virtualAddrParams, sizeof(virtualAddrParams));
    virtualAddrParams.regionParams[0].presRegion = &m_cmdInitializerCopyDataBuffer[m_encoder->m_currRecycledBufIdx][m_currentPass];
    virtualAddrParams.regionParams[1].presRegion = &m_vdencCopyBatchBuffer.OsResource;
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

    // current location to add cmds in 2nd level batch buffer
    m_vdencCopyBatchBuffer.iCurrent = 0;
    // reset starting location (offset) executing 2nd level batch buffer for each frame & each pass
    m_vdencCopyBatchBuffer.dwOffset = 0;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(cmdBuffer, &m_vdencCopyBatchBuffer));

    // This wait cmd is needed to make sure copy command is done as suggested by HW folk in encode cases
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMfxWaitCmd(cmdBuffer, nullptr, m_osInterface->osCpInterface->IsCpEnabled() ? true : false));

    return eStatus;
}

MOS_STATUS CodechalCmdInitializerG11::SetCopyDmem()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    HucComDmem* hucCmdCopyDmem;
    MOS_LOCK_PARAMS lockFlagsWriteOnly;

    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;

    hucCmdCopyDmem = (HucComDmem*)m_osInterface->pfnLockResource(
        m_osInterface, &m_cmdInitializerCopyDmemBuffer[m_encoder->m_currRecycledBufIdx][m_currentPass], &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(hucCmdCopyDmem);

    MOS_ZeroMemory(hucCmdCopyDmem, sizeof(HucComDmem));

    hucCmdCopyDmem->TotalOutputCommands = 1;

    hucCmdCopyDmem->FrameType = m_encoder->m_pictureCodingType - 1;
    hucCmdCopyDmem->OutputCOM[0].ID = 3;
    hucCmdCopyDmem->OutputCOM[0].Type = 1;
    hucCmdCopyDmem->OutputCOM[0].StartInBytes = 0;
    hucCmdCopyDmem->OutputCOM[0].BBEnd = BATCH_BUFFER_END;

    hucCmdCopyDmem->OutputSize = m_hwInterface->m_vdencCopyBatchBufferSize;

    m_osInterface->pfnUnlockResource(m_osInterface, &m_cmdInitializerCopyDmemBuffer[m_encoder->m_currRecycledBufIdx][m_currentPass]);

    return eStatus;
}

MOS_STATUS CodechalCmdInitializerG11::SetCopyData(
    HucCopyParams* params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;

    auto hucConstData = (HucComData*)m_osInterface->pfnLockResource(m_osInterface, &m_cmdInitializerCopyDataBuffer[m_encoder->m_currRecycledBufIdx][m_currentPass], &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(hucConstData);

    MOS_ZeroMemory(hucConstData, sizeof(HucComData));
    hucConstData->TotalCommands = 1;

    hucConstData->InputCOM[0].ID = CODECHAL_CMD3;
    hucConstData->InputCOM[0].SizeOfData = 0xF;

    HucInputCmd3G11 hucInputCmd3;
    MOS_ZeroMemory(&hucInputCmd3, sizeof(HucInputCmd3G11));

    hucInputCmd3.SelectionForIndData = m_osInterface->osCpInterface->IsCpEnabled() ? 4 : 0;
    hucInputCmd3.CmdMode = CMD_LIST_MODE;
    hucInputCmd3.LengthOfTable = params->lengthOfTable;
    hucInputCmd3.CopySize = params->size;

    uint64_t srcAddr = m_osInterface->pfnGetResourceGfxAddress(m_osInterface, params->presSrc);
    uint64_t destAddr = m_osInterface->pfnGetResourceGfxAddress(m_osInterface, params->presDst);
    hucInputCmd3.SrcAddrBottom = (uint32_t)(srcAddr & 0x00000000FFFFFFFF);
    hucInputCmd3.SrcAddrTop = (uint32_t)((srcAddr & 0xFFFFFFFF00000000) >> 32);

    hucInputCmd3.DestAddrBottom = (uint32_t)(destAddr & 0x00000000FFFFFFFF);
    hucInputCmd3.DestAddrTop = (uint32_t)((destAddr & 0xFFFFFFFF00000000) >> 32);

    MOS_SecureMemcpy(hucConstData->InputCOM[0].data, sizeof(HucInputCmd3G11), &hucInputCmd3, sizeof(HucInputCmd3G11));

    m_osInterface->pfnUnlockResource(m_osInterface, &m_cmdInitializerCopyDataBuffer[m_encoder->m_currRecycledBufIdx][m_currentPass]);

    return eStatus;
}
