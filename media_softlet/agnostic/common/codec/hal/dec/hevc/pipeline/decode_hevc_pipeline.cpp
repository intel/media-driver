/*
* Copyright (c) 2018-2022, Intel Corporation
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
//! \file     decode_hevc_pipeline.cpp
//! \brief    Defines the interface for hevc decode pipeline
//!
#include "decode_hevc_pipeline.h"
#include "decode_utils.h"
#include "codechal_setting.h"
#include "decode_hevc_phase_s2l.h"
#include "decode_hevc_phase_long.h"
#include "decode_hevc_phase_front_end.h"
#include "decode_hevc_phase_back_end.h"
#include "decode_hevc_phase_real_tile.h"
#include "decode_hevc_feature_manager.h"
#include "decode_hevc_downsampling_packet.h"

namespace decode {

HevcPipeline::HevcPipeline(CodechalHwInterface *hwInterface, CodechalDebugInterface *debugInterface)
    : DecodePipeline(hwInterface, debugInterface)
{
    MOS_STATUS m_status = InitUserSetting(m_userSettingPtr);
}

MOS_STATUS HevcPipeline::Initialize(void *settings)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(DecodePipeline::Initialize(settings));

    auto *codecSettings = (CodechalSetting*)settings;
    DECODE_CHK_NULL(codecSettings);
    m_shortFormatInUse  = (codecSettings->shortFormatInUse) ? true : false;

    // Create basic GPU context
    DecodeScalabilityPars scalPars;
    MOS_ZeroMemory(&scalPars, sizeof(scalPars));
    DECODE_CHK_STATUS(m_mediaContext->SwitchContext(VdboxDecodeFunc, &scalPars, &m_scalability));
    m_decodeContext = m_osInterface->pfnGetGpuContext(m_osInterface);

    m_basicFeature = dynamic_cast<HevcBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_basicFeature);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipeline::Uninitialize()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(DestoryPhaseList());
    return DecodePipeline::Uninitialize();
}

MOS_STATUS HevcPipeline::UserFeatureReport()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(DecodePipeline::UserFeatureReport());

#if (_DEBUG || _RELEASE_INTERNAL)
    WriteUserFeature(__MEDIA_USER_FEATURE_VALUE_APOGEIOS_HEVCD_ENABLE_ID, 1, m_osInterface->pOsContext);
#endif

#ifdef _MMC_SUPPORTED
    CODECHAL_DEBUG_TOOL(
        if (m_mmcState != nullptr)
        {
            m_mmcState->UpdateUserFeatureKey(&(m_basicFeature->m_destSurface));
        }
        );
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipeline::Execute()
{
    DECODE_FUNC_CALL();

#if (_DEBUG || _RELEASE_INTERNAL)
    if (GetDecodeMode() == realTileDecodeMode)
    {
        m_rtFrameCount++;
    }
    else if (GetDecodeMode() == virtualTileDecodeMode)
    {
        m_vtFrameCount++;
    }
    else if (GetDecodeMode() == separateTileDecodeMode)
    {
        m_spFrameCount++;
    }
#endif

    for (auto &phase : m_phaseList)
    {
        DECODE_ASSERT(phase != nullptr);
        if (phase->RequiresContextSwitch())
        {
            // switch context
            DecodeScalabilityOption *scalabOption = phase->GetDecodeScalabilityOption();
            DECODE_CHK_NULL(scalabOption);
            DECODE_CHK_STATUS(m_mediaContext->SwitchContext(VdboxDecodeFunc, *scalabOption, &m_scalability));
            if (scalabOption->IsScalabilityOptionMatched(m_scalabOption))
            {
                m_decodeContext = m_osInterface->pfnGetGpuContext(m_osInterface);
            }
        }

        StateParams stateProperty;
        stateProperty.currentPipe        = phase->GetPipe();
        stateProperty.currentPass        = phase->GetPass();
        stateProperty.pipeIndexForSubmit = phase->GetPipe() + 1;
        stateProperty.componentState     = phase;
        DECODE_CHK_STATUS(ActivatePacket(phase->GetPktId(), phase->ImmediateSubmit(), stateProperty));

        if (phase->ImmediateSubmit())
        {
            m_scalability->SetPassNumber(phase->GetPass() + 1);
            DECODE_CHK_STATUS(ExecuteActivePackets());
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipeline::CreateFeatureManager()
{
    DECODE_FUNC_CALL();
    m_featureManager = MOS_New(DecodeHevcFeatureManager, m_allocator, m_hwInterface);
    DECODE_CHK_NULL(m_featureManager);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipeline::CreateSubPackets(DecodeSubPacketManager& subPacketManager, CodechalSetting &codecSettings)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_STATUS(DecodePipeline::CreateSubPackets(subPacketManager, codecSettings));

#ifdef _DECODE_PROCESSING_SUPPORTED
    HevcDownSamplingPkt *downSamplingPkt = MOS_New(HevcDownSamplingPkt, this, m_hwInterface);
    DECODE_CHK_NULL(downSamplingPkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
                        DecodePacketId(this, downSamplingSubPacketId), *downSamplingPkt));
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipeline::InitContexOption(HevcScalabilityPars& scalPars)
{
    scalPars.usingHcp = true;
    scalPars.enableVE = MOS_VE_SUPPORTED(m_osInterface);
    scalPars.disableScalability = m_hwInterface->IsDisableScalability();
#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_osInterface->bHcpDecScalabilityMode == MOS_SCALABILITY_ENABLE_MODE_FALSE)
    {
        scalPars.disableScalability = true;
    }
    else if (m_osInterface->bHcpDecScalabilityMode == MOS_SCALABILITY_ENABLE_MODE_USER_FORCE)
    {
        scalPars.disableScalability = false;
    }
    scalPars.modeSwithThreshold1 =
        ReadUserFeature(m_userSettingPtr, "HCP Decode Mode Switch TH1", MediaUserSetting::Group::Sequence).Get<uint32_t>();
    scalPars.modeSwithThreshold2 =
        ReadUserFeature(m_userSettingPtr, "HCP Decode Mode Switch TH2", MediaUserSetting::Group::Sequence).Get<uint32_t>();
    scalPars.forceMultiPipe =
        ReadUserFeature(m_userSettingPtr, "HCP Decode Always Frame Split", MediaUserSetting::Group::Sequence).Get<bool>();
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipeline::InitDecodeMode(ScalabilityMode scalabMode, HevcBasicFeature &basicFeature)
{
    if (scalabMode == scalabilityVirtualTileMode)
    {
        m_decodeMode = virtualTileDecodeMode;
    }
    else if (scalabMode == scalabilityRealTileMode)
    {
        m_decodeMode = realTileDecodeMode;
    }
    else
    {
        PCODEC_HEVC_PIC_PARAMS picParams = basicFeature.m_hevcPicParams;
        DECODE_CHK_NULL(picParams);

        if (picParams->tiles_enabled_flag &&
            (basicFeature.m_isSCCIBCMode || basicFeature.m_isSCCPLTMode || basicFeature.m_isWPPMode))
        {
            m_decodeMode = separateTileDecodeMode;
        }
        else
        {
            m_decodeMode = baseDecodeMode;
        }
    }

    return MOS_STATUS_SUCCESS;
}

template<typename T>
MOS_STATUS HevcPipeline::CreatePhase(uint8_t pass, uint8_t pipe, uint8_t activePipeNum)
{
    DECODE_FUNC_CALL();
    T *phase = MOS_New(T, *this, m_scalabOption);
    DECODE_CHK_NULL(phase);
    DECODE_CHK_STATUS(phase->Initialize(pass, pipe, activePipeNum));
    m_phaseList.push_back(phase);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipeline::CreatePhaseList(HevcBasicFeature &basicFeature, const ScalabilityMode scalabMode, const uint8_t numPipe)
{
    DECODE_FUNC_CALL();
    DECODE_ASSERT(m_phaseList.empty());

    if (m_shortFormatInUse)
    {
        DECODE_CHK_STATUS(CreatePhase<HevcPhaseS2L>());
    }

    if (scalabMode == scalabilityVirtualTileMode)
    {
        DECODE_CHK_STATUS(CreatePhase<HevcPhaseFrontEnd>());
        for (uint8_t i = 0; i < numPipe; i++)
        {
            DECODE_CHK_STATUS(CreatePhase<HevcPhaseBackEnd>(0, i, numPipe));
        }
    }
    else if (scalabMode == scalabilityRealTileMode)
    {
        PCODEC_HEVC_PIC_PARAMS picParams = basicFeature.m_hevcPicParams;
        DECODE_CHK_NULL(picParams);
        uint8_t pass = 0;
        uint8_t pipe = 0;
        uint8_t activePipeNum = numPipe;
        uint32_t numTileCol = picParams->num_tile_columns_minus1 + 1;
        for (uint32_t i = 0; i < numTileCol; i++)
        {
            DECODE_CHK_STATUS(CreatePhase<HevcPhaseRealTile>(pass, pipe, activePipeNum));
            pipe++;
            if (pipe >= numPipe)
            {
                pipe = 0;
                pass++;
                uint32_t remainingCols = numTileCol - i - 1;
                activePipeNum = (remainingCols >= numPipe) ? numPipe : remainingCols;
            }
        }
    }
    else
    {
        DECODE_CHK_STATUS(CreatePhase<HevcPhaseLong>());
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipeline::DestoryPhaseList()
{
    for (auto &phase : m_phaseList)
    {
        MOS_Delete(phase);
    }
    m_phaseList.clear();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipeline::StoreDestToRefList(HevcBasicFeature &basicFeature)
{
    DECODE_CHK_NULL(basicFeature.m_hevcPicParams);
    DECODE_CHK_STATUS(basicFeature.m_refFrames.UpdateCurResource(
        *basicFeature.m_hevcPicParams, false));
    return MOS_STATUS_SUCCESS;
}

#if (_DEBUG || _RELEASE_INTERNAL)
MOS_STATUS HevcPipeline::HwStatusCheck(const DecodeStatusMfx &status)
{
    DECODE_FUNC_CALL();

    if (m_shortFormatInUse)
    {
        // Check HuC_status2 Imem loaded bit, if 0, return error
        if (((status.m_hucErrorStatus2 >> 32) && (m_hwInterface->GetHucInterface()->GetHucStatus2ImemLoadedMask())) == 0)
        {
            if (!m_reportHucStatus)
            {
                WriteUserFeature(__MEDIA_USER_FEATURE_VALUE_HUC_LOAD_STATUS_ID, 1, m_osInterface->pOsContext);
                m_reportHucStatus = true;
            }
            DECODE_ASSERTMESSAGE("Huc IMEM Loaded fails");
            MT_ERR1(MT_DEC_HEVC, MT_DEC_HUC_ERROR_STATUS2, (status.m_hucErrorStatus2 >> 32));
            return MOS_STATUS_UNKNOWN;
        }

        // Check Huc_status None Critical Error bit, bit 15. If 0, return error.
        if (((status.m_hucErrorStatus >> 32) & m_hwInterface->GetHucInterface()->GetHucStatusHevcS2lFailureMask()) == 0)
        {
            if (!m_reportHucCriticalError)
            {
                WriteUserFeature(__MEDIA_USER_FEATURE_VALUE_HUC_REPORT_CRITICAL_ERROR_ID, 1, m_osInterface->pOsContext);
                m_reportHucCriticalError = true;
            }
            DECODE_ASSERTMESSAGE("Huc Report Critical Error!");
            MT_ERR1(MT_DEC_HEVC, MT_DEC_HUC_STATUS_CRITICAL_ERROR, (status.m_hucErrorStatus >> 32));
            return MOS_STATUS_UNKNOWN;
        }
    }

    return MOS_STATUS_SUCCESS;
}
#endif

bool HevcPipeline::IsShortFormat()
{
    return m_shortFormatInUse;
}

HevcPipeline::HevcDecodeMode HevcPipeline::GetDecodeMode()
{
    return m_decodeMode;
}

bool HevcPipeline::IsFESeparateSubmission()
{
    return m_scalabOption.IsFESeparateSubmission();
}

MHW_BATCH_BUFFER* HevcPipeline::GetSliceLvlCmdBuffer()
{
    if (m_secondLevelBBArray == nullptr)
    {
        return nullptr;
    }
    return m_secondLevelBBArray->Peek();
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS HevcPipeline::DumpIQParams(
    PCODECHAL_HEVC_IQ_MATRIX_PARAMS matrixData)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrIqParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(matrixData);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    uint32_t idx;
    uint32_t idx2;
    // 4x4 block
    for (idx2 = 0; idx2 < 6; idx2++)
    {
        oss << "Qmatrix_HEVC_ucScalingLists0[" << std::dec << +idx2 << "]:" << std::endl;

        oss << "ucScalingLists0[" << +idx2 << "]:";
        for (uint8_t i = 0; i < 16; i++)
            oss << std::hex << +matrixData->ucScalingLists0[idx2][i] << " ";
        oss << std::endl;
    }

    // 8x8 block
    for (idx2 = 0; idx2 < 6; idx2++)
    {
        oss << "ucScalingLists1[" << std::dec << +idx2 << "]:" << std::endl;

        for (idx = 0; idx < 64; idx += 8)
        {
            oss << "ucScalingLists1[" << std::dec << +idx / 8 << "]:" << std::endl;
            for (uint8_t i = 0; i < 8; i++)
                oss << std::hex << +matrixData->ucScalingLists1[idx2][idx + i] << " ";
            oss << std::endl;
        }
    }

    // 16x16 block
    for (idx2 = 0; idx2 < 6; idx2++)
    {
        oss << "ucScalingLists2[" << std::dec << +idx2 << "]:" << std::endl;

        for (idx = 0; idx < 64; idx += 8)
        {
            oss << "ucScalingLists2[" << std::dec << +idx / 8 << "]:" << std::endl;
            for (uint8_t i = 0; i < 8; i++)
                oss << std::hex << +matrixData->ucScalingLists2[idx2][idx + i] << " ";
            oss << std::endl;
        }
    }
    // 32x32 block
    for (idx2 = 0; idx2 < 2; idx2++)
    {
        oss << "ucScalingLists3[" << std::dec << +idx2 << "]:" << std::endl;

        for (idx = 0; idx < 64; idx += 8)
        {
            oss << "ucScalingLists3[" << std::dec << +idx / 8 << "]:" << std::endl;
            for (uint8_t i = 0; i < 8; i++)
                oss << std::hex << +matrixData->ucScalingLists3[idx2][idx + i] << " ";
            oss << std::endl;
        }
    }

    //DC16x16 block
    oss << "ucScalingListDCCoefSizeID2: ";
    for (uint8_t i = 0; i < 6; i++)
        oss << std::hex << +matrixData->ucScalingListDCCoefSizeID2[i] << " ";

    oss << std::endl;

    //DC32x32 block

    oss << "ucScalingListDCCoefSizeID3: ";
    oss << +matrixData->ucScalingListDCCoefSizeID3[0] << " " << +matrixData->ucScalingListDCCoefSizeID3[1] << std::endl;

    const char *fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufIqParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipeline::DumpPicParams(
    PCODEC_HEVC_PIC_PARAMS     picParams,
    PCODEC_HEVC_EXT_PIC_PARAMS extPicParams,
    PCODEC_HEVC_SCC_PIC_PARAMS sccPicParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrPicParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(picParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);
    oss.setf(std::ios::hex, std::ios::basefield);

    oss << "PicWidthInMinCbsY: " << +picParams->PicWidthInMinCbsY << std::endl;
    oss << "PicHeightInMinCbsY: " << +picParams->PicHeightInMinCbsY << std::endl;
    //wFormatAndSequenceInfoFlags
    oss << "chroma_format_idc: " << +picParams->chroma_format_idc << std::endl;
    oss << "separate_colour_plane_flag: " << +picParams->separate_colour_plane_flag << std::endl;
    oss << "bit_depth_luma_minus8: " << +picParams->bit_depth_luma_minus8 << std::endl;
    oss << "bit_depth_chroma_minus8: " << +picParams->bit_depth_chroma_minus8 << std::endl;
    oss << "log2_max_pic_order_cnt_lsb_minus4: " << +picParams->log2_max_pic_order_cnt_lsb_minus4 << std::endl;
    oss << "NoPicReorderingFlag: " << +picParams->NoPicReorderingFlag << std::endl;
    oss << "ReservedBits1: " << +picParams->ReservedBits1 << std::endl;
    oss << "wFormatAndSequenceInfoFlags: " << +picParams->wFormatAndSequenceInfoFlags << std::endl;
    oss << "CurrPic FrameIdx: " << +picParams->CurrPic.FrameIdx << std::endl;
    oss << "CurrPic PicFlags: " << +picParams->CurrPic.PicFlags << std::endl;
    oss << "sps_max_dec_pic_buffering_minus1: " << +picParams->sps_max_dec_pic_buffering_minus1 << std::endl;
    oss << "log2_min_luma_coding_block_size_minus3: " << +picParams->log2_min_luma_coding_block_size_minus3 << std::endl;
    oss << "log2_diff_max_min_luma_coding_block_size: " << +picParams->log2_diff_max_min_luma_coding_block_size << std::endl;
    oss << "log2_min_transform_block_size_minus2: " << +picParams->log2_min_transform_block_size_minus2 << std::endl;
    oss << "log2_diff_max_min_transform_block_size: " << +picParams->log2_diff_max_min_transform_block_size << std::endl;
    oss << "max_transform_hierarchy_depth_intra: " << +picParams->max_transform_hierarchy_depth_intra << std::endl;
    oss << "max_transform_hierarchy_depth_inter: " << +picParams->max_transform_hierarchy_depth_inter << std::endl;
    oss << "num_short_term_ref_pic_sets: " << +picParams->num_short_term_ref_pic_sets << std::endl;
    oss << "num_long_term_ref_pic_sps: " << +picParams->num_long_term_ref_pic_sps << std::endl;
    oss << "num_ref_idx_l0_default_active_minus1: " << +picParams->num_ref_idx_l0_default_active_minus1 << std::endl;
    oss << "num_ref_idx_l1_default_active_minus1: " << +picParams->num_ref_idx_l1_default_active_minus1 << std::endl;
    oss << "init_qp_minus26: " << +picParams->init_qp_minus26 << std::endl;
    oss << "ucNumDeltaPocsOfRefRpsIdx: " << +picParams->ucNumDeltaPocsOfRefRpsIdx << std::endl;
    oss << "wNumBitsForShortTermRPSInSlice: " << +picParams->wNumBitsForShortTermRPSInSlice << std::endl;
    oss << "ReservedBits2: " << +picParams->ReservedBits2 << std::endl;
    //dwCodingParamToolFlags
    oss << "scaling_list_enabled_flag: " << +picParams->scaling_list_enabled_flag << std::endl;
    oss << "amp_enabled_flag: " << +picParams->amp_enabled_flag << std::endl;
    oss << "sample_adaptive_offset_enabled_flag: " << +picParams->sample_adaptive_offset_enabled_flag << std::endl;
    oss << "pcm_enabled_flag: " << +picParams->pcm_enabled_flag << std::endl;
    oss << "pcm_sample_bit_depth_luma_minus1: " << +picParams->pcm_sample_bit_depth_luma_minus1 << std::endl;
    oss << "pcm_sample_bit_depth_chroma_minus1: " << +picParams->pcm_sample_bit_depth_chroma_minus1 << std::endl;
    oss << "log2_min_pcm_luma_coding_block_size_minus3: " << +picParams->log2_min_pcm_luma_coding_block_size_minus3 << std::endl;
    oss << "log2_diff_max_min_pcm_luma_coding_block_size: " << +picParams->log2_diff_max_min_pcm_luma_coding_block_size << std::endl;
    oss << "pcm_loop_filter_disabled_flag: " << +picParams->pcm_loop_filter_disabled_flag << std::endl;
    oss << "long_term_ref_pics_present_flag: " << +picParams->long_term_ref_pics_present_flag << std::endl;
    oss << "sps_temporal_mvp_enabled_flag: " << +picParams->sps_temporal_mvp_enabled_flag << std::endl;
    oss << "strong_intra_smoothing_enabled_flag: " << +picParams->strong_intra_smoothing_enabled_flag << std::endl;
    oss << "dependent_slice_segments_enabled_flag: " << +picParams->dependent_slice_segments_enabled_flag << std::endl;
    oss << "output_flag_present_flag: " << +picParams->output_flag_present_flag << std::endl;
    oss << "num_extra_slice_header_bits: " << +picParams->num_extra_slice_header_bits << std::endl;
    oss << "sign_data_hiding_enabled_flag: " << +picParams->sign_data_hiding_enabled_flag << std::endl;
    oss << "cabac_init_present_flag: " << +picParams->cabac_init_present_flag << std::endl;
    oss << "ReservedBits3: " << +picParams->ReservedBits3 << std::endl;
    oss << "dwCodingParamToolFlags: " << +picParams->dwCodingParamToolFlags << std::endl;
    //dwCodingSettingPicturePropertyFlags
    oss << "constrained_intra_pred_flag: " << +picParams->constrained_intra_pred_flag << std::endl;
    oss << "transform_skip_enabled_flag: " << +picParams->transform_skip_enabled_flag << std::endl;
    oss << "cu_qp_delta_enabled_flag: " << +picParams->cu_qp_delta_enabled_flag << std::endl;
    oss << "diff_cu_qp_delta_depth: " << +picParams->diff_cu_qp_delta_depth << std::endl;
    oss << "pps_slice_chroma_qp_offsets_present_flag: " << +picParams->pps_slice_chroma_qp_offsets_present_flag << std::endl;
    oss << "weighted_pred_flag: " << +picParams->weighted_pred_flag << std::endl;
    oss << "weighted_bipred_flag: " << +picParams->weighted_bipred_flag << std::endl;
    oss << "transquant_bypass_enabled_flag: " << +picParams->transquant_bypass_enabled_flag << std::endl;
    oss << "tiles_enabled_flag: " << +picParams->tiles_enabled_flag << std::endl;
    oss << "entropy_coding_sync_enabled_flag: " << +picParams->entropy_coding_sync_enabled_flag << std::endl;
    oss << "uniform_spacing_flag: " << +picParams->uniform_spacing_flag << std::endl;
    oss << "loop_filter_across_tiles_enabled_flag: " << +picParams->loop_filter_across_tiles_enabled_flag << std::endl;
    oss << "pps_loop_filter_across_slices_enabled_flag: " << +picParams->pps_loop_filter_across_slices_enabled_flag << std::endl;
    oss << "deblocking_filter_override_enabled_flag: " << +picParams->deblocking_filter_override_enabled_flag << std::endl;
    oss << "pps_deblocking_filter_disabled_flag: " << +picParams->pps_deblocking_filter_disabled_flag << std::endl;
    oss << "lists_modification_present_flag: " << +picParams->lists_modification_present_flag << std::endl;
    oss << "slice_segment_header_extension_present_flag: " << +picParams->slice_segment_header_extension_present_flag << std::endl;
    oss << "IrapPicFlag: " << +picParams->IrapPicFlag << std::endl;
    oss << "IdrPicFlag: " << +picParams->IdrPicFlag << std::endl;
    oss << "IntraPicFlag: " << +picParams->IntraPicFlag << std::endl;
    oss << "ReservedBits4: " << +picParams->ReservedBits4 << std::endl;
    oss << "dwCodingSettingPicturePropertyFlags: " << +picParams->dwCodingSettingPicturePropertyFlags << std::endl;
    oss << "pps_cb_qp_offset: " << +picParams->pps_cb_qp_offset << std::endl;
    oss << "pps_cr_qp_offset: " << +picParams->pps_cr_qp_offset << std::endl;
    oss << "num_tile_columns_minus1: " << +picParams->num_tile_columns_minus1 << std::endl;
    oss << "num_tile_rows_minus1: " << +picParams->num_tile_rows_minus1 << std::endl;
    //Dump column width
    oss << "column_width_minus1[19]:";
    for (uint8_t i = 0; i < 19; i++)
        oss << picParams->column_width_minus1[i] << " ";
    oss << std::endl;

    //Dump row height
    oss << "row_height_minus1[21]:";
    for (uint8_t i = 0; i < 21; i++)
        oss << picParams->row_height_minus1[i] << " ";
    oss << std::endl;

    oss << "pps_beta_offset_div2: " << +picParams->pps_beta_offset_div2 << std::endl;
    oss << "pps_tc_offset_div2: " << +picParams->pps_tc_offset_div2 << std::endl;
    oss << "log2_parallel_merge_level_minus2: " << +picParams->log2_parallel_merge_level_minus2 << std::endl;
    oss << "CurrPicOrderCntVal: " << +picParams->CurrPicOrderCntVal << std::endl;

    oss.setf(std::ios::dec, std::ios::basefield);
    //Dump RefFrameList[15]
    for (uint8_t i = 0; i < 15; ++i)
    {
        oss << "RefFrameList[" << +i << "].FrameIdx:" << +picParams->RefFrameList[i].FrameIdx << std::endl;
        oss << "RefFrameList[" << +i << "].PicFlags:" << +picParams->RefFrameList[i].PicFlags << std::endl;
    }

    //Dump POC List
    oss << "PicOrderCntValList[15]:";
    for (uint8_t i = 0; i < 15; i++)
        oss << std::hex << picParams->PicOrderCntValList[i] << " ";
    oss << std::endl;

    //Dump Ref RefPicSetStCurrBefore List
    oss << "RefPicSetStCurrBefore[8]:";
    for (uint8_t i = 0; i < 8; i++)
        oss << +picParams->RefPicSetStCurrBefore[i] << " ";
    oss << std::endl;

    //Dump Ref RefPicSetStCurrAfter List
    oss << "RefPicSetStCurrAfter[16]:";
    for (uint8_t i = 0; i < 8; i++)
        oss << +picParams->RefPicSetStCurrAfter[i] << " ";
    oss << std::endl;

    //Dump Ref PicSetStCurr List
    oss << "RefPicSetLtCurr[16]:";
    for (uint8_t i = 0; i < 8; i++)
        oss << +picParams->RefPicSetLtCurr[i] << " ";
    oss << std::endl;

    //Dump Ref RefPicSetStCurrBefore List with POC
    oss << "POC of RefPicSetStCurrBefore[8]: ";
    for (uint8_t i = 0; i < 8; i++)
        oss << +picParams->PicOrderCntValList[picParams->RefPicSetStCurrBefore[i]%15] << " ";
    oss << std::endl;

    //Dump Ref RefPicSetStCurrAfter List with POC
    oss << "POC of RefPicSetStCurrAfter[16]:";
    for (uint8_t i = 0; i < 8; i++)
        oss << +picParams->PicOrderCntValList[picParams->RefPicSetStCurrAfter[i]%15] << " ";
    oss << std::endl;

    //Dump Ref PicSetStCurr List with POC
    oss << "POC of RefPicSetLtCurr[16]: ";
    for (uint8_t i = 0; i < 8; i++)
        oss << +picParams->PicOrderCntValList[picParams->RefPicSetLtCurr[i]%15] << " ";
    oss << std::endl;

    oss << "RefFieldPicFlag: " << +picParams->RefFieldPicFlag << std::endl;
    oss << "RefBottomFieldFlag: " << +picParams->RefBottomFieldFlag << std::endl;
    oss << "StatusReportFeedbackNumber: " << +picParams->StatusReportFeedbackNumber << std::endl;

    if (extPicParams)
    {
        //PicRangeExtensionFlags
        oss << "transform_skip_rotation_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.transform_skip_rotation_enabled_flag << std::endl;
        oss << "transform_skip_context_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.transform_skip_context_enabled_flag << std::endl;
        oss << "implicit_rdpcm_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.implicit_rdpcm_enabled_flag << std::endl;
        oss << "explicit_rdpcm_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.explicit_rdpcm_enabled_flag << std::endl;
        oss << "extended_precision_processing_flag: " << +extPicParams->PicRangeExtensionFlags.fields.extended_precision_processing_flag << std::endl;
        oss << "intra_smoothing_disabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.intra_smoothing_disabled_flag << std::endl;
        oss << "high_precision_offsets_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.high_precision_offsets_enabled_flag << std::endl;
        oss << "persistent_rice_adaptation_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.persistent_rice_adaptation_enabled_flag << std::endl;
        oss << "cabac_bypass_alignment_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.cabac_bypass_alignment_enabled_flag << std::endl;
        oss << "cross_component_prediction_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.cross_component_prediction_enabled_flag << std::endl;
        oss << "chroma_qp_offset_list_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.chroma_qp_offset_list_enabled_flag << std::endl;
        oss << "BitDepthLuma16: " << +extPicParams->PicRangeExtensionFlags.fields.BitDepthLuma16 << std::endl;
        oss << "BitDepthChroma16: " << +extPicParams->PicRangeExtensionFlags.fields.BitDepthChroma16 << std::endl;
        oss << "diff_cu_chroma_qp_offset_depth: " << +extPicParams->diff_cu_chroma_qp_offset_depth << std::endl;
        oss << "chroma_qp_offset_list_len_minus1: " << +extPicParams->chroma_qp_offset_list_len_minus1 << std::endl;
        oss << "log2_sao_offset_scale_luma: " << +extPicParams->log2_sao_offset_scale_luma << std::endl;
        oss << "log2_sao_offset_scale_chroma: " << +extPicParams->log2_sao_offset_scale_chroma << std::endl;
        oss << "log2_max_transform_skip_block_size_minus2: " << +extPicParams->log2_max_transform_skip_block_size_minus2 << std::endl;

        //Dump cb_qp_offset_list[6]
        oss << "cb_qp_offset_list[6]: ";
        for (uint8_t i = 0; i < 6; i++)
            oss << +extPicParams->cb_qp_offset_list[i] << " ";
        oss << std::endl;

        //Dump cr_qp_offset_list[6]
        oss << "cr_qp_offset_list[6]: ";
        for (uint8_t i = 0; i < 6; i++)
            oss << +extPicParams->cr_qp_offset_list[i] << " ";
        oss << std::endl;

        //Dump scc pic parameters
        if(sccPicParams)
        {
            oss << "pps_curr_pic_ref_enabled_flag: " << +sccPicParams->PicSCCExtensionFlags.fields.pps_curr_pic_ref_enabled_flag<< std::endl;
            oss << "palette_mode_enabled_flag: " << +sccPicParams->PicSCCExtensionFlags.fields.palette_mode_enabled_flag << std::endl;
            oss << "motion_vector_resolution_control_idc: " << +sccPicParams->PicSCCExtensionFlags.fields.motion_vector_resolution_control_idc << std::endl;
            oss << "intra_boundary_filtering_disabled_flag: " << +sccPicParams->PicSCCExtensionFlags.fields.intra_boundary_filtering_disabled_flag << std::endl;
            oss << "residual_adaptive_colour_transform_enabled_flag: " << +sccPicParams->PicSCCExtensionFlags.fields.residual_adaptive_colour_transform_enabled_flag << std::endl;
            oss << "pps_slice_act_qp_offsets_present_flag: " << +sccPicParams->PicSCCExtensionFlags.fields.pps_slice_act_qp_offsets_present_flag << std::endl;
            oss << "palette_max_size: " << +sccPicParams->palette_max_size << std::endl;
            oss << "delta_palette_max_predictor_size: " << +sccPicParams->delta_palette_max_predictor_size << std::endl;
            oss << "PredictorPaletteSize: " << +sccPicParams->PredictorPaletteSize << std::endl;

            for(uint8_t i = 0; i < 128; i++)
            {
                oss << "PredictorPaletteEntries[0][" << +i << "]: " << +sccPicParams->PredictorPaletteEntries[0][i] << std::endl;
                oss << "PredictorPaletteEntries[1][" << +i << "]: " << +sccPicParams->PredictorPaletteEntries[1][i] << std::endl;
                oss << "PredictorPaletteEntries[2][" << +i << "]: " << +sccPicParams->PredictorPaletteEntries[2][i] << std::endl;
            }

            oss << "pps_act_y_qp_offset_plus5: " << +sccPicParams->pps_act_y_qp_offset_plus5 << std::endl;
            oss << "pps_act_cb_qp_offset_plus5: " << +sccPicParams->pps_act_y_qp_offset_plus5 << std::endl;
            oss << "pps_act_cr_qp_offset_plus3: " << +sccPicParams->pps_act_y_qp_offset_plus5 << std::endl;
        }
    }

    const char *fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufPicParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipeline::DumpSliceParams(
    PCODEC_HEVC_SLICE_PARAMS     sliceParams,
    PCODEC_HEVC_EXT_SLICE_PARAMS extSliceParams,
    uint32_t                     numSlices,
    bool                         shortFormatInUse)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;
    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSlcParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(sliceParams);

    PCODEC_HEVC_SLICE_PARAMS     hevcSliceControl    = nullptr;
    PCODEC_HEVC_EXT_SLICE_PARAMS hevcExtSliceControl = nullptr;

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    for (uint16_t j = 0; j < numSlices; j++)
    {
        hevcSliceControl = &sliceParams[j];
        if (extSliceParams)
        {
            hevcExtSliceControl = &extSliceParams[j];
        }

        oss << "====================================================================================================" << std::endl;
        oss << "Data for Slice number = " << +j << std::endl;
        oss << "slice_data_size: " << +hevcSliceControl->slice_data_size << std::endl;
        oss << "slice_data_offset: " << +hevcSliceControl->slice_data_offset << std::endl;

        if (!shortFormatInUse)
        {
            //Dump Long format specific
            oss << "ByteOffsetToSliceData: " << +hevcSliceControl->ByteOffsetToSliceData << std::endl;
            oss << "slice_segment_address: " << +hevcSliceControl->slice_segment_address << std::endl;

            //Dump RefPicList[2][15]
            for (uint8_t i = 0; i < 15; ++i)
            {
                oss << "RefPicList[0][" << +i << "]";
                oss << ".FrameIdx: " << +hevcSliceControl->RefPicList[0][i].FrameIdx << std::endl;
                oss << "RefPicList[0][" << +i << "]";
                oss << ".PicFlags: " << +hevcSliceControl->RefPicList[0][i].PicFlags << std::endl;
            }
            for (uint8_t i = 0; i < 15; ++i)
            {
                oss << "RefPicList[1][" << +i << "]";
                oss << ".FrameIdx: " << +hevcSliceControl->RefPicList[1][i].FrameIdx << std::endl;
                oss << "RefPicList[1][" << +i << "]";
                oss << ".PicFlags: " << +hevcSliceControl->RefPicList[1][i].PicFlags << std::endl;
            }

            oss << "last_slice_of_pic: " << +hevcSliceControl->LongSliceFlags.fields.LastSliceOfPic << std::endl;
            oss << "dependent_slice_segment_flag: " << +hevcSliceControl->LongSliceFlags.fields.dependent_slice_segment_flag << std::endl;
            oss << "slice_type: " << +hevcSliceControl->LongSliceFlags.fields.slice_type << std::endl;
            oss << "color_plane_id: " << +hevcSliceControl->LongSliceFlags.fields.color_plane_id << std::endl;
            oss << "slice_sao_luma_flag: " << +hevcSliceControl->LongSliceFlags.fields.slice_sao_luma_flag << std::endl;
            oss << "slice_sao_chroma_flag: " << +hevcSliceControl->LongSliceFlags.fields.slice_sao_chroma_flag << std::endl;
            oss << "mvd_l1_zero_flag: " << +hevcSliceControl->LongSliceFlags.fields.mvd_l1_zero_flag << std::endl;
            oss << "cabac_init_flag: " << +hevcSliceControl->LongSliceFlags.fields.cabac_init_flag << std::endl;
            oss << "slice_temporal_mvp_enabled_flag: " << +hevcSliceControl->LongSliceFlags.fields.slice_temporal_mvp_enabled_flag << std::endl;
            oss << "slice_deblocking_filter_disabled_flag: " << +hevcSliceControl->LongSliceFlags.fields.slice_deblocking_filter_disabled_flag << std::endl;
            oss << "collocated_from_l0_flag: " << +hevcSliceControl->LongSliceFlags.fields.collocated_from_l0_flag << std::endl;
            oss << "slice_loop_filter_across_slices_enabled_flag: " << +hevcSliceControl->LongSliceFlags.fields.slice_loop_filter_across_slices_enabled_flag << std::endl;
            oss << "reserved: " << +hevcSliceControl->LongSliceFlags.fields.reserved << std::endl;
            oss << "collocated_ref_idx: " << +hevcSliceControl->collocated_ref_idx << std::endl;
            oss << "num_ref_idx_l0_active_minus1: " << +hevcSliceControl->num_ref_idx_l0_active_minus1 << std::endl;
            oss << "num_ref_idx_l1_active_minus1: " << +hevcSliceControl->num_ref_idx_l1_active_minus1 << std::endl;
            oss << "slice_qp_delta: " << +hevcSliceControl->slice_qp_delta << std::endl;
            oss << "slice_cb_qp_offset: " << +hevcSliceControl->slice_cb_qp_offset << std::endl;
            oss << "slice_cr_qp_offset: " << +hevcSliceControl->slice_cr_qp_offset << std::endl;
            oss << "slice_beta_offset_div2: " << +hevcSliceControl->slice_beta_offset_div2 << std::endl;
            oss << "slice_tc_offset_div2: " << +hevcSliceControl->slice_tc_offset_div2 << std::endl;
            oss << "luma_log2_weight_denom: " << +hevcSliceControl->luma_log2_weight_denom << std::endl;
            oss << "delta_chroma_log2_weight_denom: " << +hevcSliceControl->delta_chroma_log2_weight_denom << std::endl;

            //Dump luma_offset[2][15]
            for (uint8_t i = 0; i < 15; i++)
            {
                oss << "luma_offset_l0[" << +i << "]: " << (extSliceParams ? +hevcExtSliceControl->luma_offset_l0[i] : +hevcSliceControl->luma_offset_l0[i]) << std::endl;
                oss << "luma_offset_l1[" << +i << "]: " << (extSliceParams ? +hevcExtSliceControl->luma_offset_l1[i] : +hevcSliceControl->luma_offset_l1[i]) << std::endl;
            }
            //Dump delta_luma_weight[2][15]
            for (uint8_t i = 0; i < 15; i++)
            {
                oss << "delta_luma_weight_l0[" << +i << "]: " << +hevcSliceControl->delta_luma_weight_l0[i] << std::endl;
                oss << "delta_luma_weight_l1[" << +i << "]: " << +hevcSliceControl->delta_luma_weight_l1[i] << std::endl;
            }
            //Dump chroma_offset[2][15][2]
            for (uint8_t i = 0; i < 15; i++)
            {
                oss << "ChromaOffsetL0[" << +i << "][0]: " << (extSliceParams ? +hevcExtSliceControl->ChromaOffsetL0[i][0] : +hevcSliceControl->ChromaOffsetL0[i][0]) << std::endl;

                oss << "ChromaOffsetL0[" << +i << "][1]: " << (extSliceParams ? +hevcExtSliceControl->ChromaOffsetL0[i][1] : +hevcSliceControl->ChromaOffsetL0[i][1]) << std::endl;

                oss << "ChromaOffsetL1[" << +i << "][0]: " << (extSliceParams ? +hevcExtSliceControl->ChromaOffsetL1[i][0] : +hevcSliceControl->ChromaOffsetL1[i][0]) << std::endl;

                oss << "ChromaOffsetL1[" << +i << "][1]: " << (extSliceParams ? +hevcExtSliceControl->ChromaOffsetL1[i][1] : +hevcSliceControl->ChromaOffsetL1[i][1]) << std::endl;
            }
            //Dump delta_chroma_weight[2][15][2]
            for (uint8_t i = 0; i < 15; i++)
            {
                oss << "delta_chroma_weight_l0[" << +i << "][0]: " << +hevcSliceControl->delta_chroma_weight_l0[i][0] << std::endl;
                oss << "delta_chroma_weight_l0[" << +i << "][1]: " << +hevcSliceControl->delta_chroma_weight_l0[i][1] << std::endl;
                oss << "delta_chroma_weight_l1[" << +i << "][0]: " << +hevcSliceControl->delta_chroma_weight_l1[i][0] << std::endl;
                oss << "delta_chroma_weight_l1[" << +i << "][1]: " << +hevcSliceControl->delta_chroma_weight_l1[i][1] << std::endl;
            }

            oss << "five_minus_max_num_merge_cand: " << +hevcSliceControl->five_minus_max_num_merge_cand << std::endl;
            oss << "num_entry_point_offsets: " << +hevcSliceControl->num_entry_point_offsets << std::endl;
            oss << "EntryOffsetToSubsetArray: " << +hevcSliceControl->EntryOffsetToSubsetArray << std::endl;

            if (extSliceParams)
            {
                oss << "cu_chroma_qp_offset_enabled_flag: " << +hevcExtSliceControl->cu_chroma_qp_offset_enabled_flag << std::endl;

                //Dump scc slice parameters
                oss << "slice_act_y_qp_offset: " << +hevcExtSliceControl->slice_act_y_qp_offset<< std::endl;
                oss << "slice_act_cb_qp_offset: " << +hevcExtSliceControl->slice_act_cb_qp_offset << std::endl;
                oss << "slice_act_cr_qp_offset: " << +hevcExtSliceControl->slice_act_cr_qp_offset << std::endl;
                oss << "use_integer_mv_flag: " << +hevcExtSliceControl->use_integer_mv_flag << std::endl;
            }
        }
    }

    const char *fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufSlcParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs;
    ofs.open(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipeline::DumpSubsetsParams(PCODEC_HEVC_SUBSET_PARAMS subsetsParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;
    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSubsetsParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    if(subsetsParams)
    {
        std::ostringstream oss;
        oss.setf(std::ios::showbase | std::ios::uppercase);

        for(uint16_t i = 0; i < 440; i++)
        {
            oss << "entry_point_offset_minus1[" << +i << "]: " << +subsetsParams->entry_point_offset_minus1[i]<< std::endl;
        }

        //create the file
        const char *fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufSubsetsParams,
        CodechalDbgExtType::txt);

        std::ofstream ofs(fileName, std::ios::out);
        ofs << oss.str();
        ofs.close();
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipeline::DumpSecondLevelBatchBuffer()
{
    DECODE_FUNC_CALL();

    PMHW_BATCH_BUFFER batchBuffer = GetSliceLvlCmdBuffer();
    DECODE_CHK_NULL(batchBuffer);

    batchBuffer->iLastCurrent = batchBuffer->iSize * batchBuffer->count;
    batchBuffer->dwOffset     = 0;
    DECODE_CHK_STATUS(m_debugInterface->Dump2ndLvlBatch(
                        batchBuffer,
                        CODECHAL_NUM_MEDIA_STATES,
                        "_DEC"));
    return MOS_STATUS_SUCCESS;
}
#endif

}
