/*
* Copyright (c) 2018-2020, Intel Corporation
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
//! \file     encode_avc_vdenc_pipeline.cpp
//! \brief    Defines the interface for avc vdenc encode pipeline
//!

#include "encode_avc_vdenc_pipeline.h"
#include "encode_avc_basic_feature.h"
#include "encode_avc_brc.h"
#include "encode_scalability_defs.h"
#include "encode_status_report_defs.h"
#include "media_avc_feature_defs.h"
#include "encode_preenc_packet.h"
#include "encode_avc_vdenc_preenc.h"

// SubMbPartMask defined in CURBE for AVC ENC
#define CODECHAL_ENCODE_AVC_DISABLE_4X4_SUB_MB_PARTITION 0x40
#define CODECHAL_ENCODE_AVC_DISABLE_4X8_SUB_MB_PARTITION 0x20
#define CODECHAL_ENCODE_AVC_DISABLE_8X4_SUB_MB_PARTITION 0x10

namespace encode {

AvcVdencPipeline::AvcVdencPipeline(
    CodechalHwInterfaceNext *   hwInterface,
    CodechalDebugInterface *debugInterface)
    : EncodePipeline(hwInterface, debugInterface)
{
}

MOS_STATUS AvcVdencPipeline::Initialize(void *settings)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(InitUserSetting(m_userSettingPtr));
    ENCODE_CHK_STATUS_RETURN(EncodePipeline::Initialize(settings));

#if MHW_HWCMDPARSER_ENABLED
    mhw::HwcmdParser::InitInstance(m_osInterface, mhw::HwcmdParser::AddOnMode::AVCe);
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencPipeline::Uninitialize()
{
    ENCODE_FUNC_CALL();

    if (m_mmcState != nullptr)
    {
        MOS_Delete(m_mmcState);
    }

    ENCODE_CHK_STATUS_RETURN(EncodePipeline::Uninitialize());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencPipeline::UserFeatureReport()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(EncodePipeline::UserFeatureReport());

    ReportUserSetting(
        m_userSettingPtr,
        "AVC Encode Mode",
        m_codecFunction,
        MediaUserSetting::Group::Sequence);

#if (_DEBUG || _RELEASE_INTERNAL)
    ReportUserSettingForDebug(
        m_userSettingPtr,
        "VDENC In Use",
        1,
        MediaUserSetting::Group::Frame);

    ReportUserSettingForDebug(
        m_userSettingPtr,
        "Enable Encode VE CtxBasedScheduling",
        MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface),
        MediaUserSetting::Group::Frame);
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencPipeline::Prepare(void *params)
{
    ENCODE_FUNC_CALL();

    EncoderParams *encodeParams = (EncoderParams *)params;
    ENCODE_CHK_NULL_RETURN(encodeParams);

    //TODO: should check with m_codecFunction
    if (encodeParams->ExecCodecFunction != CODECHAL_FUNCTION_ENC_VDENC_PAK)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    ENCODE_CHK_STATUS_RETURN(EncodePipeline::Prepare(params));

    auto basicFeature = dynamic_cast<AvcBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(basicFeature);

    CODECHAL_DEBUG_TOOL
    (
        m_debugInterface->m_currPic            = basicFeature->m_currOriginalPic;
        m_debugInterface->m_bufferDumpFrameNum = basicFeature->m_frameNum + 1;
        m_debugInterface->m_frameType          = basicFeature->m_pictureCodingType;

        if (basicFeature->m_newSeq) {
            ENCODE_CHK_STATUS_RETURN(DumpSeqParams(
                basicFeature->m_seqParam,
                basicFeature->m_iqMatrixParams));
        }

        if (basicFeature->m_newVuiData) {
            ENCODE_CHK_STATUS_RETURN(DumpVuiParams(
                basicFeature->m_vuiParams));
        }

        ENCODE_CHK_STATUS_RETURN(DumpPicParams(
            basicFeature->m_picParam,
            basicFeature->m_iqMatrixParams));

        for (uint32_t i = 0; i < basicFeature->m_numSlices; i++) {
            ENCODE_CHK_STATUS_RETURN(DumpSliceParams(
                &basicFeature->m_sliceParams[i],
                basicFeature->m_picParam));
        }
    )

    PCODEC_AVC_ENCODE_PIC_PARAMS picParams = static_cast<PCODEC_AVC_ENCODE_PIC_PARAMS>(encodeParams->pPicParams);
    ENCODE_CHK_NULL_RETURN(picParams);

    ENCODE_CHK_STATUS_RETURN(SwitchContext(basicFeature->m_outputChromaFormat));

    EncoderStatusParameters inputParameters = {};
    MOS_ZeroMemory(&inputParameters, sizeof(EncoderStatusParameters));

    inputParameters.statusReportFeedbackNumber = picParams->StatusReportFeedbackNumber;
    inputParameters.codecFunction              = encodeParams->ExecCodecFunction;
    inputParameters.currRefList                = basicFeature->m_ref->GetCurrRefList();
    inputParameters.picWidthInMb               = basicFeature->m_picWidthInMb;
    inputParameters.frameFieldHeightInMb       = basicFeature->m_frameFieldHeightInMb;
    inputParameters.currOriginalPic            = basicFeature->m_currOriginalPic;
    inputParameters.pictureCodingType          = basicFeature->m_pictureCodingType;
    inputParameters.numUsedVdbox               = m_numVdbox;
    inputParameters.hwWalker                   = false;
    inputParameters.maxNumSlicesAllowed        = basicFeature->m_maxNumSlicesAllowed;
    inputParameters.numberTilesInFrame         = 1;

    ENCODE_CHK_STATUS_RETURN(m_statusReport->Init(&inputParameters));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencPipeline::Execute()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(ActivateVdencVideoPackets());
    ENCODE_CHK_STATUS_RETURN(ExecuteActivePackets());
    ENCODE_CHK_STATUS_RETURN(ResetParams());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencPipeline::GetStatusReport(void *status, uint16_t numStatus)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(m_statusReport->GetReport(numStatus, status));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencPipeline::Destroy()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(Uninitialize());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencPipeline::ActivateVdencVideoPackets()
{
    ENCODE_FUNC_CALL();

    auto brcFeature = dynamic_cast<AvcEncodeBRC*>(m_featureManager->GetFeature(AvcFeatureIDs::avcBrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);
    bool immediateSubmit = !m_singleTaskPhaseSupported;
    ENCODE_NORMALMESSAGE("immediateSubmit = %d", immediateSubmit);

    if (m_preEncEnabled)
    {
        ENCODE_CHK_STATUS_RETURN(ActivatePacket(encodePreEncPacket, immediateSubmit, 0, 0));
        ENCODE_NORMALMESSAGE("encodePreEncPacket was activated");
        if (m_encodeMode == MediaEncodeMode::MANUAL_RES_PRE_ENC || m_encodeMode == MediaEncodeMode::AUTO_RES_PRE_ENC)
        {
            m_activePacketList.back().immediateSubmit = true;
            return MOS_STATUS_SUCCESS;
        }
    }

    if (brcFeature->IsBRCInitRequired())
    {
        ENCODE_NORMALMESSAGE("HucBrcInit was activated");
        ENCODE_CHK_STATUS_RETURN(ActivatePacket(HucBrcInit, immediateSubmit, 0, 0));
    }

    for (uint8_t curPass = 0; curPass < GetPassNum(); curPass++)
    {
        if (brcFeature->IsBRCUpdateRequired())
        {
            ENCODE_NORMALMESSAGE("HucBrcUpdate was activated");
            ENCODE_CHK_STATUS_RETURN(ActivatePacket(HucBrcUpdate, immediateSubmit, curPass, 0));
        }
        ENCODE_NORMALMESSAGE("VdencPacket was activated");
        ENCODE_CHK_STATUS_RETURN(ActivatePacket(VdencPacket, immediateSubmit, curPass, 0));
    }

    SetFrameTrackingForMultiTaskPhase();

    // Last element in m_activePacketList must be immediately submitted
    m_activePacketList.back().immediateSubmit = true;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencPipeline::CreateBufferTracker()
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencPipeline::CreateStatusReport()
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencPipeline::ResetParams()
{
    ENCODE_FUNC_CALL();

    auto avcBasicfeature = dynamic_cast<AvcBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(avcBasicfeature);

    m_currRecycledBufIdx = (m_currRecycledBufIdx + 1) % CODECHAL_ENCODE_RECYCLED_BUFFER_NUM;
    if (m_currRecycledBufIdx == 0)
    {
        MOS_ZeroMemory(m_recycledBufStatusNum, sizeof(m_recycledBufStatusNum));
    }

    // Only update user features for first frame.
    if (avcBasicfeature->m_frameNum == 0)
    {
        ENCODE_CHK_STATUS_RETURN(UserFeatureReport());
    }

    avcBasicfeature->m_frameNum++;

    ENCODE_CHK_STATUS_RETURN(m_statusReport->Reset());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencPipeline::SwitchContext(uint8_t outputChromaFormat)
{
    ENCODE_FUNC_CALL();

    if (!m_scalPars)
    {
        m_scalPars = std::make_shared<EncodeScalabilityPars>();
    }

    *m_scalPars             = {};
    m_scalPars->enableVDEnc = true;
    m_scalPars->enableVE    = MOS_VE_SUPPORTED(m_osInterface);
    m_scalPars->numVdbox    = m_numVdbox;

    m_scalPars->forceMultiPipe     = false;
    m_scalPars->outputChromaFormat = outputChromaFormat;

    m_scalPars->numTileRows    = 1;
    m_scalPars->numTileColumns = 1;

    m_scalPars->IsPak = true;

    m_mediaContext->SwitchContext(VdboxEncodeFunc, &*m_scalPars, &m_scalability);
    ENCODE_CHK_NULL_RETURN(m_scalability);

    m_scalability->SetPassNumber(m_featureManager->GetNumPass());

    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS AvcVdencPipeline::DumpEncodePicReorder(
    std::ostringstream       &oss,
    uint32_t                  x,
    uint32_t                  y,
    const CODEC_PIC_REORDER * picReorder)
{
    uint8_t botField;

    CODECHAL_DEBUG_CHK_NULL(picReorder);

    botField = CodecHal_PictureIsBottomField(picReorder->Picture) ? 1 : 0;

    oss << "# PicOrder[" << std::dec << +x << "][" << std::dec << +y << "] =" << std::endl;
    oss << "# \tPicNum = " << std::dec << +picReorder->PicNum << std::endl;
    oss << "# \tPOC = " << std::dec << +picReorder->POC << std::endl;
    oss << "# \tReorderPicNumIDC = " << std::dec << +picReorder->ReorderPicNumIDC << std::endl;
    oss << "# \tDiffPicNumMinus1 = " << std::dec << +picReorder->DiffPicNumMinus1 << std::endl;
    oss << "# \tFrameIdx = " << std::dec << +picReorder->Picture.FrameIdx << std::endl;
    oss << "# \tBotField = " << std::dec << +botField << std::endl;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencPipeline::DumpSeqParams(
    const CODEC_AVC_ENCODE_SEQUENCE_PARAMS *seqParams,
    const CODEC_AVC_IQ_MATRIX_PARAMS       *matrixParams)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSeqParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(seqParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "# DDI Parameters:" << std::endl;
    oss << "FrameWidth = " << +seqParams->FrameWidth << std::endl;
    oss << "FrameHeight = " << +seqParams->FrameHeight << std::endl;
    oss << "Profile = " << +seqParams->Profile << std::endl;
    oss << "Level = " << +seqParams->Level << std::endl;
    oss << "GopPicSize = " << +seqParams->GopPicSize << std::endl;
    oss << "GopRefDist = " << +seqParams->GopRefDist << std::endl;
    oss << "GopOptFlag = " << +seqParams->GopOptFlag << std::endl;
    oss << "TargetUsage = " << +seqParams->TargetUsage << std::endl;
    oss << "RateControlMethod = " << +seqParams->RateControlMethod << std::endl;
    oss << "TargetBitRate = " << +seqParams->TargetBitRate << std::endl;
    oss << "MaxBitRate = " << +seqParams->MaxBitRate << std::endl;
    oss << "MinBitRate = " << +seqParams->MinBitRate << std::endl;
    oss << "FramesPer100Sec = " << +seqParams->FramesPer100Sec << std::endl;
    oss << "InitVBVBufferFullnessInBit = " << +seqParams->InitVBVBufferFullnessInBit << std::endl;
    oss << "VBVBufferSizeInBit = " << +seqParams->VBVBufferSizeInBit << std::endl;
    oss << "NumRefFrames = " << +seqParams->NumRefFrames / 2 << std::endl;  // this prints the value passed from DDI
    oss << "# NumRefFrames (Actual Value in CodecHal is twice the value passed from DDI) = "
        << +seqParams->NumRefFrames << std::endl;  // this prints the actual value in CodecHal seq param structure
    oss << "seq_parameter_set_id = " << +seqParams->seq_parameter_set_id << std::endl;
    oss << "chroma_format_idc = " << +seqParams->chroma_format_idc << std::endl;
    oss << "bit_depth_luma_minus8 = " << +seqParams->bit_depth_luma_minus8 << std::endl;
    oss << "bit_depth_chroma_minus8 = " << +seqParams->bit_depth_chroma_minus8 << std::endl;
    oss << "log2_max_frame_num_minus4 = " << +seqParams->log2_max_frame_num_minus4 << std::endl;
    oss << "pic_order_cnt_type = " << +seqParams->pic_order_cnt_type << std::endl;
    oss << "log2_max_pic_order_cnt_lsb_minus4 = " << +seqParams->log2_max_pic_order_cnt_lsb_minus4 << std::endl;
    oss << "num_ref_frames_in_pic_order_cnt_cycle = " << +seqParams->num_ref_frames_in_pic_order_cnt_cycle << std::endl;
    oss << "offset_for_non_ref_pic = " << +seqParams->offset_for_non_ref_pic << std::endl;
    oss << "offset_for_top_to_bottom_field = " << +seqParams->offset_for_top_to_bottom_field << std::endl;

    // Conditionally printed (only when pic_order_cnt_type = 1).  Contains 256 elements.
    if (seqParams->pic_order_cnt_type == 1)
    {
        for (uint16_t i = 0; i < 256; ++i)
        {
            oss << "offset_for_ref_frame[" << +i << "] = " << +seqParams->offset_for_ref_frame[i] << std::endl;
        }
    }

    oss << "frame_crop_left_offset = " << +seqParams->frame_crop_left_offset << std::endl;
    oss << "frame_crop_right_offset = " << +seqParams->frame_crop_right_offset << std::endl;
    oss << "frame_crop_top_offset = " << +seqParams->frame_crop_top_offset << std::endl;
    oss << "frame_crop_bottom_offset = " << +seqParams->frame_crop_bottom_offset << std::endl;
    oss << "seq_scaling_matrix_present_flag = " << +seqParams->seq_scaling_matrix_present_flag << std::endl;
    oss << "seq_scaling_list_present_flag = " << +seqParams->seq_scaling_list_present_flag[0] << std::endl;

    // seq_scaling_list_present_flag with 12 elements (only 1 element acknowledged in DDI doc)
    oss << "# seq_scaling_list_present_flag[1-11]:";
    for (uint8_t i = 1; i < 12; i++)
        oss << +seqParams->seq_scaling_list_present_flag[i] << " ";
    oss << std::endl;

    oss << "delta_pic_order_always_zero_flag = " << +seqParams->delta_pic_order_always_zero_flag << std::endl;
    oss << "frame_mbs_only_flag = " << +seqParams->frame_mbs_only_flag << std::endl;
    oss << "direct_8x8_inference_flag = " << +seqParams->direct_8x8_inference_flag << std::endl;
    oss << "vui_parameters_present_flag = " << +seqParams->vui_parameters_present_flag << std::endl;
    oss << "frame_cropping_flag = " << +seqParams->frame_cropping_flag << std::endl;
    oss << "EnableSliceLevelRateCtrl = " << +seqParams->EnableSliceLevelRateCtrl << std::endl;
    oss << "ICQQualityFactor = " << +seqParams->ICQQualityFactor << std::endl;
    oss << "InputColorSpace = " << +seqParams->InputColorSpace << std::endl;

    // begining of union/struct
    oss << "# bResetBRC = " << +seqParams->bResetBRC << std::endl;
    oss << "# bNoAcceleratorSPSInsertion = " << +seqParams->bNoAcceleratorSPSInsertion << std::endl;
    oss << "# GlobalSearch = " << +seqParams->GlobalSearch << std::endl;
    oss << "# LocalSearch = " << +seqParams->LocalSearch << std::endl;
    oss << "# EarlySkip = " << +seqParams->EarlySkip << std::endl;
    oss << "# Trellis = " << +seqParams->Trellis << std::endl;
    oss << "# MBBRC = " << +seqParams->MBBRC << std::endl;
    oss << "# bTemporalScalability = " << +seqParams->bTemporalScalability << std::endl;
    oss << "# ROIValueInDeltaQP = " << +seqParams->ROIValueInDeltaQP << std::endl;
    oss << "# bAutoMaxPBFrameSizeForSceneChange = " << +seqParams->bAutoMaxPBFrameSizeForSceneChange << std::endl;
    oss << "sFlags = " << +seqParams->sFlags << std::endl;

    // end of union/struct
    oss << "UserMaxIFrameSize = " << +seqParams->UserMaxFrameSize << std::endl;
    oss << "UserMaxPBFrameSize = " << +seqParams->UserMaxPBFrameSize << std::endl;

    // Parameters not defined in DDI (Any non-DDI parameters printed should be preceeded by #)
    oss << "# Non-DDI Parameters:" << std::endl;
    oss << "# constraint_set0_flag = " << std::hex << +seqParams->constraint_set0_flag << std::endl;
    oss << "# constraint_set1_flag = " << std::hex << +seqParams->constraint_set1_flag << std::endl;
    oss << "# constraint_set2_flag = " << std::hex << +seqParams->constraint_set2_flag << std::endl;
    oss << "# constraint_set3_flag = " << std::hex << +seqParams->constraint_set3_flag << std::endl;

    oss << "# separate_colour_plane_flag = " << std::hex << +seqParams->separate_colour_plane_flag << std::endl;
    oss << "# qpprime_y_zero_transform_bypass_flag = " << std::hex << +seqParams->qpprime_y_zero_transform_bypass_flag << std::endl;
    oss << "# gaps_in_frame_num_value_allowed_flag = " << std::hex << +seqParams->gaps_in_frame_num_value_allowed_flag << std::endl;
    oss << "# pic_width_in_mbs_minus1 = " << std::hex << +seqParams->pic_width_in_mbs_minus1 << std::endl;
    oss << "# pic_height_in_map_units_minus1 = " << std::hex << +seqParams->pic_height_in_map_units_minus1 << std::endl;
    oss << "# mb_adaptive_frame_field_flag = " << std::hex << +seqParams->mb_adaptive_frame_field_flag << std::endl;

    // Dump ScalingList4x4 (6 x 16)
    for (uint8_t i = 0; i < 16; ++i)
    {
        oss << "# ScalingList4x4[" << std::dec << +i * 6 << "-" << (+i * 6) + 5 << "][" << +i << "]";
        for (uint8_t j = 0; j < 6; j++)
            oss << std::hex << +matrixParams->ScalingList4x4[j][i] << " ";
        oss << std::endl;
    }

    // ScalingList8x8 (2 x 64)
    for (uint8_t i = 0; i < 64; ++i)
    {
        oss << "# ScalingList8x8[0 / 1][ " << std::dec << +i << "] = ";
        oss << +matrixParams->ScalingList8x8[0][i] << " / " << +matrixParams->ScalingList8x8[1][i];
        oss << std::endl;
    }

    const char *fileName = m_debugInterface->CreateFileName(
        "_DDIEnc",
        CodechalDbgBufferType::bufSeqParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDriverUltDump))
    {
        if (!m_debugInterface->m_ddiFileName.empty())
        {
            std::ofstream ofs(m_debugInterface->m_ddiFileName, std::ios::app);
            ofs << "SeqParamFile"
                << " = \"" << m_debugInterface->m_fileName << "\"" << std::endl;
            ofs.close();
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencPipeline::DumpPicParams(
    const CODEC_AVC_ENCODE_PIC_PARAMS *picParams,
    const CODEC_AVC_IQ_MATRIX_PARAMS  *matrixParams)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrPicParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(picParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "# DDI Parameters:" << std::endl;
    oss << "TargetFrameSize = " << +picParams->TargetFrameSize << std::endl;
    oss << "CurrOriginalPic = " << +picParams->CurrOriginalPic.PicEntry << std::endl;
    oss << "CurrReconstructedPic = " << +picParams->CurrReconstructedPic.PicEntry << std::endl;
    oss << "CodingType = " << +picParams->CodingType << std::endl;
    oss << "FieldCodingFlag = " << +picParams->FieldCodingFlag << std::endl;
    oss << "FieldFrameCodingFlag = " << +picParams->FieldFrameCodingFlag << std::endl;
    oss << "NumSlice = " << +picParams->NumSlice << std::endl;
    oss << "QpY = " << +picParams->QpY << std::endl;

    for (uint8_t i = 0; i < 16; ++i)
    {
        oss << "RefFrameList[" << +i << "] = " << +picParams->RefFrameList[i].PicEntry << std::endl;
    }

    oss << "UsedForReferenceFlags = " << +picParams->UsedForReferenceFlags << std::endl;
    oss << "CurrFieldOrderCnt[0] = " << +picParams->CurrFieldOrderCnt[0] << std::endl;
    oss << "CurrFieldOrderCnt[1] = " << +picParams->CurrFieldOrderCnt[1] << std::endl;

    for (uint8_t i = 0; i < 16; ++i)
    {
        for (uint8_t j = 0; j < 2; ++j)
        {
            oss << "FieldOrderCntList[" << +i << "]"
                << "[" << +j << "] = " << +picParams->FieldOrderCntList[i][j] << std::endl;
        }
    }

    oss << "frame_num = " << +picParams->frame_num << std::endl;
    oss << "bLastPicInSeq = " << +picParams->bLastPicInSeq << std::endl;
    oss << "bLastPicInStream = " << +picParams->bLastPicInStream << std::endl;

    // User Flags parameters
    oss << "# bUseRawPicForRef = " << +picParams->UserFlags.bUseRawPicForRef << std::endl;
    oss << "# bDisableAcceleratorHeaderPacking = " << +picParams->UserFlags.bDisableAcceleratorHeaderPacking << std::endl;
    oss << "# bDisableSubMBPartition = " << +picParams->UserFlags.bDisableSubMBPartition << std::endl;
    oss << "# bEmulationByteInsertion = " << +picParams->UserFlags.bEmulationByteInsertion << std::endl;
    oss << "# bEnableRollingIntraRefresh = " << +picParams->UserFlags.bEnableRollingIntraRefresh << std::endl;
    oss << "ForceRepartitionCheck =" << +picParams->UserFlags.ForceRepartitionCheck << std::endl;
    oss << "UserFlags = " << +picParams->UserFlags.Value << std::endl;
    oss << "StatusReportFeedbackNumber = " << +picParams->StatusReportFeedbackNumber << std::endl;
    oss << "bIdrPic = " << +picParams->bIdrPic << std::endl;
    oss << "pic_parameter_set_id = " << +picParams->pic_parameter_set_id << std::endl;
    oss << "seq_parameter_set_id = " << +picParams->seq_parameter_set_id << std::endl;
    oss << "num_ref_idx_l0_active_minus1 = " << +picParams->num_ref_idx_l0_active_minus1 << std::endl;
    oss << "num_ref_idx_l1_active_minus1 = " << +picParams->num_ref_idx_l1_active_minus1 << std::endl;
    oss << "chroma_qp_index_offset = " << +picParams->chroma_qp_index_offset << std::endl;
    oss << "second_chroma_qp_index_offset = " << +picParams->second_chroma_qp_index_offset << std::endl;
    oss << "entropy_coding_mode_flag = " << +picParams->entropy_coding_mode_flag << std::endl;
    oss << "pic_order_present_flag = " << +picParams->pic_order_present_flag << std::endl;
    oss << "weighted_pred_flag = " << +picParams->weighted_pred_flag << std::endl;
    oss << "weighted_bipred_idc = " << +picParams->weighted_bipred_idc << std::endl;
    oss << "constrained_intra_pred_flag = " << +picParams->constrained_intra_pred_flag << std::endl;
    oss << "transform_8x8_mode_flag = " << +picParams->transform_8x8_mode_flag << std::endl;
    oss << "pic_scaling_matrix_present_flag = " << +picParams->pic_scaling_matrix_present_flag << std::endl;
    oss << "pic_scaling_list_present_flag = " << +picParams->pic_scaling_list_present_flag[0] << std::endl;

    // pic_scaling_list_present_flag buffer contains 12 elements (only 1 acknowledged DDI document)
    oss << "# pic_scaling_list_present_flag[1-11]:";
    oss << +picParams->pic_scaling_list_present_flag[1] << " ";
    oss << +picParams->pic_scaling_list_present_flag[2] << " ";
    oss << +picParams->pic_scaling_list_present_flag[3] << " ";
    oss << +picParams->pic_scaling_list_present_flag[4] << " ";
    oss << +picParams->pic_scaling_list_present_flag[5] << " ";
    oss << +picParams->pic_scaling_list_present_flag[6] << " ";
    oss << +picParams->pic_scaling_list_present_flag[7] << " ";
    oss << +picParams->pic_scaling_list_present_flag[8] << " ";
    oss << +picParams->pic_scaling_list_present_flag[9] << " ";
    oss << +picParams->pic_scaling_list_present_flag[10] << " ";
    oss << +picParams->pic_scaling_list_present_flag[11] << std::endl;

    oss << "RefPicFlag = " << +picParams->RefPicFlag << std::endl;
    oss << "BRCPrecision = " << +picParams->BRCPrecision << std::endl;
    oss << "IntraInsertionLocation = " << +picParams->IntraRefreshMBNum << std::endl;
    oss << "IntraInsertionSize = " << +picParams->IntraRefreshUnitinMB << std::endl;
    oss << "QpDeltaForInsertedIntra = " << +picParams->IntraRefreshQPDelta << std::endl;
    oss << "SliceSizeInBytes = " << +picParams->SliceSizeInBytes << std::endl;
    oss << "bDisableRollingIntraRefreshOverlap = " << +picParams->bDisableRollingIntraRefreshOverlap << std::endl;
    oss << "NumROI = " << +picParams->NumROI << std::endl;
    oss << "MinDeltaQp = " << +picParams->MinDeltaQp << std::endl;
    oss << "MaxDeltaQp = " << +picParams->MaxDeltaQp << std::endl;

    // Dump ROI coordinates and PriorityLevelOrDQp
    for (uint16_t i = 0; i < picParams->NumROI; ++i)
    {
        oss << "ROI[" << +i << "] = [";
        oss << +picParams->ROI[i].Top << ",";
        oss << +picParams->ROI[i].Bottom << ",";
        oss << +picParams->ROI[i].Left << ",";
        oss << +picParams->ROI[i].Right << "], ";
        oss << "PriorityLevelOrDQp = " << +picParams->ROI[i].PriorityLevelOrDQp << std::endl;
    }

    oss << "NumDirtyROI = " << +picParams->NumDirtyROI << std::endl;

    // Dump Dirty ROI coordinates and PriorityLevelOrDQp
    for (uint16_t i = 0; i < picParams->NumDirtyROI; ++i)
    {
        oss << "DirtyROI[" << +i << "] = [";
        oss << +picParams->DirtyROI[i].Top << ",";
        oss << +picParams->DirtyROI[i].Bottom << ",";
        oss << +picParams->DirtyROI[i].Left << ",";
        oss << +picParams->DirtyROI[i].Right << "], ";
        oss << "PriorityLevelOrDQp = " << +picParams->DirtyROI[i].PriorityLevelOrDQp << std::endl;
    }

    oss << "SkipFrameFlag = " << +picParams->SkipFrameFlag << std::endl;
    oss << "NumSkipFrames = " << +picParams->NumSkipFrames << std::endl;
    oss << "SizeSkipFrames = " << +picParams->SizeSkipFrames << std::endl;

    // Dump Min/Max QP params
    oss << "BRCMinQp = " << +picParams->ucMinimumQP << std::endl;
    oss << "BRCMaxQp = " << +picParams->ucMaximumQP << std::endl;

    // Dump SFD threshold
    oss << "dwZMvThreshold = " << +picParams->dwZMvThreshold << std::endl;

    // Dump HME offset
    for (uint8_t i = 0; i < 16; ++i)
    {
        for (uint8_t j = 0; j < 2; ++j)
        {
            for (uint8_t k = 0; k < 2; ++k)
            {
                oss << "HMEOffset[" << +i << "][" << +j << "][" << +k << "] = " << +picParams->HMEOffset[i][j][k] << std::endl;
            }
        }
    }

    // Parameters not defined in DDI (Any non-DDI parameters printed should be preceeded by #)
    oss << "# Non-DDI Parameters:" << std::endl;
    oss << "# num_slice_groups_minus1 = " << +picParams->num_slice_groups_minus1 << std::endl;
    oss << "# pic_init_qp_minus26 = " << +picParams->pic_init_qp_minus26 << std::endl;
    oss << "# pic_init_qs_minus26 = " << +picParams->pic_init_qs_minus26 << std::endl;
    oss << "# deblocking_filter_control_present_flag = " << +picParams->deblocking_filter_control_present_flag << std::endl;
    oss << "# redundant_pic_cnt_present_flag = " << +picParams->redundant_pic_cnt_present_flag << std::endl;
    oss << "# EnableRollingIntraRefresh = " << +picParams->EnableRollingIntraRefresh << std::endl;
    oss << "# IntraRefreshMBx = " << +picParams->IntraRefreshMBx << std::endl;
    oss << "# IntraRefreshMBy = " << +picParams->IntraRefreshMBy << std::endl;
    oss << "# IntraRefreshUnitinMB = " << +picParams->IntraRefreshUnitinMB << std::endl;
    oss << "# IntraRefreshQPDelta = " << +picParams->IntraRefreshQPDelta << std::endl;

    // Dump ScalingList4x4 (6 x 16)
    for (uint8_t i = 0; i < 16; ++i)
    {
        oss << "# ScalingList4x4[" << +i << "] = ";
        oss << +matrixParams->ScalingList4x4[0][i] << " ";
        oss << +matrixParams->ScalingList4x4[1][i] << " ";
        oss << +matrixParams->ScalingList4x4[2][i] << " ";
        oss << +matrixParams->ScalingList4x4[3][i] << " ";
        oss << +matrixParams->ScalingList4x4[4][i] << " ";
        oss << +matrixParams->ScalingList4x4[5][i] << std::endl;
    }

    // ScalingList8x8 (2 x 64)
    for (uint8_t i = 0; i < 64; ++i)
    {
        oss << "# ScalingList8x8[0/1][" << +i << "] = " << +matrixParams->ScalingList8x8[0][i] << " / " << +matrixParams->ScalingList8x8[1][i] << std::endl;
    }

    const char *fileName = m_debugInterface->CreateFileName(
        "_DDIEnc",
        CodechalDbgBufferType::bufPicParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDriverUltDump))
    {
        if (!m_debugInterface->m_ddiFileName.empty())
        {
            std::ofstream ofs(m_debugInterface->m_ddiFileName, std::ios::app);
            ofs << "PicNum"
                << " = \"" << m_debugInterface->m_bufferDumpFrameNum << "\"" << std::endl;
            ofs << "PicParamFile"
                << " = \"" << m_debugInterface->m_fileName << "\"" << std::endl;
            ofs.close();
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencPipeline::DumpSliceParams(
    const CODEC_AVC_ENCODE_SLICE_PARAMS *sliceParams,
    const CODEC_AVC_ENCODE_PIC_PARAMS *picParams)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSlcParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(sliceParams);

    m_debugInterface->m_sliceId = sliceParams->slice_id;  // set here for constructing debug file name

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "# DDI Parameters:" << std::endl;
    oss << "NumMbsForSlice = " << +sliceParams->NumMbsForSlice << std::endl;

    // RefPicList (2 x 32)
    for (uint8_t i = 0; i < 2; ++i)
    {
        for (uint8_t j = 0; j < 32; ++j)
        {
            oss << "RefPicList[" << +i << "][" << +j << "] = " << +sliceParams->RefPicList[i][j].PicEntry << std::endl;
        }
    }

    // Conditionally printed (only when picture parameters weighted_pred_flag or weighted_bipred_idc are set).
    // Weights contains 192 elements (2 x 32 x 3 x 2).
    if (picParams->weighted_pred_flag || picParams->weighted_bipred_idc)
    {
        for (uint8_t i = 0; i < 2; ++i)
        {
            for (uint8_t j = 0; j < 32; ++j)
            {
                for (uint8_t k = 0; k < 3; ++k)
                {
                    for (uint8_t l = 0; l < 2; ++l)
                    {
                        oss << "Weights[" << +i << "][" << +j << "][" << +k << "][" << +l << "]: " << +sliceParams->Weights[i][j][k][l] << std::endl;
                    }
                }
            }
        }
    }

    oss << "first_mb_in_slice = " << +sliceParams->first_mb_in_slice << std::endl;
    oss << "slice_type = " << +sliceParams->slice_type << std::endl;
    oss << "pic_parameter_set_id = " << +sliceParams->pic_parameter_set_id << std::endl;
    oss << "direct_spatial_mv_pred_flag = " << +sliceParams->direct_spatial_mv_pred_flag << std::endl;
    oss << "num_ref_idx_active_override_flag = " << +sliceParams->num_ref_idx_active_override_flag << std::endl;
    oss << "long_term_reference_flag = " << +sliceParams->long_term_reference_flag << std::endl;
    oss << "idr_pic_id = " << +sliceParams->idr_pic_id << std::endl;
    oss << "pic_order_cnt_lsb = " << +sliceParams->pic_order_cnt_lsb << std::endl;
    oss << "delta_pic_order_cnt_bottom = " << +sliceParams->delta_pic_order_cnt_bottom << std::endl;
    oss << "delta_pic_order_cnt[0] = " << +sliceParams->delta_pic_order_cnt[0] << std::endl;
    oss << "delta_pic_order_cnt[1] = " << +sliceParams->delta_pic_order_cnt[1] << std::endl;
    oss << "num_ref_idx_l0_active_minus1 = " << +sliceParams->num_ref_idx_l0_active_minus1 << std::endl;
    oss << "num_ref_idx_l1_active_minus1 = " << +sliceParams->num_ref_idx_l1_active_minus1 << std::endl;
    oss << "luma_log2_weight_denom = " << +sliceParams->luma_log2_weight_denom << std::endl;
    oss << "chroma_log2_weight_denom = " << +sliceParams->chroma_log2_weight_denom << std::endl;
    oss << "cabac_init_idc = " << +sliceParams->cabac_init_idc << std::endl;
    oss << "slice_qp_delta = " << +sliceParams->slice_qp_delta << std::endl;
    oss << "disable_deblocking_filter_idc = " << +sliceParams->disable_deblocking_filter_idc << std::endl;
    oss << "slice_alpha_c0_offset_div2 = " << +sliceParams->slice_alpha_c0_offset_div2 << std::endl;
    oss << "slice_beta_offset_div2 = " << +sliceParams->slice_beta_offset_div2 << std::endl;
    oss << "slice_id = " << +sliceParams->slice_id << std::endl;
    oss << "luma_weight_flag[0] = " << +sliceParams->luma_weight_flag[0] << std::endl;
    oss << "luma_weight_flag[1] = " << +sliceParams->luma_weight_flag[1] << std::endl;
    oss << "chroma_weight_flag[0] = " << +sliceParams->chroma_weight_flag[0] << std::endl;
    oss << "chroma_weight_flag[1] = " << +sliceParams->chroma_weight_flag[1] << std::endl;

    // Parameters not in DDI (Any non-DDI parameters printed should be preceeded by #)
    oss << "# Non-DDI Parameters:" << std::endl;

    // PicOrder (2 x 32) - Dump in 32 blocks of 2 chunks per line
    for (uint16_t i = 0; i < 32; ++i)
    {
        CODECHAL_DEBUG_CHK_STATUS(DumpEncodePicReorder(
            oss,
            0,
            i,
            &(sliceParams->PicOrder[0][i])));
        CODECHAL_DEBUG_CHK_STATUS(DumpEncodePicReorder(
            oss,
            1,
            i,
            &(sliceParams->PicOrder[1][i])));
    }

    oss << "# colour_plane_id = " << +sliceParams->colour_plane_id << std::endl;
    oss << "# frame_num = " << +sliceParams->frame_num << std::endl;
    oss << "# field_pic_flag = " << std::hex << +sliceParams->field_pic_flag << std::endl;
    oss << "# bottom_field_flag = " << std::hex << +sliceParams->bottom_field_flag << std::endl;
    oss << "# redundant_pic_cnt = " << std::dec << +sliceParams->redundant_pic_cnt << std::endl;
    oss << "# sp_for_switch_flag = " << std::hex << +sliceParams->sp_for_switch_flag << std::endl;
    oss << "# slice_qs_delta = " << std::dec << +sliceParams->slice_qs_delta << std::endl;
    oss << "# ref_pic_list_reordering_flag_l0 = " << std::hex << +sliceParams->ref_pic_list_reordering_flag_l0 << std::endl;
    oss << "# ref_pic_list_reordering_flag_l1 = " << std::hex << +sliceParams->ref_pic_list_reordering_flag_l1 << std::endl;
    oss << "# no_output_of_prior_pics_flag = " << std::hex << +sliceParams->no_output_of_prior_pics_flag << std::endl;
    oss << "# adaptive_ref_pic_marking_mode_flag = " << std::hex << +sliceParams->adaptive_ref_pic_marking_mode_flag << std::endl;
    oss << "# MaxFrameNum = " << std::dec << +sliceParams->MaxFrameNum << std::endl;
    oss << "# NumReorder = " << std::dec << +sliceParams->NumReorder << std::endl;

    const char *fileName = m_debugInterface->CreateFileName(
        "_DDIEnc",
        CodechalDbgBufferType::bufSlcParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDriverUltDump))
    {
        if (!m_debugInterface->m_ddiFileName.empty())
        {
            std::ofstream ofs(m_debugInterface->m_ddiFileName, std::ios::app);
            ofs << "SlcParamFile"
                << " = \"" << m_debugInterface->m_fileName << "\"" << std::endl;
            ofs.close();
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencPipeline::DumpVuiParams(
    const CODECHAL_ENCODE_AVC_VUI_PARAMS *vuiParams)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrVuiParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(vuiParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "# DDI Parameters:" << std::endl;
    oss << "aspect_ratio_info_present_flag = " << +vuiParams->aspect_ratio_info_present_flag << std::endl;
    oss << "overscan_info_present_flag = " << +vuiParams->overscan_info_present_flag << std::endl;
    oss << "overscan_appropriate_flag = " << +vuiParams->overscan_appropriate_flag << std::endl;
    oss << "video_signal_type_present_flag = " << +vuiParams->video_signal_type_present_flag << std::endl;
    oss << "video_full_range_flag = " << +vuiParams->video_full_range_flag << std::endl;
    oss << "colour_description_present_flag = " << +vuiParams->colour_description_present_flag << std::endl;
    oss << "chroma_loc_info_present_flag = " << +vuiParams->chroma_loc_info_present_flag << std::endl;
    oss << "timing_info_present_flag = " << +vuiParams->timing_info_present_flag << std::endl;
    oss << "fixed_frame_rate_flag = " << +vuiParams->fixed_frame_rate_flag << std::endl;
    oss << "nal_hrd_parameters_present_flag = " << +vuiParams->nal_hrd_parameters_present_flag << std::endl;
    oss << "vcl_hrd_parameters_present_flag = " << +vuiParams->vcl_hrd_parameters_present_flag << std::endl;
    oss << "low_delay_hrd_flag = " << +vuiParams->low_delay_hrd_flag << std::endl;
    oss << "pic_struct_present_flag = " << +vuiParams->pic_struct_present_flag << std::endl;
    oss << "bitstream_restriction_flag = " << +vuiParams->bitstream_restriction_flag << std::endl;
    oss << "motion_vectors_over_pic_boundaries_flag = " << +vuiParams->motion_vectors_over_pic_boundaries_flag << std::endl;
    oss << "sar_width = " << +vuiParams->sar_width << std::endl;
    oss << "sar_height = " << +vuiParams->sar_height << std::endl;
    oss << "aspect_ratio_idc = " << +vuiParams->aspect_ratio_idc << std::endl;
    oss << "video_format = " << +vuiParams->video_format << std::endl;
    oss << "colour_primaries = " << +vuiParams->colour_primaries << std::endl;
    oss << "transfer_characteristics = " << +vuiParams->transfer_characteristics << std::endl;
    oss << "matrix_coefficients = " << +vuiParams->matrix_coefficients << std::endl;
    oss << "chroma_sample_loc_type_top_field = " << +vuiParams->chroma_sample_loc_type_top_field << std::endl;
    oss << "chroma_sample_loc_type_bottom_field = " << +vuiParams->chroma_sample_loc_type_bottom_field << std::endl;
    oss << "max_bytes_per_pic_denom = " << +vuiParams->max_bytes_per_pic_denom << std::endl;
    oss << "max_bits_per_mb_denom = " << +vuiParams->max_bits_per_mb_denom << std::endl;
    oss << "log2_max_mv_length_horizontal = " << +vuiParams->log2_max_mv_length_horizontal << std::endl;
    oss << "log2_max_mv_length_vertical = " << +vuiParams->log2_max_mv_length_vertical << std::endl;
    oss << "num_reorder_frames = " << +vuiParams->num_reorder_frames << std::endl;
    oss << "num_units_in_tick = " << +vuiParams->num_units_in_tick << std::endl;
    oss << "time_scale = " << +vuiParams->time_scale << std::endl;
    oss << "max_dec_frame_buffering = " << +vuiParams->max_dec_frame_buffering << std::endl;
    oss << "cpb_cnt_minus1 = " << +vuiParams->cpb_cnt_minus1 << std::endl;
    oss << "bit_rate_scale = " << +vuiParams->bit_rate_scale << std::endl;
    oss << "cpb_size_scale = " << +vuiParams->cpb_size_scale << std::endl;

    // bit_rate_value_minus1 (32 in size)
    for (uint8_t i = 0; i < 32; ++i)
    {
        oss << "bit_rate_value_minus1[" << +i << "] = " << +vuiParams->bit_rate_value_minus1[i] << std::endl;
    }

    // cpb_size_value_minus1 (32 in size)
    for (uint8_t i = 0; i < 32; ++i)
    {
        oss << "cpb_size_value_minus1[" << +i << "] = " << +vuiParams->cpb_size_value_minus1[i] << std::endl;
    }

    oss << "cbr_flag = " << +vuiParams->cbr_flag << std::endl;
    oss << "initial_cpb_removal_delay_length_minus1 = " << +vuiParams->initial_cpb_removal_delay_length_minus1 << std::endl;
    oss << "cpb_removal_delay_length_minus1 = " << +vuiParams->cpb_removal_delay_length_minus1 << std::endl;
    oss << "dpb_output_delay_length_minus1 = " << +vuiParams->dpb_output_delay_length_minus1 << std::endl;
    oss << "time_offset_length = " << +vuiParams->time_offset_length << std::endl;

    const char *fileName = m_debugInterface->CreateFileName(
        "_DDIEnc",
        CodechalDbgBufferType::bufVuiParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDriverUltDump))
    {
        if (!m_debugInterface->m_ddiFileName.empty())
        {
            std::ofstream ofs(m_debugInterface->m_ddiFileName, std::ios::app);
            ofs << "VuiParamFile"
                << " = \"" << m_debugInterface->m_fileName << "\"" << std::endl;
            ofs.close();
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencPipeline::PopulateTargetUsage()
{
    ENCODE_FUNC_CALL();

    const char *fileName = m_debugInterface->CreateFileName(
        "EncodeSequence",
        "EncodePar",
        CodechalDbgExtType::par);

    auto basicFeature = dynamic_cast<AvcBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(basicFeature);

    auto avcSeqParam = basicFeature->m_seqParam;
    ENCODE_CHK_NULL_RETURN(avcSeqParam);

    std::ifstream ifs(fileName);
    std::string   str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();
    std::ofstream ofs(fileName, std::ios::trunc);
    ofs << "TargetUsage = " << static_cast<uint32_t>(avcSeqParam->TargetUsage) << std::endl;
    ofs << str;
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencPipeline::PopulateQuantPrecision()
{
    ENCODE_FUNC_CALL();

    const char *fileName = m_debugInterface->CreateFileName(
        "EncodeSequence",
        "EncodePar",
        CodechalDbgExtType::par);

    std::ifstream ifs(fileName);
    std::string   str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();
    std::ofstream ofs(fileName, std::ios::trunc);
    ofs << "UsePrecisionDecreasingQT = 1" << std::endl;
    ofs << str;
    ofs.close();

    return MOS_STATUS_SUCCESS;
}
#endif

}
